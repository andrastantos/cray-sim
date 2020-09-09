#include "unicos_proc_list.h"
#include "cray_mainframe.h"
#include <iomanip>

#define PARTIAL_DEBUG

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

UnicosProcList_c::UnicosProcList_c(const class Mainframe_c &aMainframe, const Configuration_c &aConfig): mMainframe(aMainframe) {
	mProcTableBase = aConfig.get<CAddr_t>("ProcTableBase");
	mProcTableLength = aConfig.get<size_t>("ProcTableLength");

}

void UnicosProcList_c::TestMemAccess(CAddr_t aAddress, CInt_t aData) {
	aData = SwapBytes(aData);
//	if (aAddress == 0x000ACB8D) {
//		CFloat_t Data(aData);
//		if (!Data.IsExpValid()) {
//			std::stringstream EventStr;
//			EventStr << "shconsts.sc_highshpri is invalid " << HexPrinter(aData) << std::endl;
//			mMainframe.GetEventDispatcher().Fire(EventStr.str());
//		} else {
//			std::stringstream EventStr;
//			EventStr << "shconsts.sc_highshpri is written " << HexPrinter(aData) << " (" << Data.ToDouble() << ")" << std::endl;
//			//mMainframe.GetEventDispatcher().Fire(EventStr.str());
//			//std::cout << EventStr.str();
//		}
//	}
	if (aAddress < mProcTableBase || aAddress >= (mProcTableBase + mProcTableLength)) return;
	CAddr_t Ofs = aAddress - mProcTableBase; // Convert to offset
	Ofs %= cProcEntryLen; // Convert to offset into struct
	if (Ofs == 16) { // p_pri
		std::stringstream EventStr;
		EventStr << "Proc priority at address Watchpoint at address " << Addr(aAddress) << " written with value: " << HexPrinter(aData) << std::endl;
		mMainframe.GetEventDispatcher().Fire(EventStr.str());
		if (aData & (~0xffffull)) {
			std::stringstream EventStr;
			EventStr << "INVALID Proc priority at address Watchpoint at address " << Addr(aAddress) << " written with value: " << HexPrinter(aData) << std::endl;
			mMainframe.GetEventDispatcher().Fire(EventStr.str());
			//std::cout << EventStr.str();
		}
	}
	if (Ofs == 0x3e) { // p_upri
		std::stringstream EventStr;
		EventStr << "Proc u_pri at address Watchpoint at address " << Addr(aAddress) << " written with value: " << HexPrinter(aData) << std::endl;
		mMainframe.GetEventDispatcher().Fire(EventStr.str());
		if (aData & (~0xffffull)) {
			std::stringstream EventStr;
			EventStr << "INVALID Proc u_pri at address Watchpoint at address " << Addr(aAddress) << " written with value: " << HexPrinter(aData) << std::endl;
			mMainframe.GetEventDispatcher().Fire(EventStr.str());
			//std::cout << EventStr.str();
		}
	}
	if (Ofs == 0x3d) { // p_sharepri
		CFloat_t Data(aData);
		if (!Data.IsExpValid() && aData != 0) {
			std::stringstream EventStr;
			EventStr << "p_sharepri is invalid " << HexPrinter(aData) << std::endl;
			mMainframe.GetEventDispatcher().Fire(EventStr.str());
		}
	}
}

// A quick conversion from time_t (apparently in 10ns intervals) to H:M:S.M format
static inline std::string TimePrinter(CInt_t aTime) {
	aTime /= 1000 * 100; // convert to ms first
	std::stringstream Strm;
	CInt_t MSec = aTime % 1000;
	aTime /= 1000;
	CInt_t Sec = aTime % 60;
	aTime /= 60;
	CInt_t Min = aTime % 60;
	aTime /= 60;
	CInt_t Hour = aTime;
	Strm << DecPrinter(Hour, 4, ' ') << ":" << DecPrinter(Min, 2, '0') << ":" << DecPrinter(Sec, 2, '0') << "." << DecPrinter(MSec, 3, '0');
	return Strm.str();
}

