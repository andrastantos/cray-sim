#ifndef __CRAY_IOP_H__
#define __CRAY_IOP_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "utils.h"
#include "cray_types.h"
#include "cray_logger.h"
#include "debug_events.h"
#include <stdint.h>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/timer/timer.hpp>
#include "config_file.h"
#include "ui.h"

// Cray-1S I/O processor simulator class

// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
//
// To be true to the documentation, octal numbers are extensively used in this file.
// Watch out for the leading 0-s!
//
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!

typedef uint16_t IopParcel_t;
typedef uint16_t IopAddr_t;
typedef uint16_t IopInt_t;
typedef uint16_t IopOpRegAddr_t; // TODO: replace with proper 9-bit integer
typedef uint16_t IopExitStackPtr_t; // TODO: replace with proper 4-bit integer
typedef uint16_t IopBit_t; // TODO: replace with proper 1-bit integer
typedef uint16_t IopIoFunction_t; // TODO: replace with proper 4-bit integer

class IopChannel_i {
public:
	virtual ~IopChannel_i() {}
	virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) = 0;
	virtual IopBit_t GetBusy() = 0;
	virtual IopBit_t GetDone() = 0;
	virtual IopBit_t GetInterrupt() = 0;
	virtual size_t GetChannelIdx() const = 0;
	virtual void MasterClear() = 0;
	virtual void Tick() = 0;
	virtual void Dump(size_t aIdent=0) = 0;
	virtual bool StallIop() { return false; }
	virtual bool NeedsTick() = 0;
};

class IopPeripheral_i {
public:
	virtual ~IopPeripheral_i() {}
	virtual const IopChannel_i &GetChannel(size_t aIdx) const = 0;
	virtual size_t GetChannelCnt() const = 0;
	virtual PeripheralType_e GetType() const = 0;
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const = 0;
	virtual void RegisterCommands(CommandHooks_t &aHooks) = 0;
	virtual std::string GetName() const = 0;
};

class IopSimplePeripheral_i: public IopPeripheral_i, public IopChannel_i {
public:
	virtual ~IopSimplePeripheral_i() {}
	virtual const IopChannel_i &GetChannel(size_t aIdx) const override { return *this; }
	virtual size_t GetChannelCnt() const override { return 1; }
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Other; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override {}
	virtual void RegisterCommands(CommandHooks_t &aHooks) override {}
};

enum IopTypes_e {
	IopType_MIOP = 0,
	IopType_BIOP = 1,
	IopType_DIOP = 2,
	IopType_XIOP = 3,
};

class IopCpu_c {
protected:
	mutable CLogger_c mLogger; // This HAS to be the first data member. Otherwise implicit channel member construction fails.
public:
	explicit IopCpu_c(const Configuration_c &aConfig, uint8_t aCpuId, std::vector<uint8_t> &aIoMemory, std::vector<uint8_t> *aBufferMemory, class IopCluster_c &aIopCluster);
	explicit IopCpu_c(IopTypes_e aType, uint8_t aCpuId, std::vector<uint8_t> &aIoMemory, std::vector<uint8_t> *aBufferMemory = nullptr): 
		mLogger(Configuration_c::GetEmptyConfig()),
		mEventPoints(*this),
		RefOrB(*this),
		RefA(*this),
		RefC(*this),
		RefC_A(*this),
		RefB(*this),
		mInstCnt(0),
		mLastInstCnt(0),
		mIoMemory(aIoMemory), 
		mBufferMemory(aBufferMemory),
		mIopCluster(nullptr),
		mState(Configuration_c::GetEmptyConfig(), *this, 1, 79999),
		mCpuId(aCpuId),
		mType(aType),
		mInstructionBurstSize(1),
		mIoChannels(42),
		mExists(true)
	{
		for(auto &IoChannel: mIoChannels) IoChannel = nullptr;
		mIoChannels[0] = &mState.ChannelIOR;
		mIoChannels[1] = &mState.ChannelPFR;
		mIoChannels[2] = &mState.ChannelPXS;
		mIoChannels[3] = &mState.ChannelLME;
		mIoChannels[4] = &mState.ChannelRTC;
		mIoChannels[5] = &mState.ChannelMOS;
	}

