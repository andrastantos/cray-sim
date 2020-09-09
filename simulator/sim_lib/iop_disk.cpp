#include "iop_disk.h"
#include "cray_mainframe.h"
#include <algorithm>
#include <time.h>
#include <boost/filesystem.hpp>

/////////////////////////////////////////////////////////
// IopDiskDD29_c
/////////////////////////////////////////////////////////

IopDiskDD29_c::IopDiskDD29_c(const Configuration_c &aConfig, IopCpu_c &aParent) :
	mParent(aParent), 
	mLogger(aConfig, "DK", aConfig.get<size_t>("ChannelIdx"), 8), 
	mChannelIdx(aConfig.get<size_t>("ChannelIdx")), 
	mReads(0), 
	mWrites(0)
{
	mName = ModuleNameFormatter("DK", mChannelIdx, 8);

	mLogger.SetParent(mParent.GetLogger());
	mImageFileName = aConfig.get<std::string>("ImageFileName");
	mReadOnly = aConfig.get<bool>("ReadOnly",false);
	mNumHeads = aConfig.get<size_t>("Heads", 10);
	if (mNumHeads == 0) throw InvalidParameter_x("Heads must be greater than 0");
	mNumSectors = aConfig.get<size_t>("Sectors", 18);
	if (mNumSectors == 0) throw InvalidParameter_x("Sectors must be greater than 0");
	mNumTracks = aConfig.get<size_t>("Tracks", 823);
	if (mNumTracks == 0) throw InvalidParameter_x("Tracks must be greater than 0");
	mSectorSize = aConfig.get<size_t>("SectorSize", 4096);
	if (mSectorSize == 0) throw InvalidParameter_x("SectorSize must be greater than 0");
	MasterClear();

	if (!boost::filesystem::exists(mImageFileName)) {
		std::fstream File;

		std::cout << "Creating disk image file: " << mImageFileName << "..." << std::flush;
		File.open(mImageFileName, std::ios::out | std::ios::binary );
		CRAY_ASSERT(!File.bad());
		size_t DiskSize = mNumHeads * mNumSectors * mNumTracks * mSectorSize;
		File.seekp(DiskSize-2);
		CRAY_ASSERT(!File.bad());
		IopInt_t Data = 0;
		File.write((char*)(&Data),sizeof(Data));
		CRAY_ASSERT(!File.bad());
		File.close();
		std::cout << " done" << std::endl;
	}
}

std::string IopDiskDD29_c::GetName() const {
	return mName.c_str();
}

void IopDiskDD29_c::GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const {
	aStatus.put(aLongFormat ? "Cylinder" : "C", mCurrentTrack);
	aStatus.put(aLongFormat ? "Head" : "H", mCurrentHead);
	aStatus.put(aLongFormat ? "Sector" : "S", mCurrentSector);
	aStatus.put(aLongFormat ? "Reads" : "R", mReads);
	aStatus.put(aLongFormat ? "Writes" : "W", mWrites);
//	aStatus.put(aLongFormat ? "Reserved" : "res", mMounted);
//	aStatus.put(aLongFormat ? "File Name" : "fname", mImageFileName);
}

