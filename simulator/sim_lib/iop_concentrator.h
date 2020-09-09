#ifndef __IOP_CONCENTRATOR_H__
#define __IOP_CONCENTRATOR_H__

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

class IopConcentratorChannel_c: public IopChannel_i {
public:
	explicit IopConcentratorChannel_c(class IopConcentrator_c &aParent, const Configuration_c &aConfig, size_t aChannelIdx, bool aAllowOut, bool aAllowIn);
	virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
	virtual IopBit_t GetBusy() override { return mInActive || mOutActive; }
	virtual IopBit_t GetDone() override { return mDone; }
	virtual IopBit_t GetInterrupt() override;
	virtual size_t GetChannelIdx() const override { return mChannelIdx; }

	virtual void MasterClear() override { mDone = false; mInActive = false; mOutActive = false; mInterruptEnabled = true; mIoMemoryAddr = 0; mTransferSize = 0; mChannelActive = false; mHoldDisconnect = false; }
	virtual void Tick() override;
	virtual bool NeedsTick() override { return true; }
	virtual void Dump(size_t aIdent=0) override {}
protected:
	class IopConcentrator_c &mParent;
	IopInt_t mIoMemoryAddr;
	IopInt_t mTransferSize;

	bool mAllowIn;
	bool mAllowOut;
	bool mDone;
	bool mInActive;
	bool mOutActive;
	bool mInterruptEnabled;
	bool mChannelActive;
	bool mHoldDisconnect;
	size_t mTransferDelayCounter;
	size_t mTransferDelay;
	size_t mChannelIdx;
};

class FrontEndLcp_c {
public:
	static const size_t LcpSize = 6;
	FrontEndLcp_c() {
		mData.resize(6,0);
	}
	explicit FrontEndLcp_c(const std::vector<CInt_t> &aData): mData(aData) {
		CRAY_ASSERT(aData.size() == LcpSize);
	}
	std::vector<CInt_t> &GetData() { return mData; }
	const std::vector<CInt_t> &GetData() const { return mData; }
	#ifdef CREATE_FIELD
	#error CREATE_FIELD is alreadt defined
	#endif
	#define CREATE_FIELD(aType,aName,aEntry,aStartBit,aEndBit)                                                    \
		aType Get##aName() { return (aType)GetBits(mData[aEntry], 63-aStartBit, 63-aEndBit); }                          \
		void Set##aName(aType aValue) { mData[aEntry] = SetBits(mData[aEntry], 63-aStartBit, 63-aEndBit, aValue); }

	CREATE_FIELD(uint16_t,  DestinationId,         0,   0,  15)
	CREATE_FIELD(uint16_t,  SourceId,              0,  16,  31)
	CREATE_FIELD(uint8_t,   NumberOfSubsegments,   0,  32,  39)
	CREATE_FIELD(uint8_t,   MessageNumber,         0,  40,  47)
	CREATE_FIELD(uint8_t,   MessageCode,           0,  48,  55)
	CREATE_FIELD(uint8_t,   MessageSubcode,        0,  56,  63)

	CREATE_FIELD(uint8_t,   RequestPending,        1,   0,   0)
	CREATE_FIELD(uint8_t,   StreamNumber,          1,   4,   7)
	CREATE_FIELD(uint32_t,  SegmentNumber,         1,   8,  31)
	CREATE_FIELD(uint32_t,  DataBitsInSegment,     1,  32,  63)
	#undef CREATE_FIELD

	uint8_t GetInputStreamCtrlByte(uint8_t aStrmIdx) { return (uint8_t)GetBits(mData[3], aStrmIdx*8, aStrmIdx*8+7); }
	void SetInputStreamCtrlByte(uint8_t aStrmIdx, uint8_t aValue) { SetBits(mData[3], aStrmIdx*8, aStrmIdx*8+7, aValue); }

	uint8_t GetOutputStreamCtrlByte(uint8_t aStrmIdx) { return (uint8_t)GetBits(mData[4], aStrmIdx*8, aStrmIdx*8+7); }
	void SetOutputStreamCtrlByte(uint8_t aStrmIdx, uint8_t aValue) { SetBits(mData[4], aStrmIdx*8, aStrmIdx*8+7, aValue); }

