// Cray-1S CPU simulator class

// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
//
// To be true to the documentation, octal numbers are extensively used in this file.
// Watch out for the leading 0-s!
//
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "utils.h"
#include "cray_types.h"
#include "cray_softcpu.h"
#include "sys_task_req.h"
#include "cray_mainframe.h"

// Include other implementation files (that's needed for proper template instantiation)
#include "cray_cpu_inst_0000_0037.h"
#include "cray_cpu_inst_0040_0077.h"
#include "cray_cpu_inst_0100_0137.h"
#include "cray_cpu_inst_0140_0177.h"

// SoftCpu_c
//////////////////////////////////////
SoftCpu_c::SoftCpu_c(const Configuration_c &aConfig, const Configuration_c &aDefaultConfig, Mainframe_c &aMainframe, Mainframe_c::BreakPoints_t &aBreakPoints, size_t aCpuId):
	Cpu_c(aConfig, aMainframe, aCpuId),
	mInstCnt(0), 
	mLastInstCnt(0),
	mInReset(true),
	mBreakPoints(aBreakPoints),
	mHistory(aConfig.get<int>("HistoryBuffer", aDefaultConfig.get<int>("HistoryBuffer", 1024))),
	mCycleCount(0)
#ifdef COLLECT_PERF
	,
	mLastJump(0),
	mJumpsTaken(0),
	mJumpsNotTaken(0)
#endif
{
#ifdef COLLECT_PERF
	std::fill(mLoadUseHist.begin(), mLoadUseHist.end(), 0);
	std::fill(mBBHist.begin(), mBBHist.end(), 0);
#endif

	mMasterClearLock.clear();
	mLogger.SetParent(aMainframe.GetLogger());
	MasterClear();
	boost::optional<std::string> DefaultLogLevel = aConfig.get_optional<std::string>("DefaultLogLevel");
	if (!DefaultLogLevel.is_initialized()) DefaultLogLevel = aDefaultConfig.get_optional<std::string>("DefaultLogLevel");
	if (DefaultLogLevel.is_initialized()) {
			mLogger.SetDisplayLogLevel(FromString_LogLevels_e(DefaultLogLevel.get()));
	}
	mInstructionBurstSize = aConfig.get<size_t>("InstructionBurstSize", aDefaultConfig.get<size_t>("InstructionBurstSize", 100)) - 1;
	for (auto &Child : aDefaultConfig.get_child_safe("MemoryPokes")) {
		MemoryPoke_s Poke;
		Poke.Value = FromString<CInt_t>(Child.second.data());
		try {
			Poke.Addr = FromString(Child.first, StringFormat_e::ProgramAddr);
			if (size_t(Poke.Addr) * sizeof(CParcel_t) >= mMainframe.GetMemorySize()) throw InvalidParameter_x("Poke address outside of memory");
			Poke.IsParcelPoke = true;
			mMemoryPokes.push_back(Poke);
		}
		catch (StringFormatError_x &) {
			Poke.Addr = FromString(Child.first, StringFormat_e::DataAddr);
			if (size_t(Poke.Addr) * sizeof(CInt_t) >= mMainframe.GetMemorySize()) throw InvalidParameter_x("Poke address outside of memory");
			Poke.IsParcelPoke = false;
			mMemoryPokes.push_back(Poke);
		}
	}
	for (auto &Child: aConfig.get_child_safe("MemoryPokes")) {
		MemoryPoke_s Poke;
		Poke.Value = FromString<CInt_t>(Child.second.data());
		try {
			Poke.Addr = FromString(Child.first, StringFormat_e::ProgramAddr);
			if (size_t(Poke.Addr) * sizeof(CParcel_t) >= mMainframe.GetMemorySize()) throw InvalidParameter_x("Poke address outside of memory");
			Poke.IsParcelPoke = true;
			mMemoryPokes.push_back(Poke);
		}
		catch (StringFormatError_x &) {
			Poke.Addr = FromString(Child.first, StringFormat_e::DataAddr);
			if (size_t(Poke.Addr) * sizeof(CInt_t) >= mMainframe.GetMemorySize()) throw InvalidParameter_x("Poke address outside of memory");
			Poke.IsParcelPoke = false;
			mMemoryPokes.push_back(Poke);
		}
	}
	mTimerIncrement = aConfig.get("TimerIncrement", aDefaultConfig.get("TimerIncrement", 1000));
	if (mTimerIncrement == 0) mTimerIncrement = 1;
	InitNotIntFlags();
}

SoftCpu_c::SoftCpu_c(Mainframe_c &aMainframe):
	Cpu_c(aMainframe),
	mInstCnt(0), 
	mLastInstCnt(0),
	mInReset(true),
	mCycleCount(0)
#ifdef COLLECT_PERF
	,
	mLastJump(0),
	mJumpsTaken(0),
	mJumpsNotTaken(0)
#endif
{
#ifdef COLLECT_PERF
    std::fill(mLoadUseHist.begin(), mLoadUseHist.end(), 0);
	std::fill(mBBHist.begin(), mBBHist.end(), 0);
#endif

	mMasterClearLock.clear();
	MasterClear();
	mState.InstBaseAddr = 0;
	mState.InstLimitAddr = std::numeric_limits<CAddr_t>::max();
	mState.XMode = ::IsXmp(GetMainframe().GetMachineType());
	mState.AddrMask = mState.XMode ? std::numeric_limits<CXmpAddr_t>::max() : std::numeric_limits<CAddr_t>::max();
	InitNotIntFlags();
}

