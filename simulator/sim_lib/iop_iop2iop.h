#ifndef __IOP_IOP2IOP_H__
#define __IOP_IOP2IOP_H__

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

class IopIopChannel_c: public IopPeripheral_i {
public:
	IopIopChannel_c(const Configuration_c &aConfig, IopCpu_c &aSrcIop, IopCpu_c &aDstIop, size_t aAIChannelIdx, size_t aAOChannelIdx);
	~IopIopChannel_c() {
	}
	virtual const IopChannel_i &GetChannel(size_t aIdx) const override {
		switch (aIdx) {
			case 0: return mAI;
			case 1: return mAO;
			default: CRAY_ASSERT(false);
		}
		throw Generic_x("Unreachable code");
	}
	virtual size_t GetChannelCnt() const override { return 2; }
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Other; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override {}
	virtual void RegisterCommands(CommandHooks_t &aHooks) override {}
	virtual std::string GetName() const override { return "IopIop"; }
protected:
	IopCpu_c &mSrcIop;
	IopCpu_c &mDstIop;
	uint16_t mData;
	bool mDataValid;
	size_t mThrottle;

	class ChannelAI_c: public IopChannel_i {
	public:
		explicit ChannelAI_c(const Configuration_c &aConfig, IopIopChannel_c &aParent, size_t aChannelIdx): mParent(aParent), mChannelIdx(aChannelIdx), mLogger(aConfig, "AI") { mLogger.SetParent(mParent.mSrcIop.GetLogger()); MasterClear(); }
		virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
		virtual IopBit_t GetBusy() override { return false; }
		virtual IopBit_t GetDone() override { return mParent.mDataValid; }
		virtual IopBit_t GetInterrupt() override { return mInterruptEnabled && GetDone(); }
		virtual size_t GetChannelIdx() const override { return mChannelIdx; }
		virtual void MasterClear() override { mInterruptEnabled = true; mParent.mDataValid = false; }
		void DeadStart() { MasterClear(); }
		virtual void Tick() override {}
		virtual bool NeedsTick() override { return false; }
		virtual void Dump(size_t aIdent=0) override {}
	protected:
		IopIopChannel_c &mParent;
		const size_t mChannelIdx;
		bool mInterruptEnabled;
		CLogger_c mLogger;
	};

	class ChannelAO_c: public IopChannel_i {
	public:
		explicit ChannelAO_c(const Configuration_c &aConfig, IopIopChannel_c &aParent, size_t aChannelIdx): mParent(aParent), mChannelIdx(aChannelIdx), mLogger(aConfig, "AO") { mLogger.SetParent(mParent.mDstIop.GetLogger()); MasterClear(); }
		virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
		virtual IopBit_t GetBusy() override { return !mParent.mDataValid; }
		virtual IopBit_t GetDone() override { return mDone; }
		virtual IopBit_t GetInterrupt() override { return mInterruptEnabled && mDone; }
		virtual void MasterClear() override { mDone = false; mParent.mDataValid = false; mInterruptEnabled = true; mCtrlReg = 0; mResetStalling = false; mThrottleCnt = 0; }
		void DeadStart() { MasterClear(); }
		virtual size_t GetChannelIdx() const override { return mChannelIdx; }
		virtual void Tick() override {}
		virtual bool NeedsTick() override { return false; }
		void SetDone(bool aDone) { mDone = aDone; }
		virtual void Dump(size_t aIdent=0) override {}
		virtual bool StallIop() override;
	protected:
		IopIopChannel_c &mParent;
		const size_t mChannelIdx;
		bool mDone;
		bool mResetStalling;
		bool mInterruptEnabled;
		IopInt_t mCtrlReg;
		CLogger_c mLogger;
		size_t mThrottleCnt;
	};

	ChannelAI_c mAI;
	ChannelAO_c mAO;
};

#endif // __IOP_IOP2IOP_H__
