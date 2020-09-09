#include "ui.h"
#include "iop_bmx.h"
#include "cray_mainframe.h"
#include <algorithm>
#include <time.h>

//////////////////////////////////////////////////////////////////////////////////////////
// IopChannelBmx_c
//////////////////////////////////////////////////////////////////////////////////////////
void IopChannelBmx_c::Setup(const Configuration_c &aConfig) {
	mDevices.clear();
	mDelayLimit = aConfig.get<size_t>("DelayLimit", 10);
	size_t DeviceCount = aConfig.get<size_t>("DeviceCount", 255);
	mDevices.resize(DeviceCount);

	for(const auto &DeviceConfig: aConfig.get_child_safe("Devices")) {
		std::string DeviceType = DeviceConfig.first;
		std::shared_ptr<IopBmxDevice_i> Device;
		const Configuration_c &DeviceParams = DeviceConfig.second;

		// Create device
		if (DeviceType == "Tape") {
			// Create a tape device
			Device = std::make_shared<IopBmxTape_c>(DeviceParams, *this);
		} else {
			throw InvalidParameter_x(boost::format("Unknown device type: %1%") % DeviceType);
		}

		// Add device to all channels (these are smart pointers, so adding a device multiple times is OK)
		uint8_t DeviceAddress = Device->GetDeviceAddress();
		if (mDevices[DeviceAddress].Device != nullptr) {
			throw InvalidParameter_x(boost::format("expander channel index %1% is already in use") % DeviceAddress);
		}
		if (DeviceAddress >= mDevices.size()) {
			throw InvalidParameter_x(boost::format("expander channel index %1% is too large") % DeviceAddress);
		}
		mDevices[DeviceAddress].Device = Device;
		if (Device->NeedsTick()) {
			mTickedDevices.emplace_back(Device.get());
		}
	}
}