std::string UnicosProcList_c::Dump() {
	std::stringstream Strm;
	Strm << "ENTRY NAME             PID    STATUS PRI   SPRI        NICE     UTIME          STIME         " << std::endl;
	Strm << "----- ---------------- ------ ------ ----- ----------- -------- -------------- --------------" << std::endl;
	for (size_t Proc = 0; Proc < mProcTableLength; ++Proc) {
		CAddr_t Base = mProcTableBase + CAddr_t(Proc * cProcEntryLen);

		CInt_t State = SwapBytes(mMainframe.MemReadNoWatchpoint<CInt_t>(Base + 4)); // p_stat
		CInt_t Pid = SwapBytes(mMainframe.MemReadNoWatchpoint<CInt_t>(Base + 11)); // p_pid
		CInt_t Priority = SwapBytes(mMainframe.MemReadNoWatchpoint<CInt_t>(Base + 16)); // p_pri
		CInt_t Nice = SwapBytes(mMainframe.MemReadNoWatchpoint<CInt_t>(Base + 18)); // p_nice
		CInt_t UTime = SwapBytes(mMainframe.MemReadNoWatchpoint<CInt_t>(Base + 25)); // p_utime
		CInt_t STime = SwapBytes(mMainframe.MemReadNoWatchpoint<CInt_t>(Base + 26)); // p_stime
		CFloat_t SPri = CFloat_t(SwapBytes(mMainframe.MemReadNoWatchpoint<CInt_t>(Base + 0x3D))); // p_sharepri

		if (State == 0) continue; // Empty entries seem to have a state of 0

		Strm << DecPrinter(Proc, 5) << ' ';

		bool End = false;
		for (int i = 0; i < 16; ++i) {
			char C = mMainframe.MemReadNoWatchpointByByte<char>(Base * sizeof(CInt_t) + 16 + i); // p_comm
			if (C == 0) {
				End = true;
			}
			Strm << (End ? ' ' : C);
		}

		Strm << ' ';
		Strm << DecPrinter(Pid, 6) << ' ';
		switch (State) {
			case 1: Strm << "SLEEP "; break;
			case 2: Strm << "WAIT  "; break;
			case 3: Strm << "RUN   "; break;
			case 4: Strm << "IDL   "; break;
			case 5: Strm << "ZOMB  "; break;
			case 6: Strm << "STOP  "; break;
			case 7: Strm << "ONPROC"; break;
			default: Strm << DecPrinter(State, 6); break;
		}
		Strm << ' ';
		Strm << DecPrinter(Priority, 5) << ' ';
		Strm << std::setw(11) << SPri.ToDouble() << ' ';
		Strm << DecPrinter(Nice, 8) << ' ';
		Strm << TimePrinter(UTime) << ' ';
		Strm << TimePrinter(STime) << ' ';
		Strm << std::endl;
	}
//	CAddr_t AvailProc = CAddr_t(SwapBytes(mMainframe.MemReadNoWatchpoint<CInt_t>(CAddr_t(mProcTableBase + mProcTableLength * cProcEntryLen + 1)))); // proc *availproc; // next available proc table entry
//	CAddr_t AllProc   = CAddr_t(SwapBytes(mMainframe.MemReadNoWatchpoint<CInt_t>(CAddr_t(mProcTableBase + mProcTableLength * cProcEntryLen + 2)))); // proc *allproc;   // all allocated proc table entries
//	Strm << "ProcBase:  " << HexPrinter(mProcTableBase) << std::endl;
//	Strm << "AvailProc: " << HexPrinter(AvailProc) << std::endl;
//	Strm << "AllProc:   " << HexPrinter(AllProc) << std::endl;

//	CFloat_t ShPri = CFloat_t(SwapBytes(mMainframe.MemReadNoWatchpoint<CInt_t>(0x000ACB8D))); // shconsts.sc_highshpri
//
//	Strm << std::endl << "ShPri: " << ShPri.ToDouble() << " raw: " << HexPrinter(ShPri.Value) << std::endl;

	return Strm.str();
}

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif
