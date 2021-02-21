#include "sys_task_req.h"
#include "cray_mainframe.h"
#include "cray_cpu.h"

struct SysCallEntry_s {
	char Name[16];
	uint32_t Flags;
	uint32_t ArgCnt;
	uint64_t HandlerAddr;
	uint64_t NumCalls;
	uint64_t TotalTime;
	uint64_t MaxTime;
	uint64_t MinTime;
};

static CAddr_t SysCallTableBase;
static size_t SysCallTableSize;

void SetupSystaskParser(const Configuration_c &aConfig) {
	SysCallTableBase = aConfig.get<CAddr_t>("SysCallTableBase");
	CAddr_t SysCallTableEnd = aConfig.get<CAddr_t>("SysCallTableEnd");
	SysCallTableSize = (size_t(SysCallTableEnd) - size_t(SysCallTableBase)) / sizeof(SysCallEntry_s) * 8;
}

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

SysCallEntry_s GetSysCallEntry(size_t aSysCallNumber, const class Mainframe_c &aMainframe) {
	SysCallEntry_s Entry;
	const size_t SysCallEntrySize = sizeof(SysCallEntry_s);
	for (size_t Idx = 0; Idx < SysCallEntrySize / sizeof(uint64_t); ++Idx) {
		((uint64_t*)(&Entry))[Idx] = aMainframe.MemReadNoWatchpoint<uint64_t>(CAddr_t(SysCallTableBase + (8 * aSysCallNumber + Idx)));
		if (Idx > 1) ((uint64_t*)(&Entry))[Idx] = SwapBytes(((uint64_t*)(&Entry))[Idx]);
	}
	return Entry;
}

void DumpSyscallStats(std::ostream &aLogger, const class Mainframe_c &aMainframe, OsTypes_e aOsType) {
	if (aOsType != OsTypes_e::UNICOS) return;
	for (size_t SysCall = 0; SysCall < SysCallTableSize; ++SysCall) {
		try {
			SysCallEntry_s Entry = GetSysCallEntry(SysCall, aMainframe);
			if (Entry.NumCalls > 0) {
				aLogger << "SYSCALL: " << DecPrinter(SysCall) << " " << Entry.Name << " # calls: " << DecPrinter(Entry.NumCalls);
				aLogger << " total time: " << DecPrinter(Entry.TotalTime);
				aLogger << " max time: " << DecPrinter(Entry.MaxTime);
				aLogger << " min time: " << DecPrinter(Entry.MinTime);
				aLogger << std::endl;
			}
		}
		catch (Mainframe_c::ReadOutOfBoundsError_x &) {
			aLogger << " ~~~ invalid ~~~ ";
		}
	}
}

void PrintString(std::ostream &aLogger, CAddr_t aAddr, const Mainframe_c &aMainframe) {
	aLogger << "\"";
	try {
		bool Done = false;
		while (!Done) {
			uint64_t Data = aMainframe.MemReadNoWatchpoint<uint64_t>(aAddr);
			char *Str = (char *)(&Data);
			for (size_t Idx = 0; Idx < 8; ++Idx) {
				if (Str[Idx] == 0) {
					Done = true;
					break;
				}
				aLogger << Str[Idx];
			}
			aAddr = aAddr + 1;
		};
	}
	catch (Mainframe_c::ReadOutOfBoundsError_x &) {
		aLogger << " ~~~ invalid ~~~ ";
	}
	aLogger << "\"";
}