IopInt_t IopChannelBmx_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	uint16_t RetVal;
	switch (aFunction) {
		case 000:
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " CLEAR_CHANNEL_BUSY/DONE with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			mBusy = false;
			mDone = false;
			mInterruptEnabled = false;
			//TODO: this should disable DMAs too, but there's no function to enabled it again?!!!
		return 0;
		case 001:
			switch (aData & 0x3) {
				case 0: // Clear output tag lines
					mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " CLEAR_OUTPUT_TAG_LINES with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
					mDone = true;
					mBusy = false;
				break;
				case 1: // Interface disconnect
					mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " INTERFACE_DISCONNECT with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
					CRAY_ASSERT(GetActiveDevice() != nullptr);
					if (GetActiveDevice()->InterfaceDisconnect()) {
						// Successfull disconnect
						mDone = true;
						mBusy = false;
					} else {
						// Error in disconnect
						mDone = true;
						mBusy = true;
					}
				break;
				case 2: // Selective reset
					mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " SELECTIVE_RESET with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
					if (GetActiveDevice() != nullptr) GetActiveDevice()->Reset();
					mDone = true;
					mBusy = false;
				break;
				case 3: // System reset
					mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " SYSTEM_RESET with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
					for(auto &Device: mDevices) if (Device.Device != nullptr) Device.Device->Reset();
					mDone = true;
					mBusy = false;
				break;
			}
		return 0;
		case 002: { // CHANNEL COMMAND
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " CHANNEL_COMMAND with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			uint8_t Command = aData & 0xff;
			mLogger << setloglevel(LogLevel_IoActivity) << "Command: " << HexPrinter(Command) << std::endl;
			// This operation is a BMX-initiated transfer
			CRAY_ASSERT(GetActiveDevice() != nullptr);
			IopBmxDevice_i &ActiveDevice = *GetActiveDevice();

			// Test I/O
			if (Command == 0) {
				mDeviceStatus = ActiveDevice.TestIo(Command);
			} else {
				std::vector<uint8_t> Data;
				size_t BytesTransferred = 0;
				size_t MaxSize = mByteCounter;
				IopInt_t ByteCounter = mByteCounter;
				IopInt_t LocalMemoryAdddress = mLocalMemoryAdddress[mActiveMemoryAddressIdx];
				LogLine_c TraceLine = mLogger << setloglevel(LogLevel_IoTrace);
				switch (Command & 0xf) {
					case 0x1: // Write
					case 0x5: // Write
					case 0x9: // Write
					case 0xd: // Write
					case 0x3: // Control
					case 0x7: // Control
					case 0xb: // Control
					case 0xf: // Control
						while (ByteCounter != 0) {
							for(size_t Parcel=0;Parcel<4;++Parcel) {
								IopInt_t Word = mParent.GetDma().ReadWord((LocalMemoryAdddress & 0xfffc) | Parcel);
								Data.push_back(Word >> 8);
								--ByteCounter;
								if (ByteCounter == 0) break;
								Data.push_back(Word & 0xff);
								--ByteCounter;
								if (ByteCounter == 0) break;
							}
							LocalMemoryAdddress += 4;
						}
						switch (Command & 0x3) {
							case 0x1: mDeviceStatus = ActiveDevice.Write(Command,Data,BytesTransferred); break;
							case 0x3: mDeviceStatus = ActiveDevice.Control(Command,Data,BytesTransferred); break;
							default: CRAY_ASSERT(false); break;
						}
						CRAY_ASSERT(BytesTransferred <= mByteCounter);
						mLocalMemoryAdddress[mActiveMemoryAddressIdx] += IopInt_t(BytesTransferred / 8);
						mByteCounter -= IopInt_t(BytesTransferred);
					break;
					case 0x2: // Read
					case 0x6: // Read
					case 0xa: // Read
					case 0xe: // Read
					case 0x4: // Sense
					case 0xc: // Read backward
						switch (Command & 0xf) {
							case 0x2:
							case 0x6:
							case 0xa:
							case 0xe: mDeviceStatus = ActiveDevice.Read(Command,Data,MaxSize); break;
							case 0x4: mDeviceStatus = ActiveDevice.Sense(Command,Data,MaxSize); break;
							case 0xc: mDeviceStatus = ActiveDevice.ReadBackward(Command,Data,MaxSize); break;
							default: CRAY_ASSERT(false); break;
						}
						mLogger << setloglevel(LogLevel_IoActivity) << "received " << DecPrinter(Data.size()) << " bytes of data with command: " << HexPrinter(Command) << " to address: " << HexPrinter(mLocalMemoryAdddress[mActiveMemoryAddressIdx]) << std::endl;
						size_t i;
						for(i=0; i<Data.size(); ++i) {
							if (i%16 == 0) {
								if (i != 0) TraceLine << std::endl;
							} else {
								TraceLine << " ";
							}
							TraceLine << HexPrinter(Data[i]);
						}
						if (i%16 != 0) {
							if (i != 0) TraceLine << std::endl;
						}

						CRAY_ASSERT(Data.size() <= mByteCounter);
						BytesTransferred = 0;
						while (BytesTransferred != Data.size()) {
							for(size_t Parcel=0;Parcel<4;++Parcel) {
								IopInt_t Word = Data[BytesTransferred] << 8;
								--mByteCounter;
								++BytesTransferred;
								if (BytesTransferred != Data.size()) {
									Word |= Data[BytesTransferred];
									--mByteCounter;
									++BytesTransferred;
								}
								mParent.GetDma().WriteWord((mLocalMemoryAdddress[mActiveMemoryAddressIdx] & 0xfffc) | Parcel,Word);
								if (BytesTransferred == Data.size()) break;
							}
							mLocalMemoryAdddress[mActiveMemoryAddressIdx] += 4;
						}
					break;
					default: CRAY_ASSERT(false); break;
				}
			}
			mDone = true;
			mBusy = false;
		} return 0;
		case 003: { // READ REQUEST-IN ADDRESS
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " READ REQUEST-IN ADDRESS with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			bool HaveInterrupt = false;
			// This operation figures out the address of the interrupting I/O device.
			for (auto &Device : mDevices) {
				if (Device.Device != nullptr && Device.Device->GetRequestIn()) {
					mActiveDevice = Device.Device->GetDeviceAddress();
					mDeviceStatus = Device.Device->GetStatus();
					HaveInterrupt = true;
					break;
				}
			}
			if (HaveInterrupt) {
				mDone = true;
				mBusy = false;
			} else {
				mDone = true;
				mBusy = true;
			}
		} return 0;
		case 004: // ASYNCHRONOUS I/O
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " ASYNC_IO with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			CRAY_ASSERT(false);
			//GetActiveDevice().GetData(mStackStatusFlag);
			mDone = true;
			mBusy = false;
		return 0;
		case 005: // DELAY COUNTER DIAGNOSTICS
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " DELAY_COUNTER_DIAG with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			mLogger << setloglevel(LogLevel_IoActivity) << "Starting delay counter: " << DecPrinter(mDelayLimit) << std::endl;
			mDelayCounter = mDelayLimit;
			mDone = false;
			mBusy = true;
		return 0;
		case 006:
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " INTERRUPT_DISABLE with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			mInterruptEnabled = false;
		return 0;
		case 007:
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " INTERRUPT_ENABLE with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			mInterruptEnabled = true;
		return 0;
		case 010: // READ LOCAL MEMORY ADDRESS
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " READ_LOCAL_MEM_ADDRESS with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			RetVal = (mLocalMemoryAdddress[aData & 1] & 0xfffe) | (aData & 1);
			mChainingInterruptActive = false;
			mLogger << setloglevel(LogLevel_IoActivity) << "\treturning: " << HexPrinter(RetVal) << " (" << DecPrinter(RetVal) << ")" << std::endl;
		return RetVal;
		case 011: // READ BYTE COUNTER
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " READ_BYTE_COUNTER with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			if (mByteCountStatusRegValid) {
				mByteCountStatusRegValid = false;
				RetVal = mByteCountStatusReg;
			} else {
				RetVal = mByteCounter;
			}
			mLogger << setloglevel(LogLevel_IoActivity) << "\treturning: " << HexPrinter(RetVal) << " (" << DecPrinter(RetVal) << ")" << std::endl;
		return RetVal;
		case 012: // READ STATUS/ADDRESS
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " READ_STATUS/ADDRESS with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			if (mStatusRegValid) {
				mStatusRegValid = false;
				RetVal = mStatusReg;
			} else {
				RetVal = mDeviceStatus | (mActiveDevice << 8);
			}
			mLogger << setloglevel(LogLevel_IoActivity) << "\treturning: " << HexPrinter(RetVal) << " (" << DecPrinter(RetVal) << ")" << std::endl;
		return RetVal;
		case 013: // READ INPUT TAGS
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " READ_INPUT_TAGS with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			RetVal = // TODO: do we have to simulate other bits too?
				(mByteCounter == 0) ? (1 << 13) : 0 |
				(mActiveMemoryAddressIdx << 11) |
				(GetDeviceInterrupt() << 4);
			mLogger << setloglevel(LogLevel_IoActivity) << "\treturning: " << HexPrinter(RetVal) << " (" << DecPrinter(RetVal) << ")" << std::endl;
		return RetVal;
		case 014: // ENTER LOCAL MEMORY ADDRESS
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " ENTER_LOCAL_MEM_ADDRES with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			mLocalMemoryAdddress[aData & 1] = aData & 0xfffe;
			//CRAY_ASSERT((aData & 2) == 0); // For now, make sure chaining is not enabled
			mLoadByteCounter = true;
		return 0;
		case 015: // ENTER BYTE COUNT
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " ENTER_BYTE_COUNT with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			mByteCountStatusReg = aData;
			mByteCountStatusRegValid = true;
			if (mLoadByteCounter) mByteCounter = aData; else mNextByteCounter = aData;
			mLoadByteCounter = false;
		return 0;
		case 016: // ENTER DEVICE ADDRESS/MODE
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " ENTER_DEVICE_ADDRESS/MODE with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			mActiveDevice = (aData & 0xff);
			mSkipFlag = GetBit((uint32_t)aData,8) != 0;
			mStackStatusFlag = GetBit((uint32_t)aData,9) != 0;
			mChainingMode = (ChainingMode_e)GetBits((uint32_t)aData,10,11);
			mInterruptMode = (InterruptMode_e)GetBits((uint32_t)aData,12,13);
			//CRAY_ASSERT(mInterruptMode != InterruptMode_StatusIn);
			mStatusReg = aData;
			mStatusRegValid = true;
		return 0;
		case 017: // ENTER OUTPUT TAGS
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " ENTER_OUTPUT_TAGS with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			//CRAY_ASSERT(false);
		return 0;
		default:
			mLogger << setloglevel(LogLevel_IoActivity) << "Processing function " << OctPrinter(aFunction) << " with active device: " << DecPrinter(mActiveDevice) << " and accumulater: " << HexPrinter(aData) << " (" << DecPrinter(aData) << ")" << std::endl;
			mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