IopInt_t IopDiskDD29_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	switch (aFunction) {
		case 000:
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: CLEAR_CHANNEL (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << std::endl;
			mOperation = Operation_Idle;
			mDone = false;
		return 0;
		case 001:
			mBufferEchoMode = false;
			switch ((aData & 0777000) >> 9) {
				case 00: // Release unit
					mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - release unit" << std::endl;
					mMounted = false;
					mOperation = Operation_Idle;
					mDone = true;
				break;
				case 01: // Reserve unit
					mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - reserve unit" << std::endl;
					mMounted = true;
					mCurrentHead = 0;
					mOperation = Operation_Idle;
					mDone = true;
				break;
				case 02: // Clear fault flags
					mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - clear fault flags" << std::endl;
					//Unimplemented
					mOperation = Operation_Idle;
					mDone = true;
				break;
				case 03: // Return to cylinder 0
					mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - return to cylinder 0" << std::endl;
					mCurrentTrack = 0;
					mOperation = Operation_Idle;
					mDone = true;
				break;
				case 04: // Select cylinder margin
					mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - select cylinder margin" << std::endl;
					CRAY_ASSERT(false); // unimplemented
					mOperation = Operation_Idle;
					mDone = true;
				break;
				case 05: // Read sector number
					mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - read sector number" << std::endl;
					mStatusResponseRegister = IopInt_t(mCurrentSector); // This could in theory be any valid sector of course. It is the sector under the head at the moment
					mLogger << setloglevel(LogLevel_IoActivity) << "Setting mStatusResponseRegister to " << HexPrinter(mStatusResponseRegister) << "(" << DecPrinter(mStatusResponseRegister) << ")" << std::endl;
					mOperation = Operation_Idle;
					mDone = true;
				break;
				case 06: // Read error flags
					mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - read error flags" << std::endl;
					mStatusResponseRegister = 0; // We don't simulate errors
					mLogger << setloglevel(LogLevel_IoActivity) << "Setting mStatusResponseRegister to " << HexPrinter(mStatusResponseRegister) << "(" << DecPrinter(mStatusResponseRegister) << ")" << std::endl;
					mOperation = Operation_Idle;
					mDone = true;
				break;
				case 07:
					switch (aData & 0777) {
						case 0000: // Read cylinder register
							mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - read cylinder register" << std::endl;
							mStatusResponseRegister = IopInt_t(mCurrentTrack);
							mLogger << setloglevel(LogLevel_IoActivity) << "Setting mStatusResponseRegister to " << HexPrinter(mStatusResponseRegister) << "(" << DecPrinter(mStatusResponseRegister) << ")" << std::endl;
							mOperation = Operation_Idle;
							mDone = true;
						break;
						case 0001: // Read head register
							mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - read head register" << std::endl;
							if (mMounted) {
								size_t DiskSize = mNumHeads * mNumSectors * mNumTracks * mSectorSize;
								// Low order 4 bits: head
								// Bit 5: disk is reserved
								// Bit 6: set if disk is bigger than 600MB
								mStatusResponseRegister = (mCurrentHead & 0xf) | (1 << 5) | ((DiskSize > 600000000) ? (1 << 6) : 0);
							} else {
								// Disk is not reserved: return 0
								mStatusResponseRegister = 0;
							}
							mLogger << setloglevel(LogLevel_IoActivity) << "Setting mStatusResponseRegister to " << HexPrinter(mStatusResponseRegister) << "(" << DecPrinter(mStatusResponseRegister) << ")" << std::endl;
							mOperation = Operation_Idle;
							mDone = true;
						break;
						case 0002: // Read margin/difference register
							mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - read margin/difference register" << std::endl;
							//CRAY_ASSERT(false); // unimplemented
							mStatusResponseRegister = 0;
							mLogger << setloglevel(LogLevel_IoActivity) << "Setting mStatusResponseRegister to " << HexPrinter(mStatusResponseRegister) << "(" << DecPrinter(mStatusResponseRegister) << ")" << std::endl;
							mOperation = Operation_Idle;
							mDone = true;
						break;
						case 0003: // Read interlock register
							mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - read interlock register" << std::endl;
							mStatusResponseRegister = 0;
							mLogger << setloglevel(LogLevel_IoActivity) << "Setting mStatusResponseRegister to " << HexPrinter(mStatusResponseRegister) << "(" << DecPrinter(mStatusResponseRegister) << ")" << std::endl;
							mOperation = Operation_Idle;
							mDone = true;
						break;
						default:
							mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - unkown" << std::endl;
						break;
					}
				break;
				default:
					mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_MODE (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << " - unkown" << std::endl;
				break;
			}
			mIoMemoryAddr = aData >> 2; // Clear the lowest two bits
		return 0;
		case 002: // Read disk data
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: READ_DISK_DATA (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << std::endl;
			CRAY_ASSERT(mMounted || mBufferEchoMode);
			CRAY_ASSERT((aData & 0xe0) == 0);
			mCurrentSector = aData & 037; // Lower order bits contain the actual sector number. Upper bits specify special modes, which are not implemented at the moment
			mOperation = Operation_Read;
			mDone = false;
			ExecOperation();
		return 0;
		case 003: // Write disk data
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: WRITE_DISK_DATA (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << std::endl;
			CRAY_ASSERT(mMounted || mBufferEchoMode);
			mCurrentSector = aData & 037; // Lower order bits contain the actual sector number. Upper bits specify special modes, which are not implemented at the moment
			if ((aData & 0xe0) == 0) {
				// Write sector
				mOperation = Operation_Write;
			} else {
				// Format sector
				mOperation = Operation_Format;
			}
			mDone = false;
			ExecOperation();
		return 0;
		case 004: // Select head group
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_HEAD_GROUP (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << std::endl;
			mMounted = true; //Reseve unit here: HR-0808 Part3 7-24
			mCurrentHead = aData;
		return 0;
		case 005: { // Select cylinder
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: SELECT_CYLINDER (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << std::endl;
			mCurrentTrack = aData;
			mOperation = Operation_Idle;
			/* I think the documentation is wrong here. What we return is cylinder info in the upper bits, and don't care in the lower ones
			size_t SectorIdWord = 0;
			SectorIdWord = (mCurrentTrack & 0x3ff) << 13 | (mCurrentHead & 0xf) << 9 | (mCurrentSector & 0x1f) << 4;
			// Set up parity
			SectorIdWord |= (GetBit(SectorIdWord,20) ^ GetBit(SectorIdWord,16) ^ GetBit(SectorIdWord,12) ^ GetBit(SectorIdWord, 8) ^ GetBit(SectorIdWord,  4)) << 0;
			SectorIdWord |= (GetBit(SectorIdWord,21) ^ GetBit(SectorIdWord,17) ^ GetBit(SectorIdWord,13) ^ GetBit(SectorIdWord, 9) ^ GetBit(SectorIdWord,  5)) << 1;
			SectorIdWord |= (GetBit(SectorIdWord,22) ^ GetBit(SectorIdWord,18) ^ GetBit(SectorIdWord,14) ^ GetBit(SectorIdWord,10) ^ GetBit(SectorIdWord,  6)) << 2;
			SectorIdWord |= (GetBit(SectorIdWord,23) ^ GetBit(SectorIdWord,19) ^ GetBit(SectorIdWord,15) ^ GetBit(SectorIdWord,11) ^ GetBit(SectorIdWord,  7)) << 3;

			mStatusResponseRegister = SectorIdWord; // TODO: how to capture a 24-bit verification word in a 16-bit regiter??? HR-0808 Part 3 7-24
			*/
			mStatusResponseRegister = IopInt_t(mCurrentTrack << 5);
			mLogger << setloglevel(LogLevel_IoActivity) << "Setting mStatusResponseRegister to " << HexPrinter(mStatusResponseRegister) << "(" << DecPrinter(mStatusResponseRegister) << ")" << std::endl;
			mDone = true;
		} return 0;
		case 006:
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: INT_DIS (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << std::endl;
			mInterruptEnabled = false;
		return 0;
		case 007:
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: INT_EN (" << OctPrinter(aFunction) << ") data: " << OctPrinter(aData) << std::endl;
			mInterruptEnabled = true;
		return 0;
		case 010: // Read local memory address
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: READ_LOCAL_MEMORY_ADDR (" << OctPrinter(aFunction) << ") returning: " << HexPrinter(mIoMemoryAddr) << std::endl;
		return mIoMemoryAddr;
		case 011: // Read status response
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: READ_STATUS_RESPONSE_REG (" << OctPrinter(aFunction) << ") returning: " << HexPrinter(mStatusResponseRegister) << "(" << DecPrinter(mStatusResponseRegister) << ")" << std::endl;
		return mStatusResponseRegister;
		case 014: // Enter local memory address
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: ENTER_LOCAL_MEMORY_ADDR (" << OctPrinter(aFunction) << ") data: " << HexPrinter(aData) << std::endl;
			mIoMemoryAddr = aData;
		return 0;
		case 015: // Status response register diags
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: STATUS_RESPONSE_REG_DIAG (" << OctPrinter(aFunction) << ") data: " << HexPrinter(aData) << std::endl;
			mStatusResponseRegister = aData;
			mLogger << setloglevel(LogLevel_IoActivity) << "Setting mStatusResponseRegister to " << HexPrinter(mStatusResponseRegister) << "(" << DecPrinter(mStatusResponseRegister) << ")" << std::endl;
		return 0;
		default:
			mLogger << setloglevel(LogLevel_IoActivity) << "DoIo function: " << OctPrinter(aFunction) << " data: " << HexPrinter(aData) << std::endl;
			mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

void IopDiskDD29_c::ExecOperation() {
	size_t ImageOffset = mSectorSize * (mCurrentTrack * mNumHeads * mNumSectors + mCurrentHead * mNumSectors + mCurrentSector);
	if (mBufferEchoMode) {
		switch (mOperation) {
			case Operation_Idle: return;
			case Operation_Read:
				memcpy(&mParent.IoMemAccess<IopInt_t>(mIoMemoryAddr),mBufferAB,sizeof(mBufferAB));
				mIoMemoryAddr += sizeof(mBufferAB)/sizeof(IopInt_t);
				mDone = true;
				mOperation = Operation_Idle;
			break;
			case Operation_Write:
			case Operation_Format:
				memcpy(mBufferAB,&mParent.IoMemAccess<IopInt_t>(mIoMemoryAddr),sizeof(mBufferAB));
				mIoMemoryAddr += sizeof(mBufferAB)/sizeof(IopInt_t);
				mDone = true;
				mOperation = Operation_Idle;
			break;
			default:
				CRAY_ASSERT(false);
			break;
		}
	} else {
		LogLine_c TraceLine = mLogger << setloglevel(LogLevel_IoTrace);
		switch (mOperation) {
			case Operation_Idle: return;
			case Operation_Read: {
				CRAY_ASSERT(mMounted);
				CRAY_ASSERT(!mDone);
				mReads += mSectorSize / 1024;
				mLogger << setloglevel(LogLevel_IoActivity) << SideEffectIndent << "Reading sector " << "S:" << DecPrinter(mCurrentSector) << "H:" << DecPrinter(mCurrentHead) << "T:" << DecPrinter(mCurrentTrack) << " (offset: " << HexPrinter(ImageOffset) << ") " << " to I/O memory address " << HexPrinter(mIoMemoryAddr, 4) << " sector size: " << DecPrinter(mSectorSize) << std::endl;
				{
					std::stringstream EventStr;
					EventStr << "DK" << OctPrinter(mChannelIdx, -1) << " Reading sector " "S:" << DecPrinter(mCurrentSector) << "H:" << DecPrinter(mCurrentHead) << "T:" << DecPrinter(mCurrentTrack) << " Offset: " << HexPrinter(ImageOffset) << " I/O memory address " << HexPrinter(mIoMemoryAddr, 4) << std::endl;
					mParent.GetMainframe().GetEventDispatcher().Fire(EventStr.str());
				}
				std::fstream File(mImageFileName, std::ios::in | std::ios::binary);
				CRAY_ASSERT(!File.bad());
				File.seekg(ImageOffset);
				CRAY_ASSERT(!File.bad());
				CRAY_ASSERT(!File.eof());
				for (size_t i = 0; i < mSectorSize / sizeof(CInt_t); ++i) {
					CInt_t Data;
					File.read((char*)(&Data), sizeof(Data));
					//					Data = SwapBytes(Data);
					CRAY_ASSERT(!File.bad());
					CRAY_ASSERT(!File.eof());
					if (TraceLine.good()) {
						std::stringstream AsciiDump;
						CInt_t DumpData = SwapBytes(Data);
						CInt_t AsciiData = Data;
						TraceLine << HexPrinter(DumpData) << " ";
						for (size_t j = 0; j < sizeof(CInt_t); ++j) {
							AsciiDump << PrintableChar(char(AsciiData & 0xff));
							AsciiData >>= 8;
						}
						if (i % 4 == 3) {
							TraceLine << " - " << AsciiDump.str() << std::endl;
							AsciiDump.str(std::string());
						}
					}
					for (size_t Idx = 0; Idx < sizeof(CInt_t) / sizeof(IopInt_t); ++Idx) {
						IopInt_t IopData = ((IopInt_t *)(&Data))[Idx];
						IopData = SwapBytes(IopData);
						mParent.IoMemAccess<IopInt_t>(mIoMemoryAddr) = IopData;
						++mIoMemoryAddr;
						//						mLogger << HexPrinter(IopData) << " ";
					}
				}
				TraceLine << std::endl;
				File.close();
				mDone = true;
				mOperation = Operation_Idle;
			} break;
			case Operation_Write: {
				std::fstream File;
				CRAY_ASSERT(mMounted);
				CRAY_ASSERT(!mDone);
				CRAY_ASSERT(!mReadOnly);
				mWrites += mSectorSize / 1024;
				mLogger << setloglevel(LogLevel_IoActivity) << SideEffectIndent << "Writing sector " << "S:" << DecPrinter(mCurrentSector) << "H:" << DecPrinter(mCurrentHead) << "T:" << DecPrinter(mCurrentTrack) << " (offset: " << HexPrinter(ImageOffset) << ") " << " from I/O memory address " << HexPrinter(mIoMemoryAddr, 4) << " sector size: " << DecPrinter(mSectorSize) << std::endl;
				{
					std::stringstream EventStr;
					EventStr << "DK" << OctPrinter(mChannelIdx, -1) << " Writing sector " << "S:" << DecPrinter(mCurrentSector) << "H:" << DecPrinter(mCurrentHead) << "T:" << DecPrinter(mCurrentTrack) << " Offset: " << HexPrinter(ImageOffset) << " I/O memory address " << HexPrinter(mIoMemoryAddr, 4) << std::endl;
					mParent.GetMainframe().GetEventDispatcher().Fire(EventStr.str());
				}
				if (boost::filesystem::exists(mImageFileName)) {
					File.open(mImageFileName, std::ios::out | std::ios::in | std::ios::binary);
				}
				else {
					File.open(mImageFileName, std::ios::out | std::ios::binary);
				}
				CRAY_ASSERT(!File.bad());
				File.seekp(ImageOffset);
				CRAY_ASSERT(!File.bad());
				for (size_t i = 0; i < mSectorSize / sizeof(CInt_t); ++i) {
					IopInt_t IopData[4];
					for (size_t Idx = 0; Idx < sizeof(CInt_t) / sizeof(IopInt_t); ++Idx) {
						IopData[Idx] = SwapBytes(mParent.IoMemAccess<IopInt_t>(mIoMemoryAddr));
						++mIoMemoryAddr;
					}
					CInt_t *Data = ((CInt_t*)IopData);
					//					*Data = SwapBytes(*Data);
					if (TraceLine.good()) {
						std::stringstream AsciiDump;
						CInt_t DumpData = SwapBytes(*Data);
						CInt_t AsciiData = *Data;
						TraceLine << HexPrinter(DumpData) << " ";
						for (size_t j = 0; j < sizeof(CInt_t); ++j) {
							AsciiDump << PrintableChar(char(AsciiData & 0xff));
							AsciiData >>= 8;
						}
						if (i % 4 == 3) {
							TraceLine << " - " << AsciiDump.str() << std::endl;
							AsciiDump.str(std::string());
						}
					}
					File.write((char*)(Data), sizeof(*Data));
					CRAY_ASSERT(!File.bad());
				}
				TraceLine << std::endl;
				File.close();
				mDone = true;
				mOperation = Operation_Idle;
			} break;
			case Operation_Format: {
				std::fstream File;
				CRAY_ASSERT(mMounted);
				CRAY_ASSERT(!mDone);
				CRAY_ASSERT(!mReadOnly);
				ImageOffset = mSectorSize * (mCurrentTrack * mNumHeads * mNumSectors + mCurrentSector * mNumHeads + mCurrentHead);
				mLogger << setloglevel(LogLevel_IoActivity) << SideEffectIndent << "Formatting sector H:" << DecPrinter(mCurrentHead) << "S:" << DecPrinter(mCurrentSector) << "T:" << DecPrinter(mCurrentTrack) << " (offset: " << HexPrinter(ImageOffset) << ") " << " sector size: " << DecPrinter(mSectorSize) << std::endl;
				if (boost::filesystem::exists(mImageFileName)) {
					File.open(mImageFileName, std::ios::out | std::ios::in | std::ios::binary);
				}
				else {
					File.open(mImageFileName, std::ios::out | std::ios::binary);
				}
				CRAY_ASSERT(!File.bad());
				File.seekp(ImageOffset);
				CRAY_ASSERT(!File.bad());
				for (size_t i = 0; i < mSectorSize / sizeof(IopInt_t); ++i) {
					IopInt_t Data = 0;
					File.write((char*)(&Data), sizeof(Data));
					CRAY_ASSERT(!File.bad());
					++mIoMemoryAddr;
				}
				File.close();
				mDone = true;
				mOperation = Operation_Idle;
			} break;
			default:
				CRAY_ASSERT(false);
			break;
		}
	}
}
