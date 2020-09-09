#ifndef __IOP_IOP2CRAY_H__
#define __IOP_IOP2CRAY_H__

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

class IopCrayChannel_c: public IopSimplePeripheral_i, public Channel_i {
public:
	explicit IopCrayChannel_c(const Configuration_c &aConfig, IopCpu_c &aParent, class Mainframe_c &aMainframe, size_t aIopChannelIdx, size_t aCrayChannelIdx, bool aAllowCtoI, bool aAllowItoC):
		Channel_i(aCrayChannelIdx, aMainframe),
		mParent(aParent),
		mMainframe(aMainframe),
		mAllowCtoI(aAllowCtoI),
		mAllowItoC(aAllowItoC),
		mChannelIdx(aIopChannelIdx),
		mLogger(aConfig, aAllowCtoI?"CI":"CO")
	{
		CRAY_ASSERT(aAllowCtoI ^ aAllowItoC);
		mLogger.SetParent(mParent.GetLogger());
		MasterClear();
	}
	~IopCrayChannel_c() {
	}
	virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
	virtual IopBit_t GetBusy() override { return mItoCActive || mCtoIActive; }
	virtual IopBit_t GetDone() override { return mDone; }
	virtual IopBit_t GetInterrupt() override { return mInterruptEnabled && mDone; }
	virtual void MasterClear() override {
		mDone = false;
		mCtoIActive = false;
		mItoCActive = false;
		mInterruptEnabled = true;
		mIoMemoryAddr = 0;
		mTransferSize = 0;
		mReadyWaiting = false;
		mHoldDisconnect = false;
	}
	void DeadStart() { MasterClear(); }
	virtual size_t GetChannelIdx() const override { return mChannelIdx; }
	virtual CLogger_c &GetLogger() const override { return mLogger; }
	virtual void SetAddress(CAddr_t aAddress) override;
	void DoTransfer();
	virtual void Tick() override;
	virtual bool NeedsTick() override;
	virtual void ChannelTick() override {}
	virtual bool NeedsChannelTick() const override { return false; }
	virtual void Dump(size_t aIdent=0) override {}
	virtual std::string GetName() const override { return mAllowCtoI ? "CI" : "CO"; }
protected:
	void DecodeBuffer();
	virtual void HandleActivation() override;

	IopCpu_c &mParent;
	IopInt_t mIoMemoryAddr;
	IopInt_t mTransferSize;

	bool mAllowItoC;
	bool mAllowCtoI;
	bool mDone;
	bool mItoCActive;
	bool mCtoIActive;
	bool mInterruptEnabled;
	bool mHoldDisconnect;
	bool mReadyWaiting;
	size_t mChannelIdx;
	mutable CLogger_c mLogger;
	class Mainframe_c &mMainframe;
	std::vector<CInt_t> mMsgBuffer;
};

#endif // __IOP_IOP2CRAY_H__