void SoftCpu_c::MasterClear() {
	//BoolLock_c Lock(mInMasterClear);
	Lock_c Lock(mMasterClearLock);
	mLogger << setloglevel(LogLevel_IoActivity) << "CPU" << DecPrinter(GetCpuId()) << ": MasterClear called." << std::endl;
	// I'm not sure what master clear does, but apparently it shouldn't clear the XA (ExchangePacketAddress) register as a minimum.
	//		mState.ExchangePacketAddress = 0;
	//		mState.VectorNotUsed = true;
	mInReset = true;
	mState.TestAndSetBlocked = false;
	mState.Mode.SetMonitorMode(false);
	CalcInterruptMask();
	mState.Flags.Set(0);
	//TODO: probably would be better to clear the size. This way there's a 10-update ramp at reset...
	mLastInstCnt = 0;
	InitNotIntFlags();
}

void SoftCpu_c::DeadStart() {
	for (auto &MemoryPoke: mMemoryPokes) {
		if (MemoryPoke.IsParcelPoke) {
			mLogger << setloglevel(LogLevel_Dump) << "Overriding address " << InstAddr(MemoryPoke.Addr) << " with value " << HexPrinter(CParcel_t(MemoryPoke.Value)) << std::endl;
			mMainframe.MemWriteNoWatchpointByType<CParcel_t>(MemoryPoke.Addr, SwapBytes(CParcel_t(MemoryPoke.Value)));
		}
		else {
			mLogger << setloglevel(LogLevel_Dump) << "Overriding address " << Addr(MemoryPoke.Addr) << " with value " << HexPrinter(MemoryPoke.Value) << std::endl;
			mMainframe.MemWriteNoWatchpointByType<CInt_t>(MemoryPoke.Addr, SwapBytes(MemoryPoke.Value));
		}
	}
	std::atomic_thread_fence(std::memory_order_seq_cst);
	SetMcuInterrupt();
}

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif
ExchangePacket_c SoftCpu_c::CreateExchangePacket() {
	mLogger << setloglevel(LogLevel_InstructionTrace) << "Creating exchange packet with XP: " << HexPrinter(mState.ExchangePacketAddress) << std::endl;
	ExchangePacket_c RetVal(GetMainframe().GetMachineType());
	RetVal.SetProcessorNumber(uint8_t(mCpuId));
	RetVal.SetSyndrome(0);
	RetVal.SetProgramAddress(mState.ProgramAddress);
	if (IsXmp()) RetVal.SetXmpReadAddress(CAddr_t(0));
	RetVal.SetInstBaseAddr(mState.InstBaseAddr);
	RetVal.SetInstLimitAddr(mState.InstLimitAddr);
	RetVal.SetVectorNotUsed(mState.VectorNotUsed ? 1 : 0);
	RetVal.SetExchangeAddress(mState.ExchangePacketAddress);
	RetVal.SetVectorLength(mState.VL);
	if (IsXmp()) RetVal.SetXmpEnhancedAddressingMode(mState.XmpEnhancedAddressingMode ? 1 : 0);
	RetVal.SetDataBaseAddr(mState.DataBaseAddr);
	if (IsXmp()) {
		RetVal.SetXmpProgramState(mState.XmpProgramState);
	}
	RetVal.SetClusterNumber(mState.Cluster);
	RetVal.SetDataLimitAddr(mState.DataLimitAddr);
	RetVal.SetMode(mState.Mode);
	RetVal.SetEnableSecondVectorLogical(mState.EnableSecondVectorLogical ? 1 : 0);

	// Save only unmasked interrupt sources
	uint16_t FlagsSnapshot = mState.Flags.Get();
	std::atomic_thread_fence(std::memory_order_seq_cst);
	uint16_t InterruptMaskSnapshot = InterruptMaskFromState();
	uint16_t FiringInterupts = InterruptMaskSnapshot & FlagsSnapshot;
	Flags_c Flags(FiringInterupts);

//	mLogger << setloglevel(LogLevel_IoActivity) << "Interrupt served: " << Flags << std::endl;

	RetVal.SetFlag(Flags);
	// Clear interrupt flags that are not handled in SW
	mState.Flags.ClearBits(
		FiringInterupts &
		(
			Flags_c::IoInterrupt |
			Flags_c::InterProcessorInterrupt | 
			Flags_c::Deadlock | 
			Flags_c::McuInterrupt | 
			Flags_c::FloatingPointError | 
			Flags_c::OperandRangeError | 
			Flags_c::ProgramRangeError | 
			Flags_c::MemoryError | 
			Flags_c::NormalExit | 
			Flags_c::ErrorExit
		)
	);

	for(int i=0;i<8;++i) {
		RetVal.SetA(i,mState.A[i]);
		RetVal.SetS(i,mState.S[i]);
	}
	return RetVal;
}
#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif

void SoftCpu_c::RestoreFromExchangePacket(const ExchangePacket_c &aEP) {
	mState.ProgramAddress = aEP.GetProgramAddress();
	mState.InstBaseAddr = aEP.GetInstBaseAddr();
	mState.InstLimitAddr = aEP.GetInstLimitAddr();
	// TODO: figure out if this is correct. I believe, this bit should be set after every exchange, and cleared if there was any vector instruction executed since the last exchange.
	mState.VectorNotUsed = true; // aEP.GetVectorNotUsed() != 0;
	mState.EnableSecondVectorLogical = aEP.GetEnableSecondVectorLogical() != 0;
	mState.ExchangePacketAddress = aEP.GetExchangeAddress();
	mState.VL = aEP.GetVectorLength();
	mState.XmpEnhancedAddressingMode = IsXmp() ? aEP.GetXmpEnhancedAddressingMode() != 0 : false;
	mState.DataBaseAddr = aEP.GetDataBaseAddr();
	if (IsXmp()) {
		mState.XmpProgramState = aEP.GetXmpProgramState() != 0;
	}
	SetCluster(aEP.GetClusterNumber());
	mState.DataLimitAddr = aEP.GetDataLimitAddr();
	mState.Mode = aEP.GetMode();
	CalcInterruptMask();
	// Don't read flags here. We hold on to the existing interrupt flags so that interrupts pending due to being in monitor-mode have a chance to fire
	//mState.Flags.Set(aEP.GetFlag().Get());

	for(int i=0;i<8;++i) {
		mState.A[i] = aEP.GetA(i);
		mState.S[i] = aEP.GetS(i);
	}
	switch (GetMainframe().GetMachineType()) {
		case MachineTypes_e::XMP1xx:
		case MachineTypes_e::XMP2xx:
		case MachineTypes_e::XMP4xx:
			mState.XMode = true;
		break;
		case MachineTypes_e::J90:
		case MachineTypes_e::SV1:
			mState.XMode = false;
		break;
		case MachineTypes_e::YEL:
		case MachineTypes_e::YMP:
			mState.XMode = !GetMode().IsYmpEnhancedAddressingMode();
		break;
		default:
			throw Generic_x() << "Unknown machine type";
	}
	mState.AddrMask = mState.XMode ? std::numeric_limits<CXmpAddr_t>::max() : std::numeric_limits<CAddr_t>::max();
}