	~IopCpu_c() {}

	struct RefOr_s: public FieldFormatter_i {
		RefOr_s(uint16_t aRegIdx, IopCpu_c &aParent):
			mUseB(false),
			mRegIdx(aRegIdx),
			mParent(aParent)
		{}
		explicit RefOr_s(IopCpu_c &aParent):
			mUseB(true),
			mRegIdx(0),
			mParent(aParent)
		{}
		FORCEINLINE RefOr_s &operator=(IopInt_t aValue) FORCEINLINEATTR {
				LogLine_c LogLine = mParent.GetLogger() << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << "OR[" << DecPrinter(mUseB ? mParent.mState.B : mRegIdx) << "]" << " <== " << HexPrinter(aValue) << " (" << DecPrinter(aValue) << ")" << std::endl;
			mParent.mState.OperandRegisters[mUseB ? mParent.mState.B : mRegIdx] = aValue;
			return *this;
		}
		operator IopInt_t() const { return mParent.mState.OperandRegisters[mUseB ? mParent.mState.B : mRegIdx]; }
		virtual void Print(std::ostream &aStream) const { if (mUseB) { aStream << "OR[B]"; } else { aStream << "OR[" << DecPrinter(mRegIdx) << "]"; } }
	protected:
		bool mUseB;
		uint16_t mRegIdx;
		IopCpu_c &mParent;
	};
	void GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const;
	void RegisterCommands(CommandHooks_t &aHooks);
	void GetPeripheralStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const;
	std::string GetName() const;
	std::string GetShortDump() {
		std::stringstream Str;
		Str << "P: " << HexPrinter(mState.P);
		return Str.str();
	}
	std::string GetStackDump() {
		return mState.ChannelPXS.StackDump();
	}

	struct RefA_s: public FieldFormatter_i {
		explicit RefA_s(IopCpu_c &aParent):
			mParent(aParent)
		{}
		FORCEINLINE RefA_s &operator=(IopInt_t aValue) FORCEINLINEATTR {
			LogLine_c LogLine = mParent.GetLogger() << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << HexPrinter(aValue) << " (" << DecPrinter(aValue) << ")" << std::endl;
			mParent.mState.A = aValue;
			return *this;
		}
		operator IopInt_t() const { return mParent.mState.A; }
		virtual void Print(std::ostream &aStream) const { aStream << "A"; }
	protected:
		IopCpu_c &mParent;
	};
	

	struct RefB_s: public FieldFormatter_i {
		explicit RefB_s(IopCpu_c &aParent):
			mParent(aParent)
		{}
		FORCEINLINE RefB_s &operator=(IopInt_t aValue) FORCEINLINEATTR {
			aValue &= 0x1ff;
			LogLine_c LogLine = mParent.GetLogger() << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << HexPrinter(aValue) << " (" << DecPrinter(aValue) << ")" << std::endl;
			mParent.mState.B = aValue;
			return *this;
		}
		operator IopInt_t() const { return mParent.mState.B; }
		virtual void Print(std::ostream &aStream) const { aStream << "B"; }
	protected:
		IopCpu_c &mParent;
	};
	
	struct RefC_s: public FieldFormatter_i {
		explicit RefC_s(IopCpu_c &aParent):
			mParent(aParent)
		{}
		FORCEINLINE RefC_s &operator=(IopBit_t aValue) FORCEINLINEATTR {
			aValue = aValue & 1;
			LogLine_c LogLine = mParent.GetLogger() << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << HexPrinter(aValue) << " (" << DecPrinter(aValue) << ")" << std::endl;
			mParent.mState.Carry = aValue;
			return *this;
		}
		operator IopBit_t() const { return mParent.mState.Carry; }
		virtual void Print(std::ostream &aStream) const { aStream << "C"; }
	protected:
		IopCpu_c &mParent;
	};
	