	//TODO: This list is highly incomplete. I'll have to finish it up if/when we implement a full front-end interface
	enum MessageCodes_e {
		MessageCode_Logon                   = 0001,
		MessageCode_Logoff                  = 0003,
		MessageCode_Start                   = 0004,
		MessageCode_Restart                 = 0005,
		MessageCode_DatasetHeader           = 0006,
		MessageCode_DatasetSegment          = 0007,
		MessageCode_Control                 = 0011,
		MessageCode_MessageError            = 0012,
		MessageCode_DatasetTransferRequest  = 0013,
		MessageCode_DatasetTransferReply    = 0014,
		MessageCode_EnterLogfileRequest     = 0015,
		MessageCode_EnterLogfileReply       = 0016
	};
protected:
	std::vector<CInt_t> mData;
};

class IopConcentrator_c: public IopPeripheral_i {
public:
	explicit IopConcentrator_c(const Configuration_c &aConfig, IopCpu_c &aParent):
		mParent(aParent),
		mLogger(aConfig, "CONC"),
		mInputChannel(*this,aConfig,aConfig.get<size_t>("InputChannelIdx"),false,true),
		mOutputChannel(*this,aConfig,aConfig.get<size_t>("OutputChannelIdx"),true,false),
		mState(State_InMasterClear),
		mPollDelay(aConfig.get<uint32_t>("PollDelay", 10)),
		mPollTimer(0),
		mMainframeId(0),
		mToCrayIdx(0),
		mToCrayLast(true),
		mToCrayPacketIdx(0),
		mMessageNumber(0)
	{
		std::string SidStr =aConfig.get<std::string>("SID", "FE");
		CRAY_ASSERT(SidStr.length() == 2);
		mFrontEndId = ((uint16_t)SidStr[0]) << 8 | ((uint16_t)SidStr[1]) << 0;
		mMainframeId = 0x4331; // This is 'C1' but how should he concentrator know about it? It's not the SID of the RESET request...
		mLogger.SetParent(mParent.GetLogger());
		MasterClear(false);
	}
	void MasterClear(bool aValue);
	CLogger_c &GetLogger() { return mLogger; }
	bool HasData();
	CInt_t GetData();
	void SetData(CInt_t aData);
	void ProcessFromCrayData();
	void TransferCompleted();
	virtual const IopChannel_i &GetChannel(size_t aIdx) const override {
		switch (aIdx) {
			case 0: return mInputChannel;
			case 1: return mOutputChannel;
			default: CRAY_ASSERT(false);
		}
		throw Generic_x("Unreachable code");
	}
	virtual size_t GetChannelCnt() const override { return 2; }
	IopCpu_c &GetParent() { return mParent; }
	void BufferClearComplete();
	enum States_e {
		State_InMasterClear,
		State_ClearingBuffers,
		State_Operational
	};
	States_e GetState() const { return mState; }
	void LogState(const char *aHeader) const;
	void Tick();
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Other; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override {}
	virtual void RegisterCommands(CommandHooks_t &aHooks)override {}
	virtual std::string GetName() const override { return "CONC"; }
protected:
	void ProcessFromCrayLcp();
	void SetupNextTransfer();
	void SetToCrayData(std::vector<CInt_t> &aData, bool aIsLast, size_t aPacketIdx = 0) {
		mToCrayData = aData; mToCrayIdx = 0; mToCrayLast = aIsLast; mToCrayPacketIdx = aPacketIdx;
		mLogger << setloglevel(LogLevel_IoActivity) << "Setting up data tansfer mToCrayData.size()=" << DecPrinter(mToCrayData.size()) << " mToCrayLast=" << (mToCrayLast?"true":"false") << std::endl;
	}

	std::vector<CInt_t> mFromCrayData;
	FrontEndLcp_c mFromCrayLcp;
	std::vector<CInt_t> mToCrayData;
	size_t mToCrayIdx;
	bool mToCrayLast;
	size_t mToCrayPacketIdx;
	uint32_t mPollDelay;
	uint32_t mPollTimer;
	IopCpu_c &mParent;
	IopConcentratorChannel_c mInputChannel;
	IopConcentratorChannel_c mOutputChannel;
	mutable CLogger_c mLogger;
	States_e mState;
	uint16_t mFrontEndId;
	uint16_t mMainframeId;
	uint8_t mMessageNumber;
};

#endif // __IOP_CONCENTRATOR_H__