void PrintMemDump(std::ostream &aLogger, CAddr_t aAddr, size_t aSize, const Mainframe_c &aMainframe) {
	aLogger << "\"";
	try {
		for (size_t i = 0; i < aSize;) {
			uint64_t Data = aMainframe.MemReadNoWatchpoint<uint64_t>(aAddr);
			char *Str = (char *)(&Data);
			for (size_t Idx = 0; Idx < 8; ++Idx, ++i) {
				aLogger << PrintableChar(Str[Idx]);
				if (i >= aSize) break;
			}
			aAddr = aAddr + 1;
		};
	}
	catch (Mainframe_c::ReadOutOfBoundsError_x &) {
		aLogger << " ~~~ invalid ~~~ ";
	}
	aLogger << "\" ";
	bool Done = false;
	try {
		for (size_t i = 0; i < aSize;) {
			uint64_t Data = aMainframe.MemReadNoWatchpoint<uint64_t>(aAddr);
			char *Str = (char *)(&Data);
			for (size_t Idx = 0; Idx < 8; ++Idx, ++i) {
				aLogger << HexPrinter(Str[Idx], 2) << " ";
				if (i >= aSize) break;
			}
			aAddr = aAddr + 1;
		};
	}
	catch (Mainframe_c::ReadOutOfBoundsError_x &) {
		aLogger << " ~~~ invalid ~~~ ";
	}
}

void PrintMemArrayArg(std::ostream &aLogger, size_t aArg, size_t aSizeArg, const ExchangePacket_c &aExchangePacket, const Mainframe_c &aMainframe) {
	CInt_t S1 = aExchangePacket.GetS(1);
	CAddr_t DataBase = aExchangePacket.GetDataBaseAddr();
	CAddr_t Args = CAddr_t(S1) + DataBase;

	try {
		CAddr_t Addr = CAddr_t(SwapBytes(aMainframe.MemReadNoWatchpoint<uint64_t>(CAddr_t(Args + aArg))));
		CAddr_t Size = CAddr_t(SwapBytes(aMainframe.MemReadNoWatchpoint<uint64_t>(CAddr_t(Args + aSizeArg))));
		aLogger << HexPrinter(Addr) << " (" << HexPrinter(Addr + DataBase) << "), len: " << DecPrinter(Size);
		if (Size < 256) {
			aLogger << " content: ";
			PrintMemDump(aLogger, Addr + DataBase, Size, aMainframe);
		}
	}
	catch (Mainframe_c::ReadOutOfBoundsError_x &) {
		aLogger << " ~~~ invalid ~~~ ";
	}
}

void PrintStringArg(std::ostream &aLogger, size_t aArg, const ExchangePacket_c &aExchangePacket, const Mainframe_c &aMainframe) {
	CInt_t S1 = aExchangePacket.GetS(1);
	CAddr_t DataBase = aExchangePacket.GetDataBaseAddr();
	CAddr_t Args = CAddr_t(S1) + DataBase;

	try {
		CAddr_t Addr = CAddr_t(SwapBytes(aMainframe.MemReadNoWatchpoint<uint64_t>(CAddr_t(Args + aArg))));
		aLogger << HexPrinter(Addr) << " (" << HexPrinter(Addr + DataBase) << ") ";
		PrintString(aLogger, Addr + DataBase, aMainframe);
	}
	catch (Mainframe_c::ReadOutOfBoundsError_x &) {
		aLogger << " ~~~ invalid ~~~ ";
	}
}

void PrintStringArrayArg(std::ostream &aLogger, size_t aArg, const ExchangePacket_c &aExchangePacket, const Mainframe_c &aMainframe) {
	CInt_t S1 = aExchangePacket.GetS(1);
	CAddr_t DataBase = aExchangePacket.GetDataBaseAddr();
	CAddr_t Args = CAddr_t(S1) + DataBase;

	try {
		CAddr_t ArrAddr = CAddr_t(SwapBytes(aMainframe.MemReadNoWatchpoint<uint64_t>(CAddr_t(Args + aArg))));
		aLogger << HexPrinter(ArrAddr) << " (" << HexPrinter(ArrAddr + DataBase) << ") ";

		if (ArrAddr != 0) {
			aLogger << " [ ";
			bool First = true;
			CAddr_t Ptr = CAddr_t(SwapBytes(aMainframe.MemReadNoWatchpoint<uint64_t>(ArrAddr + DataBase)));
			while (Ptr != 0) {
				if (!First) aLogger << " , ";
				First = false;
				aLogger << HexPrinter(Ptr) << " (" << HexPrinter(Ptr + DataBase) << ") ";
				PrintString(aLogger, Ptr + DataBase, aMainframe);
				++ArrAddr;
				Ptr = CAddr_t(SwapBytes(aMainframe.MemReadNoWatchpoint<uint64_t>(ArrAddr + DataBase)));
			}
			aLogger << " ]";
		}
	}
	catch (Mainframe_c::ReadOutOfBoundsError_x &) {
		aLogger << " ~~~ invalid ~~~ ";
	}
}

