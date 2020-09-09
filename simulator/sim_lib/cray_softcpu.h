#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mutex>
#include "utils.h"
#include "cray_types.h"
#include "cray_cpu.h"
#include <stdint.h>
#include <string.h>
#include <vector>
#include <map>
#include <limits.h>
#include "cray_mainframe.h"
#include "exchange_packet.h"
#include "config_file.h"
#include <boost/circular_buffer.hpp>

// Cray-1S CPU simulator class

// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
//
// To be true to the documentation, octal numbers are extensively used in this file.
// Watch out for the leading 0-s!
//
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!

// NOTE: The #define COLLECT_PERF controls whether performance collection for certain operations (such as jump prediction accuracy, caches, life-time) is collected
//       It is defauled undefined, but can ben defined to enable such statistics
//#define COLLECT_PERF

#ifdef CRAY_UNIMPLEMENTED
#error CRAY_UNIMPLEMENTED is already defined
#endif

#define CRAY_UNKNOWN throw UnknownInstError_x(__FILE__,__LINE__);

#define CRAY_UNIMPLEMENTED                                       \
	do {														 \
		if (!aDoExecute) {										 \
			aDisassembly << "*** UNK ***";						 \
		}														 \
		if (aDoExecute) {										 \
			throw InstUnimplementedError_x(__FILE__,__LINE__);	 \
		}														 \
		return 1;                                                \
	} while (false);


inline size_t FirstParcel(uint64_t aInst) { return size_t((aInst >> 32) & 0xffff); }
inline size_t SecondParcel(uint64_t aInst) { return size_t((aInst >> 16) & 0xffff); }
inline size_t ThirdParcel(uint64_t aInst) { return size_t((aInst >> 0) & 0xffff); }

inline size_t FieldG(uint64_t aInst)    { return (FirstParcel(aInst) & 0170000) >> 12; }
inline size_t FieldH(uint64_t aInst)    { return (FirstParcel(aInst) & 0007000) >> 9; }
inline size_t FieldGH(uint64_t aInst)   { return (FirstParcel(aInst) & 0177000) >> 9; }
inline size_t FieldI(uint64_t aInst)    { return (FirstParcel(aInst) & 0000700) >> 6; }
inline size_t FieldJ(uint64_t aInst)    { return (FirstParcel(aInst) & 0000070) >> 3; }
inline size_t FieldK(uint64_t aInst)    { return (FirstParcel(aInst) & 0000007) >> 0; }
inline size_t FieldJK(uint64_t aInst)   { return (FirstParcel(aInst) & 0000077) >> 0; }
inline size_t FieldIJK(uint64_t aInst)  { return (FirstParcel(aInst) & 0000777) >> 0; }
inline size_t FieldM(uint64_t aInst)    { return SecondParcel(aInst); }
inline size_t FieldJKM(uint64_t aInst)  { return size_t((aInst & 0x003fffff0000) >> 16); }
inline size_t FieldIJKM(uint64_t aInst) { return size_t((aInst & 0x01ffffff0000) >> 16); }
inline size_t FieldMN(uint64_t aInst)   { return SecondParcel(aInst) | (ThirdParcel(aInst) << 16); }

#define EXTRACT_FIELDS(aaInst) \
	size_t g = FieldG(aaInst); \
	size_t h = FieldH(aaInst); \
	size_t gh = FieldGH(aaInst); \
	size_t i = FieldI(aaInst); \
	size_t j = FieldJ(aaInst); \
	size_t k = FieldK(aaInst); \
	size_t jk = FieldJK(aaInst); \
	size_t ijk = FieldIJK(aaInst); \
	size_t m = FieldM(aaInst); \
	size_t jkm = FieldJKM(aaInst); \
	size_t ijkm = FieldIJKM(aaInst); \
	size_t mn = FieldMN(aaInst);

class SoftCpu_c: public Cpu_c {
public:
	explicit SoftCpu_c(const Configuration_c &aConfig, const Configuration_c &aDefaultConfig, Mainframe_c &aMainframe, Mainframe_c::BreakPoints_t &aBreakpoints, size_t aCpuId);
	explicit SoftCpu_c(Mainframe_c &aMainframe);
	virtual ~SoftCpu_c() override {}

	virtual void MasterClear() override; // Enters the CPU into it's reset state, stops executing
	virtual void DeadStart() override;   // Removes the CPU from it's reset state, starts executing

	virtual void SetDeadLockInterrupt() override {
		mNotDeadLockInterrupt.clear();
//		mState.Flags.SetDeadlock();
	}
	virtual void SetIoInterrupt() override {
		mNotIoInterrupt.clear();
//		mState.Flags.SetIoInterrupt();
	}
	virtual void Tick() override;
	virtual bool IsTsBlocked() const override { return mState.TestAndSetBlocked; }
	virtual bool IsSelectedForExternalInterrupts() const override { return mState.Mode.IsSelectedForExternalInterrupts(); }
	virtual uint8_t GetCluster() const override { return mState.Cluster; }

	virtual CProgramAddr_t GetAbsoluteProgramCounter() const override { return mState.InstBaseAddr * 4 + mState.ProgramAddress; }

	virtual void GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const override;
	virtual std::string GetName() const override;
	virtual void Dump(size_t aIdent = 0) const override;
	virtual uint8_t GetExchangePacketAddress() const override { return mState.ExchangePacketAddress; }

	size_t Decode(
		CProgramAddr_t aInstAddr,
		std::ostream &aDisassembly,
		std::ostream &aExplanation,
		std::ostream &aInstCodes,
		std::ostream &aHexInstCodes
	); // Returns the increment to the ProgramAddress. Might throw all sorts of memory access exceptions, or even InstFetchOutOfBoundsError_x if aIsSecondParcelValid == false

	void SetInstBaseAddr(const CAddr_t &aAddr) { mState.InstBaseAddr = aAddr; }
	void SetDataBaseAddr(const CAddr_t &aAddr) { mState.DataBaseAddr = aAddr; }