IopBit_t IopChannelBmx_c::GetInterrupt() {
	if (mInterruptEnabled && mDone) {
		mLogger << setloglevel(LogLevel_IoActivity) << "Sending DONE interrupt" << std::endl;
		return 1;
	}
	return GetDeviceInterrupt();
}

// These could be further optimized but not all that interesting at the moment...
void IopChannelBmx_c::RequestInChanged(uint8_t aDeviceAddr, bool aValue) {
	UpdateXxxInCnts();
}

void IopChannelBmx_c::StatusInChanged(uint8_t aDeviceAddr, bool aValue) {
	UpdateXxxInCnts();
}

void IopChannelBmx_c::DisconnectInChanged(uint8_t aDeviceAddr, bool aValue) {
	UpdateXxxInCnts();
}

void IopChannelBmx_c::UpdateXxxInCnts() {
	mRequestInCnt = 0;
	mStatusInCnt = 0;
	mDisconnectInCnt = 0;
	for (auto &Device : mDevices) {
		if (Device.Device != nullptr) {
			if (Device.Device->GetRequestIn()) ++mRequestInCnt;
			if (Device.Device->GetStatusIn()) ++mStatusInCnt;
			if (Device.Device->GetDisconnectIn()) ++mDisconnectInCnt;
		}
	}
}