bool SoftCpu_c::CheckInterrupts() {
	MirrorExternalInterrupts();

	// Short-circuit common case: if there are no pendig interrupts, return immmediately...
	uint16_t FlagsSnapshot = mState.Flags.Get();
	if (FlagsSnapshot == 0) return false;
	// Calculate which interrupts are actually enabled and let them fire if they do.
	uint16_t InterruptMaskSnapshot = InterruptMaskFromState();
	uint16_t FiringInterupts = InterruptMaskSnapshot & FlagsSnapshot;
	return FiringInterupts != 0;
}

void SoftCpu_c::GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const {
	if (!mInReset) {
		aStatus.put("State", "ON ");
	} else {
		aStatus.put("State", "off");
	}
	double InstCntDelta = double(mInstCnt) - double(mLastInstCnt);
	mLastInstCnt = uint64_t(mInstCnt);
	double AvgInstCnt = InstCntDelta / aElapsedTime * 1000.0; // Get instructions per microsecond or virtual MHz...
	std::stringstream SpeedStr;
	SpeedStr << DoublePrinter(AvgInstCnt, 5, 3);
//	aStatus.put("C", mInstCnt);
	aStatus.put("MIPS", SpeedStr.str());
	aStatus.put("CLN", uint16_t(GetCluster()));
}

std::string SoftCpu_c::ReportHist() const {
	std::stringstream Strm;
#ifdef COLLECT_PERF
	for (size_t i = 0; i < mLoadUseHist.size(); ++i) {
		Strm << DecPrinter(i, 3) << ": " << DecPrinter(int(double(mLoadUseHist[i]) / double(mCycleCount) * 100.0), 3) << "%" << std::endl;
	}
#else
	Strm << "This build doesn't have histogram functions enabled." << std::endl;
#endif
	return Strm.str();
}

std::string SoftCpu_c::GetName() const {
	std::stringstream Name;
	Name << "CPU" << DecPrinter(mCpuId);
	return Name.str();
}

void SoftCpu_c::Dump(size_t aIdent) const {
	#ifdef DUMPREGS
	#error DUMPREGS is already defined
	#endif
	#define DUMPREGS(aReg, aPrinter, aWidth)                                                                                                                   \
		for(size_t Idx=0;Idx<sizeof(mState.aReg)/sizeof(mState.aReg[0]);++Idx) {                                                                               \
			mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << #aReg << DecPrinter(Idx,0) << ": " << aPrinter(mState.aReg[Idx],aWidth) << std::endl;        \
		}

	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "InReset: " << (mInReset ? "yes" : "no") << std::endl;
	if (mInReset) return;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Program Address: " << InstAddr(mState.ProgramAddress) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "VL: " << DecPrinter(mState.VL) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "VM: " << HexPrinter(mState.VM) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Exchange Packet Address: " << HexPrinter(mState.ExchangePacketAddress) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Inst Base Addr: " << HexPrinter(mState.InstBaseAddr) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Inst Limit Addr: " << HexPrinter(mState.InstLimitAddr) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Data Base Addr: " << HexPrinter(mState.DataBaseAddr) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Data Limit Addr: " << HexPrinter(mState.DataLimitAddr) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Periodic Interrupt Limit: " << HexPrinter(mState.PeriodicInterruptLimit) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Periodic Interrupt Cout-down: " << HexPrinter(mState.PeriodicInterruptCountDown) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Periodic Interrupt Enabled: " << (mState.PeriodicInterruptEnabled ? "yes" : "no") << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Test-and-set Blocked: " << (mState.TestAndSetBlocked ? "yes" : "no") << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Vector Not Used: " << (mState.VectorNotUsed ? "yes" : "no") << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Second Vector Logical Enabled: " << (mState.EnableSecondVectorLogical ? "yes" : "no") << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Cluster: " << HexPrinter(mState.Cluster) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Mode: " << mState.Mode << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Flags: " << mState.Flags << std::endl;
	DUMPREGS(A,HexPrinter,6);
	DUMPREGS(S,HexPrinter,16);
	DUMPREGS(B,HexPrinter,6);
	DUMPREGS(T,HexPrinter,16);
	for(size_t RegIdx=0;RegIdx<sizeof(mState.V)/sizeof(mState.V[0]);++RegIdx) {
		LogLine_c LogLine = mLogger << setloglevel(LogLevel_Dump);
		LogLine << Indent(aIdent) << "V[" << DecPrinter(RegIdx,0) << "]: ";
		for(size_t Idx=0;Idx<mState.V[RegIdx].size();++Idx) {
			LogLine << HexPrinter(mState.V[RegIdx][Idx], 16) << " ";
		}
		LogLine << std::endl;
	}
	#undef DUMPREGS
}

void SoftCpu_c::SetProgrammableClockInterrupt() {
	mState.Flags.SetProgrammableClockInterrupt();
}

