#ifndef __IOP_EXPANDER_H__
#define __IOP_EXPANDER_H__

#include "cray_iop.h"
#include <fstream>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/variant.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/timer/timer.hpp>
#include "cray_channels.h"
#ifndef _WIN32
#include <unistd.h>
#endif //_WIN32
#include "config_file.h"
#include "poked_tap_file.h"
#include "ui.h"

class IopExpDevice_i {
public:
	// This is a bit-field enum
	enum Controls_e {
		Control_Start = 1,
		Control_Clear = 2,
		Control_Pulse = 4,
		Control_Reset = 8
	};
	// This is a bit-field enum (there are others too, but they come from the Exp, not frome the device
	enum Status_e {
		Status_DmaEnabled            = 1 << 7,
		Status_IntEnabled            = 1 << 9,
		Status_DelayedFunctionActive = 1 << 10,
		Status_Int                   = 1 << 13,
		Status_DeviceBusy            = 1 << 14,
		Status_DeviceDone            = 1 << 15
	};

	virtual ~IopExpDevice_i() {}
	virtual void SetRegA(uint16_t aDeviceAddr, uint16_t aValue) = 0;
	virtual void SetRegB(uint16_t aDeviceAddr, uint16_t aValue) = 0;
	virtual void SetRegC(uint16_t aDeviceAddr, uint16_t aValue) = 0;
	virtual void SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) = 0;
	virtual uint16_t GetRegA(uint16_t aDeviceAddr) = 0;
	virtual uint16_t GetRegB(uint16_t aDeviceAddr) = 0;
	virtual uint16_t GetRegC(uint16_t aDeviceAddr) = 0;
	virtual uint16_t GetStatus(uint16_t aDeviceAddr) = 0;
	virtual void Control(uint16_t aDeviceAddr, uint16_t aControl) = 0;
	virtual void Tick() = 0;
	virtual bool NeedsTick() = 0;
	virtual size_t GetChannelCnt() const = 0;
	virtual uint16_t GetChannelIdx(size_t aIdx) const = 0;
	virtual PeripheralType_e GetType() const = 0;
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const = 0;
	virtual void RegisterCommands(CommandHooks_t &aHooks) = 0;
	virtual std::string GetName() const = 0;
	// Whenever interrupt status or enable changes, the device is expected to call the parents method:
	// IntStatusChanged with the proper new values
};

// A helper class that emulates a tape through a directory in the file-system
// Files within the directory are numbered: 1.dat, 2.dat etc.
// Since many tape drivers didn't support precise writes (it was not possible to overwrite parts of a file),
// This class deletes all files after the current one when a write operation is initiated.
// So be careful when you enable writes to tape!!!

// Finally some breakthrough: the problem with the EOF detection on the tape was that we have to set 0x8000 with the EOF status bit.
// This change also contains some (rather hacky) workaround to make seeks around EOF markers work. The current implementation is a crude joke at best, but
// at least it's functional. Essentially what happens:
// 
// 1. Reads or seeks bump into the EOF marker: return the number of WORDs successfully recovered with EOF
// 2. Seek forward 1 is issued. This should NOT return EOF
// 3. Seek backward 1 is issued. This should NOT return EOF (I think)
// 4. Read 2048 words is issued: this should return 0 bytes with EOF (I think)
// 5. Seek backward 1 is issued. This should position us back to the beginning of the file
// 6. Read issued: now we should be able to read from the next file.
// 
// Step 2 and 3 might get skipped.
// 
// So, it seems we have a few things going on:
// - Seek forward by 1 followed by a Seek backward by 1 should be a NO-OP, no matter what.
// - Reading at an EOF marker should return 0 WORDs, but move the head to the next token (beginning of next file)
// 
// This BTW is not even close to what the current code is doing. It has a lot of special-casing nightmare code to make something that behaves similarly. Should be cleaned up
class TapeImage_c {
public:
	TapeImage_c(const boost::filesystem::path &aPath, const Configuration_c &aConfig, CLogger_c &aLogger);
	~TapeImage_c();

	void SeekToNextFile();
	void SeekToPrevFile();
	void SeekToFile(int aFileIdx);

	template <typename tElemType> void Write(const std::vector<tElemType> &aData) {
		Write(&aData[0],aData.size() * sizeof(aData[0]));
	}
	void Write(const void *aData, size_t aDataSize);
	void WriteEOF();