	virtual void DumpHistory() override;
	virtual void ClearHistory() override { mHistory.clear(); }

#ifdef COLLECT_PERF
	static const size_t mHistSize = 35;
	std::array<uint64_t, mHistSize> GetLoadUseHist() const { return mLoadUseHist; }
	std::array<uint64_t, mHistSize> GetBBHist() const { return mBBHist; }
	virtual uint64_t GetJumpsTaken() const override { return mJumpsTaken; }
	virtual uint64_t GetJumpsNotTaken() const override { return mJumpsNotTaken; }
#endif
	virtual void SetInterProcessorInterrupt() override {
		// TODO: this needs proper testing: originally these interrupts didn't get flagged, unless the target processor was in user mode and monitor-mode interrupts were disabled
		//       i.e. inter-processor interrupts were eaten if the target was not in monitor mode, instead of getting buffered until the interrupt could be served on the target.
		//		if (!IsMonitorMode() && !IsInterruptMonitorMode()) {
		mNotInterProcessorInterrupt.clear();
		//		mState.Flags.SetInterProcessorInterrupt();
		//		}
	}

	virtual void Exchange() override;
	CProgramAddr_t GetProgramCounter() const { return mState.ProgramAddress; }
	bool IsMonitorMode() { return mState.Mode.IsMonitorMode(); }
	CInt_t GetSReg(size_t aRegIdx) const { return mState.S[aRegIdx]; }
	CInt_t GetTReg(size_t aRegIdx) const { return mState.T[aRegIdx]; }
	CAddr_t GetAReg(size_t aRegIdx) const { return mState.A[aRegIdx]; }
	void SetAReg(size_t aRegIdx, CAddr_t aValue) {
		mLogger << setloglevel(LogLevel_InstructionTrace) << "Overriding register A" << DecPrinter(aRegIdx) << " with value " << HexPrinter(aValue) << std::endl;
		mState.A[aRegIdx] = aValue;
	}
	CAddr_t GetBReg(size_t aRegIdx) const { return mState.B[aRegIdx]; }
	CInt_t GetVReg(size_t aRegIdx, size_t aElementIdx) const { return mState.V[aRegIdx][aElementIdx]; }
protected:
	virtual void SetProgrammableClockInterrupt() override;

	std::atomic_flag mNotInterProcessorInterrupt;
	std::atomic_flag mNotDeadLockInterrupt;
	std::atomic_flag mNotMcuInterrupt;
	std::atomic_flag mNotIoInterrupt;
	void InitNotIntFlags() {
		mNotInterProcessorInterrupt.test_and_set();
		mNotDeadLockInterrupt.test_and_set();
		mNotMcuInterrupt.test_and_set();
		mNotIoInterrupt.test_and_set();
	}
	void MirrorExternalInterrupts() {
		if (!mNotInterProcessorInterrupt.test_and_set()) mState.Flags.SetInterProcessorInterrupt();
		if (!mNotDeadLockInterrupt.test_and_set()) mState.Flags.SetDeadlock();
		if (!mNotMcuInterrupt.test_and_set()) mState.Flags.SetMcuInterrupt();
		if (!mNotIoInterrupt.test_and_set()) mState.Flags.SetIoInterrupt();
	}
	void SetMcuInterrupt() {
		mNotMcuInterrupt.clear();
		//		mState.Flags.SetMcuInterrupt();
	}
	bool IsInterruptMonitorMode() { return mState.Mode.IsInterruptMonitorMode(); }
	Mode_c GetMode() const { return mState.Mode; }
	CInt_t GetSR(size_t aSrIdx);
	void SetSR(size_t aSrIdx, CInt_t aValue);
	CAddr_t GetAddrMask() const { return mState.AddrMask; }
	bool IsXMode() const { return mState.XMode; }
	bool IsXmp() const { return ::IsXmp(GetMainframe().GetMachineType()); }
	bool IsJ90OrSV1() const {
		return (GetMainframe().GetMachineType() == MachineTypes_e::J90) || (GetMainframe().GetMachineType() == MachineTypes_e::SV1);
	}
	bool IsSV1() const { return (GetMainframe().GetMachineType() == MachineTypes_e::SV1); }
	void TestBreakPoints();

	Mainframe_c::BreakPoints_t mBreakPoints;

	int32_t SignExtend(const CAddr_t &aA) {
		if (IsXMode()) return ::SignExtend(CXmpAddr_t(aA));
		return ::SignExtend(aA);
	}

	uint64_t mInstCnt;
	mutable uint64_t mLastInstCnt;
	uint64_t mInstCntBase;
	AtomicBool mInReset;
	std::atomic_flag mMasterClearLock;