void SoftCpu_c::HandleCounters() {
	if (mState.PeriodicInterruptCountDown == 0) {
/*		if (mState.PeriodicInterruptLimit != 0) {
			if (mState.PeriodicInterruptEnabled) {
				cout << "PI will fire!" << std::endl;
			} else {
				cout << "PI would fire but not enabled" << std::endl;
			}
		}*/
		if (mState.PeriodicInterruptEnabled) {
			SetProgrammableClockInterrupt();
		}
		mState.PeriodicInterruptCountDown = mState.PeriodicInterruptLimit;
	}
	else if (mState.PeriodicInterruptCountDown < mTimerIncrement) {
		mState.PeriodicInterruptCountDown = 0;
	}
	else {
		mState.PeriodicInterruptCountDown -= mTimerIncrement;
	}
}
	
CInt_t SoftCpu_c::GetSR(size_t aSrIdx) {
	CInt_t RetVal = 0;
	switch (aSrIdx) {
		case 0:
			RetVal = SetBits(RetVal, 63, 63, mState.Cluster != 0 ? 1 : 0);
			if (IsXmp()) {
				RetVal = SetBits(RetVal, 57, 57, mState.XmpProgramState ? 1 : 0);
			}
			if (IsJ90OrSV1()) {
				// The same bit is used as 'PS' in SV1
				RetVal = SetBits(RetVal, 57, 57, mState.Mode.IsSv1BitMatrixLoaded() ? 1 : 0);
			}
			RetVal = SetBits(RetVal, 51, 51, mState.Mode.IsFloatingPointErrorStatus() ? 1 : 0);
			RetVal = SetBits(RetVal, 50, 50, mState.Mode.IsFloatingPointErrorMode() ? 1 : 0);
			RetVal = SetBits(RetVal, 49, 49, mState.Mode.IsOperandRangeErrorMode() ? 1 : 0);
			RetVal = SetBits(RetVal, 48, 48, mState.Mode.IsBidirectionalMemoryMode() ? 1 : 0);
			RetVal = SetBits(RetVal, 44, 40, mCpuId);
			RetVal = SetBits(RetVal, 37, 32, mState.Cluster);
		break;
		default:
			throw Generic_x() << "Unknwon SR register index: " << OctPrinter(aSrIdx);
	}
	return RetVal;
}

void SoftCpu_c::SetSR(size_t aSrIdx, CInt_t aValue) {
	throw Generic_x() << "Unknwon SR register index: " << OctPrinter(aSrIdx);
}

CInt_t SoftCpu_c::ReadDataMem(CAddr_t aAddr) {
	const size_t cMemSize = mMainframe.GetMemorySizeInWords();
	CAddr_t PhysicalAddress = (aAddr + mState.DataBaseAddr);
#ifndef CRAY_TURBO
	if (PhysicalAddress > cMemSize) mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << " Wrapping memory read (" << HexPrinter(PhysicalAddress) << ") access at the end of physical memory (" << HexPrinter(cMemSize) << ")" << std::endl;
#endif
	PhysicalAddress %= cMemSize; // Wrap around addresses at the size of the memory
	if ((uint32_t(PhysicalAddress)) >= uint32_t(mState.DataLimitAddr)) {
		if (mState.Mode.IsOperandRangeErrorMode()) {
			CProgramAddr_t PhysicalInstAddress = mState.ProgramAddress + mState.InstBaseAddr * cParcelsPerWord;
			std::stringstream ErrorStr;
			ErrorStr <<
				"FAILED mem read " << DataSymbol(mState.DataBaseAddr, aAddr) << " (" << HexPrinter(PhysicalAddress) << ")" <<
				" limit: " << HexPrinter(mState.DataLimitAddr) << 
				" P: " << InstAddr(mState.ProgramAddress) << " (" << InstAddr(PhysicalInstAddress) << ")" << 
				std::endl;
			mLogger << setloglevel(LogLevel_Error) << ErrorStr.str() << std::endl;
			mMainframe.GetEventDispatcher().Fire(ErrorStr.str());
			throw DataReadOutOfBoundsError_x();
		}
		return 0;
	}
	CRAY_ASSERT(size_t(PhysicalAddress) < cMemSize);
	CInt_t RetVal = SwapBytes(mMainframe.MemRead<CInt_t>(PhysicalAddress));
#ifndef CRAY_TURBO
	LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
	if (LogLine.good()) LogLine << SideEffectIndent << "mem read " << DataSymbol(mState.DataBaseAddr, aAddr) << " (" << HexPrinter(PhysicalAddress) << ") value: " << HexPrinter(RetVal) << " '" << AsciiDumpPrinter(RetVal) << "'" << std::endl;
#endif
	return RetVal;
}

void SoftCpu_c::WriteDataMem(CAddr_t aAddr, CInt_t aData) {
	const size_t cMemSize = mMainframe.GetMemorySizeInWords();
	CAddr_t PhysicalAddress = (aAddr + mState.DataBaseAddr);
#ifndef CRAY_TURBO
	if (PhysicalAddress > cMemSize) mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << " Wrapping memory write access at the end of physical memory" << std::endl;
#endif
	PhysicalAddress %= cMemSize; // Wrap around addresses at the size of the memory
	if ((uint32_t(PhysicalAddress)) >= uint32_t(mState.DataLimitAddr)) {
		if (mState.Mode.IsOperandRangeErrorMode()) {
			CProgramAddr_t PhysicalInstAddress = mState.ProgramAddress + mState.InstBaseAddr * cParcelsPerWord;
			std::stringstream ErrorStr;
			ErrorStr <<
				"FAILED mem write " << DataSymbol(mState.DataBaseAddr, aAddr) << " (" << HexPrinter(PhysicalAddress) << ")" <<
				" limit: " << HexPrinter(mState.DataLimitAddr) <<
				" value: " << HexPrinter(aData) <<
				" P: " << InstAddr(mState.ProgramAddress) << " (" << InstAddr(PhysicalInstAddress) << ")";
			mLogger << setloglevel(LogLevel_Error) << ErrorStr.str() << std::endl;
			mMainframe.GetEventDispatcher().Fire(ErrorStr.str());
			throw DataWriteOutOfBoundsError_x();
		}
		return;
	}
	CRAY_ASSERT(size_t(PhysicalAddress) < cMemSize);
#ifndef CRAY_TURBO
	LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
	if (LogLine.good()) LogLine << SideEffectIndent << "mem write " << DataSymbol(mState.DataBaseAddr, aAddr) << " (" << HexPrinter(PhysicalAddress) << ") value: " << HexPrinter(aData) << " '" << AsciiDumpPrinter(aData) << "'" << std::endl;
#endif
	mMainframe.MemWrite(PhysicalAddress, SwapBytes(aData));
}