	enum ReadStatus_e {
		ReadStatus_OK,
		ReadStatus_EOF,
		ReadStatus_EOT,
		ReadStatus_BOT
	};
	template <typename tElemType> ReadStatus_e Read(std::vector<tElemType> &aData) { // vector size must be set up before call. Resizes vector to actual number of words read
		size_t NewSize = aData.size() * sizeof(aData[0]);
		ReadStatus_e RetVal = Read(&aData[0],NewSize);
		aData.resize(NewSize / sizeof(aData[0]));
		return RetVal;
	}

	ReadStatus_e Read(void *aData, size_t &aDataSize);
	ReadStatus_e SeekForward(size_t &aDataSize);
	ReadStatus_e SeekBackward(size_t &aDataSize);

	bool IsEOF() const;
	bool IsBOT() const;
	bool IsEOT() const;
	bool IsReadOnly() const { return mReadOnly; }
	std::streamoff GetFileOffset();
	const boost::filesystem::path &GetImagePath() const { return mImagePath; }
	int GetCurrentFileIdx() const { return mCurrentFileIdx; }
protected:
	CLogger_c &mLogger;

	boost::filesystem::path MakeFileName();
	void ProcessPokeMap(void *aData, size_t aDataSize, size_t aBaseOffset);

	bool mReadOnly;
	boost::filesystem::path mImagePath;
	typedef std::map<size_t,uint8_t> PokeMap_t;
	std::vector<PokeMap_t> mPokes;
	std::fstream mFile;
	int mCurrentFileIdx;
	std::fstream::pos_type mCurrentFileSize;
	bool mWriting;
};

class IopExpTape_c: public IopExpDevice_i {
public:
	IopExpTape_c(const Configuration_c &aConfig, class IopChannelExp_c &aParent);
	virtual ~IopExpTape_c() override {}
	virtual void SetRegA(uint16_t aDeviceAddr, uint16_t aValue) override { mRegA = aValue; mRegAStat = 0; }
	virtual void SetRegB(uint16_t aDeviceAddr, uint16_t aValue) override { mRegB = aValue; }
	virtual void SetRegC(uint16_t aDeviceAddr, uint16_t aValue) override { mRegC = aValue; }
	virtual void SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) override;
	virtual uint16_t GetRegA(uint16_t aDeviceAddr) override;
	virtual uint16_t GetRegB(uint16_t aDeviceAddr) override { return mRegB; }
	virtual uint16_t GetRegC(uint16_t aDeviceAddr) override { return mRegC; }
	virtual uint16_t GetStatus(uint16_t aDeviceAddr) override;
	virtual void Control(uint16_t aDeviceAddr, uint16_t aControl) override;
	virtual void Tick() override;
	virtual bool NeedsTick() override { return false; }
	virtual size_t GetChannelCnt() const override { return 1; }
	virtual uint16_t GetChannelIdx(size_t aIdx) const override { CRAY_ASSERT(aIdx == 0); return mChannelIdx; }
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Tape; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override;
	virtual void RegisterCommands(CommandHooks_t &aHooks) override;
	virtual std::string GetName() const override { return "XMT"; }
	void MountTape(const std::string &aTapeFileName);
protected:
	void SetIntActive(bool aActive);

	uint16_t mRegA;
	uint16_t mRegAStat;
	uint16_t mRegB;
	uint16_t mRegC;
	PokedTapFile_c mTapeImage;
	int mCurrentTapeImage; // set to -1 to indicate no current image file
	uint16_t mChannelIdx;

	bool mBusy;
	bool mDone;
	bool mInterruptEnabled;
	bool mInterruptActive;
	class IopChannelExp_c &mParent;
	CLogger_c mLogger;
	bool mAtStartOfTape;
	bool mAtEndOfTape;
	bool mIsReadOnly;