	struct MemoryPoke_s {
		CAddr_t Addr;
		CInt_t Value;
		bool IsParcelPoke;
	};
	std::vector<MemoryPoke_s> mMemoryPokes;
	uint32_t mTimerIncrement;
	void SetCluster(uint8_t aCluster) {
		if (mState.Cluster != aCluster) {
			if (mState.Cluster != 0) {
//				std::unique_lock<std::mutex> Lock(mMainframe.GetCluster(mState.Cluster - 1).ClusterMutex);
				Lock_c Lock(mMainframe.GetCluster(mState.Cluster - 1).ClusterMutex);
				--mMainframe.GetCluster(mState.Cluster - 1).ClusterCpuCnt;
			}
			if (aCluster != 0) {
//				std::unique_lock<std::mutex> Lock(mMainframe.GetCluster(aCluster - 1).ClusterMutex);
				Lock_c Lock(mMainframe.GetCluster(aCluster - 1).ClusterMutex);
				++mMainframe.GetCluster(aCluster - 1).ClusterCpuCnt;
			}
		}
		mState.Cluster = aCluster;
	}
	uint16_t InterruptMaskFromState() const {
		// First see which interrupts are enabled. This depends on various mode bits
		if (mState.Mode.IsMonitorMode()) {
			// In monitor mode
			if (mState.Mode.IsInterruptMonitorMode()) {
				return
					//Flags_c::InterProcessorInterrupt |
					Flags_c::Deadlock |
					//Flags_c::ProgrammableClockInterrupt |
					Flags_c::McuInterrupt |
					Flags_c::FloatingPointError |
					Flags_c::OperandRangeError |
					Flags_c::ProgramRangeError |
					Flags_c::MemoryError |
					//Flags_c::IoInterrupt |
					Flags_c::ErrorExit |
					Flags_c::NormalExit;
			}
			else {
				return
					(mInReset ? Flags_c::InterProcessorInterrupt : 0) |
					//Flags_c::Deadlock |
					//Flags_c::ProgrammableClockInterrupt |
					Flags_c::McuInterrupt |
					//Flags_c::FloatingPointError |
					//Flags_c::OperandRangeError |
					//Flags_c::ProgramRangeError |
					Flags_c::MemoryError |
					//Flags_c::IoInterrupt |
					Flags_c::ErrorExit |
					Flags_c::NormalExit;
			}
		}
		else {
			// Not in monitor mode
			return
				Flags_c::InterProcessorInterrupt |
				Flags_c::Deadlock |
				Flags_c::ProgrammableClockInterrupt |
				Flags_c::McuInterrupt |
				(mState.Mode.IsFloatingPointErrorMode() ? Flags_c::FloatingPointError : 0) |
				(mState.Mode.IsOperandRangeErrorMode() ? Flags_c::OperandRangeError : 0) |
				Flags_c::ProgramRangeError |
				((mState.Mode.IsCorrectableMemoryErrorMode() | mState.Mode.IsUncorrectableMemoryErrorMode()) ? Flags_c::MemoryError : 0) | //TODO: This is not precise here, but since we don't simulate memory errors, it'll do
				Flags_c::IoInterrupt |
				Flags_c::ErrorExit |
				Flags_c::NormalExit;
		}
	}
	void CalcInterruptMask() {
		mState.InterruptMask.Data = InterruptMaskFromState();
	}
	enum class Reg_e {
		P = 0,
		VL,
		VM,
		XP,
		Exchange,
		Special = 0x0000,
		ABase = 0x1000,
		SBase = 0x2000,
		BBase = 0x3000,
		TBase = 0x4000,
		SBBase = 0x5000,
		STBase = 0x6000,
		VBase = 0x7000,
		BaseMask = 0xff000,
		RegMask = 0xfff
	};
	static std::string RegName(Reg_e aReg) {
		switch (Reg_e(int(aReg) & int(Reg_e::BaseMask))) {
			case Reg_e::Special:
				switch (Reg_e(int(aReg) & int(Reg_e::RegMask))) {
					case Reg_e::P: return "P";
					case Reg_e::VL: return "VL";
					case Reg_e::VM: return "VM";
					case Reg_e::XP: return "XP";
					case Reg_e::Exchange: return "Exchange";
					default: CRAY_ASSERT(false);
				}
			case Reg_e::ABase: {
				std::stringstream Strm;
				int RegIdx = int(aReg) & int(Reg_e::RegMask);
				CRAY_ASSERT(RegIdx < 8);
				Strm << "A" << DecPrinter(RegIdx);
				return Strm.str();
			}
			case Reg_e::SBase: {
				std::stringstream Strm;
				int RegIdx = int(aReg) & int(Reg_e::RegMask);
				CRAY_ASSERT(RegIdx < 8);
				Strm << "S" << DecPrinter(RegIdx);
				return Strm.str();
			}
			case Reg_e::BBase: {
				std::stringstream Strm;
				int RegIdx = int(aReg) & int(Reg_e::RegMask);
				CRAY_ASSERT(RegIdx < 64);
				Strm << "B" << DecPrinter(RegIdx);
				return Strm.str();
			}
			case Reg_e::TBase: {
				std::stringstream Strm;
				int RegIdx = int(aReg) & int(Reg_e::RegMask);
				CRAY_ASSERT(RegIdx < 64);
				Strm << "T" << DecPrinter(RegIdx);
				return Strm.str();
			}
			case Reg_e::SBBase: {
				std::stringstream Strm;
				int RegIdx = int(aReg) & int(Reg_e::RegMask);
				CRAY_ASSERT(RegIdx < 64);
				Strm << "SB" << DecPrinter(RegIdx);
				return Strm.str();
			}
			case Reg_e::STBase: {
				std::stringstream Strm;
				int RegIdx = int(aReg) & int(Reg_e::RegMask);
				CRAY_ASSERT(RegIdx < 64);
				Strm << "ST" << DecPrinter(RegIdx);
				return Strm.str();
			}
			case Reg_e::VBase: {
				std::stringstream Strm;
				int RegIdx = int(aReg) & int(Reg_e::RegMask);
				int VReg = RegIdx / 64;
				RegIdx %= 64;
				Strm << "V" << DecPrinter(VReg) << "[" << DecPrinter(RegIdx) << "]";
				return Strm.str();
			}
			default: CRAY_ASSERT(false);
		}
		throw Generic_x("Invalid register type");
	}
	size_t RegSize(Reg_e aReg) {
		switch (Reg_e(int(aReg) & int(Reg_e::BaseMask))) {
			case Reg_e::Special:
				switch (Reg_e(int(aReg) & int(Reg_e::RegMask))) {
					case Reg_e::P: return IsXMode() ? 24 : 32;
					case Reg_e::VL: return 5;
					case Reg_e::VM: return 64;
					case Reg_e::XP: return 9;
					default: CRAY_ASSERT(false);
				}
			case Reg_e::ABase: return IsXMode() ? 24 : 32;
			case Reg_e::SBase: return 64;
			case Reg_e::BBase: return IsXMode() ? 24 : 32;
			case Reg_e::TBase: return 64;
			case Reg_e::SBBase: return IsXMode() ? 24 : 32;
			case Reg_e::STBase: return 64;
			case Reg_e::VBase: return 64;
			default: CRAY_ASSERT(false);
		}
		throw Generic_x("Invalid register type");
	}