void SoftCpu_c::TestBreakPoints() {
	auto BreakPointIt = mBreakPoints.find(GetProgramCounter());
	if (BreakPointIt != mBreakPoints.end()) {
		// At least one breakpoint fired. Check all breakpoints to find all matches
		//for (auto &BreakPoint : BreakPointIt->second) {
		for (auto &BreakPoints : mBreakPoints) {
			if (BreakPoints.first == GetProgramCounter()) {
				for (auto &BreakPoint : BreakPoints.second) {
					if (BreakPoint->Test(*this, mMainframe)) {
						BreakPoint->Fire(*this);
					}
				}
			}
		}
	}
}

inline SoftCpu_c::InstMemRead_s SoftCpu_c::ReadInstMem(CProgramAddr_t aAddr) {
	InstMemRead_s RetVal;
	CProgramAddr_t PhysicalAddress = aAddr + mState.InstBaseAddr * cParcelsPerWord;
	const size_t cMemSize = mMainframe.GetMemorySizeInParcels();
#ifndef CRAY_TURBO
	if (PhysicalAddress > cMemSize) mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << " Wrapping instruction fetch access at the end of physical memory" << std::endl;
#endif
	PhysicalAddress %= cMemSize; // Wrap around addresses at the size of the memory
	if (((PhysicalAddress / cParcelsPerWord)) >= mState.InstLimitAddr) {
		throw InstFetchOutOfBoundsError_x();
	}
	// Read the next three parcels (with wrap-around) and compose the return value
/*	CParcel_t First = SwapBytes(mMainframe.MemReadByType<CParcel_t>(PhysicalAddress));
	CParcel_t Second = SwapBytes(mMainframe.MemReadByType<CParcel_t>((PhysicalAddress + 1) % cMemSize));
	CParcel_t Third = SwapBytes(mMainframe.MemReadByType<CParcel_t>((PhysicalAddress + 2) % cMemSize));
	RetVal.Data = (uint64_t(First) << 32) | (uint64_t(Second) << 16) | (uint64_t(Third) << 0);
	RetVal.Size = 1;
	if ((((PhysicalAddress + 1) / cParcelsPerWord) & cAddressMask) < mState.InstLimitAddr) RetVal.Size++;
	if ((((PhysicalAddress + 2) / cParcelsPerWord) & cAddressMask) < mState.InstLimitAddr) RetVal.Size++;*/
	// TODO: this is a fast fetch implementation but it doesn't work well at the edge of the instruction limit address or at the very end of memory. Let's just hope that we don't need that for a while.
	CParcel_t *Addr = mMainframe.MemAccessByType<CParcel_t>(PhysicalAddress);
	RetVal.Data = uint64_t(SwapBytes(Addr[0])) << 32 | uint64_t(SwapBytes(Addr[1])) << 16 | uint64_t(SwapBytes(Addr[2])) << 0;
//	RetVal.Data = SwapBytesInWords(uint64_t(Addr[0]) << 32 | uint64_t(Addr[1]) << 16 | uint64_t(Addr[2]) << 0);
	RetVal.Size = 3;
	return RetVal;
}

ExchangePacket_c SoftCpu_c::ReadExchangePacket(CAddr_t aMemAddr) {
	return ExchangePacket_c(GetMainframe().GetMachineType(), &mMainframe.GetMemory()[aMemAddr * 8]);
}

void SoftCpu_c::WriteExchangePacket(const ExchangePacket_c &aExchangePacket, CAddr_t aMemAddr) {
	aExchangePacket.Write(&mMainframe.GetMemory()[aMemAddr * 8]);
}

void SoftCpu_c::Exchange() {
	ExchangePacket_c NewContext = ReadExchangePacket(mState.ExchangePacketAddress * 16);
	ExchangePacket_c CurrentContext = CreateExchangePacket();
	NewContext.ClearFlags(); // Clear any pending interrupts on the way in
	if (mState.TestAndSetBlocked) {
		// TODO: special-case interrupt handling during blocked instruction: we have to make sure that P is set so that we'll retry the test-and-set operation.
		// NOTE: we might not have to do anything: the actual implementation might be that we only advance P once test-and-set succeeds. That way we'll keep retrying the same instruction in SingleStep and this
		//       special case is not needed at all.
	}
	WriteExchangePacket(CurrentContext, mState.ExchangePacketAddress * 16);
	{
		LogLine_c LogLine = mLogger << setloglevel(LogLevel_Event);
		LogLine << "*************** EXCHANGE PACKET SWAP BEGINS ***************" << std::endl;
		if (LogLine.good()) {
			// There is an interrupt pending, do the EP exchange
			LogLine << "Current Exchange Packet:" << std::endl;
			LogLine << "-----------------------------------------------------------" << std::endl;
			LogLine << CurrentContext;
			LogLine << "New Exchange Packet:" << std::endl;
			LogLine << "-----------------------------------------------------------" << std::endl;
			LogLine << NewContext;
		}
		LogLine << "*************** EXCHANGE PACKET SWAP ENDS ***************" << std::endl;
	}
	std::stringstream SyscallStr;
	ParseExchangePacket(CurrentContext, NewContext, SyscallStr, *this, mMainframe.GetOsType());
	if (SyscallStr.str().length() > 0) {
		mMainframe.GetEventDispatcher().Fire(SyscallStr.str());
		mLogger << setloglevel(LogLevel_Syscall) << SyscallStr.str() << std::endl;
	}
	PushHistory(StateChange_s(Reg_e::Exchange, CInt_t(mState.ExchangePacketAddress), CAddr_t(CurrentContext.GetFlag().Get())));
	RestoreFromExchangePacket(NewContext);
}

