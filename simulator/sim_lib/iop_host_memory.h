#ifndef __IOP_HOST_MEMORY_H__
#define __IOP_HOST_MEMORY_H__

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

class ChannelHostMemory_c: public IopSimplePeripheral_i {
public:
	explicit ChannelHostMemory_c(const Configuration_c &aConfig, IopCpu_c &aParent, std::vector<uint8_t> &aCentralMemory, size_t aChannelIdx, bool aAllowCtoI, bool aAllowItoC):
		mParent(aParent),
		mCentralMemory(aCentralMemory),
		mAllowCtoI(aAllowCtoI),
		mAllowItoC(aAllowItoC),
		mChannelIdx(aChannelIdx),
		mLogger(aConfig, aAllowCtoI?"HI":"HO")
	{
		CRAY_ASSERT(aAllowCtoI ^ aAllowItoC);
		mLogger.SetParent(mParent.GetLogger());
		MasterClear();
	}
	virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
	virtual IopBit_t GetBusy() override { return mItoCActive || mCtoIActive; }
	virtual IopBit_t GetDone() override { return mDone; }
	virtual IopBit_t GetInterrupt() override { return mInterruptEnabled && mDone; }
	virtual void MasterClear() override { mDone = false; mCtoIActive = false; mItoCActive = false; mInterruptEnabled = true; mCentralMemoryAddr = 0; mIoMemoryAddr = 0; mTransferSize = 0; }
	virtual size_t GetChannelIdx() const override { return mChannelIdx; }
	void DeadStart() { MasterClear(); }
	void DoTransfer();
	virtual void Tick() override;
	virtual bool NeedsTick() override { return false; }
	virtual void Dump(size_t aIdent = 0) override {}
	virtual std::string GetName() const override { return mAllowCtoI ? "HI" : "HO"; }
protected:
	IopCpu_c &mParent;
	CAddr_t mCentralMemoryAddr;
	IopInt_t mIoMemoryAddr;
	IopInt_t mTransferSize;
	std::vector<uint8_t> &mCentralMemory;

	bool mAllowItoC;
	bool mAllowCtoI;
	bool mDone;
	bool mItoCActive;
	bool mCtoIActive;
	bool mInterruptEnabled;
	size_t mChannelIdx;
	CLogger_c mLogger;
};

#endif // __IOP_HOST_MEMORY_H__