	struct StateChange_s {
		StateChange_s(Reg_e aReg, CInt_t aVal) : Reg(aReg), Val(aVal) {};
		StateChange_s(Reg_e aReg, CAddr_t aVal) : Reg(aReg), Val(aVal) {};
		StateChange_s(Reg_e aReg, CAddr_t aVal1, CAddr_t aVal2) : Reg(aReg), Val(aVal1), Val2(aVal2) {};
		StateChange_s(size_t aReg, CInt_t aVal) : Reg(Reg_e(aReg)), Val(aVal) {};
		StateChange_s(size_t aReg, CAddr_t aVal) : Reg(Reg_e(aReg)), Val(aVal) {};
		Reg_e Reg;
		CAddr_t Val2;
		CInt_t Val;
	};

#ifndef CRAY_TURBO
	void PushHistory(const StateChange_s &aElem) { mHistory.push_back(aElem); }
#else
	void PushHistory(const StateChange_s &aElem) {}
#endif
	boost::circular_buffer<StateChange_s> mHistory;

	struct CpuState_s {
		CpuState_s() {
			for(size_t i=0;i<sizeof(A)/sizeof(A[0]);++i) { A[i] = 0; ALastChange[i] = 0; }
			for(size_t i=0;i<sizeof(B)/sizeof(B[0]);++i) { B[i] = 0; BLastChange[i] = 0; }
			for(size_t i=0;i<sizeof(S)/sizeof(S[0]);++i) { S[i] = 0; SLastChange[i] = 0; }
			for(size_t i=0;i<sizeof(T)/sizeof(T[0]);++i) { T[i] = 0; TLastChange[i] = 0; }
			for (size_t i = 0; i < sizeof(V) / sizeof(V[0]); ++i) { for (size_t j = 0; j < 64; ++j) { V[i][j] = 0; } VLastChange[i] = 0; }
			VL = 0;
			VM = 0;
			ExchangePacketAddress = 0;
			ProgramAddress = 0;
			InstBaseAddr = 0;
			InstLimitAddr = 0;
			DataBaseAddr = 0;
			DataLimitAddr = 0;			
			//Mode;
			//Flags;
			PeriodicInterruptLimit = 0;
			PeriodicInterruptCountDown = 0;
			PeriodicInterruptEnabled = false;
			Cluster = 0;
			TestAndSetBlocked = false;
			VectorNotUsed = true;
			EnableSecondVectorLogical = false;
			XmpProgramState = false;
			XmpEnhancedAddressingMode = false;
			AddrMask = std::numeric_limits<CAddr_t>::max();
			XMode = false;
		}

		// ISA-visible registers
		CAddr_t         A[8];                                // 8 primary address registers
		uint64_t        ALastChange[8];
		CAddr_t         B[64];                               // 64 address (branch) registers
		uint64_t        BLastChange[64];

		CInt_t          S[8];                                // 8 primary scalar registers
		uint64_t        SLastChange[8];
		CInt_t          T[64];                               // 64 backup scalar registers
		uint64_t        TLastChange[64];

		CVec_t          V[8];                                // 8 64-entry vector registers
		uint64_t        VLastChange[8];
		uint8_t         VL;                                  // vector length register (TODO: this is a 7-bit register) - value of 0100 is the same as 0000, i.e. 0 specifies maximum length as well
		CInt_t          VM;                                  // 64-bit vector-mask register

		// Internal state registers - these might have ISA access to them, but are not used extensively. Using human-readable names here
		uint8_t         ExchangePacketAddress;               // exchange packet pointer
		CProgramAddr_t  ProgramAddress;                      // instruction pointer

		CAddr_t         InstBaseAddr;                        // instruction base-address register (TODO: this is an 18-bit register. The lower 4 bits are 0)
		CAddr_t         InstLimitAddr;                       // instruction limit address register (TODO: this is an 18-bit register. The lower 4 bits are 0)

		CAddr_t         DataBaseAddr;                        // data base-address register (TODO: this is an 18-bit register. The lower 4 bits are 0)
		CAddr_t         DataLimitAddr;                       // data limit address register (TODO: this is an 18-bit register. The lower 4 bits are 0)

		Mode_c          Mode;                                // mode register
		Flags_c         Flags;                               // interrupt flags register
		Flags_c			InterruptMask;                       // cached interrupt mask

		uint32_t        PeriodicInterruptLimit;              // periodic interrupt limit register
		uint32_t        PeriodicInterruptCountDown;          // periodic interrupt countdown register
		bool            PeriodicInterruptEnabled;            // enables/disables periodic interrupts
		uint8_t         Cluster;                             // current cluster (actual limits depend on X-MP model 1x, 2x or 4x)
		bool            TestAndSetBlocked;                   // set if the CPU is blocked on a test-and-set operation
		bool            VectorNotUsed;                       // set once a vector instructions is executed. Cleared on reset and potentially though an EP exchange
		bool            EnableSecondVectorLogical;
		bool            XmpProgramState;                     // TODO: does it get preserved?
		CVec_t			BitMatrix;
		//bool            InReset;
		CAddr_t         AddrMask;
		bool            XMode;
		bool            XmpEnhancedAddressingMode;
	} mState;

	std::vector<CpuState_s> mCallStack;

#ifdef COLLECT_PERF
	std::array<uint64_t, mHistSize> mLoadUseHist;
	std::array<uint64_t, mHistSize> mBBHist;
    uint64_t mLastJump;
	uint64_t mJumpsTaken;
	uint64_t mJumpsNotTaken;
#endif

	void UpdateJumpStats(bool aTaken) {
#ifdef COLLECT_PERF
        CRAY_ASSERT(mLastJump <= mCycleCount);
		uint64_t BBLen = mCycleCount - mLastJump;
        CRAY_ASSERT(mBBHist.size() > 0);
		BBLen = std::min(BBLen, uint64_t(mBBHist.size() - 1));
		mBBHist[BBLen]++;
		if (aTaken) {
			++mJumpsTaken;
		} else {
			++mJumpsNotTaken;
		}
		mLastJump = mCycleCount + 1; // Start counting from the following cycle
#endif
    }

