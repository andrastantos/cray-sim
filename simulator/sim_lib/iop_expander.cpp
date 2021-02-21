#include "ui.h"
#include "iop_expander.h"
#include "cray_mainframe.h"
#include <algorithm>
#include <time.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/bind.hpp>
#include "commands.h"

//////////////////////////////////////////////////////////////////////////////////////////
// IopExpTape_c
//////////////////////////////////////////////////////////////////////////////////////////
IopExpTape_c::IopExpTape_c(const Configuration_c &aConfig, IopChannelExp_c &aParent):
	mParent(aParent),
	mBusy(false),
	mDone(true),
	mRegA(0),
	mRegAStat(0),
	mRegB(0),
	mRegC(0),
	mInterruptActive(false),
	mInterruptEnabled(false),
	mLogger(aConfig, "MT"),
	mCurrentTapeImage(-1),
	mChannelIdx(aConfig.get<uint16_t>("ChannelIdx")),
	mReads(0),
	mWrites(0),
	mIsReadOnly(aConfig.get<bool>("ReadOnly",false)),
	mTapeImage(mLogger, aConfig)
{
	mLogger.SetParent(mParent.GetLogger());
	mTapeImage.open(aConfig.get<std::string>("Tape"));
}

uint16_t IopExpTape_c::GetStatus(uint16_t aDeviceAddr) {
	uint16_t RetVal = 0;
	RetVal |= Status_DmaEnabled;
	if (mInterruptEnabled) RetVal |= Status_IntEnabled;
	if (mInterruptActive) RetVal |= Status_Int;
	if (mBusy) RetVal |= Status_DeviceBusy;
	if (mDone) RetVal |= Status_DeviceDone;
	return RetVal;
}

void IopExpTape_c::SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) {
	mInterruptEnabled = aInterruptEnabled;
	mParent.IntStatusChanged();
}

void IopExpTape_c::SetIntActive(bool aActive) {
	mInterruptActive = aActive;
	mParent.IntStatusChanged();
}

// Status bits - returned in mRegA
// 0x8000 - ???? Tested first sometimes for 0x5422 mask tests, but not always
// 0x4000 - DATA LATE
// 0x2000 - REWINDING (if this bit is set when XTAPEC is entered, it waits until it clears)
// 0x1000 - ????
// 0x0800 - ---------------
// 0x0400 - DATA PARITY ERROR
// 0x0200 - UNKOWN ERROR
// 0x0100 - END OF FILE
// 0x0080 - LOAD POINT DETECTED
// 0x0040 - ---------------
// 0x0020 - BAD TYPE
// 0x0010 - ---------------
// 0x0008 - ---------------
// 0x0004 - NO WRITE RING - set to 1 for read-only
// 0x0002 - WORD COUNT ERROR
// 0x0001 - READY	

uint16_t IopExpTape_c::GetRegA(uint16_t aDeviceAddr) {
	uint16_t RetVal = mRegAStat;
	if (mIsReadOnly) RetVal |= Status_ReadOnly;
	switch (mTapeImage.GetState()) {
		case TapFile_c::State_e::BeginningOfTape: RetVal |= Status_BeginOfTape; break;
		case TapFile_c::State_e::EndOfTape: RetVal |= Status_EndOfTape; break;
		case TapFile_c::State_e::EndOfFile: RetVal |= Status_EndOfFile; break; // TODO: Should we set the error bit here?
		default: break;
	}
	mLogger << setloglevel(LogLevel_IoActivity) << "returning status with RegA: " << HexPrinter(RetVal, 4) << std::endl;
	return RetVal;
}

void IopExpTape_c::FinalizeControl(uint16_t aStatus) {
	mDone = true;
	mBusy = false;
	mRegAStat |= aStatus;
	SetIntActive(true);
}

// NOTE: rewind closes the tape so the tape image can be manipulated outside the simulator.
void IopExpTape_c::Control(uint16_t aDeviceAddr, uint16_t aControl) {
	CRAY_ASSERT((aControl & ~(Control_Start | Control_Clear)) == 0);
	if ((aControl & Control_Clear) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'CLEAR' recevied. Clearing pending interrupt" << std::endl;
		SetIntActive(false);
	}
	if ((aControl & Control_Start) != 0) {
		mRegAStat = 0;
		mLogger << setloglevel(LogLevel_IoActivity) << "'START' recevied with RegA: " << HexPrinter(mRegA,4) << " RegB: " << HexPrinter(mRegB,4) << " RegC: " << HexPrinter(mRegC,4) << std::endl;
		switch ((mRegA >> 3) & 7) {
			// 52 - Erase                      - RegB: not set                    RegC: not set
			// TODO: implement erase command - note: it's never issued by IOS it seems
			//
			// Read and space operations stop if they encounter an EOF mark. To get to the beginning of the next file, you have to issue another space forward with 1 as the SpaceSize
			// Operation 8 (File search backward) and probably all other backward operations stop when they encounter the LOAD POINT, that is the front of the tape. They set the LOAD_POINT_DETECT status bit
			//
			// The READY bit is set to indicate that the device is finished processing the current command
			// EOF is a special mark on the tape, that you can position to. The read of a block that stops at the last byte of the file does *NOT* set the EOF bit.
			// When you SPACE or READ, the head will stop on an EOF
			// To position to the start of a file, issue a SPACE_FORWARD '1' after the SPACE or READ stopped on an EOF
			// NOTE: Space commands count in 4096-byte (record) increments. Read and write count in 16-bit word increments.
			//       On the real tape reads would stop at the end of a record, and each write command would write a record on the tape
			//       This behavior is not simulated at the moment: record size is fixed at 4096 bytes, except for the last partial block of a file.
			//       Since this is the only operation the IOS is using, it is fine for now at least.

			//  0 - read forward               - RegB: set to destination buffer  RegC: set to TransferSize*-1
			case Command_ReadForward: {
				mTapeImage.ReOpen();
				int WordsToRead = -int16_t(mRegC);
				CRAY_ASSERT(WordsToRead > 0);
				mLogger << setloglevel(LogLevel_IoActivity) << "Loading " << DecPrinter(WordsToRead) << " words to address " << HexPrinter(mRegB) << " from tape file " << mTapeImage.GetFileName() << " offset " << HexPrinter(mTapeImage.tellg()) << std::endl;
				mReads += WordsToRead / 2048;
				std::vector<uint8_t> TmpBuf = mTapeImage.Read(WordsToRead * sizeof(IopInt_t));
				size_t BytesRead = TmpBuf.size();
				if (BytesRead % 2 != 0) TmpBuf.resize(BytesRead + 1);
				// Swap all words and copy to target buffer
				for(size_t i=0;i<TmpBuf.size();i+=2) {
					uint16_t Data = uint16_t(TmpBuf[i]) << 8 | uint16_t(TmpBuf[i+1]);
					mParent.GetDma().WriteWord(mRegB,Data);
					++mRegB;
					++mRegC;
				}
				if (BytesRead / 2 != size_t(WordsToRead)) {
					mLogger << setloglevel(LogLevel_IoActivity) << "partial buffer returned. Only " << DecPrinter(BytesRead) << " bytes are read. RegB is: " << HexPrinter(mRegB) << std::endl;
				}
				switch (mTapeImage.GetState()) {
				case TapFile_c::State_e::EndOfTape: FinalizeControl(Status_Ready | Status_Error); break;
				case TapFile_c::State_e::EndOfFile: FinalizeControl(Status_Ready | Status_Error); break; // TODO: Should we set the error bit here?
				default: FinalizeControl(Status_Ready); break;
				}
			} break;
			//  8 - rewind                     - RegB: not set                    RegC: not set
			case Command_Rewind: {
				mTapeImage.ReOpen();
				mLogger << setloglevel(LogLevel_IoActivity) << "Rewiding" << std::endl;
				mTapeImage.SeekToBeginningOfTape();
				FinalizeControl(Status_Ready);
				mTapeImage.close();
			} break;
			// 24 - space forward              - RegB: not set                    RegC: set to SpaceSize*-1
			case Command_SpaceForward: {
				mTapeImage.ReOpen();
				int RecordsToSkip = -int16_t(mRegC);
				CRAY_ASSERT(RecordsToSkip > 0);
				mLogger << setloglevel(LogLevel_IoActivity) << "Spacing " << DecPrinter(RecordsToSkip) << " records forward in tape file " << mTapeImage.GetFileName() << " offset " << HexPrinter(mTapeImage.tellg()) << std::endl;
				for (int i = 0; i < RecordsToSkip; ++i) {
					mTapeImage.SeekToNextRecord();
					if (mTapeImage.GetState() == TapFile_c::State_e::EndOfTape) {
						mLogger << setloglevel(LogLevel_IoActivity) << "spacing cut short by EOT condition after " << DecPrinter(i) << " records." << std::endl;
						FinalizeControl(Status_Ready | Status_Error);
						break;
					}
					if (mTapeImage.GetState() == TapFile_c::State_e::EndOfFile) {
						mLogger << setloglevel(LogLevel_IoActivity) << "spacing cut short by EOF condition after " << DecPrinter(i) << " records." << std::endl;
						FinalizeControl(Status_Ready | Status_Error);
						break;
					} // TODO: Should we set the error bit here?
					++mRegC;
				}
				if (mRegC == 0) FinalizeControl(Status_Ready);
			} break;
			// 32 - space backward             - RegB: not set                    RegC: set to SpaceSize*-1
			case Command_SpaceBackward: {
				mTapeImage.ReOpen();
				int RecordsToSkip = -int16_t(mRegC);
				CRAY_ASSERT(RecordsToSkip > 0);
				mLogger << setloglevel(LogLevel_IoActivity) << "Spacing " << DecPrinter(RecordsToSkip) << " records backward in tape file " << mTapeImage.GetFileName() << " offset " << HexPrinter(mTapeImage.tellg()) << std::endl;
				for (int i = 0; i < RecordsToSkip; ++i) {
					mTapeImage.SeekToPrevRecord();
					if (mTapeImage.GetState() == TapFile_c::State_e::EndOfTape) {
						mLogger << setloglevel(LogLevel_IoActivity) << "spacing cut short by EOT condition after " << DecPrinter(i) << " records." << std::endl;
						FinalizeControl(Status_Ready | Status_Error);
						break;
					}
					if (mTapeImage.GetState() == TapFile_c::State_e::EndOfFile) {
						mLogger << setloglevel(LogLevel_IoActivity) << "spacing cut short by EOF condition after " << DecPrinter(i) << " records." << std::endl;
						FinalizeControl(Status_Ready | Status_Error);
						break;
					} // TODO: Should we set the error bit here?
					++mRegC;
				}
				if (mRegC == 0) FinalizeControl(Status_Ready);
			} break;
			// 40 - write forward              - RegB: set to destination buffer  RegC: set to TransferSize*-1
			case Command_WriteForward : {
				CRAY_ASSERT(!mIsReadOnly);
				mTapeImage.ReOpen();
				int WordsToWrite = -int16_t(mRegC);
				CRAY_ASSERT(WordsToWrite > 0);
				mLogger << setloglevel(LogLevel_IoActivity) << "Writing " << DecPrinter(WordsToWrite) << " words from address " << HexPrinter(mRegB) << " to tape file " << mTapeImage.GetFileName() << " offset " << HexPrinter(mTapeImage.tellg()) << std::endl;
				mWrites += WordsToWrite / 2048;
				std::vector<uint8_t> TmpBuf(WordsToWrite*sizeof(IopInt_t));
				// Swap all words and copy to target buffer
				for(size_t i=0;i<TmpBuf.size();i+=2) {
					uint16_t Data = mParent.GetDma().ReadWord(mRegB);
					TmpBuf[i] = (Data >> 8) & 0xff;
					TmpBuf[i+1] = (Data >> 0) & 0xff;
					++mRegB;
					++mRegC;
				}
				mTapeImage.WriteRecord(TmpBuf);
				FinalizeControl(Status_Ready);
			} break;
			// 48 - write EOF forward          - RegB: not set                    RegC: not set
			case Command_WriteEOF: {
				CRAY_ASSERT(!mIsReadOnly);
				mTapeImage.ReOpen();
				mLogger << setloglevel(LogLevel_IoActivity) << "Writing EOF to tape file " << mTapeImage.GetFileName() << " offset " << HexPrinter(mTapeImage.tellg()) << std::endl;
				mTapeImage.WriteEndOfFile();
				FinalizeControl(Status_Ready);
			} break;
			case Command_Erase: {
				mLogger << setloglevel(LogLevel_Error) << "ERASE command to tape file " << mTapeImage.GetFileName() << " offset " << HexPrinter(mTapeImage.tellg()) << " command not implemented" << std::endl;
				CRAY_ASSERT(false);
			} break;
			default:
				CRAY_ASSERT(false);
		}
	}
}