void SoftCpu_c::Tick() {
	//BoolLock_c Lock(mInBurst);
	Lock_c Lock(mMasterClearLock);

	HandleCounters();
	if (CheckInterrupts()) {
		mCallStack.clear();
		mLogger << setloglevel(LogLevel_Communication) << "at " << CurrentTime() << " Handling interrupt with flags: " << mState.Flags << std::endl;
		if (mInReset) {

			std::stringstream EventStr;
			EventStr << "CPU" << DecPrinter(mCpuId) << " Coming out of reset" << std::endl;
			mMainframe.GetEventDispatcher().Fire(EventStr.str());
//			cout << EventStr.str();
			mInstCnt = 0;
			mInReset = false;
		}
		Exchange();
		return;
	}
	if (!mInReset) {
		try {
			for(size_t i=0;i<=mInstructionBurstSize;++i) {
				mInstCnt++;
				TestBreakPoints();
//				if (mInReset) break;
				if (SingleStep()) break;
			}
		}
		catch (InstFetchOutOfBoundsError_x &) {
			mState.Flags.SetProgramRangeError();
		}
		catch (DataReadOutOfBoundsError_x &) {
			mState.Flags.SetOperandRangeError();
		}
		catch (DataWriteOutOfBoundsError_x &) {
			mState.Flags.SetOperandRangeError();
		}
	}
}

size_t SoftCpu_c::Decode(
	CProgramAddr_t aInstAddr,
	std::ostream &aDisassembly,
	std::ostream &aExplanation,
	std::ostream &aInstCodes,
	std::ostream &aHexInstCodes
) {
	InstMemRead_s Parcels = ReadInstMem(aInstAddr);

	bool BreakBurst;
	try {
		size_t RetVal = Decode<false>(Parcels.Data, Parcels.Size, aDisassembly, aExplanation, BreakBurst);
		if (IsXMode()) {
			switch (RetVal) {
				case 1: aInstCodes << OctPrinter(FirstParcel(Parcels.Data), 6) << " " << "       "; break;
				case 2: aInstCodes << OctPrinter(FirstParcel(Parcels.Data), 6) << ":" << OctPrinter(SecondParcel(Parcels.Data), 6); break;
				default: CRAY_ASSERT(false);
			}
		} else {
			switch (RetVal) {
				case 1: aInstCodes << OctPrinter(FirstParcel(Parcels.Data), 6) << " " << "       " << " " << "       "; break;
				case 2: aInstCodes << OctPrinter(FirstParcel(Parcels.Data), 6) << ":" << OctPrinter(SecondParcel(Parcels.Data), 6) << " " << "       "; break;
				case 3: aInstCodes << OctPrinter(FirstParcel(Parcels.Data), 6) << ":" << OctPrinter(SecondParcel(Parcels.Data), 6) << ":" << OctPrinter(ThirdParcel(Parcels.Data), 6); break;
				default: CRAY_ASSERT(false);
			}
		}
		if (IsXMode()) {
			switch (RetVal) {
				case 1: aHexInstCodes << HexPrinter(FirstParcel(Parcels.Data), 4) << " " << "      "; break;
				case 2: aHexInstCodes << HexPrinter(FirstParcel(Parcels.Data), 4) << ":" << HexPrinter(SecondParcel(Parcels.Data), 6); break;
				default: CRAY_ASSERT(false);
			}
		} else {
			switch (RetVal) {
				case 1: aHexInstCodes << HexPrinter(FirstParcel(Parcels.Data), 4) << " " << "      " << " " << "      "; break;
				case 2: aHexInstCodes << HexPrinter(FirstParcel(Parcels.Data), 4) << ":" << HexPrinter(SecondParcel(Parcels.Data), 4) << " " << "      "; break;
				case 3: aHexInstCodes << HexPrinter(FirstParcel(Parcels.Data), 4) << ":" << HexPrinter(SecondParcel(Parcels.Data), 4) << ":" << HexPrinter(ThirdParcel(Parcels.Data), 4); break;
				default: CRAY_ASSERT(false);
			}
		}
		return RetVal;
	}
	catch (UnknownInstError_x &) {
		aDisassembly << "*** UNK ***";
		if (IsXMode()) {
			aInstCodes << OctPrinter(FirstParcel(Parcels.Data), 6) << " " << "       ";
		} else {
			aInstCodes << OctPrinter(FirstParcel(Parcels.Data), 6) << " " << "       " << " " << "       ";
		}
		return CProgramAddr_t(1);
	}
}

bool SoftCpu_c::SingleStep() {
	InstMemRead_s Parcels = ReadInstMem(mState.ProgramAddress);

	// First just get the disassembly, but don't execute
	bool BreakBurst;
	LogLine_c LogLine = mLogger << setloglevel(LogLevel_InstructionTrace);
	if (LogLine.good()) {
		std::stringstream Disassembly;
		std::stringstream Explanation;
		try {
			Decode<false>(Parcels.Data, Parcels.Size, Disassembly, Explanation, BreakBurst);
		}
		catch (UnknownInstError_x &) {
			Disassembly << "*** UNK ***";
		}
		LogLine << "XA:" << HexPrinter(mState.ExchangePacketAddress) << " exec " << CodeSymbol(mState.InstBaseAddr, mState.ProgramAddress) << " (" << InstAddr(mState.ProgramAddress) << ") " << std::left << std::setw(30) << Disassembly.str() << std::setw(0) << " | " << Explanation.str() << std::endl;
	}
	static NullStrm_c NullStrm;
	CProgramAddr_t ProgramAddressIncrement = CProgramAddr_t(Decode<true>(Parcels.Data, Parcels.Size, NullStrm, NullStrm, BreakBurst));
	mCycleCount++;

	mState.ProgramAddress = mState.ProgramAddress + ProgramAddressIncrement;
	PushHistory(StateChange_s(Reg_e::P, mState.ProgramAddress, mState.InstBaseAddr));

	return BreakBurst;
}