	void UpdateLoadUseStats(uint64_t aLastChange) {
#ifdef COLLECT_PERF
		CRAY_ASSERT(aLastChange <= mCycleCount);
		aLastChange = mCycleCount - aLastChange;
        CRAY_ASSERT(mLoadUseHist.size() > 0);
		aLastChange = std::min(aLastChange, uint64_t(mLoadUseHist.size()-1));
		mLoadUseHist[aLastChange]++;
#endif
    }

	#define RefA0 RefAx_s(0, *this, false, CAddr_t(0))
	#define RefAh RefAx_s(h, *this, true,  CAddr_t(0))
	#define RefAi RefAx_s(i, *this, false, CAddr_t(0))
	#define RefAj RefAx_s(j, *this, true,  CAddr_t(0))
	#define RefAk RefAx_s(k, *this, true,  CAddr_t(1))
	#define RefA0Target RefAx_s(0, *this, false, CAddr_t(0))
	#define RefAhTarget RefAx_s(h, *this, false, CAddr_t(0))
	#define RefAiTarget RefAx_s(i, *this, false, CAddr_t(0))
	#define RefAjTarget RefAx_s(j, *this, false, CAddr_t(0))
	#define RefAkTarget RefAx_s(k, *this, false, CAddr_t(0))
	struct RefAx_s: public FieldFormatter_i {
		RefAx_s(size_t aRegIdx, SoftCpu_c &aParent, bool aHasSpecialValue, CAddr_t aSpecialValue):
			mRegIdx(aRegIdx),
			mParent(aParent),
			mHasSpecialValue(aHasSpecialValue),
			mSpecialValue(aSpecialValue)
		{}
		RefAx_s &operator=(CAddr_t aValue) {
			aValue &= mParent.GetAddrMask();
			LogLine_c LogLine = mParent.mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << HexPrinter(aValue) << " (" << DecPrinter(aValue,0) << ")" << std::endl;
			mParent.PushHistory(StateChange_s(int(Reg_e::ABase) + mRegIdx, aValue));
			mParent.mState.A[mRegIdx] = aValue;
			mParent.UpdateLoadUseStats(mParent.mState.ALastChange[mRegIdx]);
			mParent.mState.ALastChange[mRegIdx] = mParent.mCycleCount;
			return *this;
		}
		operator CAddr_t() const { if (mRegIdx == 0 && mHasSpecialValue) return CAddr_t(mSpecialValue); else return mParent.GetAddrMask() & mParent.mState.A[mRegIdx]; }
		virtual void Print(std::ostream &aStream) const { if (mRegIdx == 0 && mHasSpecialValue) aStream << DecPrinter(mSpecialValue,0); else aStream << "A" << DecPrinter(mRegIdx,0); }
	protected:
		size_t mRegIdx;
		SoftCpu_c &mParent;
		bool mHasSpecialValue;
		CAddr_t mSpecialValue;
	};

	#define RefS0 RefSx_s(0, *this, false, 0)
	#define RefSi RefSx_s(i, *this, false, 0)
	#define RefSj RefSx_s(j, *this, true,  0)
	#define RefSk RefSx_s(k, *this, true,  0x8000000000000000ULL)
	#define RefSiTarget RefSx_s(i, *this, false, 0)
	struct RefSx_s: public FieldFormatter_i {
		RefSx_s(size_t aRegIdx, SoftCpu_c &aParent, bool aHasSpecialValue, CInt_t aSpecialValue):
			mRegIdx(aRegIdx),
			mParent(aParent),
			mHasSpecialValue(aHasSpecialValue),
			mSpecialValue(aSpecialValue)
		{}
		RefSx_s &operator=(CInt_t aValue) {
			LogLine_c LogLine = mParent.mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << HexPrinter(aValue) << " (" << DecPrinter(aValue, 0) << ")" << std::endl;
			mParent.PushHistory(StateChange_s(int(Reg_e::SBase) + mRegIdx, aValue));
			mParent.mState.S[mRegIdx] = aValue;
			mParent.UpdateLoadUseStats(mParent.mState.SLastChange[mRegIdx]);
			mParent.mState.SLastChange[mRegIdx] = mParent.mCycleCount;
			return *this;
		}
		CInt_t Value() const { if (mRegIdx == 0 && mHasSpecialValue) return mSpecialValue; else return mParent.mState.S[mRegIdx]; }
		operator CInt_t() const { return Value(); }
//		operator CFloat_t() const { return CFloat_t(Value()); }
		virtual void Print(std::ostream &aStream) const {
			if (mRegIdx == 0 && mHasSpecialValue) {
				if (mSpecialValue != 0) aStream << HexPrinter(mSpecialValue); else aStream << "0";
			} else {
				aStream << "S" << DecPrinter(mRegIdx,0);
			}
		}
	protected:
		size_t mRegIdx;
		SoftCpu_c &mParent;
		bool mHasSpecialValue;
		CInt_t mSpecialValue;
	};


	#define RefBjk RefBjk_s(jk, *this)
	#define RefB(x) RefBjk_s(x, *this)
	#define RefBjkTarget RefBjk_s(jk, *this)
	#define RefBTarget(x) RefBjk_s(x, *this)
	struct RefBjk_s: public FieldFormatter_i {
		RefBjk_s(size_t aRegIdx, SoftCpu_c &aParent):
			mRegIdx(aRegIdx),
			mParent(aParent)
		{}
		RefBjk_s &operator=(CAddr_t aValue) {
			aValue &= mParent.GetAddrMask();
			LogLine_c LogLine = mParent.mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << HexPrinter(aValue) << " (" << DecPrinter(aValue, 0) << ")" << std::endl;
			mParent.PushHistory(StateChange_s(int(Reg_e::BBase) + mRegIdx, aValue));
			mParent.mState.B[mRegIdx] = aValue;
			mParent.UpdateLoadUseStats(mParent.mState.BLastChange[mRegIdx]);
			mParent.mState.BLastChange[mRegIdx] = mParent.mCycleCount;
			return *this;
		}
		operator CAddr_t() const { return mParent.GetAddrMask() & mParent.mState.B[mRegIdx]; }
		virtual void Print(std::ostream &aStream) const { aStream << "B" << DecPrinter(mRegIdx, 0); }
	protected:
		size_t mRegIdx;
		SoftCpu_c &mParent;
	};

	#define RefTjk RefTjk_s(jk, *this)
	#define RefT(x) RefTjk_s(x, *this)
	#define RefTjkTarget RefTjk_s(jk, *this)
	#define RefTTarget(x) RefTjk_s(x, *this)
	struct RefTjk_s: public FieldFormatter_i {
		RefTjk_s(size_t aRegIdx, SoftCpu_c &aParent):
			mRegIdx(aRegIdx),
			mParent(aParent)
		{}
		RefTjk_s &operator=(CInt_t aValue) {
			LogLine_c LogLine = mParent.mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << HexPrinter(aValue) << " (" << DecPrinter(aValue, 0) << ")" << std::endl;
			mParent.PushHistory(StateChange_s(int(Reg_e::TBase) + mRegIdx, aValue));
			mParent.mState.T[mRegIdx] = aValue;
			mParent.UpdateLoadUseStats(mParent.mState.TLastChange[mRegIdx]);
			mParent.mState.TLastChange[mRegIdx] = mParent.mCycleCount;
			return *this;
		}
		operator CInt_t() const { return mParent.mState.T[mRegIdx]; }
		virtual void Print(std::ostream &aStream) const { aStream << "T" << DecPrinter(mRegIdx,0); }
	protected:
		size_t mRegIdx;
		SoftCpu_c &mParent;
	};

	#define RefSBj RefSBj_s(j, *this)
	#define RefSBjTarget RefSBj_s(j, *this)
	struct RefSBj_s: public FieldFormatter_i {
		RefSBj_s(size_t aRegIdx, SoftCpu_c &aParent):
			mRegIdx(aRegIdx),
			mParent(aParent)
		{}
		RefSBj_s &operator=(CAddr_t aValue) {
			aValue &= mParent.GetAddrMask();
			CRAY_ASSERT(mParent.mState.Cluster > 0);
			LogLine_c LogLine = mParent.mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << HexPrinter(aValue) << " (" << DecPrinter(aValue, 0) << ")" << std::endl;
			mParent.PushHistory(StateChange_s(int(Reg_e::SBBase) + mRegIdx, aValue));
			mParent.mMainframe.GetCluster(mParent.mState.Cluster-1).SB[mRegIdx] = aValue;
			return *this;
		}
		operator CAddr_t() const { CRAY_ASSERT(mParent.mState.Cluster > 0); return mParent.GetAddrMask() & mParent.mMainframe.GetCluster(mParent.mState.Cluster-1).SB[mRegIdx]; }
		virtual void Print(std::ostream &aStream) const { aStream << "SB" << DecPrinter(mRegIdx,0); }
	protected:
		size_t mRegIdx;
		SoftCpu_c &mParent;
	};

	#define RefSTj RefSTj_s(j, *this)
	#define RefSTjTarget RefSTj_s(j, *this)
	struct RefSTj_s: public FieldFormatter_i {
		RefSTj_s(size_t aRegIdx, SoftCpu_c &aParent):
			mRegIdx(aRegIdx),
			mParent(aParent)
		{}
		RefSTj_s &operator=(CInt_t aValue) {
			CRAY_ASSERT(mParent.mState.Cluster > 0);
			LogLine_c LogLine = mParent.mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << HexPrinter(aValue) << " (" << DecPrinter(aValue, 0) << ")" << std::endl;
			mParent.PushHistory(StateChange_s(int(Reg_e::STBase) + mRegIdx, aValue));
			mParent.mMainframe.GetCluster(mParent.mState.Cluster-1).ST[mRegIdx] = aValue;
			return *this;
		}
		operator CInt_t() const { CRAY_ASSERT(mParent.mState.Cluster > 0); return mParent.mMainframe.GetCluster(mParent.mState.Cluster-1).ST[mRegIdx]; }
		virtual void Print(std::ostream &aStream) const { aStream << "ST" << DecPrinter(mRegIdx,0); }
	protected:
		size_t mRegIdx;
		SoftCpu_c &mParent;
	};

//	#define RefSM RefSM_s(*this)
//	#define RefSMTarget RefSM_s(*this)
//	struct RefSM_s: public FieldFormatter_i {
//		RefSM_s(SoftCpu_c &aParent):
//			mParent(aParent)
//		{}
//		RefSM_s &operator=(uint64_t aValue) {
//			CRAY_ASSERT(mParent.mState.Cluster > 0);
//			LogLine_c LogLine = mParent.mLogger << setloglevel(LogLevel_SideEffects);
//			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << HexPrinter(aValue) << " in cluster: " << DecPrinter(mParent.mState.Cluster) << std::endl;
//			auto &Cluster = mParent.mMainframe.GetCluster(mParent.mState.Cluster - 1);
//			for (int i = 0; i < 32; ++i) {
//				Cluster.SM[i].store(GetBit(aValue, 63 - i) != 0));
//			}
//			return *this;
//		}
//		operator uint64_t() const {
//			CRAY_ASSERT(mParent.mState.Cluster > 0);
//			uint64_t RetVal = 0;
//			auto &Cluster = mParent.mMainframe.GetCluster(mParent.mState.Cluster - 1);
//			for (int i = 0; i < 32; ++i) {
//				if (Cluster.SM[i].load()) RetVal |= (1ULL << 32);
//				RetVal <<= 1;
//			}
//			return RetVal;
//		}
//		virtual void Print(std::ostream &aStream) const { aStream << "SM"; }
//	protected:
//		SoftCpu_c &mParent;
//	};

	#define RefSRj RefSRj_s(j, *this)
	#define RefSRjTarget RefSRj_s(j, *this)
	struct RefSRj_s: public FieldFormatter_i {
		RefSRj_s(size_t aRegIdx, SoftCpu_c &aParent):
			mRegIdx(aRegIdx),
			mParent(aParent)
		{}
		operator CInt_t() const { return mParent.GetSR(mRegIdx); }
		virtual void Print(std::ostream &aStream) const { aStream << "SR" << DecPrinter(mRegIdx,0); }
	protected:
		size_t mRegIdx;
		SoftCpu_c &mParent;
	};