// Status bits - returned in mRegA
// The tape drive interface is documented in "015 - 000021 - 00_Prog_Ref_Peripherals_Nov74.pdf" at page 126
	enum Status_e {
		Status_Ready                 = 0x0001, //   The selected transport is ready
		Status_WordCountError        = 0x0002, // ? Record contains odd number of bytes
		Status_ReadOnly              = 0x0004, //   Tape is write-protected
		Status_FirstChar             = 0x0008, //   Used for maintenance
		Status_SendClock             = 0x0010, //   Used for maintenance
		Status_BadTape               = 0x0020, // * The section of tape just processed is of poor quality.
		Status_9Track                = 0x0040, //   9-track or PE transport
		Status_BeginOfTape           = 0x0080, // * Beginning of tape mark
		Status_EndOfFile             = 0x0100, // * End of file mark encoundered during read, space or after writing an EOF mark
		Status_EndOfTape             = 0x0200, // * End of tape. Space reverse or rewind resets this
		Status_DataParityError       = 0x0400, // * parity bit or (for NRZI tapes) record LPCC doesn't match
		Status_HighDensity           = 0x0800, //   800 or 1600 (PE) bpi transport
		Status_Illegal               = 0x1000, // * A start command was issued while 1) Transport was not ready 2) Space-reverse command at BOT 3) Write, Write EOF or Erase while read-only
		Status_Rewinding             = 0x2000, //   The selected transport is currently rewiding
		Status_DataLate              = 0x4000, // * The data channel failed to respond in tome oto a data channel request
		Status_Error                 = 0x8000  //   Set if any of the '*' marked above are set
	};

	enum Commands_e {
		Command_ReadForward   = 0, // RegB: set to destination buffer  RegC: set to TransferSize*-1
		Command_Rewind        = 1, // RegB: not set                    RegC: not set
		Command_Reserved      = 2,
		Command_SpaceForward  = 3, // RegB: not set                    RegC: set to NumRec*-1
		Command_SpaceBackward = 4, // RegB: not set                    RegC: set to NumRec*-1
		Command_WriteForward  = 5, // RegB: set to destination buffer  RegC: set to TransferSize*-1
		Command_WriteEOF      = 6, // RegB: not set                    RegC: not set
		Command_Erase         = 7, // RegB: not set                    RegC: not set
	};

	void FinalizeControl(uint16_t aStatus);

	size_t mReads;
	size_t mWrites;
};