IopBit_t IopChannelBmx_c::GetDeviceInterrupt() {
	if (mInterruptMode == InterruptMode_RequestIn) return mRequestInCnt > 0 ? 1 : 0;
	if (mInterruptMode == InterruptMode_StatusIn) return mStatusInCnt > 0 ? 1 : 0;
	if (mInterruptMode == InterruptMode_DisconnectIn) return mDisconnectInCnt > 0 ? 1 : 0;
	return 0;
}

std::string IopChannelBmx_c::GetName() const {
	return mName;
}

void IopChannelBmx_c::Tick() {
	for (auto &Device : mTickedDevices) {
		Device->Tick();
	}
	if (mDelayCounter != 0) {
		--mDelayCounter;
		if (mDelayCounter == 0) {
			mLogger << setloglevel(LogLevel_IoActivity) << "Delay counter expired" << std::endl;
			mDone = true;
			mBusy = false;
		}
	}
}

void IopChannelBmx_c::GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const {
	for (auto &Device : mDevices) {
		if (Device.Device != nullptr && Device.Device->GetType() == aFilter) {
			StatusReport_c Status;
			Device.Device->GetStatus(Status, aFilter, aLongFormat);
			aStatus.put_child(Device.Device->GetName(), Status);
		}
	}
}

void IopChannelBmx_c::RegisterCommands(CommandHooks_t &aHooks) {
	for (auto &Device : mDevices) {
		if (Device.Device != nullptr) {
			Device.Device->RegisterCommands(aHooks);
		}
	}
}



//////////////////////////////////////////////////////////////////////////////////////////
// IopBmxTape_c
//////////////////////////////////////////////////////////////////////////////////////////

// First byte is the channel status byte

// Second byte is the unit status byte
//    Bit  Designation 
//     0  Attention 
//            not used
//     1  Status modifier 
//            present with busy to indicate interrupt pending
//     2  Control unit end 
//            signals completion (with or without error) of an operation
//     3  Busy 
//            busy
//     4  Channel end 
//     5  Device end 
//     6  Unit check 
//            set if any bytes of the channel status byte is set or bit 7 (below) is set
//     7  Unit exception 
enum BmxStat_e {
	BmxStat_Attention          = 1 << 0,
	BmxStat_StatusModifier     = 1 << 1,
	BmxStat_ControlUnitEnd     = 1 << 2,
	BmxStat_Busy               = 1 << 3,
	BmxStat_ChannelEnd         = 1 << 4,
	BmxStat_DeviceEnd          = 1 << 5,
	BmxStat_UnitCheck          = 1 << 6,
	BmxStat_UnitException      = 1 << 7
};

IopBmxTape_c::IopBmxTape_c(const Configuration_c &aConfig, IopChannelBmx_c &aParent):
	mParent(aParent),
	mLogger(aConfig, "BMT",aConfig.get<size_t>("DeviceAddress")),
	mDeviceAddress(aConfig.get<uint8_t>("DeviceAddress")),
	mReads(0),
	mWrites(0),
	mTapeImage(mLogger, aConfig),
	mIsReadOnly(aConfig.get<bool>("ReadOnly", false))
{
	boost::optional<std::string> Name = aConfig.get_optional<std::string>("DeviceName");
	mName = (Name.is_initialized()) ? Name.get() : ModuleNameFormatter("BMT", aConfig.get<size_t>("DeviceAddress"));
	mLogger.SetHeader(mName.c_str());
	mLogger.SetParent(mParent.GetLogger());
	ClearState();
	mTapeImage.open(aConfig.get<std::string>("Tape"));
}

uint8_t IopBmxTape_c::TestIo(uint8_t aCommand) {
	return BmxStat_DeviceEnd | BmxStat_ControlUnitEnd;
}

uint8_t IopBmxTape_c::Write(uint8_t aCommand, std::vector<uint8_t> &aData, size_t &aBytesTransferred) {
	mLogger << setloglevel(LogLevel_IoActivity) << "writing " << DecPrinter(aData.size()) << " bytes of data with command: " << HexPrinter(aCommand) << std::endl;
	CRAY_ASSERT(!mIsReadOnly);
	LogLine_c TraceLine = mLogger << setloglevel(LogLevel_IoTrace);
	size_t i;
	for(i=0; i<aData.size(); ++i) {
		if (i%16 == 0) {
			if (i != 0) TraceLine << std::endl;
		} else {
			TraceLine << " ";
		}
		TraceLine << HexPrinter(aData[i]);
	}
	if (i%16 != 0) {
		if (i != 0) TraceLine << std::endl;
	}
	mTapeImage.ReOpen();
	mTapeImage.WriteRecord(aData);
	aBytesTransferred = aData.size();
	mWrites += aData.size();
	uint8_t RetVal = BmxStat_ControlUnitEnd;
	if (mTapeImage.GetState() == TapFile_c::State_e::EndOfFile) RetVal |= BmxStat_UnitException;
	return RetVal;
}