void PrintPtrArg(std::ostream &aLogger, size_t aArg, const ExchangePacket_c &aExchangePacket, const Mainframe_c &aMainframe) {
	CInt_t S1 = aExchangePacket.GetS(1);
	CAddr_t DataBase = aExchangePacket.GetDataBaseAddr();
	CAddr_t Args = CAddr_t(S1) + DataBase;

	try {
		CAddr_t Addr = CAddr_t(SwapBytes(aMainframe.MemReadNoWatchpoint<uint64_t>(CAddr_t(Args + aArg))));
		aLogger << HexPrinter(Addr) << " (" << HexPrinter(Addr + DataBase) << ") --> " << HexPrinter(SwapBytes(aMainframe.MemReadNoWatchpoint<uint64_t>(Addr + DataBase)));
	}
	catch (Mainframe_c::ReadOutOfBoundsError_x &) {
		aLogger << " ~~~ invalid ~~~ ";
	}
}

void PrintIntArg(std::ostream &aLogger, size_t aArg, const ExchangePacket_c &aExchangePacket, const Mainframe_c &aMainframe) {
	CInt_t S1 = aExchangePacket.GetS(1);
	CAddr_t DataBase = aExchangePacket.GetDataBaseAddr();
	CAddr_t Args = CAddr_t(S1) + DataBase;

	try {
		CInt_t Data = (SwapBytes(aMainframe.MemReadNoWatchpoint<uint64_t>(CAddr_t(Args + aArg))));
		aLogger << HexPrinter(Data) << " (" << DecPrinter(Data) << ")";
	}
	catch (Mainframe_c::ReadOutOfBoundsError_x &) {
		aLogger << " ~~~ invalid ~~~ ";
	}
}

void ParseUnicosExchangePacket(const ExchangePacket_c &aCurrentEP, const ExchangePacket_c &aNewEP, std::ostream &aLogger, const Cpu_c &aCpu) {
	static size_t LastSysCall = SIZE_MAX;
	const Mainframe_c &Mainframe = aCpu.GetMainframe();

	if (aCurrentEP.GetMode().IsMonitorMode()) {
		CInt_t S[8];
		for (size_t Idx = 0; Idx<sizeof(S) / sizeof(S[0]); ++Idx) S[Idx] = aNewEP.GetS(Idx);
		CInt_t A[8];
		for (size_t Idx = 0; Idx<sizeof(A) / sizeof(A[0]); ++Idx) A[Idx] = aNewEP.GetA(Idx);

		//aLogger << "return from monitor mode with XA: " << HexPrinter(aCpu.GetExchangePacketAddress()) << " " << "IBA: " << HexPrinter(aCurrentEP.GetInstBaseAddr());

		// Swapping out from monitor mode - parse it as a reply
		if (LastSysCall < SysCallTableSize) {
			try {
				SysCallEntry_s Entry = GetSysCallEntry(LastSysCall, Mainframe);
				aLogger << "SYSCALL: " << DecPrinter(LastSysCall) << " " << Entry.Name << " returning " << HexPrinter(S[1]) << " (" << DecPrinter(S[1]) << ")";
			}
			catch (Mainframe_c::ReadOutOfBoundsError_x &) {
				aLogger << "SYSCALL: ~~~ invalid ~~~ ";
			}
		} else {
//			aLogger << "GOING OUT OF MONITOR MODE";
		}
		LastSysCall = SIZE_MAX;
	} else {
		CInt_t S[8];
		for (size_t Idx = 0; Idx<sizeof(S) / sizeof(S[0]); ++Idx) S[Idx] = aCurrentEP.GetS(Idx);
		CInt_t A[8];
		for (size_t Idx = 0; Idx<sizeof(A) / sizeof(A[0]); ++Idx) A[Idx] = aCurrentEP.GetA(Idx);

		LastSysCall = SIZE_MAX;
		if (aCurrentEP.GetFlag().IsNormalExit()) {
			// Swapping in to monitor mode - parse it as a request
			size_t SysCall = size_t(S[0]);
			if (SysCall < SysCallTableSize && aCurrentEP.GetInstBaseAddr() != 0) {
				aLogger << "XA: " << HexPrinter(aCpu.GetExchangePacketAddress()) << " " << "IBA: " << HexPrinter(aCurrentEP.GetInstBaseAddr()) << " ";
				LastSysCall = SysCall;
				CAddr_t DataBase = aCurrentEP.GetDataBaseAddr();
				CAddr_t Args = CAddr_t(S[1]) + DataBase;
				try {
					SysCallEntry_s Entry = GetSysCallEntry(SysCall, Mainframe);
					aLogger << "SYSCALL: " << DecPrinter(SysCall) << " " << Entry.Name << "(";
					switch (SysCall) {
						case 4: // write
							PrintIntArg(aLogger, 0, aCurrentEP, Mainframe);
							aLogger << " , ";
							PrintMemArrayArg(aLogger, 1, 2, aCurrentEP, Mainframe);
						case 5: // open
							PrintStringArg(aLogger, 0, aCurrentEP, Mainframe);
							aLogger << " , ";
							PrintIntArg(aLogger, 1, aCurrentEP, Mainframe);
						break;
						case 0x3b: // exece
							PrintStringArg(aLogger, 0, aCurrentEP, Mainframe);
							aLogger << " , ";
							PrintStringArrayArg(aLogger, 1, aCurrentEP, Mainframe);
							aLogger << " , ";
							PrintStringArrayArg(aLogger, 2, aCurrentEP, Mainframe);
						break;
						case 0x9b: // waitpid
							PrintIntArg(aLogger, 0, aCurrentEP, Mainframe);
							aLogger << " , ";
							PrintPtrArg(aLogger, 1, aCurrentEP, Mainframe);
							aLogger << " , ";
							PrintIntArg(aLogger, 2, aCurrentEP, Mainframe);
						break;
						case 125: // mtimes
							PrintPtrArg(aLogger, 0, aCurrentEP, Mainframe);
						break;
						default:
							for (uint32_t ArgIdx = 0; ArgIdx < std::min(Entry.ArgCnt, uint32_t(10)); ++ArgIdx) {
								if (ArgIdx != 0) aLogger << " , ";
								try {
									aLogger << HexPrinter(
										SwapBytes(
											Mainframe.MemReadNoWatchpoint<uint64_t>(Args + ArgIdx)
										)
									);
								}
								catch (Mainframe_c::ReadOutOfBoundsError_x &) {
									aLogger << " ~~~ invalid ~~~ ";
								}
							}
						break;
					}
					aLogger << ")";
				}
				catch (Mainframe_c::ReadOutOfBoundsError_x &) {
					aLogger << "SYSCALL: ~~~ invalid ~~~ ";
				}
			} else {
				//aLogger << "XA: " << HexPrinter(aCpu.GetExchangePacketAddress()) << " " << "IBA: " << HexPrinter(aCurrentEP.GetInstBaseAddr()) << " ";
				//aLogger << "unknown SYSCALL with S0 = " << HexPrinter(S[0]);
			}
		} else {
			//aLogger << "entering monitor mode with XA: " << HexPrinter(aCpu.GetExchangePacketAddress()) << " " << "IBA: " << HexPrinter(aCurrentEP.GetInstBaseAddr()) << " ";
		}
	}
}

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif

void ParseCosExchangePacket(const ExchangePacket_c &aExchangePacket, std::ostream &aLogger, const class Mainframe_c &aMainframe) {
	CInt_t S[8];
	for(size_t Idx=0;Idx<sizeof(S)/sizeof(S[0]);++Idx) S[Idx] = aExchangePacket.GetS(Idx);
/*	CInt_t A[8];
	for(size_t Idx=0;Idx<sizeof(A)/sizeof(A[0]);++Idx) A[Idx] = aExchangePacket.GetA(Idx);*/
	if (aExchangePacket.GetMode().IsMonitorMode()) {
		// Swapping out from monitor mode - parse it as a reply
		aLogger << "GOING OUT OF MONITOR MODE";
	} else {
		// Swapping in to monitor mode - parse it as a request
		uint8_t RequestCode = uint8_t(GetBitsReverse(S[7],55,63));
		switch (RequestCode) {
			case 001: { // Create Task
				char TaskName[9];
				CInt_t Tmp = SwapBytes(S[5]);
				memcpy(TaskName,&Tmp,sizeof(Tmp));
				TaskName[8] = 0;
				uint8_t Priority = uint8_t(GetBitsReverse(S[6],48,55));
				uint8_t Id = uint8_t(GetBitsReverse(S[6],56,63));
				CProgramAddr_t StartAddr = CProgramAddr_t(GetBitsReverse(S[7], 16, 39));
				std::stringstream Event;
				Event << "SYS_TASK_REQ - CREATE_TASK - TASK NAME: " << TaskName << " PRIORITY: " << DecPrinter(Priority,0) << " ID: " << DecPrinter(Id,0) << " START ADDRESS: " << InstAddr(StartAddr);
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 002: { // Ready system task
				bool T = bool(GetBitsReverse(S[7],0,0) != 0);
				std::stringstream Event;
				Event << "SYS_TASK_REQ - READY_SYSTEM_TASK - T: " << (T ? "yes":"no") << " - S1: " << HexPrinter(S[1]) << " - S2: " << HexPrinter(S[2]) << " - TASK: " << OctPrinter(S[6]);
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 003: { // Self-suspend
				std::stringstream Event;
				Event << "SYS_TASK_REQ - SELF-SUSPEND";
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 005: { // Front-end driver request
				uint8_t RequestKind = uint8_t(GetBitsReverse(S[6],0,3));
				CAddr_t Lxt = CAddr_t(GetBitsReverse(S[6],16,19));
				CAddr_t Lit = CAddr_t(GetBitsReverse(S[6],40,63));
				uint8_t Cht = uint8_t(GetBitsReverse(S[7],37,40));
				uint8_t Cho = uint8_t(GetBitsReverse(S[7],41,48));
				uint8_t Chn = uint8_t(GetBitsReverse(S[7],49,54));
				std::stringstream Event;
				Event << "SYS_TASK_REQ - FRONT-END DRIVER REQUEST - REQ: " << DecPrinter(RequestKind,0) << " LXT: " << Addr(Lxt) << " LIT: " << Addr(Lit) << " CHT: " << DecPrinter(Cht,0) << " CHO: " << DecPrinter(Cho,0) << " CHN: " << DecPrinter(Chn,0);
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 006: { // Task-delay
				std::stringstream Event;
				Event << "SYS_TASK_REQ - TASK-DELAY - RT: " << HexPrinter(S[6]) << " - clock cycles from now: " << DecPrinter(S[6] - aMainframe.GetRealTimeClock(),0);
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 010: { // Start second CPU
				std::stringstream Event;
				Event << "SYS_TASK_REQ - START_SECOND_CPU";
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 011: { // Disk blok I/O request
				CAddr_t EQT = CAddr_t(GetBitsReverse(S[6],40,63));
				CAddr_t DCT = CAddr_t(GetBitsReverse(S[7],16,39));
				CAddr_t CHT = CAddr_t(GetBitsReverse(S[7],40,54));
				std::stringstream Event;
				Event << "SYS_TASK_REQ - DISK_BLOCK_IO - EQT: " << Addr(EQT) << " - DCT: " << Addr(DCT) << " - CHT: " << Addr(CHT);
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 014: { // Ready system task and self-syspend
				bool T = bool(GetBitsReverse(S[7],0,0) != 0);
				std::stringstream Event;
				Event << "SYS_TASK_REQ - READY_SYSTEM_TASK_AND-SELF_SUSPEND (RTSS) - T: " << (T ? "yes":"no") << " - S1: " << HexPrinter(S[1]) << " - S2: " << HexPrinter(S[2]) << " - TASK: " << OctPrinter(S[6]);
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 016: { // Connect user task tp CPU
				bool RCLDCL = bool(GetBitsReverse(S[6],0,0) != 0);
				uint8_t RCCLN = uint8_t(GetBitsReverse(S[6],1,5));
				uint32_t RCTXT = uint32_t(GetBitsReverse(S[6],32,63));
				uint64_t RCTS = uint64_t(GetBitsReverse(S[7],0,54));
				std::stringstream Event;
				Event << "SYS_TASK_REQ - CONNECT_TASK_TO_CPU (RCP) - RCLDCL: " << (RCLDCL ? "yes":"no") << " - RCCLN: " << DecPrinter(RCCLN) << " - RCTXT: " << HexPrinter(RCTXT) << " RCTS: " << DecPrinter(RCTS);
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 017: { // Disconnect user task tp CPU
				bool DSCTCL = bool(GetBitsReverse(S[6],0,0) != 0);
				uint32_t DCTXT = uint32_t(GetBitsReverse(S[6],32,63));
				std::stringstream Event;
				Event << "SYS_TASK_REQ - DISCONNECT_TASK_FROM_CPU (DCP) - DSCTCL: " << (DSCTCL ? "yes":"no") << " - DCTXT: " << HexPrinter(DCTXT);
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 020: { // Post message in history buffer
				uint8_t FirstS = uint8_t(GetBitsReverse(S[7],42,44));
				uint8_t SecondS = uint8_t(GetBitsReverse(S[7],45,47));
				uint32_t DebugFnCode = uint32_t(GetBitsReverse(S[7],48,54));
				std::stringstream Event;
				Event << "SYS_TASK_REQ - POST - FirstS: " << DecPrinter(FirstS) << " [" << HexPrinter(S[FirstS]) << "]" << " - SecondS: " << DecPrinter(SecondS) << " [" << HexPrinter(S[SecondS]) << "]" << " - DebugFnCode: " << OctPrinter(DebugFnCode);
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 022: { // Packet I/O request
				CAddr_t SCT = CAddr_t(GetBitsReverse(S[6],40,63));
				uint8_t PFC = uint8_t(GetBitsReverse(S[7],52,54));
				std::stringstream Event;
				Event << "SYS_TASK_REQ - PACKET_IO (PIO) - SCT : " << Addr(SCT) << " - PFC: " << DecPrinter(PFC);
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			case 025: { // System Stop
				std::stringstream Event;
				Event << "SYS_TASK_REQ - STOP";
				aLogger << Event.str() << std::endl;
				aMainframe.GetEventDispatcher().Fire(Event.str());
			} break;
			default:
				aLogger << "UNKNOWN EXCHANGE PACKET - REQUEST CODE: " << DecPrinter(RequestCode,0) << " - S6: " << HexPrinter(S[6]) << " - S7: " << HexPrinter(S[7]);
			break;
		}
	}
}

void ParseExchangePacket(const ExchangePacket_c &aCurrentEP, const ExchangePacket_c &aNewEP, std::ostream &aLogger, const class Cpu_c &aCpu, OsTypes_e aOsType) {
	switch (aOsType) {
		case OsTypes_e::None: break;
		case OsTypes_e::COS:
			ParseCosExchangePacket(aCurrentEP, aLogger, aCpu.GetMainframe());
		break;
		case OsTypes_e::UNICOS:
			ParseUnicosExchangePacket(aCurrentEP, aNewEP, aLogger, aCpu);
		break;
	}
}