	struct RefC_A_s: public FieldFormatter_i {
		explicit RefC_A_s(IopCpu_c &aParent):
			mParent(aParent)
		{}
		FORCEINLINE RefC_A_s &operator=(uint32_t aValue) FORCEINLINEATTR {
			mParent.mState.A = aValue & 0xffff;
			mParent.mState.Carry = (aValue & 0x10000) >> 16;

			LogLine_c LogLine = mParent.GetLogger() << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) {
				LogLine << SideEffectIndent << "state-change A <== " << HexPrinter(mParent.mState.A) << " (" << DecPrinter(mParent.mState.A) << ")" << std::endl;
				LogLine << SideEffectIndent << "state-change C <== " << HexPrinter(mParent.mState.Carry) << " (" << DecPrinter(mParent.mState.Carry) << ")" << std::endl;
			}
			return *this;
		}
		operator uint32_t() const { return mParent.mState.A | (mParent.mState.Carry << 16); }
		virtual void Print(std::ostream &aStream) const { CRAY_ASSERT(false); }
	protected:
		IopCpu_c &mParent;
	};
	
	// A small abstraction for memory accesses into the I/O memory of an IOP. This is better than passing parents all around and gives better logging capability as well
	class Dma_c {
	public:
		~Dma_c() {}
		// TODO: do we have to swap bytes here?
		//void Read(size_t aAddr, size_t aLength, void *aDst) const;
		uint16_t ReadWord(size_t aAddr) const { return mParent.IoMemAccess<uint16_t>(aAddr); }
		//void Write(size_t aAddr, size_t aLength, void *aSrc);
		void WriteWord(size_t aAddr, uint16_t aData) { mParent.IoMemAccess<uint16_t>(aAddr) = aData; }
	protected:
		explicit Dma_c(IopCpu_c &aParent): mParent(aParent) {}

		IopCpu_c &mParent;

		friend class IopCpu_c;
	};
	Dma_c GetDma() {
		return Dma_c(*this);
	}
	CLogger_c &GetLogger() { return mLogger; }
	class IopCluster_c &GetIopCluster() { return *mIopCluster; }
	class Mainframe_c &GetMainframe();

	void MasterClear();
	void Tick();
	bool SingleStep();
	void DeadStart() { MasterClear(); mState.ChannelMOS.DeadStart(); }
	const IopChannel_i *Connect(const IopChannel_i *aChannel) { IopChannel_i *RetVal = mIoChannels[aChannel->GetChannelIdx()]; mIoChannels[aChannel->GetChannelIdx()] = (IopChannel_i *)aChannel; return RetVal; }
	uint8_t GetCpuId() const { return mCpuId; }
	IopInt_t GetProgramCounter() const { return mState.P; }
	void SetProgramCounter(IopInt_t aP) { mState.P = aP; }

	IopInt_t Disassemble(IopInt_t aP, std::ostream &aDisassembly, std::ostream &aExplanation, std::ostream &InstFields);

	template <typename aType> aType &IoMemAccess(size_t aAddr) { return *(aType *)(&mIoMemory[size_t(aAddr) * sizeof(IopInt_t)]); } // Default addresses are by 16-bit parcels
	template <typename aType> aType &IoMemAccessByType(size_t aAddr) { return *(aType *)(&mIoMemory[size_t(aAddr) * sizeof(aType)]); }
	template <typename aType> aType &BufferMemAccess(size_t aAddr) { CRAY_ASSERT(mBufferMemory != nullptr); return *(aType *)(&(*mBufferMemory)[ (size_t(aAddr) * sizeof(uint64_t)) % mBufferMemory->size() ] ); } // Default addresses are by 64-bit parcels
	template <typename aType> aType &BufferMemAccessByType(size_t aAddr) { CRAY_ASSERT(mBufferMemory != nullptr); return *(aType *)(&(*mBufferMemory)[ (size_t(aAddr) * sizeof(aType)) % mBufferMemory->size() ] ); }

	void Dump(size_t aIdent=0) const;

	IopTypes_e GetType() const { return mType; }

	void HandleDebugEvent(const DebugEvent_t &aEvent) { mEventPoints.Fire(aEvent); }
	bool IsInReset() const { return mState.InReset; }
	void Reset(bool aValue) { mState.InReset = aValue; }
	bool Exists() const { return mExists; }
protected:
	// Built-in channel handlers
	class ChannelIOR_c: public IopChannel_i {
	public:
		explicit ChannelIOR_c(const Configuration_c &aConfig, IopCpu_c &aParent): mParent(aParent), mLogger(aConfig, "IOR") { mLogger.SetParent(mParent.GetLogger()); MasterClear(); }
		virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
		virtual IopBit_t GetBusy() override { return 0; }
		virtual IopBit_t GetDone() override { return 1; }
		virtual IopBit_t GetInterrupt() override { return 0; }
		virtual void MasterClear() override {}
		virtual size_t GetChannelIdx() const override { return 0; }
		virtual void Tick() override {}
		virtual bool NeedsTick() override { return false; }
		virtual void Dump(size_t aIdent=0) override {}
	protected:
		IopCpu_c &mParent;
		mutable CLogger_c mLogger;
	};
	class ChannelPFR_c: public IopChannel_i {
	public:
		explicit ChannelPFR_c(const Configuration_c &aConfig, IopCpu_c &aParent): mParent(aParent), mRequestIdx(0), mLogger(aConfig, "PFR") { mLogger.SetParent(mParent.GetLogger()); MasterClear(); }
		virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
		virtual IopBit_t GetBusy() override { return 0; }
		virtual IopBit_t GetDone() override { return mRequestFlag; }
		virtual IopBit_t GetInterrupt() override { 
			bool RetVal = mInterruptEnabled && mRequestFlag; 
			return RetVal;
		}
		virtual void MasterClear() override { mRequestFlag = false; mInterruptEnabled = false; }
		virtual size_t GetChannelIdx() const override { return 1; }
		virtual void Tick() override {}
		virtual bool NeedsTick() override { return false; }
		void SetRequestIdx(IopOpRegAddr_t aRequestIdx) {
			mRequestIdx = aRequestIdx; 
			mRequestFlag = true; 
		}
		virtual void Dump(size_t aIdent=0) override {}
	protected:
		IopCpu_c &mParent;
		IopOpRegAddr_t mRequestIdx;
		bool mRequestFlag;
		bool mInterruptEnabled;
		mutable CLogger_c mLogger;
	};
	class ChannelPXS_c: public IopChannel_i {
	public:
		explicit ChannelPXS_c(const Configuration_c &aConfig, IopCpu_c &aParent): mParent(aParent), mLogger(aConfig, "PXS") { mLogger.SetParent(mParent.GetLogger()); MasterClear(); }
		virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
		virtual IopBit_t GetBusy() override { return 0; }
		virtual IopBit_t GetDone() override { return mBundaryFlag; }
		virtual IopBit_t GetInterrupt() override { return mInterruptEnabled && mBundaryFlag; }
		virtual void MasterClear() override { mE = 0; memset(mProgramExitStack,0,sizeof(mProgramExitStack)); mBundaryFlag = false; mInterruptEnabled = false; }
		virtual size_t GetChannelIdx() const override { return 2; }
		virtual void Tick() override {}
		virtual bool NeedsTick() override { return false; }
		void SetE(uint8_t aE) {
			mE = aE & 0xf;
			LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "E <== " << DecPrinter(mE) << std::endl;
			CRAY_ASSERT(mE <=15);
		}
		uint8_t GetE() const { return mE; }
		void SetAtE(IopAddr_t aAddr) { mProgramExitStack[mE] = aAddr; }
		IopAddr_t GetAtE() const { return mProgramExitStack[mE]; }
		void SetAtE(IopAddr_t aAddr, uint8_t aE) { mProgramExitStack[aE & 0xf] = aAddr; }
		IopAddr_t GetAt(uint8_t aE) const { return mProgramExitStack[aE & 0xf]; }
		void Push(IopAddr_t aAddr) {
			CRAY_ASSERT(mE <=15);
			mE = (mE + 1) & 0xf;
			LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "++E <== " << DecPrinter(mE) << std::endl;
			mProgramExitStack[mE] = aAddr;
			if (!mBundaryFlag) {
				mBundaryFlag = (mE >= 14);
				if (mBundaryFlag) {
					LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
					if (LogLine.good()) LogLine << SideEffectIndent << "PXS boundary flag set due to overflow" << std::endl;
				}
			}
		}
		IopAddr_t Pop() {
			CRAY_ASSERT(mE <=15);
			if (!mBundaryFlag) {
				mBundaryFlag = (mE == 0);
				if (mBundaryFlag) {
					LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
					if (LogLine.good()) LogLine << SideEffectIndent << "PXS boundary flag set due to underflow" << std::endl;
				}
			}
			IopAddr_t RetVal = mProgramExitStack[mE];
			mE = (mE - 1) & 0xf;
			LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "--E <== " << DecPrinter(mE) << std::endl;
			return RetVal;
		}
		virtual void Dump(size_t aIdent=0) override;
		std::string StackDump(bool aSingleLine = false);
	protected:
		IopCpu_c &mParent;
		bool mBundaryFlag;
		bool mInterruptEnabled;
		uint8_t mE;
		IopAddr_t mProgramExitStack[16];
		mutable CLogger_c mLogger;
	};
	class ChannelLME_c: public IopChannel_i {
	public:
		explicit ChannelLME_c(const Configuration_c &aConfig, IopCpu_c &aParent): mParent(aParent), mLogger(aConfig, "LME") { mLogger.SetParent(mParent.GetLogger()); MasterClear(); }
		virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
		virtual IopBit_t GetBusy() override { return 0; }
		virtual IopBit_t GetDone() override { return 0; }
		virtual IopBit_t GetInterrupt() override { return 0; }
		virtual void MasterClear() override {}
		virtual size_t GetChannelIdx() const override { return 3; }
		virtual void Tick() override {}
		virtual bool NeedsTick() override { return false; }
		virtual void Dump(size_t aIdent=0) override {}
	protected:
		IopCpu_c &mParent;
		mutable CLogger_c mLogger;
	};
	class ChannelRTC_c: public IopChannel_i {
	public:
		explicit ChannelRTC_c(const Configuration_c &aConfig, IopCpu_c &aParent, uint64_t aClockFrequency, uint64_t aTimerLimit):
			mParent(aParent),
			mTimerLimit(aTimerLimit),
			mClockFrequency(aClockFrequency),
			mLogger(aConfig, "RTC")
		{
			mLogger.SetParent(mParent.GetLogger());
			MasterClear();
		}
		virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
		virtual IopBit_t GetBusy() override { return 0; }
		virtual IopBit_t GetDone() override { return mInterrupt; }
		virtual IopBit_t GetInterrupt() override { return mInterruptEnabled && mInterrupt; }
		virtual void MasterClear() override { mInterrupt = false; mInterruptEnabled = false; mTimer.start(); mLastIntTick = 0; mTickCnt = 0; }
		virtual size_t GetChannelIdx() const override { return 4; }
		virtual void Tick() override;
		virtual bool NeedsTick() override { return true; }
		virtual void Dump(size_t aIdent=0) override {}
	protected:
		uint64_t TimeToTick(boost::timer::nanosecond_type aTime) {
			return uint64_t(aTime / boost::timer::nanosecond_type(10000ULL) * boost::timer::nanosecond_type(mClockFrequency / 100000ULL));
		}

		IopCpu_c &mParent;
		uint64_t mTimerLimit;
		uint64_t mClockFrequency;
		uint64_t mLastIntTick;
		uint32_t mTickCnt;
		bool mInterrupt;
		bool mInterruptEnabled;
		mutable CLogger_c mLogger;
		boost::timer::cpu_timer mTimer;
	};
	class ChannelMOS_c: public IopChannel_i {
	public:
		explicit ChannelMOS_c(const Configuration_c &aConfig, IopCpu_c &aParent): mParent(aParent), mLogger(aConfig, "MOS") { mLogger.SetParent(mParent.GetLogger()); MasterClear(); }
		virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
		virtual IopBit_t GetBusy() override { return mItoBActive | mBtoIActive; }
		virtual IopBit_t GetDone() override { return mDone; }
		virtual IopBit_t GetInterrupt() override { return mInterruptEnabled && mDone; }
		virtual void MasterClear() override { mDone = false; mBtoIActive = false; mItoBActive = false; mInterruptEnabled = true; mBufferMemoryAddr = 0; mIoMemoryAddr = 0; mTransferSize = 0; }
		void DeadStart() { MasterClear(); mBtoIActive = true; DoTransfer(); } // Set up a 64k transfer on deadstart to load the IoMemory from BufferMemory
		virtual size_t GetChannelIdx() const override { return 5; }
		virtual void Tick() override {}
		virtual bool NeedsTick() override { return false; }
		virtual void Dump(size_t aIdent=0) override {}
	protected:
		void DoTransfer();
		IopCpu_c &mParent;
		CAddr_t mBufferMemoryAddr;
		IopInt_t mIoMemoryAddr;
		IopInt_t mTransferSize;
		bool mDone;
		bool mItoBActive;
		bool mBtoIActive;
		bool mInterruptEnabled;
		mutable CLogger_c mLogger;
	};

	class BreakPoint_i {
	public:
		BreakPoint_i() {}
		virtual ~BreakPoint_i() {}
		virtual void Fire(IopCpu_c &aIop) = 0;
		virtual bool Test(const Dma_c &aMemory) = 0;
	};
	class BreakPointBase_c: public BreakPoint_i {
	public:
		explicit BreakPointBase_c(const Configuration_c &aConfig) {
			try {
				for(const auto &DataCondition: aConfig.get_child_safe("DataConditions")) {
					IopAddr_t Addr = FromString<IopAddr_t>(DataCondition.first);
//					IopInt_t Value = SwapBytes(DataCondition.second.get_value<IopInt_t>());
					IopInt_t Value = DataCondition.second.get_value<IopInt_t>();
					mDataConditions.push_back(DataCondition_s(Addr,Value));
				}
			}
			catch(std::exception &Ex) {
				mDataConditions.clear();
				throw Ex;
			}
			mTriggerCnt = aConfig.get<size_t>("TriggerCnt",0);
			mPassCnt = 0;
		};
		virtual bool Test(const Dma_c &aMemory) override {
			for(auto &DataCondition: mDataConditions) {
				if (aMemory.ReadWord(DataCondition.Addr) != DataCondition.Value) {
					//cout << "Breakpoint doesn't hit, memory at address " << HexPrinter(DataCondition->Addr) << " is " << HexPrinter(aMemory.ReadWord(DataCondition->Addr)) << " instead of " << HexPrinter(DataCondition->Value) << std::endl;
					return false;
				}
			}
			//cout << "Event point with mTriggerCnt:" << mTriggerCnt << " and mPassCnt:" << mPassCnt << std::endl;
			++mPassCnt;
			if (mTriggerCnt > mPassCnt) return false;
			return true;
		}
	protected:
		struct DataCondition_s {
			IopAddr_t Addr;
			IopInt_t Value;
			DataCondition_s(IopAddr_t aAddr, IopInt_t aValue): Addr(aAddr), Value(aValue) {}
		};
		size_t mPassCnt;
		size_t mTriggerCnt;
		std::vector<DataCondition_s> mDataConditions;
	};
	class EventFirePoint_c: public BreakPointBase_c {
		public:
			EventFirePoint_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig), mEvent(aConfig.get<DebugEvent_t>("Event")) {}
			virtual void Fire(IopCpu_c &aIop) override;
		protected:
			DebugEvent_t mEvent;
	};
	class TracePoint_c: public BreakPointBase_c {
	public:
		TracePoint_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig), mTrace(aConfig.get<std::string>("Message")) {}
		virtual void Fire(IopCpu_c &aIop) {
			std::string FinalTrace = mTrace;
			FinalTrace = Replace(FinalTrace, "{stack}", aIop.mState.ChannelPXS.StackDump(true));
			aIop.GetLogger() << setloglevel(LogLevel_EventFire) << FinalTrace << std::endl;
		}
	protected:
		std::string mTrace;
	};
	class LogOn_c: public BreakPointBase_c {
	public:
		LogOn_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {}
		virtual void Fire(IopCpu_c &aIop) { aIop.GetLogger().SetDisplayLogLevel(LogLevel_All); }
	};
	class Terminate_c: public BreakPointBase_c {
	public:
		Terminate_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {}
		virtual void Fire(IopCpu_c &aIop) { aIop.GetLogger() << setloglevel(LogLevel_Event) << "Terminate class breakpoint hit. TERMINATING" << std::endl; CRAY_ASSERT(false); }
	};
	class LogOff_c: public BreakPointBase_c {
	public:
		LogOff_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {}
		virtual void Fire(IopCpu_c &aIop) { aIop.GetLogger().SetDisplayLogLevel(LogLevel_None); }
	};
	class LogLevel_c: public BreakPointBase_c {
	public:
		LogLevel_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {
			std::string Level = aConfig.get<std::string>("Level");
			mLogLevel = FromString_LogLevels_e(Level);
		}
		virtual void Fire(IopCpu_c &aIop) { aIop.GetLogger().SetDisplayLogLevel(mLogLevel); }
	protected:
		LogLevels_e mLogLevel;
	};
	class LogLevelPush_c: public BreakPointBase_c {
	public:
		LogLevelPush_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {
			std::string Level = aConfig.get<std::string>("Level");
			mLogLevel = FromString_LogLevels_e(Level);
		}
		virtual void Fire(IopCpu_c &aIop) { aIop.GetLogger().PushDisplayLogLevel(mLogLevel); }
	protected:
		LogLevels_e mLogLevel;
	};
	class LogLevelPop_c: public BreakPointBase_c {
	public:
		LogLevelPop_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {}
		virtual void Fire(IopCpu_c &aIop) { aIop.GetLogger().PopDisplayLogLevel(); }
	};

	static std::shared_ptr<BreakPoint_i> CreateBreakPoint(const std::string &aBreakPointType, const Configuration_c &aConfig);

	class EventPoints_c: public EventPoints_t<BreakPoint_i> {
	public:
		explicit EventPoints_c(IopCpu_c &aParent): mParent(aParent) {}
	protected:
		virtual void FireBreakPoint(BreakPoint_i *aBreakPoint) override { if (aBreakPoint->Test(mParent.GetDma())) aBreakPoint->Fire(mParent); }
		virtual std::shared_ptr<BreakPoint_i> CreateBreakPoint(const std::string &aBreakPointType, const Configuration_c &aConfig) override { return IopCpu_c::CreateBreakPoint(aBreakPointType, aConfig); }
		IopCpu_c &mParent;
	};
	EventPoints_c mEventPoints;

	RefOr_s RefOrB;
	RefA_s RefA;
	RefC_s RefC;
	RefC_A_s RefC_A;
	RefB_s RefB;

	// Internal registers
	struct State_s {
		explicit State_s(const Configuration_c &aConfig, IopCpu_c &aParent, uint64_t aClockFrequency, uint64_t aTimerLimit):
			A(0),
			Carry(0),
			B(0),
			P(0),
			I(1),
			ISetIsWaiting(false),
			ChannelIOR(aConfig.get_child_safe("IOR"), aParent),
			ChannelPFR(aConfig.get_child_safe("PFR"), aParent),
			ChannelPXS(aConfig.get_child_safe("PXS"), aParent),
			ChannelLME(aConfig.get_child_safe("LME"), aParent),
			ChannelRTC(aConfig.get_child_safe("RTC"), aParent, aClockFrequency, aTimerLimit),
			ChannelMOS(aConfig.get_child_safe("MOS"), aParent)
		{
			memset(OperandRegisters,0,sizeof(OperandRegisters));
			InReset = true;
		}
		IopInt_t A;
		IopBit_t Carry;
		IopOpRegAddr_t B;  // 9-bit operand register address register
		IopInt_t P; // 16-bit Program Address register
		IopInt_t OperandRegisters[512];
		IopBit_t I; // System interrupt enable flag
		bool ISetIsWaiting; // Setting I is delayed until the next non-branch or non-I/O instruction (Part3 6-6)
		AtomicBool InReset;
		ChannelIOR_c ChannelIOR;
		ChannelPFR_c ChannelPFR;
		ChannelPXS_c ChannelPXS;
		ChannelLME_c ChannelLME;
		ChannelRTC_c ChannelRTC;
		ChannelMOS_c ChannelMOS;
	};
	//vector<State_s> mCallStack;
	uint64_t mInstCnt;
	mutable uint64_t mLastInstCnt;

	std::vector<uint8_t> &mIoMemory;
	std::vector<uint8_t> *mBufferMemory;
	class IopCluster_c *mIopCluster;
	State_s mState;
	uint8_t mCpuId;
	IopTypes_e mType;
	size_t mInstructionBurstSize;

	IopInt_t ReadMem(size_t aAddr) {
		IopInt_t RetVal = IoMemAccess<IopInt_t>(aAddr);
		LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
		if (LogLine.good()) LogLine << SideEffectIndent << "read " << HexPrinter(aAddr, 4) << " value: " << HexPrinter(RetVal) << std::endl;
		return RetVal;
	}
	void WriteMem(size_t aAddr, IopInt_t aData) {
		LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
		if (LogLine.good()) LogLine << SideEffectIndent << "write " << HexPrinter(aAddr, 4) << " value: " << HexPrinter(aData) << std::endl;
		IoMemAccess<IopInt_t>(aAddr) = aData;
	}

	std::vector<IopChannel_i *> mIoChannels;
	std::vector<IopChannel_i *> mIoTickChannels;
	std::vector<std::shared_ptr<IopPeripheral_i>> mPeripherals;
	std::map<IopInt_t, std::vector<std::shared_ptr<BreakPoint_i>>> mBreakPoints;
	bool mExists;

	template <bool aDoExecute> IopInt_t Decode(
		IopParcel_t aFirstParcel, 
		IopParcel_t aSecondParcel,
		bool aInSimulation,
		std::ostream &aDisassembly,
		std::ostream &aExplanation,
		bool &aDelayISet,
		bool &aBreakBurst
	);
	template <bool aDoExecute, uint16_t aFieldF> IopInt_t DecodeJump(
		IopParcel_t aFirstParcel,
		IopParcel_t aSecondParcel,
		bool aInSimulation,
		std::ostream &aDisassembly,
		std::ostream &aExplanation,
		bool &aDelayISet,
		bool &aBreakBurst
	);
	template <bool aDoExecute, uint16_t aFieldF> IopInt_t DecodeIo(
		IopParcel_t aFirstParcel,
		IopParcel_t aSecondParcel,
		bool aInSimulation,
		std::ostream &aDisassembly,
		std::ostream &aExplanation,
		bool &aDelayISet,
		bool &aBreakBurst
	);
	template <bool aDoExecute, uint16_t aFieldF> IopInt_t Decode(
		IopParcel_t aFirstParcel,
		IopParcel_t aSecondParcel,
		bool aInSimulation,
		std::ostream &aDisassembly,
		std::ostream &aExplanation,
		bool &aDelayISet,
		bool &aBreakBurst
	);
	typedef IopInt_t (IopCpu_c::* InstImplementation)(
		IopParcel_t aFirstParcel,
		IopParcel_t aSecondParcel,
		bool aInSimulation,
		std::ostream &aDisassembly,
		std::ostream &aExplanation,
		bool &aDelayISet,
		bool &aBreakBurst
	);
	static const struct InstImplementation_s {
		InstImplementation Exec, Disasm;
	} mInstImplementations[];

	void mShiftRight(IopInt_t aAmount);
	void mRotateRight(IopInt_t aAmount);
	void mShiftLeft(IopInt_t aAmount);
	void mRotateLeft(IopInt_t aAmount);
	void Increment(IopInt_t aAmount);
	void Decrement(IopInt_t aAmount);

};

#endif // __CRAY_IOP_H__
