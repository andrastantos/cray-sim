#ifndef __IOP_ERA_H__
#define __IOP_ERA_H__

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

class ChannelERA_c: public IopSimplePeripheral_i {
public:
	explicit ChannelERA_c(const Configuration_c &aConfig, IopCpu_c &aParent, size_t aChannelIdx): mParent(aParent), mChannelIdx(aChannelIdx), mLogger(aConfig, "ERA") { mLogger.SetParent(mParent.GetLogger()); MasterClear(); }
	virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
	virtual IopBit_t GetBusy() override { return false; }
	virtual IopBit_t GetDone() override { return mDone; }
	virtual IopBit_t GetInterrupt() override { return mInterruptEnabled && mDone; }
	virtual void MasterClear() override { mDone = false; mInterruptEnabled = false; }
	virtual size_t GetChannelIdx() const override { return mChannelIdx; }
	void DeadStart() { MasterClear(); }
	virtual void Tick() override {}
	virtual bool NeedsTick() override { return false; }
	virtual void Dump(size_t aIdent=0) override {}
	virtual std::string GetName() const override { return "ERA"; }
protected:
	IopCpu_c &mParent;
	bool mInterruptEnabled;
	bool mDone;
	CLogger_c mLogger;
	size_t mChannelIdx;
};

#endif // __IOP_ERA_H__