void SoftCpu_c::DumpHistory() {
//	auto LogLine = mLogger << setloglevel(LogLevel_Dump);
//	if (!LogLine.good()) return;
//	LogLine << std::endl;
	for (auto &Elem : mHistory) {
		if (Elem.Reg == Reg_e::P) {
			// Re-create disassembly...
			InstMemRead_s Parcels;

			CProgramAddr_t PhysicalAddress = CProgramAddr_t(Elem.Val) + Elem.Val2 * cParcelsPerWord;
			const size_t cMemSize = mMainframe.GetMemorySizeInParcels();
#ifndef CRAY_TURBO
			if (PhysicalAddress > cMemSize) mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << " Wrapping instruction fetch access at the end of physical memory" << std::endl;
#endif
			PhysicalAddress %= cMemSize; // Wrap around addresses at the size of the memory
			CParcel_t *Addr = mMainframe.MemAccessByType<CParcel_t>(PhysicalAddress);
			Parcels.Data = uint64_t(SwapBytes(Addr[0])) << 32 | uint64_t(SwapBytes(Addr[1])) << 16 | uint64_t(SwapBytes(Addr[2])) << 0;
			Parcels.Size = 3;

			std::stringstream Disassembly;
			std::stringstream Explanation;
			bool BreakBurst;
			try {
				Decode<false>(Parcels.Data, Parcels.Size, Disassembly, Explanation, BreakBurst);
			}
			catch (UnknownInstError_x &) {
				Disassembly << "*** UNK ***";
			}
			mLogger << setloglevel(LogLevel_Dump) << "------- exec " << InstAddr(PhysicalAddress) << " (" << InstAddr(CAddr_t(Elem.Val)) << ") " << std::left << std::setw(30) << Disassembly.str() << std::setw(0) << " | " << Explanation.str() << std::endl;
			//mLogger << setloglevel(LogLevel_Dump)  << RegName(Elem.Reg) << " <== " << InstAddr(Elem.Val & ValMask) << std::endl;
		}
		else if (Elem.Reg == Reg_e::Exchange) {
			mLogger << setloglevel(LogLevel_Dump) << "======================== EXCHANGE TO XP ADDRESS: " << HexPrinter(Elem.Val) << " with flags: " << Flags_c(uint16_t(Elem.Val2)) << " ===========================" << std::endl;
		}
		else {
			mLogger << setloglevel(LogLevel_Dump) << SideEffectIndent << RegName(Elem.Reg) << " <== " << HexPrinter(Elem.Val) << " (" << DecPrinter(Elem.Val) << ")" << std::endl;
		}
	}
}

#define DECODE(aFieldGH) { &SoftCpu_c::Decode##aFieldGH<true>, &SoftCpu_c::Decode##aFieldGH<false> }

const SoftCpu_c::InstImplementation_s SoftCpu_c::mInstImplementations[] = {
	DECODE(0000),
	DECODE(0001),
	DECODE(0002),
	DECODE(0003),
	DECODE(0004),
	DECODE(0005),
	DECODE(0006),
	DECODE(0007),
	DECODE(0010),
	DECODE(0011),
	DECODE(0012),
	DECODE(0013),
	DECODE(0014),
	DECODE(0015),
	DECODE(0016),
	DECODE(0017),
	DECODE(0020),
	DECODE(0021),
	DECODE(0022),
	DECODE(0023),
	DECODE(0024),
	DECODE(0025),
	DECODE(0026),
	DECODE(0027),
	DECODE(0030),
	DECODE(0031),
	DECODE(0032),
	DECODE(0033),
	DECODE(0034),
	DECODE(0035),
	DECODE(0036),
	DECODE(0037),
	DECODE(0040),
	DECODE(0041),
	DECODE(0042),
	DECODE(0043),
	DECODE(0044),
	DECODE(0045),
	DECODE(0046),
	DECODE(0047),
	DECODE(0050),
	DECODE(0051),
	DECODE(0052),
	DECODE(0053),
	DECODE(0054),
	DECODE(0055),
	DECODE(0056),
	DECODE(0057),
	DECODE(0060),
	DECODE(0061),
	DECODE(0062),
	DECODE(0063),
	DECODE(0064),
	DECODE(0065),
	DECODE(0066),
	DECODE(0067),
	DECODE(0070),
	DECODE(0071),
	DECODE(0072),
	DECODE(0073),
	DECODE(0074),
	DECODE(0075),
	DECODE(0076),
	DECODE(0077),
	DECODE(010x),
	DECODE(010x),
	DECODE(010x),
	DECODE(010x),
	DECODE(010x),
	DECODE(010x),
	DECODE(010x),
	DECODE(010x),
	DECODE(011x),
	DECODE(011x),
	DECODE(011x),
	DECODE(011x),
	DECODE(011x),
	DECODE(011x),
	DECODE(011x),
	DECODE(011x),
	DECODE(012x),
	DECODE(012x),
	DECODE(012x),
	DECODE(012x),
	DECODE(012x),
	DECODE(012x),
	DECODE(012x),
	DECODE(012x),
	DECODE(013x),
	DECODE(013x),
	DECODE(013x),
	DECODE(013x),
	DECODE(013x),
	DECODE(013x),
	DECODE(013x),
	DECODE(013x),
	DECODE(0140),
	DECODE(0141),
	DECODE(0142),
	DECODE(0143),
	DECODE(0144),
	DECODE(0145),
	DECODE(0146),
	DECODE(0147),
	DECODE(0150),
	DECODE(0151),
	DECODE(0152),
	DECODE(0153),
	DECODE(0154),
	DECODE(0155),
	DECODE(0156),
	DECODE(0157),
	DECODE(0160),
	DECODE(0161),
	DECODE(0162),
	DECODE(0163),
	DECODE(0164),
	DECODE(0165),
	DECODE(0166),
	DECODE(0167),
	DECODE(0170),
	DECODE(0171),
	DECODE(0172),
	DECODE(0173),
	DECODE(0174),
	DECODE(0175),
	DECODE(0176),
	DECODE(0177)
};
#undef DECODE