void IopExpTape_c::Tick() {
}

void IopExpTape_c::MountTape(const std::string &aTapeFileName) {
	if (mTapeImage.is_open()) throw Generic_x("Can't unmount tape while it's in use.");
	mTapeImage.open(aTapeFileName);
	mTapeImage.close();
}

void IopExpTape_c::GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const {
	aStatus.put(aLongFormat ? "Reads" : "R", mReads); 
	aStatus.put(aLongFormat ? "Write" : "W", mWrites);
	if (mTapeImage.is_open()) aStatus.put(aLongFormat ? "Position" : "Pos", const_cast<PokedTapFile_c*>(&mTapeImage)->tellg()); else aStatus.put(aLongFormat ? "Position" : "Pos", "-");	
	aStatus.put(aLongFormat ? "File Name" : "FName", mTapeImage.GetFileName());
}


void IopExpTape_c::RegisterCommands(CommandHooks_t &aHooks) {
	class CmdMount_c: public CmdFactoryBase_i {
	public:
		explicit CmdMount_c(IopExpTape_c &aParent) : mParent(aParent) {}
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
			} else {
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
		IopExpTape_c &mParent;
	};

	aHooks.emplace_back(std::make_unique<CmdMount_c>(*this));
/*	UIHook_s Hook;
	Hook.Callback = boost::bind(&IopExpTape_c::ExecUI, const_cast<IopExpTape_c*>(this));
	Hook.TriggerKey = 0x14; // ^T
	aHooks.emplace_back(Hook);*/
}

//////////////////////////////////////////////////////////////////////////////////////////
// IopExpDummyDevice_c
//////////////////////////////////////////////////////////////////////////////////////////
IopExpDummyDevice_c::IopExpDummyDevice_c(const Configuration_c &aConfig, IopChannelExp_c &aParent, const std::string &aLogName):
	mParent(aParent),
	mBusy(false),
	mDone(true),
	mRegA(0),
	mRegB(0),
	mRegC(0),
	mInterruptActive(false),
	mInterruptEnabled(false),
	mLogger(aConfig, aLogName.c_str()),
	mChannelIdx(aConfig.get<uint16_t>("ChannelIdx"))
{
	mLogger.SetParent(mParent.GetLogger());
}

IopExpDummyDevice_c::IopExpDummyDevice_c(const Configuration_c &aConfig, class IopChannelExp_c &aParent, const std::string &aLogName, uint16_t aChannelIdx) :
	mParent(aParent),
	mBusy(false),
	mDone(true),
	mRegA(0),
	mRegB(0),
	mRegC(0),
	mInterruptActive(false),
	mInterruptEnabled(false),
	mLogger(aConfig, aLogName.c_str()),
	mChannelIdx(aChannelIdx)
{
	mLogger.SetParent(mParent.GetLogger());
}

void IopExpDummyDevice_c::SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) {
	mInterruptEnabled = aInterruptEnabled;
	mParent.IntStatusChanged();
}

void IopExpDummyDevice_c::SetIntActive(bool aActive) {
	mInterruptActive = aActive;
	mParent.IntStatusChanged();
}

uint16_t IopExpDummyDevice_c::GetStatus(uint16_t aDeviceAddr) {
	uint16_t RetVal = 0;
	RetVal |= Status_DmaEnabled;
	if (mInterruptEnabled) RetVal |= Status_IntEnabled;
	if (mInterruptActive) RetVal |= Status_Int;
	if (mBusy) RetVal |= Status_DeviceBusy;
	if (mDone) RetVal |= Status_DeviceDone;
	return RetVal;
}