uint8_t IopBmxTape_c::Control(uint8_t aCommand, std::vector<uint8_t> &aData, size_t &aBytesTransferred) {
	mLogger << setloglevel(LogLevel_IoActivity) << "control with " << DecPrinter(aData.size()) << " bytes of data and command: " << HexPrinter(aCommand) << std::endl;
	LogLine_c TraceLine = mLogger << setloglevel(LogLevel_IoTrace);
	size_t i;
	for(i=0; i<aData.size(); ++i) {
		if (i%16 == 0) {
			if (i != 0) TraceLine << std::endl;
		} else {
			TraceLine << " ";
		}
		TraceLine << HexPrinter(aData[i]);
	}
	if (i%16 != 0) {
		if (i != 0) TraceLine << std::endl;
	}
	uint8_t RetVal = BmxStat_DeviceEnd | BmxStat_ControlUnitEnd;
	switch ((aCommand >> 2 ) & 1) {
		case 0: // mode setting operations
			mLogger << setloglevel(LogLevel_IoActivity) << "Mode setting" << std::endl; break;
		break;
		case 1: // Tape motion operations
			switch ((aCommand >> 3) & 7) {
				case 0: mLogger << setloglevel(LogLevel_IoActivity) << "Rewind" << std::endl; 
					mTapeImage.ReOpen();
					mTapeImage.SeekToBeginningOfTape();
					mTapeImage.close();
				break;
				case 1: mLogger << setloglevel(LogLevel_IoActivity) << "Rewind and Unload" << std::endl;
					mTapeImage.ReOpen();
					mTapeImage.SeekToBeginningOfTape();
					mTapeImage.close();
					if (mTapeImage.GetState() == TapFile_c::State_e::BeginningOfTape) RetVal |= BmxStat_UnitCheck;
				break;
				case 2: mLogger << setloglevel(LogLevel_IoActivity) << "Erase Gap" << std::endl; break;
				case 3: mLogger << setloglevel(LogLevel_IoActivity) << "Write Tape Mark" << std::endl;
					CRAY_ASSERT(!mIsReadOnly);
					mTapeImage.ReOpen();
					mTapeImage.WriteEndOfFile();
				break;
				case 4: mLogger << setloglevel(LogLevel_IoActivity) << "Backspace Block" << std::endl;
					mTapeImage.ReOpen();
					mTapeImage.SeekToPrevRecord();
					if (mTapeImage.GetState() == TapFile_c::State_e::EndOfFile) RetVal |= BmxStat_UnitException;
					if (mTapeImage.GetState() == TapFile_c::State_e::BeginningOfTape) RetVal |= BmxStat_UnitCheck;
				break;
				case 5: mLogger << setloglevel(LogLevel_IoActivity) << "Backspace File" << std::endl;
					mTapeImage.ReOpen();
					do {
						mTapeImage.SeekToPrevRecord();
						if (mTapeImage.GetState() == TapFile_c::State_e::EndOfTape) {
							mLogger << setloglevel(LogLevel_IoActivity) << "spacing cut short by EOT condition after " << DecPrinter(i) << " records." << std::endl;
							break;
						}
					} while (mTapeImage.GetState() != TapFile_c::State_e::EndOfFile);
					if (mTapeImage.GetState() == TapFile_c::State_e::BeginningOfTape) RetVal |= BmxStat_UnitCheck;
				break;
				case 6: mLogger << setloglevel(LogLevel_IoActivity) << "Forward Space Block" << std::endl;
					mTapeImage.ReOpen();
					mTapeImage.SeekToNextRecord();
					if (mTapeImage.GetState() == TapFile_c::State_e::EndOfFile) RetVal |= BmxStat_UnitException;
					if (mTapeImage.GetState() == TapFile_c::State_e::EndOfTape) RetVal |= BmxStat_UnitCheck;
				break;
				case 7: mLogger << setloglevel(LogLevel_IoActivity) << "Forward Space File" << std::endl; 
					mTapeImage.ReOpen();
					do {
						mTapeImage.SeekToNextRecord();
						if (mTapeImage.GetState() == TapFile_c::State_e::EndOfTape) {
							mLogger << setloglevel(LogLevel_IoActivity) << "spacing cut short by EOT condition after " << DecPrinter(i) << " records." << std::endl;
							break;
						}
					} while (mTapeImage.GetState() != TapFile_c::State_e::EndOfFile);
					if (mTapeImage.GetState() == TapFile_c::State_e::EndOfTape) RetVal |= BmxStat_UnitCheck;
				break;
				default: CRAY_ASSERT(false); break;
			}
		break;
		default: CRAY_ASSERT(false); break;
	}
	aBytesTransferred = aData.size();
	return RetVal;
}

uint8_t IopBmxTape_c::Read(uint8_t aCommand, std::vector<uint8_t> &aData, size_t aMaxSize) {
	mLogger << setloglevel(LogLevel_IoActivity) << "read with " << DecPrinter(aMaxSize) << " bytes of buffer and command: " << HexPrinter(aCommand) << std::endl;
	uint8_t RetVal = BmxStat_ControlUnitEnd;
	mTapeImage.ReOpen();
	aData = mTapeImage.Read(uint32_t(aMaxSize));
	mReads += aData.size();
	if (mTapeImage.GetState() == TapFile_c::State_e::EndOfFile) RetVal |= BmxStat_UnitException;
	if (mTapeImage.GetState() == TapFile_c::State_e::EndOfTape) RetVal |= BmxStat_UnitCheck;
	return RetVal;
}

uint8_t IopBmxTape_c::Sense(uint8_t aCommand, std::vector<uint8_t> &aData, size_t aMaxSize) {
	mLogger << setloglevel(LogLevel_IoActivity) << "sense with " << DecPrinter(aMaxSize) << " bytes of buffer and command: " << HexPrinter(aCommand) << std::endl;
	// Sense data is documented in
	// A22-6866-4_2400_Tape_Unit_2803_2804_Tape_Controls_Component_Description_Sep68.pdf 
	// page 29-32
	CRAY_ASSERT(aMaxSize >= 6);
	aData.resize(6);
	aData[0] = 0;
	//         TU Status A    File Protect
	aData[1] = (1 << 1)     | ((mIsReadOnly ? 1:0) << 6);
	aData[2] = 0;
	aData[3] = 0;
	aData[4] = 0;
	aData[5] = 0;
/*	aData[0] = 0x36;
	//         TU Status A    File Protect
	aData[1] = (1 << 1)     | (1 << 6);
	aData[2] = 0x47;
	aData[3] = 0x58;
	aData[4] = 0x69;
	aData[5] = 0x7a;*/
	return BmxStat_ControlUnitEnd;
}

