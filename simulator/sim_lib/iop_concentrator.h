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

	virtual void MasterClear() override {
		mDone = false;
		mHoldDisconnect = false;
		mInActive = false;
		mInterruptEnabled = true;
		mIoMemoryAddr = 0;
		mOutActive = false;
		mReadyWaiting = false;
		mTransferSize = 0;
		mWriteDisconnect = false;
	}
	virtual void Tick() override;
	virtual bool NeedsTick() override { return true; }
	virtual void Dump(size_t aIdent=0) override {}
protected:
	class IopConcentrator_c &mParent;
	IopInt_t mIoMemoryAddr;
	IopInt_t mTransferSize;

	bool mAllowIn;
	bool mAllowOut;
	size_t mChannelIdx;
	bool mDone;
	bool mHoldDisconnect;
	bool mInActive;
	bool mInterruptEnabled;
	bool mOutActive;
	bool mReadyWaiting;
	size_t mTransferDelay;
	size_t mTransferDelayCounter;
	bool mWriteDisconnect;
};

#define BytesPerLCP     48
#define MaxTransmitBufs 16
#define WordsPerLCP     6
/*
 *  Key message codes
 */
#define McLogon   001
#define McStart   004
#define McControl 011

class IopConcentrator_c: public IopPeripheral_i {
public:
	enum RcvStates_e {
		State_RcvLcpPduLength,
		State_RcvLcpPduContent,
		State_RcvSegPduLength,
		State_RcvSegPduContent
	};
	enum XmtStates_e {
		State_XmtLcpPduStart,
		State_XmtLcpPduContent,
		State_XmtSegPduStart,
		State_XmtSegPduContent
	};
	explicit IopConcentrator_c(const Configuration_c &aConfig, IopCpu_c &aParent):
		mAccepted(false),
		mBytesTransmitted(0),
                mConnectionSocket(nullptr),
		mInputChannel(*this,aConfig,aConfig.get<size_t>("InputChannelIdx"),false,true),
		mLastRcvdMessageCode(0),
		mLastXmitMessageCode(0),
                mListenSocket(nullptr),
		mLogger(aConfig, "CONC"),
		mOutputChannel(*this,aConfig,aConfig.get<size_t>("OutputChannelIdx"),true,false),
		mParent(aParent),
		mPduLength(0),
                mPort(aConfig.get<uint16_t>("Port",9000)),
		mRcvState(State_RcvLcpPduLength),
		mSegmentSize(0),
		mToCrayIdx(0),
		mTransmitBufIdx(0),
		mXmtState(State_XmtLcpPduStart)
	{
		mLogger.SetParent(mParent.GetLogger());
		StartListen();
		MasterClear();
	}
        void ClearBuffers();
	virtual const IopChannel_i &GetChannel(size_t aIdx) const override {
		switch (aIdx) {
			case 0: return mInputChannel;
			case 1: return mOutputChannel;
			default: CRAY_ASSERT(false);
		}
		throw Generic_x("Unreachable code");
	}
	virtual size_t GetChannelCnt() const override { return 2; }
	CInt_t GetData();
	size_t GetDataSize();
	CLogger_c &GetLogger() { return mLogger; }
	virtual std::string GetName() const override { return "CONC"; }
	IopCpu_c &GetParent() { return mParent; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override {}
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Other; }
	bool HasData();
	bool IsReceptionComplete();
	void MasterClear();
	void ProcessFromCrayData();
	virtual void RegisterCommands(CommandHooks_t &aHooks)override {}
	void SetData(CInt_t aData);
	void Tick();
protected:
        void AcceptHandler(const boost::system::error_code& aError);
        void ClearReceiveBuffers();
        void ClearTransmitBuffers();
        void CloseConnSocket();
        void Poll();
	void ProcessReceivedBytes();
        void ReadHandler(const boost::system::error_code& aError, std::size_t bytesTransferred);
        void StartAccept();
        void StartListen();
        void StartReceive();
        void StartTransmit();
        void WriteHandler(const boost::system::error_code& aError, std::size_t bytesTransferred);

	bool mAccepted;
	size_t mBytesTransmitted;
        std::shared_ptr<boost::asio::ip::tcp::socket> mConnectionSocket;
	std::vector<CInt_t> mFromCrayData;
	IopConcentratorChannel_c mInputChannel;
        static boost::asio::io_context mIoService;
	uint8_t mLastRcvdMessageCode;
	uint8_t mLastXmitMessageCode;
        std::shared_ptr<boost::asio::ip::tcp::acceptor> mListenSocket;
	mutable CLogger_c mLogger;
	IopConcentratorChannel_c mOutputChannel;
	IopCpu_c &mParent;
	size_t mPduLength;
        size_t mPollCnt;
        uint16_t mPort;
        boost::asio::streambuf mReceiveBuf;
	RcvStates_e mRcvState;
	size_t mSegmentSize;
	size_t mSubsegmentCount;
	std::vector<CInt_t> mToCrayData;
	size_t mToCrayIdx;
	size_t mTransmitBufIdx;
	boost::asio::streambuf mTransmitBufs[MaxTransmitBufs];
	XmtStates_e mXmtState;
};

#endif // __IOP_CONCENTRATOR_H__