void IopExpDummyDevice_c::Control(uint16_t aDeviceAddr, uint16_t aControl) {
	CRAY_ASSERT((aControl & ~(Control_Start | Control_Clear | Control_Pulse)) == 0);
	if ((aControl & Control_Clear) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'CLEAR' recevied. Clearing pending interrupt" << std::endl;
		SetIntActive(false);
	}
	if ((aControl & Control_Start) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'START' recevied with RegA: " << HexPrinter(mRegA,4) << " RegB: " << HexPrinter(mRegB,4) << " RegC: " << HexPrinter(mRegC,4) << std::endl;
	}
	if ((aControl & Control_Pulse) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'PULSE' recevied with RegA: " << HexPrinter(mRegA,4) << " RegB: " << HexPrinter(mRegB,4) << " RegC: " << HexPrinter(mRegC,4) << std::endl;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// IopExpPrinterDevice_c
//////////////////////////////////////////////////////////////////////////////////////////
IopExpPrinterDevice_c::IopExpPrinterDevice_c(const Configuration_c &aConfig, IopChannelExp_c &aParent):
	mParent(aParent),
	mBusy(false),
	mDone(true),
	mRegA(0),
	mRegB(0),
	mRegC(0),
	mInterruptActive(false),
	mInterruptEnabled(false),
	mLogger(aConfig, aConfig.get<std::string>("Name").c_str()),
	mStatus(0),
	mChannelIdx(aConfig.get<uint16_t>("ChannelIdx")),
	mInGraphicsMode(false),
	mWrites(0)
{
	mLogger.SetParent(mParent.GetLogger());
	mPrintFileName = aConfig.get<std::string>("PrintFileName");
}

void IopExpPrinterDevice_c::GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const {
	aStatus.put(aLongFormat ? "Writes" : "W", mWrites);
	aStatus.put(aLongFormat ? "Mode" : "M", mInGraphicsMode ? "Gr" : "Txt");
	aStatus.put(aLongFormat ? "File Name" : "FName", mPrintFileName);
}

void IopExpPrinterDevice_c::SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) {
	mInterruptEnabled = aInterruptEnabled;
	mParent.IntStatusChanged();
}

void IopExpPrinterDevice_c::SetIntActive(bool aActive) {
	mInterruptActive = aActive;
	mParent.IntStatusChanged();
}

uint16_t IopExpPrinterDevice_c::GetStatus(uint16_t aDeviceAddr) {
	uint16_t RetVal = 0;
	RetVal |= Status_DmaEnabled;
	if (mInterruptEnabled) RetVal |= Status_IntEnabled;
	if (mInterruptActive) RetVal |= Status_Int;
	if (mBusy) RetVal |= Status_DeviceBusy;
	if (mDone) RetVal |= Status_DeviceDone;
//	mLogger << setloglevel(LogLevel_IoActivity) << "returning status: " << OctPrinter(RetVal) << " with mInterruptEnabled: " << (mInterruptEnabled?"true":"false") << " mInterruptActive: " << (_InterruptActive?"true":"false") << std::endl;
	return RetVal;
}

void IopExpPrinterDevice_c::SetRegC(uint16_t aDeviceAddr, uint16_t aValue) {
	mRegC = aValue;
	mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegC: " << HexPrinter(mRegC,4) << std::endl;

	mBusy = false;
	mDone = true;
	SetIntActive(true);

	mPrintFile.open(mPrintFileName, std::ios::app);
	LogLine_c LogLine = mLogger << setloglevel(LogLevel_IoActivity);
	LogLine << "Printing: ";
	while (mRegB != 0) {
		IopInt_t Data = mParent.GetDma().ReadWord(mRegC);
		LogLine << " " << HexPrinter(Data);
		if (mInGraphicsMode) {
			for(int i=0;i<16;++i) {
				mPrintFile << ((Data & 0x8000) == 0?' ':'X');
				Data <<= 1;
			}
			mWrites++;
		} else {
			mPrintFile << (char)(Data >> 8) << (char)(Data & 0xff);
			mWrites++;
		}
		++mRegC;
		++mRegB;
	}
	mPrintFile.close();
	LogLine << std::endl;
	mStatus = StatusCode_Done;
}

uint16_t IopExpPrinterDevice_c::GetRegA(uint16_t aDeviceAddr) {
	mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegA: " << HexPrinter(mStatus,4) << std::endl;
	return mStatus;
}


void IopExpPrinterDevice_c::Control(uint16_t aDeviceAddr, uint16_t aControl) {
	CRAY_ASSERT((aControl & ~(Control_Start | Control_Clear | Control_Pulse)) == 0);
	if ((aControl & Control_Clear) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'CLEAR' recevied. Clearing pending interrupt" << std::endl;
		SetIntActive(false);
	}
	if ((aControl & Control_Start) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'START' recevied with RegA: " << HexPrinter(mRegA,4) << " RegB: " << HexPrinter(mRegB,4) << " RegC: " << HexPrinter(mRegC,4) << std::endl;
	}
	if ((aControl & Control_Pulse) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'PULSE' recevied with RegA: " << HexPrinter(mRegA,4) << " RegB: " << HexPrinter(mRegB,4) << " RegC: " << HexPrinter(mRegC,4) << std::endl;
		mDone = true;
		mBusy = false;
		switch (mRegA) {
			case CommandCode_ClearInt:
				mDone = true;
				mBusy = false;
				SetIntActive(false);
				mStatus = 0;
			break;
			case CommandCode_4:
				mDone = true;
				mBusy = false;
				mStatus = 0;
				mInGraphicsMode = false;
			break;
			case CommandCode_GraphicsMode:
				mDone = true;
				mBusy = false;
				mStatus = 0;
				mInGraphicsMode = true;
			break;
			case CommandCode_NewLine:
				mDone = true;
				SetIntActive(true);
				mPrintFile.open(mPrintFileName, std::ios::app);
				mPrintFile << std::endl;
				mPrintFile.close();
				mStatus = StatusCode_Done;
			break;
			case CommandCode_NewPage:
				mDone = true;
				mBusy = false;
				SetIntActive(true);
				mPrintFile.open(mPrintFileName, std::ios::app);
				mPrintFile << '\f';
				mPrintFile.close();
				mStatus = StatusCode_Done;
			break;
			default:
				mDone = true;
				mBusy = false;
				mPrintFile.open(mPrintFileName, std::ios::app);
				mPrintFile << "------------------------- UNKOWN CODE " << mRegA << " --------------------------" << std::endl;
				mPrintFile.close();
				mStatus = 0;
			break;
		}
//		mLogger << setloglevel(LogLevel_IoActivity) << "new status: mInterruptEnabled: " << (mInterruptEnabled?"true":"false") << " mInterruptActive: " << (_InterruptActive?"true":"false") << std::endl;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// IopExpClockDevice_c
//////////////////////////////////////////////////////////////////////////////////////////
// Hayes communication protocol:
// Read Date
//    --> ATRD\x0d
//    <-- YYMMDD\x0d (each being one digit)
// Read Time
//    --> ATRT\x0d
//    <-- HHMMSS\x0d (each being one digit)
// Set Date
//    --> ATSDYYMMDD\x0d
//    <-- 0\x0d
// Set Time
//    --> ATSTHHMMSS\x0d
//    <-- 0\x0d
// ???
//    --> ATLC\x0d
//    <-- 0\x0d
// ???
//    --> ATVD\x0d
//    <-- 0\x0d
// ???
//    --> ATVT\x0d
//    <-- 0\x0d
// Each byte sent to the clock in RegA with a 'START' pulse. The clock generates an interrupt on the reception of each byte
// Each byte sent by the clock in RegA with an interrupt.
// Actually, this is a Hayes Chronograph, attached to an RS232 port. Documentation: http://tinymicros.com/mediawiki/images/7/7a/Hayes_Chronograph.pdf
IopExpClockDevice_c::IopExpClockDevice_c(const Configuration_c &aConfig, IopChannelExp_c &aParent):
	mParent(aParent),
	mBusy(false),
	mDone(false),
	mRegA(0),
	mRegB(0),
	mRegC(0),
	mInterruptEnabled(false),
	mLogger(aConfig, aConfig.get<std::string>("Name").c_str()),
	mResponseTimer(0),
	mResponseTimeout(aConfig.get<size_t>("ResponseTimeout", 100)),
	mPrimaryChannelIdx(aConfig.get<uint16_t>("PrimaryChannelIdx")),
	mRequestChannelIdx(aConfig.get<uint16_t>("RequestChannelIdx", mPrimaryChannelIdx+1)),
	mResponseChannelIdx(aConfig.get<uint16_t>("ResponseChannelIdx", mPrimaryChannelIdx+0)),
	mYearLimit(aConfig.get<bool>("YearLimit", true))
{
	for(size_t i=0;i<sizeof(mInterruptActive)/sizeof(bool);++i) mInterruptActive[i] = false;
	mLogger.SetParent(mParent.GetLogger());
}

void IopExpClockDevice_c::SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) {
	mInterruptEnabled = aInterruptEnabled;
	mParent.IntStatusChanged();
}

void IopExpClockDevice_c::SetIntActive(uint16_t aDeviceAddr, bool aActive) {
	CRAY_ASSERT(aDeviceAddr < sizeof(mInterruptActive) / sizeof(bool));
	mInterruptActive[aDeviceAddr] = aActive;
	mParent.IntStatusChanged();
}

void IopExpClockDevice_c::SetRegA(uint16_t aDeviceAddr, uint16_t aValue) {
	mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegA: " << HexPrinter(aValue,4) << " on address " << OctPrinter(aDeviceAddr) << std::endl;
	mRegA = aValue;
	CRAY_ASSERT(aDeviceAddr < sizeof(mInterruptActive)/sizeof(bool));
	if (mInterruptActive[aDeviceAddr]) {
		mLogger << setloglevel(LogLevel_IoActivity) << "Clearing interrupt while updating RegA" << std::endl;
		SetIntActive(aDeviceAddr, false); //TODO: do we need to do this?
	}
}

uint16_t IopExpClockDevice_c::GetRegA(uint16_t aDeviceAddr) {
	IopInt_t RetVal = 0;
	if (aDeviceAddr == mResponseChannelIdx) {
		if (mResponse.empty()) {
			mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegA, but empty response string: " << HexPrinter(RetVal,4) << " on address " << OctPrinter(aDeviceAddr) << std::endl;
		} else {
			RetVal = mResponse.front();
			mResponse = &mResponse[0]+1;
			mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegA: " << HexPrinter(RetVal,4) << " on address " << OctPrinter(aDeviceAddr) << std::endl;
			SetIntActive(aDeviceAddr, true);
		}
	}
	return RetVal;
}

uint16_t IopExpClockDevice_c::GetStatus(uint16_t aDeviceAddr) {
	uint16_t RetVal = 0;
	RetVal |= Status_DmaEnabled;
	CRAY_ASSERT(aDeviceAddr < sizeof(mInterruptActive)/sizeof(bool));
	if (mInterruptEnabled) RetVal |= Status_IntEnabled;
	if (mInterruptActive[aDeviceAddr]) RetVal |= Status_Int;
	if (mBusy) RetVal |= Status_DeviceBusy;
	if (mDone) RetVal |= Status_DeviceDone;
	return RetVal;
}

void IopExpClockDevice_c::ProcessRequest() {
	if (mRequest == "ATLC") {
		mLogger << setloglevel(LogLevel_IoActivity) << "Received LC" << std::endl;
		mResponse = "0\x0d";
	} else if (mRequest == "ATVD") {
		mLogger << setloglevel(LogLevel_IoActivity) << "Received VD" << std::endl;
		mResponse = "0\x0d";
	} else if (mRequest == "ATVT") {
		mLogger << setloglevel(LogLevel_IoActivity) << "Received VT" << std::endl;
		mResponse = "0\x0d";
	} else if (mRequest == "ATRT") {
		mLogger << setloglevel(LogLevel_IoActivity) << "Received RT" << std::endl;
		boost::local_time::local_date_time CurrentTime(boost::posix_time::second_clock::local_time(), boost::local_time::time_zone_ptr());
		auto Time = CurrentTime.local_time().time_of_day();
		mResponse = (boost::format("%02d%02d%02d\x0d") % Time.hours() % Time.minutes() % Time.seconds()).str();
	} else if (mRequest == "ATRD") {
		mLogger << setloglevel(LogLevel_IoActivity) << "Received RD" << std::endl;
		boost::local_time::local_date_time CurrentTime(boost::posix_time::second_clock::local_time(), boost::local_time::time_zone_ptr());
		auto Date = CurrentTime.local_time().date().year_month_day();
		uint16_t Year = Date.year;
		// COS only accepts years between 1980 and 1999. So, we'll limit ourselves to that 20 year period
		if (mYearLimit) Year = (Year % 20) + 80;
		mResponse = (boost::format("%02d%02d%02d\x0d") % Year % Date.month.as_number() % Date.day.as_number()).str();
	} else if (boost::starts_with(mRequest, "ATSD")) {
		mLogger << setloglevel(LogLevel_IoActivity) << "Received SD, full request:" << mRequest << std::endl;
		mResponse = "0\x0d";
	} else if (boost::starts_with(mRequest, "ATST")) {
		mLogger << setloglevel(LogLevel_IoActivity) << "Received ST, full request:" << mRequest << std::endl;
		mResponse = "0\x0d";
	} else {
		mLogger << setloglevel(LogLevel_IoActivity) << "Received unknown command:" << mRequest << std::endl;
		mResponse = "0\x0d";
	}
	//CRAY_ASSERT(mResponseTimer == 0);
	mResponseTimer = 1; // Start the response timer
}

void IopExpClockDevice_c::Control(uint16_t aDeviceAddr, uint16_t aControl) {
	//mParent.GetParent().GetLogger().SetDisplayLogLevel(LogLevel_All);
	CRAY_ASSERT((aControl & ~(Control_Start | Control_Clear | Control_Pulse)) == 0);
	if ((aControl & Control_Clear) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'CLEAR' recevied on address " << OctPrinter(aDeviceAddr) << ". Clearing pending interrupt" << std::endl;
		SetIntActive(aDeviceAddr, false);
		if (aDeviceAddr == mResponseChannelIdx && !mResponse.empty()) {
			// Schedule another interrupt
			mResponseTimer = 1;
		}
	}
	if ((aControl & Control_Start) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'START' recevied on address " << OctPrinter(aDeviceAddr) << " with RegA: " << HexPrinter(mRegA,4) << " RegB: " << HexPrinter(mRegB,4) << " RegC: " << HexPrinter(mRegC,4) << std::endl;
		if ((mRegA & 0xff) == '\x0d') {
			ProcessRequest();
			mRequest.clear();
		} else {
			mRequest += (char)(mRegA & 0xff);
		}
		SetIntActive(aDeviceAddr, true);
	}
	if ((aControl & Control_Pulse) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'PULSE' recevied on address " << OctPrinter(aDeviceAddr) << " with RegA: " << HexPrinter(mRegA,4) << " RegB: " << HexPrinter(mRegB,4) << " RegC: " << HexPrinter(mRegC,4) << std::endl;
	}
}

void IopExpClockDevice_c::Tick() {
	if (mResponseTimer > 0) {
		mResponseTimer++;
		if (mResponseTimer > mResponseTimeout) {
			//mLogger << setloglevel(LogLevel_IoActivity) << "Sending interrupt on channel:" << DecPrinter(mResponseAddr) << std::endl;
			SetIntActive(mResponseChannelIdx, true);
			mResponseTimer = 0;
		}
	}
}

uint16_t IopExpClockDevice_c::GetChannelIdx(size_t aIdx) const {
	switch (aIdx) {
		case 0: return mPrimaryChannelIdx;
		case 1: return mRequestChannelIdx;
//		case 2: return mResponseChannelIdx;
		default: CRAY_ASSERT(false);
	}
	throw Generic_x("Unreachable code");
}

//////////////////////////////////////////////////////////////////////////////////////////
// IopExpDiskAmpexDM980_c
//////////////////////////////////////////////////////////////////////////////////////////
// Must be an Ampex DM980 from this: http://bitsavers.trailing-edge.com/pdf/dilog/30006-1_DQ202A_Sep81.pdf - though as far as format goes, it seems identical to the CDC SMD9762
// It's a 80MB drive, but formatted as a 64MB one (leaving the last sector on each track empty)
// Protocol seems to be:
// RegA 0x4000        
// RegB <Track>       RegC 5
// RegB <Head>        RegC 1
// RegB <StartSector> RegC 2
// RegB <SectorCnt>   RegC 3
// RegB <buffer ptr>  RegC 8 - write sector; 0 - read sector
// START

// AMPEX commands (DOC)
const uint16_t Ampex_Read              = 000;
const uint16_t Ampex_Write             = 010;
const uint16_t Ampex_Format            = 020;
const uint16_t Ampex_Checksum          = 040;
const uint16_t Ampex_StartSeek         = 060;
const uint16_t Ampex_SelectDrive       = 070;
const uint16_t Ampex_AttemptCorrection = 0110;
const uint16_t Ampex_ReturnToZero      = 0120; // Starts format

// AMPEX register steering codes (DOC)

const uint16_t Ampex_RegSelHead     = 1;
const uint16_t Ampex_RegSelSector   = 2;
const uint16_t Ampex_RegSelCount    = 3;
const uint16_t Ampex_RegSelDrive    = 4;
const uint16_t Ampex_RegSelCylinder = 5;

// AMPEX option codes (DOA)
const uint16_t Ampex_OptionNoSeek      = 040000;
const uint16_t Ampex_OptionEnableRetry = 0100000;

IopExpDiskAmpexDM980_c::IopExpDiskAmpexDM980_c(const Configuration_c &aConfig, IopChannelExp_c &aParent):
	mParent(aParent),
	mBusy(false),
	mDone(true),
	mRegA(0),
	mRegB(0),
	mRegC(0),
	mInterruptActive(false),
	mInterruptEnabled(false),
	mLogger(aConfig, "DK"),
	mHead(0),
	mStartSector(0),
	mSectorCnt(0),
	mTrack(0),
	mChannelIdx(aConfig.get<uint16_t>("ChannelIdx")),
	mReads(0),
	mWrites(0)
{
	mLogger.SetParent(mParent.GetLogger());
	mImageFileName = aConfig.get<std::string>("ImageFileName");
	mReadOnly = aConfig.get<bool>("ReadOnly",false);
	mNumHeads = aConfig.get<size_t>("Heads", 5);
	if (mNumHeads == 0) throw InvalidParameter_x("Heads must be greater than 0");
	mNumSectors = aConfig.get<size_t>("Sectors", 35);
	if (mNumSectors == 0) throw InvalidParameter_x("Sectors must be greater than 0");
	mNumTracks = aConfig.get<size_t>("Tracks", 823);
	if (mNumTracks == 0) throw InvalidParameter_x("Tracks must be greater than 0");
	mSectorSize = aConfig.get<size_t>("SectorSize",512);
	if (mSectorSize == 0) throw InvalidParameter_x("SectorSize must be greater than 0");
}

void IopExpDiskAmpexDM980_c::SetIntActive(bool aActive) {
	mInterruptActive = aActive;
	mParent.IntStatusChanged();
}

void IopExpDiskAmpexDM980_c::SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) {
	mInterruptEnabled = aInterruptEnabled;
	mParent.IntStatusChanged();
}

void IopExpDiskAmpexDM980_c::SetRegA(uint16_t aDeviceAddr, uint16_t aValue) {
	mRegA = aValue;
	mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegA: " << HexPrinter(mRegA,4) << std::endl;
}

void IopExpDiskAmpexDM980_c::SetRegB(uint16_t aDeviceAddr, uint16_t aValue) {
	mRegB = aValue;
	mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegB: " << HexPrinter(mRegB,4) << std::endl;
}
void IopExpDiskAmpexDM980_c::SetRegC(uint16_t aDeviceAddr, uint16_t aValue) {
	mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegC: " << HexPrinter(mRegC,4) << std::endl;
	mRegC = aValue;
	switch (aValue) {
		// These are register select operations - load RegB to the appropriate place
		case Ampex_RegSelCylinder:
			mTrack = mRegB;
		break;
		case Ampex_RegSelHead:
			mHead = mRegB;
		break;
		case Ampex_RegSelSector:
			mStartSector = mRegB;
		break;
		case Ampex_RegSelCount:
			mSectorCnt = mRegB;
		break;
		case Ampex_RegSelDrive:
			// Not implemented for now
		break;
		// These are actions - wait for the START to actually do anything about it
		case Ampex_Read:
		case Ampex_Write:
		case Ampex_Format:
		case Ampex_Checksum:
		case Ampex_StartSeek:
		case Ampex_SelectDrive:
		case Ampex_AttemptCorrection:
		case Ampex_ReturnToZero:
		break;

		default:
			mLogger << setloglevel(LogLevel_Error) << "ERROR: unknown mRegC value: " << HexPrinter(mRegC,4) << std::endl;
//			CRAY_ASSERT(false);
		break;
	}
}

uint16_t IopExpDiskAmpexDM980_c::GetStatus(uint16_t aDeviceAddr) {
	uint16_t RetVal = 0;
	RetVal |= Status_DmaEnabled;
	if (mInterruptEnabled) RetVal |= Status_IntEnabled;
	if (mInterruptActive) RetVal |= Status_Int;
	if (mBusy) RetVal |= Status_DeviceBusy;
	if (mDone) RetVal |= Status_DeviceDone;
	return RetVal;
}


void IopExpDiskAmpexDM980_c::Create() {
	if (boost::filesystem::exists(mImageFileName)) return;
	std::fstream File(mImageFileName, std::ios::out | std::ios::binary);
	std::vector<char> Sector(mSectorSize);
	for (size_t i = 0; i < mNumSectors*mNumHeads*mNumTracks; ++i) File.write(&(Sector[0]), mSectorSize);
	CRAY_ASSERT(File.good());
}

void IopExpDiskAmpexDM980_c::Control(uint16_t aDeviceAddr, uint16_t aControl) {
	CRAY_ASSERT((aControl & ~(Control_Start | Control_Clear)) == 0);
	if ((aControl & Control_Clear) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'CLEAR' recevied. Clearing pending interrupt" << std::endl;
		SetIntActive(false);
	}
	if ((aControl & Control_Start) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'START' recevied with RegA: " << HexPrinter(mRegA,4) << " RegB: " << HexPrinter(mRegB,4) << " RegC: " << HexPrinter(mRegC,4) << std::endl;
		size_t SectorIdx = mTrack * mNumSectors * mNumHeads + mHead * mNumSectors + mStartSector;
		switch (mRegC) {
			// AMPEX commands (DOC)
			case Ampex_Read: { // Read operation
				Create();
				mReads += mSectorCnt * mSectorSize / 1024;
				LogLine_c LogLine = mLogger << setloglevel(LogLevel_IoActivity);
				LogLine << "Reading sector " << DecPrinter(SectorIdx) 
					<< " (SHT-L:" << DecPrinter(mStartSector) << "," << DecPrinter(mHead) << "," << DecPrinter(mTrack) << "," << DecPrinter(mSectorCnt) << ")"
					<< " from file offset " << HexPrinter(SectorIdx * mSectorSize,8) << " to buffer at " << HexPrinter(mRegB,4) << std::endl;
				std::fstream File(mImageFileName, std::ios::in | std::ios::binary);
				File.seekg(SectorIdx * mSectorSize);
				CRAY_ASSERT(!File.fail());
				uint16_t Data;
				LogLine_c TraceLine = mLogger << setloglevel(LogLevel_IoTrace);
				for(size_t i=0;i<mSectorSize*mSectorCnt/sizeof(Data);++i) {
					File.read((char*)(&Data),sizeof(Data));

					if (i % 4 == 0) TraceLine << "Transfered ";
					TraceLine << HexPrinter(Data, 4) << " ";
					if (i % 4 == 3) TraceLine << std::endl;

					Data = SwapBytes(Data);
					mParent.GetDma().WriteWord(mRegB, Data);
					CRAY_ASSERT(!File.fail());
					++mRegB;
				}
				mDone = true;
				mBusy = false;
				mRegA &= ~1;
				SetIntActive(true);
			} break;
			case Ampex_Write: // Write operation
			case Ampex_Format: { // Format operation
				Create();
				mWrites += mSectorCnt * mSectorSize / 1024;
				mLogger << setloglevel(LogLevel_IoActivity) << "Writing sector " << DecPrinter(SectorIdx) 
					<< " (SHT-L:" << DecPrinter(mStartSector) << "," << DecPrinter(mHead) << "," << DecPrinter(mTrack) << "," << DecPrinter(mSectorCnt) << ")"
					<< " to file offset " << HexPrinter(SectorIdx * mSectorSize,8) << " from buffer at " << HexPrinter(mRegB,4) << std::endl;
				std::fstream File;
				File.open(mImageFileName, std::ios::out | std::ios::in | std::ios::binary);
				File.seekp(SectorIdx * mSectorSize);
				CRAY_ASSERT(!File.fail());
				uint16_t Data = 0;
				for(size_t i=0;i<mSectorSize*mSectorCnt/sizeof(Data);++i) {
					if (mRegC == Ampex_Write) {
						Data = mParent.GetDma().ReadWord(mRegB);
						Data = SwapBytes(Data);
					}
					File.write((char*)(&Data),sizeof(Data));
					CRAY_ASSERT(!File.fail());
					++mRegB;
				}
				mDone = true;
				mBusy = false;
				mRegA &= ~1;
				SetIntActive(true);
			} break;
			case Ampex_ReturnToZero: {
				mLogger << setloglevel(LogLevel_IoActivity) << "RETURN TO ZERO recevied with RegA: " << HexPrinter(mRegA, 4) << " RegB: " << HexPrinter(mRegB, 4) << " RegC: " << HexPrinter(mRegC, 4) << std::endl;
				mTrack = 0;
				mHead = 0;
				mStartSector = 0;
				mDone = true;
				mBusy = false;
				mRegA &= ~1;
				SetIntActive(true);
			} break;
			case Ampex_Checksum: {
				mDone = true;
				mBusy = false;
				mRegA &= ~1;
				SetIntActive(true);
			} break;
			case Ampex_StartSeek:
			case Ampex_SelectDrive:
			case Ampex_AttemptCorrection:
				mLogger << setloglevel(LogLevel_Error) << "UNIMPLEMENTED 'START' recevied with RegA: " << HexPrinter(mRegA, 4) << " RegB: " << HexPrinter(mRegB, 4) << " RegC: " << HexPrinter(mRegC, 4) << std::endl;
			break;
			default:
				mLogger << setloglevel(LogLevel_Error) << "UNKNOWN 'START' recevied with RegA: " << HexPrinter(mRegA, 4) << " RegB: " << HexPrinter(mRegB, 4) << " RegC: " << HexPrinter(mRegC, 4) << std::endl;
			break;
		}
	}
}

void IopExpDiskAmpexDM980_c::Tick() {
}

void IopExpDiskAmpexDM980_c::GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const {
	aStatus.put(aLongFormat ? "Cylinder" : "C", mTrack);
	aStatus.put(aLongFormat ? "Head" : "H", mHead);
	aStatus.put(aLongFormat ? "Sector" : "S", mStartSector);
	aStatus.put(aLongFormat ? "Reads" : "R", mReads);
	aStatus.put(aLongFormat ? "Writes" : "W", mWrites);
	//	aStatus.put("res", mMounted);
	//	aStatus.put("fname", mImageFileName);
}


//////////////////////////////////////////////////////////////////////////////////////////
// IopExpDiskCDC9448_96_c
//////////////////////////////////////////////////////////////////////////////////////////
// Must be a CDC9448-96 from this: http://bitsavers.trailing-edge.com/pdf/dilog/30006-1_DQ202A_Sep81.pdf
// It's a 96MB drive, but formatted as a 64MB one (leaving the last few sectors on each track empty)
// Protocol seems to be:
// RegA: 0x100
// RegC: cylinder
// PULSE
// RegA: command (0 for read 0x700 for write)
// RegC: bits 9-5: high order bits of sector; bits 4-0: high order bits of (twos complement of) sector count
// RegC: bits 14-10: head bits 9-5: low order bits of sector; bits 4-0: low order bits of (twos complement of) sector count
// RegB: buffer address
// START

// CDC device function codes(DOA)
const uint16_t CDC_Read          = 00000;
const uint16_t CDC_Recalibrate   = 00200;
const uint16_t CDC_Seek          = 00400;
const uint16_t CDC_StopDisk      = 00600;
const uint16_t CDC_OffsetForward = 01000;
const uint16_t CDC_OffsetReverse = 01200;
const uint16_t CDC_WriteDisable  = 01400;
const uint16_t CDC_ReleaseDrive  = 01600;
const uint16_t CDC_Treoass       = 02000;
const uint16_t CDC_SetAltMode1   = 02200;
const uint16_t CDC_SetAltMode2   = 02400;
const uint16_t CDC_ExamineRam    = 02600;
const uint16_t CDC_DataVerify    = 03000;
const uint16_t CDC_ReadBuffers   = 03200;
const uint16_t CDC_Write         = 03400;
const uint16_t CDC_Format        = 03600;

// CDC accumulator offsets(DOC2)
const uint16_t CDC_CountShiftFactor  = 0;
const uint16_t CDC_SectorShiftFactor = 5;
const uint16_t CDC_HeadShiftFactor   = 10;

IopExpDiskCDC9448_96_c::IopExpDiskCDC9448_96_c(const Configuration_c &aConfig, IopChannelExp_c &aParent) :
	mParent(aParent),
	mBusy(false),
	mDone(true),
	mRegA(0),
	mRegB(0),
	mRegC(0),
	mInterruptActive(false),
	mInterruptEnabled(false),
	mLogger(aConfig, "DK"),
	mHead(0),
	mStartSector(0),
	mSectorCnt(0),
	mTrack(0),
	mChannelIdx(aConfig.get<uint16_t>("ChannelIdx")),
	mReads(0),
	mWrites(0),
	mState(State_Seek)
{
	mLogger.SetParent(mParent.GetLogger());
	mImageFileName = aConfig.get<std::string>("ImageFileName");
	mReadOnly = aConfig.get<bool>("ReadOnly", false);
	mNumHeads = aConfig.get<size_t>("Heads", 5);
	if (mNumHeads == 0) throw InvalidParameter_x("Heads must be greater than 0");
	mNumSectors = aConfig.get<size_t>("Sectors", 35);
	if (mNumSectors == 0) throw InvalidParameter_x("Sectors must be greater than 0");
	mNumTracks = aConfig.get<size_t>("Tracks", 823);
	if (mNumTracks == 0) throw InvalidParameter_x("Tracks must be greater than 0");
	mSectorSize = aConfig.get<size_t>("SectorSize", 4096);
	if (mSectorSize == 0) throw InvalidParameter_x("SectorSize must be greater than 0");
}

void IopExpDiskCDC9448_96_c::SetIntActive(bool aActive) {
	mInterruptActive = aActive;
	mParent.IntStatusChanged();
}

void IopExpDiskCDC9448_96_c::SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) {
	mInterruptEnabled = aInterruptEnabled;
	mParent.IntStatusChanged();
}