uint8_t IopBmxTape_c::ReadBackward(uint8_t aCommand, std::vector<uint8_t> &aData, size_t aMaxSize) {
	mLogger << setloglevel(LogLevel_IoActivity) << "read backward " << DecPrinter(aMaxSize) << " bytes of buffer and command: " << HexPrinter(aCommand) << " - UNIMPLEMENTED!!!" << std::endl;
	CRAY_ASSERT(false); // This is not implemented
	aData.resize(aMaxSize);
	for(size_t i=0; i<aData.size(); ++i) {
		aData[i] = i % 255;
	}
	return BmxStat_ControlUnitEnd;
}

// This is only called if the device is requesting an interrupt. Since we never do it, it won't be called ever.
uint8_t IopBmxTape_c::GetStatus() const {
	uint8_t RetVal = BmxStat_DeviceEnd | BmxStat_ControlUnitEnd;
	return RetVal;
}

bool IopBmxTape_c::GetRequestIn() const {
	return false;
}

bool IopBmxTape_c::GetStatusIn() const {
	return false;
}

bool IopBmxTape_c::GetDisconnectIn() const {
	return false;
}

void IopBmxTape_c::GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const {
	aStatus.put(aLongFormat ? "Reads" : "R", mReads);
	aStatus.put(aLongFormat ? "Writes" : "W", mWrites);
	if (mTapeImage.is_open()) aStatus.put(aLongFormat ? "Position" : "Pos", const_cast<PokedTapFile_c*>(&mTapeImage)->tellg()); else aStatus.put(aLongFormat ? "Position" : "Pos", "-");
	aStatus.put(aLongFormat ? "File Name" : "FName", mTapeImage.GetFileName());
}

std::string IopBmxTape_c::GetName() const {
	return mName;
}

void IopBmxTape_c::MountTape(const std::string &aTapeFileName) {
	if (mTapeImage.is_open()) throw Generic_x("Can't unmount tape while it's in use.");
	mTapeImage.open(aTapeFileName);
	mTapeImage.close();
}

void IopBmxTape_c::RegisterCommands(CommandHooks_t &aHooks) {
	class CmdMount_c : public CmdFactoryBase_i {
	public:
		explicit CmdMount_c(IopBmxTape_c &aParent) : mParent(aParent) {}
		virtual bool ParseAndExec(TokenStream_t::const_iterator aBegin, TokenStream_t::const_iterator aEnd) override {
			bool DoHelp = false;
			if (aBegin->mValue == "help") {
				DoHelp = true;
				++aBegin; if (aBegin == aEnd) return false;
			}
			if (aBegin->mValue != "mount") return false;
			++aBegin; if (aBegin == aEnd) return false;
			if (aBegin->mValue != mParent.GetName()) return false;
			if (DoHelp) {
				std::cout << "mount " << mParent.GetName() << " <file name>" << std::endl;
				std::cout << "    Mounts a new virtual tape on the tape drive" << std::endl;
				return true;
			}
			else {
				++aBegin; if (aBegin == aEnd) return false;
				std::string FileName = aBegin->mValue;
				++aBegin;
				if (aBegin != aEnd && aBegin->mTokenId != TokenTypes_e::EndOfLine) return false;

				// TODO: add warning about existing file
				try {
					mParent.MountTape(FileName);
					std::cout << "New virtual tape " << FileName << " mounted" << std::endl;
				}
				catch (std::exception &Ex) {
					std::cout << "Mount failed" << std::endl;
					std::cout << Ex.what() << std::endl;
				}
				return true;
			}
		}
		virtual std::string GetCommandName() const override { return "mount"; }
		virtual std::string GetDeviceName() const override { return mParent.GetName(); }
	protected:
		IopBmxTape_c &mParent;
	};

	aHooks.emplace_back(std::make_unique<CmdMount_c>(*this));
}