// Return value
//   0 for failed test-and-set
//   1 for single-parcel, non-jump instructions
//   2 for double-parcel, non-jump instructions
//   0 for jump instructions (mState.ProgramAddress already modified to the target address)
//   0 for instructions that raise an interrupt (mostly address range problems or floating-point exceptions) :::TODO: is this what we want to do????
template <bool aDoExecute> size_t SoftCpu_c::Decode(
	uint64_t aParcels,
	size_t aMaxParcelCnt,
	std::ostream &aDisassembly,
	std::ostream &aExplanation,
	bool &aBreakBurst
) {
	// Its unfortunate, but the various fields inside the parcels are called by single-letter names in the manual.
	// To make it simple to reference back to the manual, we'll keep those conventions
	size_t gh = FieldGH(aParcels);
	aBreakBurst = false;

	CRAY_ASSERT(gh < 128);
	InstImplementation_s InstImpl = mInstImplementations[gh];
	if (aDoExecute) {
		return (this->*InstImpl.Exec)(aParcels, aMaxParcelCnt, aDisassembly, aExplanation, aBreakBurst);
	} else {
		return (this->*InstImpl.Disasm)(aParcels, aMaxParcelCnt, aDisassembly, aExplanation, aBreakBurst);
	}
/***
#define DECODE(aFieldGH) 		case aFieldGH: return Decode##aFieldGH<aDoExecute>(aParcels, aMaxParcelCnt, aDisassembly, aExplanation, aBreakBurst);
#define DECODE2(aFieldGH, aMethod) 		case aFieldGH: return Decode##aMethod<aDoExecute>(aParcels, aMaxParcelCnt, aDisassembly, aExplanation, aBreakBurst);

	switch (gh) {
		DECODE(0000)
		DECODE(0001)
		DECODE(0002)
		DECODE(0003)
		DECODE(0004)
		DECODE(0005)
		DECODE(0006)
		DECODE(0007)
		DECODE(0010)
		DECODE(0011)
		DECODE(0012)
		DECODE(0013)
		DECODE(0014)
		DECODE(0015)
		DECODE(0016)
		DECODE(0017)
		DECODE(0020)
		DECODE(0021)
		DECODE(0022)
		DECODE(0023)
		DECODE(0024)
		DECODE(0025)
		DECODE(0026)
		DECODE(0027)
		DECODE(0030)
		DECODE(0031)
		DECODE(0032)
		DECODE(0033)
		DECODE(0034)
		DECODE(0035)
		DECODE(0036)
		DECODE(0037)
		DECODE(0040)
		DECODE(0041)
		DECODE(0042)
		DECODE(0043)
		DECODE(0044)
		DECODE(0045)
		DECODE(0046)
		DECODE(0047)
		DECODE(0050)
		DECODE(0051)
		DECODE(0052)
		DECODE(0053)
		DECODE(0054)
		DECODE(0055)
		DECODE(0056)
		DECODE(0057)
		DECODE(0060)
		DECODE(0061)
		DECODE(0062)
		DECODE(0063)
		DECODE(0064)
		DECODE(0065)
		DECODE(0066)
		DECODE(0067)
		DECODE(0070)
		DECODE(0071)
		DECODE(0072)
		DECODE(0073)
		DECODE(0074)
		DECODE(0075)
		DECODE(0076)
		DECODE(0077)
		DECODE2(0100, 010x)
		DECODE2(0101, 010x)
		DECODE2(0102, 010x)
		DECODE2(0103, 010x)
		DECODE2(0104, 010x)
		DECODE2(0105, 010x)
		DECODE2(0106, 010x)
		DECODE2(0107, 010x)
		DECODE2(0110, 011x)
		DECODE2(0111, 011x)
		DECODE2(0112, 011x)
		DECODE2(0113, 011x)
		DECODE2(0114, 011x)
		DECODE2(0115, 011x)
		DECODE2(0116, 011x)
		DECODE2(0117, 011x)
		DECODE2(0120, 012x)
		DECODE2(0121, 012x)
		DECODE2(0122, 012x)
		DECODE2(0123, 012x)
		DECODE2(0124, 012x)
		DECODE2(0125, 012x)
		DECODE2(0126, 012x)
		DECODE2(0127, 012x)
		DECODE2(0130, 013x)
		DECODE2(0131, 013x)
		DECODE2(0132, 013x)
		DECODE2(0133, 013x)
		DECODE2(0134, 013x)
		DECODE2(0135, 013x)
		DECODE2(0136, 013x)
		DECODE2(0137, 013x)
		DECODE(0140)
		DECODE(0141)
		DECODE(0142)
		DECODE(0143)
		DECODE(0144)
		DECODE(0145)
		DECODE(0146)
		DECODE(0147)
		DECODE(0150)
		DECODE(0151)
		DECODE(0152)
		DECODE(0153)
		DECODE(0154)
		DECODE(0155)
		DECODE(0156)
		DECODE(0157)
		DECODE(0160)
		DECODE(0161)
		DECODE(0162)
		DECODE(0163)
		DECODE(0164)
		DECODE(0165)
		DECODE(0166)
		DECODE(0167)
		DECODE(0170)
		DECODE(0171)
		DECODE(0172)
		DECODE(0173)
		DECODE(0174)
		DECODE(0175)
		DECODE(0176)
		DECODE(0177)
		default:
			CRAY_ASSERT(false);
			return 1;
	}
#undef DECODE
#undef DECODE2
***/
}
