#ifndef __IOP_CONSOLE_H__
#define __IOP_CONSOLE_H__

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
#include "console.h"

class IopConsole_c: public IopPeripheral_i, public Console_c {
public:
	IopConsole_c(const Configuration_c &aConfig, IopCpu_c &aParent, bool aDisableAutoTerminal):
		Console_c(
			aConfig,
			mLogger,
			aConfig.get<unsigned short>("Port", aConfig.get<unsigned short>("BasePort", 20000)),
			!aConfig.get_optional<unsigned short>("Port").is_initialized(),
			aDisableAutoTerminal
		),
		mTI(*this,aConfig.get<size_t>("TIChannelIdx")),
		mTO(*this,aConfig.get<size_t>("TOChannelIdx")),
		mParent(aParent),
		mLogger(aConfig, "CON", aConfig.get<size_t>("TIChannelIdx")),
		mTitle(aConfig.get<std::string>("Title",""))
	{
		mLogger.SetParent(mParent.GetLogger());
		std::stringstream Strm;
		Strm << "CON_" << OctPrinter(aConfig.get<size_t>("TIChannelIdx"), 2);
		mName = Strm.str();
	}
	virtual ~IopConsole_c() override {}
	virtual const IopChannel_i &GetChannel(size_t aIdx) const override {
		switch (aIdx) {
			case 0: return mTI;
			case 1: return mTO;
			default: CRAY_ASSERT(false);
		}
		throw Generic_x("Unreachable code");
	}
	virtual size_t GetChannelCnt() const override { return 2; }
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Console; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override;
	virtual void RegisterCommands(CommandHooks_t &aHooks) override {}
	virtual std::string GetName() const override { return mName; }
protected:
	IopCpu_c &mParent;

	void Tick();

	virtual bool ReceiveHandlerDetail() override;
	virtual void SendHandlerDetail() override;
	virtual void ResetDetail() override;
	virtual void ConnectDetail() override;

	std::string mName;

	class ChannelTI_c: public IopChannel_i {
	public:
		explicit ChannelTI_c(IopConsole_c &aParent, size_t aChannelIdx) : mParent(aParent), mChannelIdx(aChannelIdx) { MasterClear(); }
		virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
		virtual IopBit_t GetBusy() override { return false; }
		virtual IopBit_t GetDone() override { return mDone; }
		virtual IopBit_t GetInterrupt() override { return mInterruptEnabled && mDone; }
		virtual size_t GetChannelIdx() const override { return mChannelIdx; }
		virtual void MasterClear() override { mDone = false; mInterruptEnabled = true; }
		void DeadStart() { MasterClear(); }
		virtual void Tick() override;
		virtual bool NeedsTick() override { return true; }
		virtual void Dump(size_t aIdent=0) override {}
		void SetDone() { mDone = true; }
	protected:
		IopConsole_c &mParent;
		bool mDone;
		bool mInterruptEnabled;
		size_t mChannelIdx;
	};

	class ChannelTO_c: public IopChannel_i {
	public:
		explicit ChannelTO_c(IopConsole_c &aParent, size_t aChannelIdx): mParent(aParent), mChannelIdx(aChannelIdx) { MasterClear(); }
		virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
		virtual IopBit_t GetBusy() override { return mBusy; }
		virtual IopBit_t GetDone() override { return mDone; }
		virtual IopBit_t GetInterrupt() override { return mInterruptEnabled && mDone; }
		virtual size_t GetChannelIdx() const override { return mChannelIdx; }
		virtual void MasterClear() override { mDone = false; mBusy = false; mInterruptEnabled = true; }
		void DeadStart() { MasterClear(); }
		virtual void Tick() override {}
		virtual bool NeedsTick() override { return false; }
		virtual void Dump(size_t aIdent=0) override {}

		void SetDone();
	protected:
		void Send(char aCh);
		void Send(std::string aStr);
		IopConsole_c &mParent;
		bool mDone;
		bool mBusy;
		bool mInterruptEnabled;
		char mDataOut;
		size_t mChannelIdx;
		std::string mEscSequence;
	};

	ChannelTI_c mTI;
	ChannelTO_c mTO;

	friend ChannelTI_c;
	friend ChannelTO_c;

	CLogger_c mLogger;

	std::string mTitle;
};

#endif // __IOP_CONSOLE_H__