	#define RefVi RefVx_s(i, *this)
	#define RefVj RefVx_s(j, *this)
	#define RefVk RefVx_s(k, *this)
	#define RefViTarget RefVx_s(i, *this)
	#define RefVjTarget RefVx_s(j, *this)
	#define RefVkTarget RefVx_s(k, *this)
	struct RefVx_s: public FieldFormatter_i {
		RefVx_s(size_t aRegIdx, SoftCpu_c &aParent):
			mRegIdx(aRegIdx),
			mParent(aParent)
		{}
		void LogState(const char *aPrefix = nullptr) {
			LogLine_c LogLine = mParent.mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) {
				LogLine << SideEffectIndent << (aPrefix == nullptr ? "state-change " : aPrefix) << *this << " <== (";
				for (size_t i = 0; i<mParent.mState.V[mRegIdx].size(); ++i) LogLine << (i == 0 ? " " : ", ") << HexPrinter(mParent.mState.V[mRegIdx][i]);
				LogLine << ")" << std::endl;
			}
			mParent.UpdateLoadUseStats(mParent.mState.VLastChange[mRegIdx]);
			mParent.mState.VLastChange[mRegIdx] = mParent.mCycleCount;
			for (size_t i = 0; i < mParent.mState.V[mRegIdx].size(); ++i) {
				mParent.PushHistory(StateChange_s(int(Reg_e::VBase) + mRegIdx * 64 + i, mParent.mState.V[mRegIdx][i]));
			}
		}
		const CInt_t operator [](size_t aIdx) const { return mParent.mState.V[mRegIdx][aIdx]; }
		CInt_t operator [](size_t aIdx) {
			mParent.mState.VectorNotUsed = false;
			return mParent.mState.V[mRegIdx][aIdx];
		}
		virtual void Print(std::ostream &aStream) const {
			aStream << "V" << DecPrinter(mRegIdx,0);
		}
	protected:
		size_t mRegIdx;
		SoftCpu_c &mParent;
	};

	#define RefBM RefBM_s(*this)
	struct RefBM_s : public FieldFormatter_i {
		RefBM_s(SoftCpu_c &aParent):
			mParent(aParent)
		{}
		void LogState(const char *aPrefix = nullptr) {
			LogLine_c LogLine = mParent.mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) {
				LogLine << SideEffectIndent << (aPrefix == nullptr ? "state-change " : aPrefix) << *this << " <== (";
				for (size_t i = 0; i<mParent.mState.BitMatrix.size(); ++i) LogLine << (i == 0 ? " " : ", ") << HexPrinter(mParent.mState.BitMatrix[i]);
				LogLine << ")" << std::endl;
			}
		}
		const CInt_t operator [](size_t aIdx) const { return mParent.mState.BitMatrix[aIdx]; }
		CInt_t operator [](size_t aIdx) {
			mParent.mState.VectorNotUsed = false;
			return mParent.mState.BitMatrix[aIdx];
		}
		virtual void Print(std::ostream &aStream) const {
			aStream << "BM";
		}
	protected:
		SoftCpu_c &mParent;
	};

	#define RefVM RefVM_s(*this)
	struct RefVM_s: public FieldFormatter_i {
		RefVM_s(SoftCpu_c &aParent):
			mParent(aParent)
		{}
		RefVM_s &operator=(CInt_t aValue) {
			LogLine_c LogLine = mParent.mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << HexPrinter(aValue) << std::endl;
			mParent.mState.VM = aValue;
			mParent.mState.VectorNotUsed = false;
			return *this;
		}
		operator CInt_t() const { return mParent.mState.VM; }
		virtual void Print(std::ostream &aStream) const {
			aStream << "VM";
		}
	protected:
		SoftCpu_c &mParent;
	};

	#define RefVL RefVL_s(*this)
	struct RefVL_s: public FieldFormatter_i {
		RefVL_s(SoftCpu_c &aParent):
			mParent(aParent)
		{}
		RefVL_s &operator=(uint8_t aValue) {
			LogLine_c LogLine = mParent.mLogger << setloglevel(LogLevel_SideEffects);
			if (LogLine.good()) LogLine << SideEffectIndent << "state-change " << *this << " <== " << DecPrinter(aValue, 0) << std::endl;
			mParent.mState.VL = aValue;
			return *this;
		}
		operator uint8_t() const { return mParent.mState.VL; }
		virtual void Print(std::ostream &aStream) const {
			aStream << "VL";
		}
	protected:
		SoftCpu_c &mParent;
	};

	size_t mInstructionBurstSize;

	uint64_t mCycleCount;

	ExchangePacket_c CreateExchangePacket(); // Creates an exchange packet from the current state of the CPU
	void RestoreFromExchangePacket(const ExchangePacket_c &aEP); // Restores state from the specified exchange packet
	bool SingleStep();
	bool CheckInterrupts();
	void HandleCounters();

	std::string ReportHist() const;

	// These routines use IBA and DBA offsets, checks for out-of-boundary conditions and raise exceptions on failures
	struct InstFetchOutOfBoundsError_x: public Generic_x { InstFetchOutOfBoundsError_x(): Generic_x("Instruction fetch out of bounds") {} };
	struct DataReadOutOfBoundsError_x: public Generic_x { DataReadOutOfBoundsError_x() : Generic_x("Data read out of bounds") {} };
	struct DataWriteOutOfBoundsError_x: public Generic_x { DataWriteOutOfBoundsError_x() : Generic_x("Data write out of bounds") {} };
	struct InstDecodeError_x : public Generic_x { InstDecodeError_x() : Generic_x("Instruction decode error") {} };
	struct UnknownInstError_x : public Generic_x { UnknownInstError_x(const char *aFile, size_t aLine) : Generic_x("Unknown instruction error at") { *this << aFile << ":" << DecPrinter(aLine, 0); } };
	struct TerminateInstError_x : public Generic_x { TerminateInstError_x() : Generic_x("Terminate instruction executed") {} };
	struct InstExecError_x : public Generic_x { InstExecError_x() : Generic_x("Instruction execution error") {} };
	struct InstUnimplementedError_x : public Generic_x { InstUnimplementedError_x(const char *aFile, size_t aLine) { *this << "Unimplemented instruction at " << aFile << ":" << DecPrinter(aLine, 0); ResetSpacePrinted(); } };

	CInt_t ReadDataMem(CAddr_t aAddr);
	void WriteDataMem(CAddr_t aAddr, CAddr_t aData) { WriteDataMem(aAddr, CInt_t(ZeroExtend(aData))); }
	#ifndef CRAY_TURBO
	virtual void WriteDataMem(CAddr_t aAddr, CInt_t aData);
	#else
	void WriteDataMem(CAddr_t aAddr, CInt_t aData);
	#endif // CRAY_TURBO
	virtual ExchangePacket_c ReadExchangePacket(CAddr_t aMemAddr);
	virtual void WriteExchangePacket(const ExchangePacket_c &aExchangePacket, CAddr_t aMemAddr);

	struct InstMemRead_s {
		uint64_t Data;
		size_t Size;
	};
	InstMemRead_s ReadInstMem(CProgramAddr_t aAddr);

	template <bool aDoExecute> size_t Decode(
		uint64_t aParcels,
		size_t aMaxParcelCnt,
		std::ostream &aDisassembly,
		std::ostream &aExplanation,
		bool &aBreakBurst
	); // Returns the increment to the ProgramAddress. Might throw all sorts of memory access exceptions, or even InstFetchOutOfBoundsError_x if aIsSecondParcelValid == false

	template <bool aDoExecute> size_t Decode0000(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0001(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0002(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0003(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0004(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0005(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0006(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0007(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode0010(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0011(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0012(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0013(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0014(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0015(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0016(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0017(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode0020(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0021(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0022(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0023(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0024(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0025(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0026(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0027(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode0030(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0031(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0032(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0033(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0034(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0035(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0036(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0037(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode0040(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0041(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0042(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0043(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0044(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0045(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0046(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0047(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode0050(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0051(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0052(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0053(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0054(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0055(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0056(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0057(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode0060(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0061(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0062(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0063(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0064(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0065(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0066(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0067(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode0070(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0071(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0072(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0073(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0074(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0075(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0076(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0077(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode010x(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode011x(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode012x(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode013x(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode0140(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0141(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0142(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0143(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0144(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0145(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0146(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0147(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode0150(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0151(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0152(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0153(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0154(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0155(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0156(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0157(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode0160(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0161(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0162(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0163(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0164(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0165(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0166(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0167(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	template <bool aDoExecute> size_t Decode0170(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0171(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0172(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0173(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0174(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0175(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0176(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);
	template <bool aDoExecute> size_t Decode0177(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst);

	typedef size_t (SoftCpu_c::* InstImplementation)(
		uint64_t aParcels,
		size_t aMaxParcelCnt,
		std::ostream &aDisassembly,
		std::ostream &aExplanation,
		bool &aBreakBurst
	);
	static const struct InstImplementation_s {
		InstImplementation Exec, Disasm;
	} mInstImplementations[];


	static CAddr_t PopulationCnt(CInt_t aS) {
		CInt_t Mask = 1;
		CAddr_t RetVal = CAddr_t(0);
		for(int i=0;i<64;++i) {
			if ((aS & Mask) != 0) ++RetVal;
			Mask <<= 1;
		}
		return RetVal;
	}

	static CAddr_t PopulationCntParity(CInt_t aS) {
		return PopulationCnt(aS) & CAddr_t(1);
	}

	static CAddr_t LeadingZeroCnt(CInt_t aS) {
		CInt_t Mask = 0x8000000000000000ULL;
		CAddr_t RetVal(0);
		for(int i=0;i<64;++i) {
			if ((aS & Mask) != 0) return RetVal;
			++RetVal;
			Mask >>= 1;
		}
		return RetVal;
	}

	static CInt_t ShiftRight(CInt_t aValue, CAddr_t aAmount) {
		if (aAmount >= CAddr_t(64)) return 0;
		return ((uint64_t)aValue) >> size_t(aAmount);
	}
	static CInt_t ShiftLeft(CInt_t aValue, CAddr_t aAmount) {
		if (aAmount >= CAddr_t(64)) return 0;
		return aValue << size_t(aAmount);
	}
	static CInt_t DoubleShiftRight(CInt_t aFirst, CInt_t aSecond, CAddr_t aAmount) {
		if (aAmount >= CAddr_t(128)) return 0;
		if (aAmount >= CAddr_t(64)) return ((uint64_t)aFirst >> (size_t(aAmount)-64));
		if (aAmount == CAddr_t(0)) return aSecond;
		return ((uint64_t)aSecond >> size_t(aAmount)) | (aFirst << (64-size_t(aAmount)));
	}

	static CInt_t DoubleShiftLeft(CInt_t aFirst, CInt_t aSecond, CAddr_t aAmount) {
		if (aAmount >= CAddr_t(128)) return 0;
		if (aAmount >= CAddr_t(64)) return (aSecond << (size_t(aAmount)-64));
		if (aAmount == CAddr_t(0)) return aFirst;
		return (aFirst << size_t(aAmount)) | ((uint64_t)aSecond >> (64-size_t(aAmount)));
	}

	static std::string CodeSymbol(CAddr_t aBaseAddr, CProgramAddr_t aProgAddr) {
		std::stringstream StrStrm;
		StrStrm << HexPrinter(aProgAddr / 4 + aBaseAddr) << ":p" << DecPrinter(aProgAddr % 4);
		return StrStrm.str();
	}

	static std::string DataSymbol(CAddr_t aBaseAddr, CAddr_t aAddr) {
		std::stringstream StrStrm;
		StrStrm << HexPrinter(aAddr);
		return StrStrm.str();
	}
};