void IopExpDiskCDC9448_96_c::SetRegA(uint16_t aDeviceAddr, uint16_t aValue) {
	mRegA = aValue;
	mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegA: " << HexPrinter(mRegA, 4) << std::endl;
}

void IopExpDiskCDC9448_96_c::SetRegB(uint16_t aDeviceAddr, uint16_t aValue) {
	mRegB = aValue;
	mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegB: " << HexPrinter(mRegB, 4) << std::endl;
}
void IopExpDiskCDC9448_96_c::SetRegC(uint16_t aDeviceAddr, uint16_t aValue) {
	mRegC = aValue;
	mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegC: " << HexPrinter(mRegC, 4) << std::endl;
	switch (mState) {
		case State_Seek:
			mTrack = mRegC;
			break;
		case State_GetHighHeadSector:
			mStartSector = (mRegC >> 5) & 32;
			mSectorCnt = mRegC & 32;
			mState = State_GetLowHeadSector;
			break;
		case State_GetLowHeadSector:
			mStartSector |= (mRegC >> 5) & 31;
			mSectorCnt |= (mRegC >> 0) & 31;
			mHead = (mRegC >> 10) & 31;
			break;
	}
}

uint16_t IopExpDiskCDC9448_96_c::GetStatus(uint16_t aDeviceAddr) {
	uint16_t RetVal = 0;
	RetVal |= Status_DmaEnabled;
	if (mInterruptEnabled) RetVal |= Status_IntEnabled;
	if (mInterruptActive) RetVal |= Status_Int;
	if (mBusy) RetVal |= Status_DeviceBusy;
	if (mDone) RetVal |= Status_DeviceDone;
	return RetVal;
}


void IopExpDiskCDC9448_96_c::Create() {
	if (boost::filesystem::exists(mImageFileName)) return;
	std::fstream File(mImageFileName, std::ios::out | std::ios::binary);
	std::vector<char> Sector(mSectorSize);
	for (size_t i = 0; i < mNumSectors*mNumHeads*mNumTracks; ++i) File.write(&(Sector[0]), mSectorSize);
	CRAY_ASSERT(File.good());
}

void IopExpDiskCDC9448_96_c::Control(uint16_t aDeviceAddr, uint16_t aControl) {
	if ((aControl & Control_Clear) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'CLEAR' recevied. Clearing pending interrupt" << std::endl;
		SetIntActive(false);
		mState = State_Seek;
	}
	if ((aControl & Control_Pulse) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'PULSE' recevied. Switching mode" << std::endl;
		mState = State_GetHighHeadSector;
	}
	if ((aControl & Control_Start) != 0) {
		mLogger << setloglevel(LogLevel_IoActivity) << "'START' recevied with RegA: " << HexPrinter(mRegA, 4) << " RegB: " << HexPrinter(mRegB, 4) << " RegC: " << HexPrinter(mRegC, 4) << std::endl;
		CRAY_ASSERT(mState == State_GetLowHeadSector);
		mState = State_Seek;
		size_t SectorIdx = mTrack * mNumSectors * mNumHeads + mHead * mNumSectors + mStartSector;
		switch (mRegA) {
		case CDC_Read: { // Read operation
			Create();
			mLogger << setloglevel(LogLevel_IoActivity) << "mSectorCnt before: " << HexPrinter(mSectorCnt) << std::endl;
			// Adjust sector count: it's a 6-bit 2-s complement value...
			if ((mSectorCnt & 32) != 0) mSectorCnt |= ~63;
			mSectorCnt = -mSectorCnt;
			mLogger << setloglevel(LogLevel_IoActivity) << "mSectorCnt after: " << HexPrinter(mSectorCnt) << std::endl;
			mReads += mSectorCnt * mSectorSize / 1024;
			LogLine_c LogLine = mLogger << setloglevel(LogLevel_IoActivity);
			LogLine << "Reading sector " << DecPrinter(SectorIdx)
				<< " (SHT-L:" << DecPrinter(mStartSector) << "," << DecPrinter(mHead) << "," << DecPrinter(mTrack) << "," << DecPrinter(mSectorCnt) << ")"
				<< " from file offset " << HexPrinter(SectorIdx * mSectorSize, 8) << " to buffer at " << HexPrinter(mRegB, 4) << std::endl;
			std::fstream File(mImageFileName, std::ios::in | std::ios::binary);
			File.seekg(SectorIdx * mSectorSize);
			CRAY_ASSERT(!File.fail());
			uint16_t Data;
			LogLine_c TraceLine = mLogger << setloglevel(LogLevel_IoTrace);
			for (size_t i = 0; i<mSectorSize*mSectorCnt / sizeof(Data); ++i) {
				File.read((char*)(&Data), sizeof(Data));

				if (i % 4 == 0) TraceLine << "Transfered ";
				TraceLine << HexPrinter(Data, 4) << " ";
				if (i % 4 == 3) TraceLine << std::endl;

				Data = SwapBytes(Data);
				mParent.GetDma().WriteWord(mRegB, Data);
				CRAY_ASSERT(!File.fail());
				++mRegB;
			}
			mDone = true;
			mBusy = false;
			mRegA &= ~0x8000;
			SetIntActive(true);
		} break;
		case CDC_Format:
		case CDC_Write: { // Write operation
			Create();
			mWrites += mSectorCnt * mSectorSize / 1024;
			mLogger << setloglevel(LogLevel_IoActivity) << "Writing sector " << DecPrinter(SectorIdx)
				<< " (SHT-L:" << DecPrinter(mStartSector) << "," << DecPrinter(mHead) << "," << DecPrinter(mTrack) << "," << DecPrinter(mSectorCnt) << ")"
				<< " to file offset " << HexPrinter(SectorIdx * mSectorSize, 8) << " from buffer at " << HexPrinter(mRegB, 4) << std::endl;
			std::fstream File;
			File.open(mImageFileName, std::ios::out | std::ios::in | std::ios::binary);
			File.seekp(SectorIdx * mSectorSize);
			CRAY_ASSERT(!File.fail());
			uint16_t Data = 0;
			for (size_t i = 0; i<mSectorSize*mSectorCnt / sizeof(Data); ++i) {
				if (mRegA == CDC_Write) {
					Data = mParent.GetDma().ReadWord(mRegB);
					Data = SwapBytes(Data);
				}
				File.write((char*)(&Data), sizeof(Data));
				CRAY_ASSERT(!File.fail());
				++mRegB;
			}
			mDone = true;
			mBusy = false;
			mRegA &= ~0x8000;
			SetIntActive(true);
		} break;
		default:
			CRAY_ASSERT(false);
			break;
		}
	}
}

void IopExpDiskCDC9448_96_c::Tick() {
}

void IopExpDiskCDC9448_96_c::GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const {
	aStatus.put(aLongFormat ? "Cylinder" : "C", mTrack);
	aStatus.put(aLongFormat ? "Head" : "H", mHead);
	aStatus.put(aLongFormat ? "Sector" : "S", mStartSector);
	aStatus.put(aLongFormat ? "Reads" : "R", mReads);
	aStatus.put(aLongFormat ? "Writes" : "W", mWrites);
	//	aStatus.put("res", mMounted);
	//	aStatus.put("fname", mImageFileName);
}


//////////////////////////////////////////////////////////////////////////////////////////
// IopChannelExp_c
//////////////////////////////////////////////////////////////////////////////////////////
void IopChannelExp_c::Setup(const Configuration_c &aConfig) {
	mDevices.clear();
	mDeviceIntMapping.clear();
	mDevices.resize(aConfig.get<size_t>("DeviceCount", 64));
	mDeviceIntMapping.resize(64);

	for(const auto &DeviceConfig: aConfig.get_child_safe("Devices")) {
		std::string DeviceType = DeviceConfig.first;
		std::shared_ptr<IopExpDevice_i> Device;
		const Configuration_c &DeviceParams = DeviceConfig.second;

		uint8_t DeviceInt = DeviceParams.get<uint8_t>("Interrupt");
		if (DeviceInt >= 16) {
			throw InvalidParameter_x(boost::format("device interrupt %1% too large") % DeviceInt);
		}

		// Create device
		if (DeviceType == "Tape") {
			// Create a tape device
			Device = std::make_shared<IopExpTape_c>(DeviceParams, *this);
		} else if (DeviceType == "AmpexDM980Disk") {
			// Create a tape device
			Device = std::make_shared<IopExpDiskAmpexDM980_c>(DeviceParams, *this);
		} else if (DeviceType == "CDC9448Disk") {
			// Create a tape device
			Device = std::make_shared<IopExpDiskCDC9448_96_c>(DeviceParams, *this);
		} else if (DeviceType == "Dummy") {
			// Create a dummy device
			std::string DeviceName = DeviceParams.get<std::string>("Name");
			Device = std::make_shared<IopExpDummyDevice_c>(DeviceParams, *this, DeviceName);
		} else if (DeviceType == "Printer") {
			// Create a printer device
			Device = std::make_shared<IopExpPrinterDevice_c>(DeviceParams, *this);
		} else if (DeviceType == "Clock") {
			// Create a clock device
			Device = std::make_shared<IopExpClockDevice_c>(DeviceParams, *this);
		} else {
			throw InvalidParameter_x(boost::format("Unknown device type: %1%") % DeviceType);
		}

		// Add device to all channels (these are smart pointers, so adding a device multiple times is OK)
		for(size_t i=0;i<Device->GetChannelCnt();++i) {
			uint16_t ChannelIdx = Device->GetChannelIdx(i);
			if (mDevices[ChannelIdx] != nullptr) {
				throw InvalidParameter_x(boost::format("expander channel index %1% is already in use") % ChannelIdx);
			}
			if (ChannelIdx >= mDevices.size()) {
				throw InvalidParameter_x(boost::format("expander channel index %1% is too large") % ChannelIdx);
			}
			mDevices[ChannelIdx] = Device;
		}

		// Make sure that noone used up our interrupt already
		for(size_t i=0;i<mDeviceIntMapping.size();++i) {
			if (mDeviceIntMapping[i] == DeviceInt && mDevices[i] != nullptr) {
				throw InvalidParameter_x(boost::format("device interrupt %1% is already in use") % DeviceInt);
			}
		}

		// Register our interrupt for all channels
		for(size_t i=0;i<Device->GetChannelCnt();++i) {
			uint16_t ChannelIdx = Device->GetChannelIdx(i);
			mDeviceIntMapping[ChannelIdx] = DeviceInt;
		}
	}
	// Fill-in unused devices with dummies if asked for
	bool UseDummyDevices = aConfig.get<bool>("UseDummyDevices", false);
	if (UseDummyDevices) {
		for(size_t Device=0;Device<mDevices.size();++Device) {
			std::stringstream DeviceName;
			DeviceName << "DUMMY" << OctPrinter(Device,3);
			if (mDevices[Device] == nullptr) {
				mDevices[Device] = std::make_shared<IopExpDummyDevice_c>(aConfig.get_child_safe("DefaultDummy"), *this, DeviceName.str(), uint16_t(Device));
			}
		}
	}
	for (auto &Device : mDevices) {
		if (Device != nullptr && Device->NeedsTick()) {
			// Make sure the device doesn't already exist in the list.
			bool Found = false;
			for (auto &TickedDevice : mTickedDevices) { if (TickedDevice == Device.get()) { Found = true; break; } }
			if (!Found) mTickedDevices.emplace_back(Device.get());
		}
	}
	IntStatusChanged();
}