class IopExpDummyDevice_c: public IopExpDevice_i {
public:
	IopExpDummyDevice_c(const Configuration_c &aConfig, class IopChannelExp_c &aParent, const std::string &aLogName);
	IopExpDummyDevice_c(const Configuration_c &aConfig, class IopChannelExp_c &aParent, const std::string &aLogName, uint16_t aChannelIdx);
	virtual ~IopExpDummyDevice_c() override {}
	virtual void SetRegA(uint16_t aDeviceAddr, uint16_t aValue) override { mRegA = aValue; mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegA: " << HexPrinter(mRegA,4) << std::endl; }
	virtual void SetRegB(uint16_t aDeviceAddr, uint16_t aValue) override { mRegB = aValue; mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegB: " << HexPrinter(mRegB,4) << std::endl; }
	virtual void SetRegC(uint16_t aDeviceAddr ,uint16_t aValue) override { mRegC = aValue; mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegC: " << HexPrinter(mRegC,4) << std::endl; }
	virtual void SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) override;
	virtual uint16_t GetRegA(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegA: " << HexPrinter(mRegA,4) << std::endl; return mRegA; }
	virtual uint16_t GetRegB(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegB: " << HexPrinter(mRegB,4) << std::endl; return mRegB; }
	virtual uint16_t GetRegC(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegC: " << HexPrinter(mRegC,4) << std::endl; return mRegC; }
	virtual uint16_t GetStatus(uint16_t aDeviceAddr) override;
	virtual void Control(uint16_t aDeviceAddr, uint16_t aControl) override;
	virtual void Tick() override {}
	virtual bool NeedsTick() override { return false; }
	virtual size_t GetChannelCnt() const override { return 1; }
	virtual uint16_t GetChannelIdx(size_t aIdx) const override { CRAY_ASSERT(aIdx == 0); return mChannelIdx; }
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Other; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override {}
	virtual void RegisterCommands(CommandHooks_t &aHooks) override {}
	virtual std::string GetName() const override { return "DUMMY"; }
protected:
	void SetIntActive(bool aActive);

	uint16_t mRegA;
	uint16_t mRegB;
	uint16_t mRegC;
	bool mBusy;
	bool mDone;
	bool mInterruptEnabled;
	bool mInterruptActive;
	class IopChannelExp_c &mParent;
	CLogger_c mLogger;
	uint16_t mChannelIdx;
};

class IopExpDiskAmpexDM980_c: public IopExpDevice_i {
public:
	IopExpDiskAmpexDM980_c(const Configuration_c &aConfig, class IopChannelExp_c &aParent);
	virtual ~IopExpDiskAmpexDM980_c() override {}
	virtual void SetRegA(uint16_t aDeviceAddr, uint16_t aValue) override;
	virtual void SetRegB(uint16_t aDeviceAddr, uint16_t aValue) override;
	virtual void SetRegC(uint16_t aDeviceAddr, uint16_t aValue) override;
	virtual void SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) override;
	virtual uint16_t GetRegA(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegA: " << HexPrinter(mRegA,4) << std::endl; return mRegA; }
	virtual uint16_t GetRegB(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegB: " << HexPrinter(mRegB,4) << std::endl; return mRegB; }
	virtual uint16_t GetRegC(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegC: " << HexPrinter(mRegC,4) << std::endl; return mRegC; }
	virtual uint16_t GetStatus(uint16_t aDeviceAddr) override;
	virtual void Control(uint16_t aDeviceAddr, uint16_t aControl) override;
	virtual void Tick() override;
	virtual bool NeedsTick() override { return false; }
	virtual size_t GetChannelCnt() const override { return 1; }
	virtual uint16_t GetChannelIdx(size_t aIdx) const override { CRAY_ASSERT(aIdx == 0); return mChannelIdx; }
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Disk; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override;
	virtual void RegisterCommands(CommandHooks_t &aHooks) override {}
	virtual std::string GetName() const override { return "XDK"; }
protected:
	void SetIntActive(bool aActive);
	void Create();

	uint16_t mRegA;
	uint16_t mRegB;
	uint16_t mRegC;
	struct TapeImage_s {
		std::string ImageFileName;
		std::map<size_t,IopInt_t> Pokes;
		bool ReadOnly;
	};
	// Configuration parameters
	bool mReadOnly;
	std::string mImageFileName;
	size_t mNumHeads;
	size_t mNumSectors;
	size_t mNumTracks;
	size_t mSectorSize;
	// Operational parameters
/*	size_t mCurrentHead;
	size_t mCurrentHeadMul;
	size_t mCurrentSector;
	size_t mCurrentTrack;*/
	size_t mTrack;
	size_t mHead;
	size_t mStartSector;
	size_t mSectorCnt;
	uint16_t mChannelIdx;

	bool mBusy;
	bool mDone;
	bool mInterruptEnabled;
	bool mInterruptActive;
	class IopChannelExp_c &mParent;
	CLogger_c mLogger;

	size_t mReads;
	size_t mWrites;
};

class IopExpDiskCDC9448_96_c : public IopExpDevice_i {
public:
	IopExpDiskCDC9448_96_c(const Configuration_c &aConfig, class IopChannelExp_c &aParent);
	virtual ~IopExpDiskCDC9448_96_c() override {}
	virtual void SetRegA(uint16_t aDeviceAddr, uint16_t aValue) override;
	virtual void SetRegB(uint16_t aDeviceAddr, uint16_t aValue) override;
	virtual void SetRegC(uint16_t aDeviceAddr, uint16_t aValue) override;
	virtual void SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) override;
	virtual uint16_t GetRegA(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegA: " << HexPrinter(mRegA, 4) << std::endl; return mRegA; }
	virtual uint16_t GetRegB(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegB: " << HexPrinter(mRegB, 4) << std::endl; return mRegB; }
	virtual uint16_t GetRegC(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegC: " << HexPrinter(mRegC, 4) << std::endl; return mRegC; }
	virtual uint16_t GetStatus(uint16_t aDeviceAddr) override;
	virtual void Control(uint16_t aDeviceAddr, uint16_t aControl) override;
	virtual void Tick() override;
	virtual bool NeedsTick() override { return false; }
	virtual size_t GetChannelCnt() const override { return 1; }
	virtual uint16_t GetChannelIdx(size_t aIdx) const override { CRAY_ASSERT(aIdx == 0); return mChannelIdx; }
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Disk; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override;
	virtual void RegisterCommands(CommandHooks_t &aHooks) override {}
	virtual std::string GetName() const override { return "XDK"; }
protected:
	void SetIntActive(bool aActive);
	void Create();

	uint16_t mRegA;
	uint16_t mRegB;
	uint16_t mRegC;
	struct TapeImage_s {
		std::string ImageFileName;
		std::map<size_t, IopInt_t> Pokes;
		bool ReadOnly;
	};
	// Configuration parameters
	bool mReadOnly;
	std::string mImageFileName;
	size_t mNumHeads;
	size_t mNumSectors;
	size_t mNumTracks;
	size_t mSectorSize;
	// Operational parameters
	size_t mTrack;
	size_t mHead;
	size_t mStartSector;
	int32_t mSectorCnt;
	uint16_t mChannelIdx;

	bool mBusy;
	bool mDone;
	bool mInterruptEnabled;
	bool mInterruptActive;
	class IopChannelExp_c &mParent;
	CLogger_c mLogger;

	size_t mReads;
	size_t mWrites;

	enum States_e {
		State_Seek,
		State_GetHighHeadSector,
		State_GetLowHeadSector
	};
	States_e mState;
};

class IopExpPrinterDevice_c: public IopExpDevice_i {
public:
	IopExpPrinterDevice_c(const Configuration_c &aConfig, class IopChannelExp_c &aParent);
	virtual ~IopExpPrinterDevice_c() override {}
	virtual void SetRegA(uint16_t aDeviceAddr, uint16_t aValue) override { mRegA = aValue; mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegA: " << HexPrinter(mRegA,4) << std::endl; }
	virtual void SetRegB(uint16_t aDeviceAddr, uint16_t aValue) override { mRegB = aValue; mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegB: " << HexPrinter(mRegB,4) << std::endl; }
	virtual void SetRegC(uint16_t aDeviceAddr, uint16_t aValue) override;
	virtual void SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) override;
	virtual uint16_t GetRegA(uint16_t aDeviceAddr) override;
	virtual uint16_t GetRegB(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegB: " << HexPrinter(mRegB,4) << std::endl; return mRegB; }
	virtual uint16_t GetRegC(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegC: " << HexPrinter(mRegC,4) << std::endl; return mRegC; }
	virtual uint16_t GetStatus(uint16_t aDeviceAddr) override;
	virtual void Control(uint16_t aDeviceAddr, uint16_t aControl) override;
	virtual void Tick() override {}
	virtual bool NeedsTick() override { return false; }
	virtual size_t GetChannelCnt() const override { return 1; }
	virtual uint16_t GetChannelIdx(size_t aIdx) const override { CRAY_ASSERT(aIdx == 0); return mChannelIdx; }
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Printer; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override;
	virtual void RegisterCommands(CommandHooks_t &aHooks) override {}
	virtual std::string GetName() const override { return "XPR"; }
protected:
	void SetIntActive(bool aActive);

	enum StatusCodes_e {
		StatusCode_Done = 0x4000
	};
	enum CommandCodes_e {
		CommandCode_ClearInt = 6,
		CommandCode_4 = 4,
		CommandCode_NewLine = 3,
		CommandCode_GraphicsMode = 1,
		CommandCode_NewPage = 0
	};
	uint16_t mRegA;
	uint16_t mRegB;
	uint16_t mRegC;
	uint16_t mStatus;
	bool mBusy;
	bool mDone;
	bool mInterruptEnabled;
	bool mInterruptActive;
	class IopChannelExp_c &mParent;
	CLogger_c mLogger;
	std::string mPrintFileName;
	std::ofstream mPrintFile;
	uint16_t mChannelIdx;
	bool mInGraphicsMode;
	size_t mWrites;
};

class IopExpClockDevice_c: public IopExpDevice_i {
public:
	IopExpClockDevice_c(const Configuration_c &aConfig, class IopChannelExp_c &aParent);
	virtual ~IopExpClockDevice_c() override {}
	virtual void SetRegA(uint16_t aDeviceAddr, uint16_t aValue) override;
	virtual void SetRegB(uint16_t aDeviceAddr, uint16_t aValue) override { mRegB = aValue; mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegB: " << HexPrinter(mRegB,4) << std::endl; }
	virtual void SetRegC(uint16_t aDeviceAddr, uint16_t aValue) override { mRegC = aValue; mLogger << setloglevel(LogLevel_IoActivity) << "Recevied RegC: " << HexPrinter(mRegC,4) << std::endl; }
	virtual void SetIntEnabled(uint16_t aDeviceAddr, bool aInterruptEnabled) override;
	virtual uint16_t GetRegA(uint16_t aDeviceAddr) override;
	virtual uint16_t GetRegB(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegB: " << HexPrinter(mRegB,4) << std::endl; return mRegB; }
	virtual uint16_t GetRegC(uint16_t aDeviceAddr) override { mLogger << setloglevel(LogLevel_IoActivity) << "Returning RegC: " << HexPrinter(mRegC,4) << std::endl; return mRegC; }
	virtual uint16_t GetStatus(uint16_t aDeviceAddr) override;
	virtual void Control(uint16_t aDeviceAddr, uint16_t aControl) override;
	virtual void Tick() override;
	virtual bool NeedsTick() override { return true; }
	//	virtual size_t GetChannelCnt() const override { return 3; }
	virtual size_t GetChannelCnt() const override { return 2; }
	virtual uint16_t GetChannelIdx(size_t aIdx) const override;
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Other; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override {}
	virtual void RegisterCommands(CommandHooks_t &aHooks) override {}
	virtual std::string GetName() const override { return "XCK"; }
protected:
	void SetIntActive(uint16_t aDeviceAddr, bool aActive);

	std::string mRequest;
	std::string mResponse;
	uint16_t mPrimaryChannelIdx;
	uint16_t mRequestChannelIdx;
	uint16_t mResponseChannelIdx;
	uint16_t mRegA;
	uint16_t mRegB;
	uint16_t mRegC;
	bool mYearLimit;
	size_t mResponseTimer;
	size_t mResponseTimeout;
	bool mBusy;
	bool mDone;
	bool mInterruptEnabled;
	bool mInterruptActive[64];
	class IopChannelExp_c &mParent;
	CLogger_c mLogger;

	void ProcessRequest();
};

class IopChannelExp_c: public IopSimplePeripheral_i {
public:
	explicit IopChannelExp_c(const Configuration_c &aConfig, IopCpu_c &aParent): mParent(aParent), mLogger(aConfig, "Exp"), mChannelIdx(aConfig.get<size_t>("ChannelIdx")) {
		mLogger.SetParent(mParent.GetLogger());
		try {
			MasterClear(); 
			Setup(aConfig);
		} catch(...) {
			mDeviceIntMapping.clear();
			mDevices.clear();
			throw;
		}
	}
	~IopChannelExp_c() {}
	IopCpu_c::Dma_c GetDma() { return mParent.GetDma(); }

	virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
	virtual IopBit_t GetBusy() override { return mBusy; }
	virtual IopBit_t GetDone() override { return mDone; }
	virtual IopBit_t GetInterrupt() override;
	virtual void MasterClear() override { mDone = false; mBusy = false; mInterruptEnabled = true; mActiveChannel = -1; mOperation = Operation_Idle; }
	virtual size_t GetChannelIdx() const override { return mChannelIdx; }
	virtual void Tick() override;
	virtual bool NeedsTick() override { return true; }
	CLogger_c &GetLogger() { return mLogger; }
	virtual void Dump(size_t aIdent=0) override {}
	virtual std::string GetName() const override { return "Exp"; }
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Expander; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override;
	virtual void RegisterCommands(CommandHooks_t &aHooks) override;
	void IntStatusChanged();
	IopCpu_c &GetParent() { return mParent; }
protected:
	void Setup(const Configuration_c &aConfig);

	IopCpu_c &mParent;

	bool mDone;
	bool mBusy;
	bool mInterruptEnabled;
	bool mDeviceInterruptEnabled;
	bool mDeviceInterruptPending;
	std::vector<std::shared_ptr<IopExpDevice_i>> mDevices;
	std::vector<IopExpDevice_i*> mTickedDevices;
	std::vector<uint8_t> mDeviceIntMapping;
	int mActiveChannel;
	size_t mChannelIdx;
	CLogger_c mLogger;

	enum Operations_e {
		Operation_ReadA,
		Operation_ReadB,
		Operation_ReadC,
		Operation_ReadInterrupt,
		Operation_Idle
	} mOperation;

	enum Status_e {
		Status_ExpanderIntEnabled = 1 << 8,
		Status_ExpanderBusy       = 1 << 11,
		Status_ExpanderDone       = 1 << 12
	};
};

#endif // __IOP_EXPANDER_H__