IopInt_t IopChannelExp_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	uint16_t Status = 0;
	if ((aFunction != 005 && mActiveChannel < 0) || (mActiveChannel >= 0 && mActiveChannel < (int)mDevices.size() && mDevices[mActiveChannel] == nullptr)) {
		mLogger << setloglevel(LogLevel_Warning) << "Issuing command " << OctPrinter(aFunction) << " to non-existent channel: " << OctPrinter(mActiveChannel) << std::endl;
	}
	switch (aFunction) {
		case 000:
			mBusy = false;
			mDone = false;
			mInterruptEnabled = false;
			//TODO: this should disable DMAs too, but there's no function to enabled it again?!!!
		return 0;
		case 001:
			CRAY_ASSERT(mActiveChannel != -1);
			mOperation = Operation_ReadA;
			mBusy = false;
			mDone = true;
		return 0;
		case 002:
			CRAY_ASSERT(mActiveChannel != -1);
			mOperation = Operation_ReadB;
			mBusy = false;
			mDone = true;
		return 0;
		case 003:
			CRAY_ASSERT(mActiveChannel != -1);
			mOperation = Operation_ReadC;
			mBusy = false;
			mDone = true;
		return 0;
		case 004:
			CRAY_ASSERT(mActiveChannel != -1);
			mOperation = Operation_ReadInterrupt;
			mBusy = false;
			mDone = true;
		return 0;
		case 005:
			mActiveChannel = aData & 63;
		return 0;
		case 006:
			mLogger << setloglevel(LogLevel_IoActivity) << "Setting channel interrupt mask to " << HexPrinter(aData) << std::endl;
			if (aData == 0 || aData == 0xffff) {
				for(size_t Device=0;Device<mDevices.size();++Device) {
					if (mDevices[Device] != nullptr) {
						mDevices[Device]->SetIntEnabled(mActiveChannel, aData == 0);
					}
				}
			} else {
				for(size_t Int = 0;Int<16;++Int) {
					for(size_t Device=0;Device<mDevices.size();++Device) {
						if (mDevices[Device] != nullptr && mDeviceIntMapping[Device] == Int) {
							mDevices[Device]->SetIntEnabled(mActiveChannel, (aData & (1 << Int)) == 0);
						}
					}
				}
			}
		return 0;
		case 007:
			mInterruptEnabled =       (aData & 1) != 0;
			mDeviceInterruptEnabled = (aData & 2) != 0;
			mLogger << setloglevel(LogLevel_IoActivity) << "Interrupts " << (mInterruptEnabled?"enabled":"disabled") << ", device interrupts " << (mDeviceInterruptEnabled?"enabled":"disabled") << std::endl;
		return 0;
		case 010:
			CRAY_ASSERT(mActiveChannel != -1);
			if (mDevices[mActiveChannel] == nullptr) {
				std::cout << "operation 010 to non-existent EXP channel: " << OctPrinter(mActiveChannel) << std::endl;
				mParent.GetLogger().SetDisplayLogLevel(LogLevel_All);
			}
			if (mDevices[mActiveChannel] != nullptr) {
				switch (mOperation) {
					case Operation_ReadA: return mDevices[mActiveChannel]->GetRegA(mActiveChannel);
					case Operation_ReadB: return mDevices[mActiveChannel]->GetRegB(mActiveChannel);
					case Operation_ReadC: return mDevices[mActiveChannel]->GetRegC(mActiveChannel);
					default: CRAY_ASSERT(false);
				}
			} else {
				return 0;
			}
		return 0;
		case 011:
			CRAY_ASSERT(mActiveChannel != -1);
			mLogger << setloglevel(LogLevel_IoActivity) << "Querying status with operation code: " << DecPrinter(mOperation) << std::endl;
			Status = 0;
			switch (mOperation) {
				default: // I bet this is a bug in XCLOCK, but it seems it doesn't request for the status, it just queries it...
				case Operation_ReadInterrupt:
					if (mDevices[mActiveChannel] != nullptr) Status = mDevices[mActiveChannel]->GetStatus(mActiveChannel);
					if (mInterruptEnabled) Status |= Status_ExpanderIntEnabled;
					if (mBusy) Status |= Status_ExpanderBusy;
					if (mDone) Status |= Status_ExpanderDone;
					// Check for the highest priority device with an interrupt enabled
					for(uint16_t i=0;i<uint16_t(mDevices.size());++i) {
						if (mDevices[i] != nullptr && (mDevices[i]->GetStatus(i) & IopExpDevice_i::Status_Int) != 0) {
							Status |= i;
							break;
						}
					}
					mLogger << setloglevel(LogLevel_IoActivity) << "Returning status 1: " << HexPrinter(Status) << " with active channel: " << OctPrinter(mActiveChannel) << std::endl;
				return Status;
//				default: CRAY_ASSERT(false);
			}
		return 0;
		case 013:
			CRAY_ASSERT(mActiveChannel != -1);
			Status = 0;
			if (mDevices[mActiveChannel] != nullptr) Status = mDevices[mActiveChannel]->GetStatus(mActiveChannel);
			if (mInterruptEnabled) Status |= Status_ExpanderIntEnabled;
			if (mBusy) Status |= Status_ExpanderBusy;
			if (mDone) Status |= Status_ExpanderDone;
			Status |= mActiveChannel;
			mLogger << setloglevel(LogLevel_IoActivity) << "Returning status 2: " << HexPrinter(Status) << " with active channel: " << OctPrinter(mActiveChannel) << std::endl;
		return Status;
		case 014:
			CRAY_ASSERT(mActiveChannel != -1);
			if (mDevices[mActiveChannel] == nullptr) std::cout << "operation 014 to non-existent EXP channel: " << OctPrinter(mActiveChannel) << std::endl;
			//CRAY_ASSERT(mDevices[mActiveChannel] != nullptr);
			if (mDevices[mActiveChannel] != nullptr) mDevices[mActiveChannel]->SetRegA(mActiveChannel, aData);
			mBusy = false;
			mDone = true;
		return 0;
		case 015:
			CRAY_ASSERT(mActiveChannel != -1);
			if (mDevices[mActiveChannel] == nullptr) std::cout << "operation 015 to non-existent EXP channel: " << OctPrinter(mActiveChannel) << std::endl;
			//CRAY_ASSERT(mDevices[mActiveChannel] != nullptr);
			if (mDevices[mActiveChannel] != nullptr) mDevices[mActiveChannel]->SetRegB(mActiveChannel, aData);
			mBusy = false;
			mDone = true;
		return 0;
		case 016:
			CRAY_ASSERT(mActiveChannel != -1);
			if (mDevices[mActiveChannel] == nullptr) std::cout << "operation 016 to non-existent EXP channel: " << OctPrinter(mActiveChannel) << std::endl;
			//CRAY_ASSERT(mDevices[mActiveChannel] != nullptr);
			if (mDevices[mActiveChannel] != nullptr) mDevices[mActiveChannel]->SetRegC(mActiveChannel, aData);
			mBusy = false;
			mDone = true;
		return 0;
		case 017:
			CRAY_ASSERT(mActiveChannel != -1);
			if (mDevices[mActiveChannel] == nullptr) std::cout << "operation 017 to non-existent EXP channel: " << OctPrinter(mActiveChannel) << std::endl;
			//CRAY_ASSERT(mDevices[mActiveChannel] != nullptr);
			if (mDevices[mActiveChannel] != nullptr) mDevices[mActiveChannel]->Control(mActiveChannel, aData);
			mBusy = false;
			mDone = true;
		return 0;
		default:
			mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

void IopChannelExp_c::IntStatusChanged() {
	mDeviceInterruptPending = false;
	for (uint16_t Device = 0; Device<uint16_t(mDevices.size()); ++Device) {
		if (mDevices[Device] != nullptr) {
			uint16_t DevStatus = mDevices[Device]->GetStatus(Device);
			//mLogger << setloglevel(LogLevel_IoActivity) << "Device " << OctPrinter(Device) << " returning status: " << HexPrinter(DevStatus) << std::endl;
			if (((DevStatus & IopExpDevice_i::Status_IntEnabled) != 0) && ((DevStatus & IopExpDevice_i::Status_Int) != 0)) {
				mDeviceInterruptPending = true;
				break;
			}
		}
	}
}

IopBit_t IopChannelExp_c::GetInterrupt() {
	if (mInterruptEnabled && mDone) return 1;
/*	if (mDeviceInterruptEnabled) {
		for(uint16_t Device=0;Device<uint16_t(mDevices.size());++Device) {
			if (mDevices[Device] != nullptr) {
				uint16_t DevStatus = mDevices[Device]->GetStatus(Device);
				//mLogger << setloglevel(LogLevel_IoActivity) << "Device " << OctPrinter(Device) << " returning status: " << HexPrinter(DevStatus) << std::endl;
				if (((DevStatus & IopExpDevice_i::Status_IntEnabled) != 0) && ((DevStatus & IopExpDevice_i::Status_Int) != 0)) {
					return 1;
				}
			}
		}
	}*/
	if (mDeviceInterruptEnabled && mDeviceInterruptPending) return 1;
	return 0;
}

void IopChannelExp_c::Tick() {
/*	for(uint16_t Device=0;Device<uint16_t(mDevices.size());++Device) {
		if (mDevices[Device] != nullptr) {
			mDevices[Device]->Tick(Device);
		}
	}*/
	for (auto &Device : mTickedDevices) {
		Device->Tick();
	}
}

void IopChannelExp_c::GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const {
	for (auto &Device : mDevices) {
		if (Device != nullptr && Device->GetType() == aFilter) {
			StatusReport_c Status;
			Device->GetStatus(Status, aFilter, aLongFormat);
			aStatus.put_child(Device->GetName(), Status);
		}
	}
}

void IopChannelExp_c::RegisterCommands(CommandHooks_t &aHooks) {
	for (auto &Device : mDevices) {
		if (Device != nullptr) {
			Device->RegisterCommands(aHooks);
		}
	}
}
