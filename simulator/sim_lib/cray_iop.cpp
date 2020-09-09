// Cray-XMP I/O Processor simulator class

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
#include "utils.h"
#include "cray_iop.h"
#include "cray_mainframe.h"
#include "iop_bmx.h"
#include "iop_concentrator.h"
#include "iop_console.h"
#include "iop_disk.h"
#include "iop_expander.h"
#include "iop_host_memory.h"
#include "iop_iop2cray.h"
#include "iop_iop2iop.h"
#include "iop_era.h"
#include "iop_cluster.h"

enum IopChannelTypes_e {
	IopChannelType_IOR,
	IopChannelType_PFR,
	IopChannelType_PXS,
	IopChannelType_LME,
	IopChannelType_RTC,
	IopChannelType_MOS,
	IopChannelType_AI,
	IopChannelType_AO,
	IopChannelType_ERA,
	IopChannelType_EXB,
	IopChannelType_CI,
	IopChannelType_CO,
	IopChannelType_TI,
	IopChannelType_TO,
	IopChannelType_HI,
	IopChannelType_HO,
	IopChannelType_DK,
	IopChannelType_BM,
	IopChannelType_Unkown
};

IopChannelTypes_e GetChannelType(IopTypes_e aCpuType, uint16_t aChannel) {
	switch (aCpuType) {
		case IopType_MIOP:
			switch(aChannel) {
				case 000: return IopChannelType_IOR;
				case 001: return IopChannelType_PFR;
				case 002: return IopChannelType_PXS;
				case 003: return IopChannelType_LME;
				case 004: return IopChannelType_RTC;
				case 005: return IopChannelType_MOS;
				case 006: return IopChannelType_AI;
				case 007: return IopChannelType_AO;
				case 010: return IopChannelType_AI;
				case 011: return IopChannelType_AO;
				case 012: return IopChannelType_AI;
				case 013: return IopChannelType_AO;
				case 016: return IopChannelType_ERA;
				case 017: return IopChannelType_EXB;
				case 020: return IopChannelType_CI;
				case 021: return IopChannelType_CO;
				case 024: return IopChannelType_CI;
				case 025: return IopChannelType_CO;
				case 030: return IopChannelType_CI;
				case 031: return IopChannelType_CO;
				case 034: return IopChannelType_CI;
				case 035: return IopChannelType_CO;
				case 040: return IopChannelType_TI;
				case 041: return IopChannelType_TO;
				case 042: return IopChannelType_TI;
				case 043: return IopChannelType_TO;
				case 044: return IopChannelType_TI;
				case 045: return IopChannelType_TO;
				case 046: return IopChannelType_TI;
				case 047: return IopChannelType_TO;
				default:  return IopChannelType_Unkown;
			}
		break;
		case IopType_BIOP:
			switch(aChannel) {
				case 000: return IopChannelType_IOR;
				case 001: return IopChannelType_PFR;
				case 002: return IopChannelType_PXS;
				case 003: return IopChannelType_LME;
				case 004: return IopChannelType_RTC;
				case 005: return IopChannelType_MOS;
				case 006: return IopChannelType_AI;
				case 007: return IopChannelType_AO;
				case 010: return IopChannelType_AI;
				case 011: return IopChannelType_AO;
				case 012: return IopChannelType_AI;
				case 013: return IopChannelType_AO;
				case 014: return IopChannelType_HI;
				case 015: return IopChannelType_HO;
				case 020: return IopChannelType_DK;
				case 021: return IopChannelType_DK;
				case 022: return IopChannelType_DK;
				case 023: return IopChannelType_DK;
				case 024: return IopChannelType_DK;
				case 025: return IopChannelType_DK;
				case 026: return IopChannelType_DK;
				case 027: return IopChannelType_DK;
				default:  return IopChannelType_Unkown;
			}
		break;
		case IopType_DIOP:
			switch(aChannel) {
				case 000: return IopChannelType_IOR;
				case 001: return IopChannelType_PFR;
				case 002: return IopChannelType_PXS;
				case 003: return IopChannelType_LME;
				case 004: return IopChannelType_RTC;
				case 005: return IopChannelType_MOS;
				case 006: return IopChannelType_AI;
				case 007: return IopChannelType_AO;
				case 010: return IopChannelType_AI;
				case 011: return IopChannelType_AO;
				case 012: return IopChannelType_AI;
				case 013: return IopChannelType_AO;
				case 020: return IopChannelType_DK;
				case 021: return IopChannelType_DK;
				case 022: return IopChannelType_DK;
				case 023: return IopChannelType_DK;
				case 024: return IopChannelType_DK;
				case 025: return IopChannelType_DK;
				case 026: return IopChannelType_DK;
				case 027: return IopChannelType_DK;
				case 030: return IopChannelType_DK;
				case 031: return IopChannelType_DK;
				case 032: return IopChannelType_DK;
				case 033: return IopChannelType_DK;
				case 034: return IopChannelType_DK;
				case 035: return IopChannelType_DK;
				case 036: return IopChannelType_DK;
				case 037: return IopChannelType_DK;
				default:  return IopChannelType_Unkown;
			}
		break;
		case IopType_XIOP:
			switch(aChannel) {
				case 000: return IopChannelType_IOR;
				case 001: return IopChannelType_PFR;
				case 002: return IopChannelType_PXS;
				case 003: return IopChannelType_LME;
				case 004: return IopChannelType_RTC;
				case 005: return IopChannelType_MOS;
				case 006: return IopChannelType_AI;
				case 007: return IopChannelType_AO;
				case 010: return IopChannelType_AI;
				case 011: return IopChannelType_AO;
				case 012: return IopChannelType_AI;
				case 013: return IopChannelType_AO;
				case 020: return IopChannelType_BM;
				case 021: return IopChannelType_BM;
				case 022: return IopChannelType_BM;
				case 023: return IopChannelType_BM;
				case 024: return IopChannelType_BM;
				case 025: return IopChannelType_BM;
				case 026: return IopChannelType_BM;
				case 027: return IopChannelType_BM;
				case 030: return IopChannelType_BM;
				case 031: return IopChannelType_BM;
				case 032: return IopChannelType_BM;
				case 033: return IopChannelType_BM;
				case 034: return IopChannelType_BM;
				case 035: return IopChannelType_BM;
				case 036: return IopChannelType_BM;
				case 037: return IopChannelType_BM;
				default:  return IopChannelType_Unkown;
			}
		break;
		default: return IopChannelType_Unkown;
	}
}

class ChannelPrinter: public FieldFormatter_i {
public:
	explicit ChannelPrinter(IopTypes_e aCpuType, uint16_t aChannel): Channel(aChannel), CpuType(aCpuType) {}
	virtual void Print(std::ostream &aStream) const {
		std::stringstream Str;
		IopChannelTypes_e ChannelType = GetChannelType(CpuType, Channel);
		switch (ChannelType) {
			case IopChannelType_IOR: Str << OctPrinter(Channel,3) << " (IOR)"; break;
			case IopChannelType_PFR: Str << OctPrinter(Channel,3) << " (PFR)"; break;
			case IopChannelType_PXS: Str << OctPrinter(Channel,3) << " (PXS)"; break;
			case IopChannelType_LME: Str << OctPrinter(Channel,3) << " (LME)"; break;
			case IopChannelType_RTC: Str << OctPrinter(Channel,3) << " (RTC)"; break;
			case IopChannelType_MOS: Str << OctPrinter(Channel,3) << " (MOS)"; break;
			case IopChannelType_AI:
				switch (CpuType) {
					case IopType_MIOP:
						switch (Channel) {
							case 006: Str << OctPrinter(Channel,3) << " (AIA_BIOP)"; break;
							case 010: Str << OctPrinter(Channel,3) << " (AIB_DIOP)"; break;
							case 012: Str << OctPrinter(Channel,3) << " (AIC_XIOP)"; break;
							default:  Str << OctPrinter(Channel,3); break;
						}
					break;
					case IopType_BIOP:
						switch (Channel) {
							case 006: Str << OctPrinter(Channel,3) << " (AIA_MIOP)"; break;
							case 010: Str << OctPrinter(Channel,3) << " (AIB_DIOP)"; break;
							case 012: Str << OctPrinter(Channel,3) << " (AIC_XIOP)"; break;
							default:  Str << OctPrinter(Channel,3); break;
						}
					break;
					case IopType_DIOP:
						switch (Channel) {
							case 006: Str << OctPrinter(Channel,3) << " (AIA_MIOP)"; break;
							case 010: Str << OctPrinter(Channel,3) << " (AIB_BIOP)"; break;
							case 012: Str << OctPrinter(Channel,3) << " (AIC_XIOP)"; break;
							default:  Str << OctPrinter(Channel,3); break;
						}
					break;
					case IopType_XIOP:
						switch (Channel) {
							case 006: Str << OctPrinter(Channel,3) << " (AIA_MIOP)"; break;
							case 010: Str << OctPrinter(Channel,3) << " (AIB_BIOP)"; break;
							case 012: Str << OctPrinter(Channel,3) << " (AIC_DIOP)"; break;
							default:  Str << OctPrinter(Channel,3); break;
						}
					break;
					default:  Str << OctPrinter(Channel,3); break;
				}
			break;
			case IopChannelType_AO:
				switch (CpuType) {
					case IopType_MIOP:
						switch (Channel) {
							case 007: Str << OctPrinter(Channel,3) << " (AOA_BIOP)"; break;
							case 011: Str << OctPrinter(Channel,3) << " (AOB_DIOP)"; break;
							case 013: Str << OctPrinter(Channel,3) << " (AOC_XIOP)"; break;
							default:  Str << OctPrinter(Channel,3); break;
						}
					break;
					case IopType_BIOP:
						switch (Channel) {
							case 007: Str << OctPrinter(Channel,3) << " (AOA_MIOP)"; break;
							case 011: Str << OctPrinter(Channel,3) << " (AOB_DIOP)"; break;
							case 013: Str << OctPrinter(Channel,3) << " (AOC_XIOP)"; break;
							default:  Str << OctPrinter(Channel,3); break;
						}
					break;
					case IopType_DIOP:
						switch (Channel) {
							case 007: Str << OctPrinter(Channel,3) << " (AOA_MIOP)"; break;
							case 011: Str << OctPrinter(Channel,3) << " (AOB_BIOP)"; break;
							case 013: Str << OctPrinter(Channel,3) << " (AOC_XIOP)"; break;
							default:  Str << OctPrinter(Channel,3); break;
						}
					break;
					case IopType_XIOP:
						switch (Channel) {
							case 007: Str << OctPrinter(Channel,3) << " (AOA_MIOP)"; break;
							case 011: Str << OctPrinter(Channel,3) << " (AOB_BIOP)"; break;
							case 013: Str << OctPrinter(Channel,3) << " (AOC_DIOP)"; break;
							default:  Str << OctPrinter(Channel,3); break;
						}
					break;
					default: Str << OctPrinter(Channel,3); break;
				}
			break;
			case IopChannelType_ERA: Str << OctPrinter(Channel,3) << " (ERA)"; break;
			case IopChannelType_EXB: Str << OctPrinter(Channel,3) << " (EXB)"; break;
			case IopChannelType_CI: Str << OctPrinter(Channel,3) << " (CI)"; break;
			case IopChannelType_CO: Str << OctPrinter(Channel,3) << " (CO)"; break;
			case IopChannelType_TI: Str << OctPrinter(Channel,3) << " (TI)"; break;
			case IopChannelType_TO: Str << OctPrinter(Channel,3) << " (TO)"; break;
			case IopChannelType_HI: Str << OctPrinter(Channel,3) << " (HI)"; break;
			case IopChannelType_HO: Str << OctPrinter(Channel,3) << " (HO)"; break;
			case IopChannelType_DK: Str << OctPrinter(Channel,3) << " (DK)"; break;
			case IopChannelType_BM: Str << OctPrinter(Channel,3) << " (BM)"; break;
			case IopChannelType_Unkown: Str << OctPrinter(Channel,3); break;
			default: CRAY_ASSERT(false);
		}

		aStream << Str.str();
	}
	uint16_t Channel;
	IopTypes_e CpuType;
};

#ifdef DEFAULT_FUNCTIONS
#error DEFAULT_FUNCTIONS is already defined
#endif
#define DEFAULT_FUNCTIONS                                                   \
	case 006: Str << "Clear the channel interrupt enable flag"; break;      \
	case 007: Str << "Set the channel interrupt enable flag"; break;

class IoFunctionNotePrinter: public FieldFormatter_i {
public:
	IoFunctionNotePrinter(IopTypes_e aCpuType, uint16_t aChannel, uint16_t aFunction): CpuType(aCpuType), Channel(aChannel), Function(aFunction) {}
	virtual void Print(std::ostream &aStream) const {
		std::stringstream Str;
		IopChannelTypes_e ChannelType = GetChannelType(CpuType, Channel);
		switch(ChannelType) {
			case IopChannelType_IOR:
				switch(Function) {
					case 010: Str << "Read interrupt channel number (highest priority channel with active interrupt request)"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_PFR:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear program fetch request flag"; break;
					case 010: Str << "Read origin of the fetch request (the OR register number that was 0 during a jump)"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_PXS:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear the exit stack boundary flag"; break;
					case 010: Str << "Read the E (stack pointer) register"; break;
					case 011: Str << "Read the value pointed to by the E (stack pointer) register"; break;
					case 014: Str << "Write the E (stack pointer) register"; break;
					case 015: Str << "Write the value pointed to by the E (stack pointer) register"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_LME:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear the I/O memory parity error flag"; break;
					case 010: Str << "Read the memory error position info"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_RTC:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear the channel done flag (next interrupt in 1ms)"; break;
					case 010: Str << "Read the top 16 bits of the 17-bit RTC"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_MOS:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear the channel busy and done flags"; break;
					case 001: Str << "Write the I/O memory address (in QWORDS)"; break;
					case 002: Str << "Write the 15 high-order bits on the Buffer memory address (in QWORDS)"; break;
					case 003: Str << "Write the 9 low-order bits of the buffer memory address (in QWORDS)"; break;
					case 004: Str << "Set transfer length and start Buffer --> I/O memory transfer (in QWORDS, 0 is 128kBytes)"; break;
					case 005: Str << "Set transfer length and start I/O --> Buffer memory transfer (in QWORDS, 0 is 128kBytes)"; break;
					case 014: Str << "Set control register (bit 0: disable error correction, bit 1: disable write checks, bit 2: disable refersh)"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_AI:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear the channel done flag"; break;
					case 010: Str << "Read A of remote IOP"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_AO:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear the channel busy and done flags"; break;
					case 001: Str << "Set control register (bit 0: Master clear, bit 1: Deadstart, bit 2: Dead dump)"; break;
					case 014: Str << "Send A to remote IOP"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_ERA:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Idle channel"; break;
					case 010: Str << "Read error status"; break;
					case 011: Str << "Read 1st error parameter"; break;
					case 012: Str << "Read 2nd error parameter"; break;
					case 013: Str << "Read 3rd error parameter"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_EXB:
				switch(Function) {
					case 000: Str << "Idle the channel"; break;
					case 001: Str << "Request data input from A register (DIA)"; break;
					case 002: Str << "Request data input from B register (DIB)"; break;
					case 003: Str << "Request data input from C register (DIC)"; break;
					case 004: Str << "Request read busy/done flag, interrupt number"; break;
					case 005: Str << "Load device address"; break;
					case 006: Str << "Send interface mask (MSKO)"; break;
					case 007: Str << "Set interrupt mode"; break;
					case 010: Str << "Read data bus status"; break;
					case 011: Str << "Read status 1"; break;
					case 013: Str << "Read status 2"; break;
					case 014: Str << "Data output to A register (DOA)"; break;
					case 015: Str << "Data output to B register (DOB)"; break;
					case 016: Str << "Data output to C register (DOC)"; break;
					case 017: Str << "Send control"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_CI:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear channel"; break;
					case 001: Str << "Enter I/O memory address, start input"; break;
					case 002: Str << "Enter parcel count"; break;
					case 003: Str << "Clear channel parity error flags"; break;
					case 004: Str << "Clear ready waiting flag"; break;
					case 010: Str << "Read I/O memory address"; break;
					case 011: Str << "Read status (ready waiting, parity errors)"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_CO:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear channel"; break;
					case 001: Str << "Enter I/O memory address, start output"; break;
					case 002: Str << "Enter parcel count"; break;
					case 003: Str << "Clear error flag"; break;
					case 004: Str << "Set/clear external control signals"; break;
					case 010: Str << "Read I/O memory address"; break;
					case 011: Str << "Read status (4-bit channel data, error)"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_TI:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear the channel done flag"; break;
					case 010: Str << "Read data"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_TO:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear the channel busy and done flag"; break;
					case 014: Str << "Send data to display"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_HI:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear the channel busy and done flag"; break;
					case 001: Str << "Set I/O memory address"; break;
					case 002: Str << "Set upper Central Memory address"; break;
					case 003: Str << "Set lower Central Memory address"; break;
					case 004: Str << "Set Block length, start I/O memory <-- Central Memory transfer"; break;
					case 014: Str << "Enter diagnostic mode"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_HO:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear the channel busy and done flag"; break;
					case 001: Str << "Set I/O memory address"; break;
					case 002: Str << "Set upper Central Memory address"; break;
					case 003: Str << "Set lower Central Memory address"; break;
					case 005: Str << "Set Block length, start I/O memory --> Central Memory transfer"; break;
					case 014: Str << "Enter diagnostic mode"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_DK:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear channel control"; break;
					case 001: Str << "Select mode or request status"; break;
					case 002: Str << "Read data into I/O memory"; break;
					case 003: Str << "Write data from I/O memory"; break;
					case 004: Str << "Select new head group"; break;
					case 005: Str << "Select new cylinder"; break;
					case 010: Str << "Read I/O Memory current address"; break;
					case 011: Str << "Read status response"; break;
					case 014: Str << "Set I/O Memory start address"; break;
					case 015: Str << "Status response register diagnostic"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_BM:
				switch(Function) {
					DEFAULT_FUNCTIONS
					case 000: Str << "Clear channel control"; break;
					case 001: Str << "Send reset functions"; break;
					case 002: Str << "Channel command"; break;
					case 003: Str << "Read request-in address"; break;
					case 004: Str << "Asynchronous I/O"; break;
					case 005: Str << "Delay counter diagnostics"; break;
					case 010: Str << "Read I/O Memory address"; break;
					case 011: Str << "Read byte count"; break;
					case 012: Str << "Read status"; break;
					case 013: Str << "Read input tags"; break;
					case 014: Str << "Set I/O Memory address"; break;
					case 015: Str << "Set byte count"; break;
					case 016: Str << "Set device address"; break;
					case 017: Str << "Set output tags"; break;
					default: Str << "**** Undefined function ****"; break;
				}
			break;
			case IopChannelType_Unkown:
				Str << "**** Undefined function ****"; break;
			default: CRAY_ASSERT(false);
		}
		aStream << Str.str();
	}
	uint16_t Channel;
	uint16_t Function;
	IopTypes_e CpuType;
};
#undef DEFAULT_FUNCTIONS

std::shared_ptr<IopCpu_c::BreakPoint_i> IopCpu_c::CreateBreakPoint(const std::string &aBreakPointType, const Configuration_c &aConfig) {
	if (aBreakPointType == "LogOn") {
		return std::make_shared<LogOn_c>(aConfig);
	} else if (aBreakPointType == "Terminate") {
		return std::make_shared<Terminate_c>(aConfig);
	} else if (aBreakPointType == "LogOff") {
		return std::make_shared<LogOff_c>(aConfig);
	} else if (aBreakPointType == "LogLevel") {
		return std::make_shared<LogLevel_c>(aConfig);
	} else if (aBreakPointType == "LogLevelPush") {
		return std::make_shared<LogLevelPush_c>(aConfig);
	} else if (aBreakPointType == "LogLevelPop") {
		return std::make_shared<LogLevelPop_c>(aConfig);
	} else if (aBreakPointType == "Trace") {
		return std::make_shared<TracePoint_c>(aConfig);
	} else if (aBreakPointType == "Event") {
		return std::make_shared<EventFirePoint_c>(aConfig);
	} else {
		throw InvalidParameter_x(boost::format("Unkown breakpoint type:") % aBreakPointType);
	}
}

IopCpu_c::IopCpu_c(const Configuration_c &aConfig, uint8_t aCpuId, std::vector<uint8_t> &aIoMemory, std::vector<uint8_t> *aBufferMemory, class IopCluster_c &aIopCluster):
	mIoMemory(aIoMemory), 
	mCpuId(aCpuId),
	mState(
		aConfig,
		*this,
		aConfig.get<uint64_t>("ClockFrequency", 80000000),
		aConfig.get<uint64_t>("TimerLimit", 79999)
	),
	mBufferMemory(aBufferMemory),
	mIopCluster(&aIopCluster),
	mLogger(aConfig,"IOP",aCpuId),
	mEventPoints(*this),
	mInstCnt(0),
	mLastInstCnt(0),
	RefOrB(*this),
	RefA(*this),
	RefC(*this),
	RefC_A(*this),
	RefB(*this)
{
	mExists = aConfig.get<bool>("Exists", true);
	mLogger.SetParent(aIopCluster.GetLogger());
	try {
		boost::optional<std::string> DefaultLogLevel = aConfig.get_optional<std::string>("DefaultLogLevel");
		if (DefaultLogLevel.is_initialized()) {
			mLogger.SetDisplayLogLevel(FromString_LogLevels_e(DefaultLogLevel.get()));
		}

		std::string IopType = aConfig.get<std::string>("Type");
		if (IopType == "MIOP") mType = IopType_MIOP; else
		if (IopType == "BIOP") mType = IopType_BIOP; else
		if (IopType == "DIOP") mType = IopType_DIOP; else
		if (IopType == "XIOP") mType = IopType_XIOP; else
			throw boost::property_tree::ptree_bad_data("Type",IopType);
		mIoChannels.resize(aConfig.get<size_t>("ChannelCount", 42),nullptr);
		mIoChannels[mState.ChannelIOR.GetChannelIdx()] = &mState.ChannelIOR;
		mIoChannels[mState.ChannelPFR.GetChannelIdx()] = &mState.ChannelPFR;
		mIoChannels[mState.ChannelPXS.GetChannelIdx()] = &mState.ChannelPXS;
		mIoChannels[mState.ChannelLME.GetChannelIdx()] = &mState.ChannelLME;
		mIoChannels[mState.ChannelRTC.GetChannelIdx()] = &mState.ChannelRTC;
		mIoChannels[mState.ChannelMOS.GetChannelIdx()] = &mState.ChannelMOS;

		// Load all channels
		for(const auto &ChannelConfig: aConfig.get_child_safe("Channels")) {
			std::string PeripheralType = ChannelConfig.first;
			// Create the appropriate peripheral
			std::shared_ptr<IopPeripheral_i> Peripheral;
			if (PeripheralType == "ERA") {
				Peripheral = std::make_shared<ChannelERA_c>(ChannelConfig.second, *this, ChannelConfig.second.get<size_t>("ChannelIdx"));
			} else if (PeripheralType == "Console") {
				Peripheral = std::make_shared<IopConsole_c>(ChannelConfig.second, *this, mIopCluster->GetMainframe().DoDisableAutoTerminal());
			} else if (PeripheralType == "HIA") {
				Peripheral = std::make_shared<ChannelHostMemory_c>(ChannelConfig.second, *this, mIopCluster->GetMainframe().GetMemory(), ChannelConfig.second.get<size_t>("ChannelIdx"), true, false);
			} else if (PeripheralType == "HOA") {
				Peripheral = std::make_shared<ChannelHostMemory_c>(ChannelConfig.second, *this, mIopCluster->GetMainframe().GetMemory(), ChannelConfig.second.get<size_t>("ChannelIdx"), false, true);
			} else if (PeripheralType  == "Expander") {
				Peripheral = std::make_shared<IopChannelExp_c>(ChannelConfig.second, *this);
			} else if (PeripheralType == "DD29") {
				Peripheral = std::make_shared<IopDiskDD29_c>(ChannelConfig.second, *this);
			} else if (PeripheralType == "Bmx") {
				Peripheral = std::make_shared<IopChannelBmx_c>(ChannelConfig.second, *this);
			} else if (PeripheralType == "CI") {
				size_t CrayChannelIdx = ChannelConfig.second.get<size_t>("CrayChannelIdx");
				size_t IopChannelIdx = ChannelConfig.second.get<size_t>("IopChannelIdx");
				std::shared_ptr<IopCrayChannel_c> LocalChannel = std::make_shared<IopCrayChannel_c>(ChannelConfig.second, *this, mIopCluster->GetMainframe(), IopChannelIdx, CrayChannelIdx, true, false);
				mIopCluster->GetMainframe().SetChannel(CrayChannelIdx-8,*LocalChannel);
				Peripheral = LocalChannel;
			} else if (PeripheralType == "CO") {
				size_t CrayChannelIdx = ChannelConfig.second.get<size_t>("CrayChannelIdx");
				size_t IopChannelIdx = ChannelConfig.second.get<size_t>("IopChannelIdx");
				std::shared_ptr<IopCrayChannel_c> LocalChannel = std::make_shared<IopCrayChannel_c>(ChannelConfig.second, *this, mIopCluster->GetMainframe(), IopChannelIdx, CrayChannelIdx ,false, true);
				mIopCluster->GetMainframe().SetChannel(CrayChannelIdx-8,*LocalChannel);
				Peripheral = LocalChannel;
			} else if (PeripheralType == "CONC") {
				Peripheral = std::make_shared<IopConcentrator_c>(ChannelConfig.second, *this);
			} else {
				throw InvalidParameter_x(boost::format("Unkown channel type : %1$s") % PeripheralType);
			}
			// Connect the peripheral to the channels
			for(size_t i=0;i<Peripheral->GetChannelCnt();++i) {
				const IopChannel_i &Channel = Peripheral->GetChannel(i);
				const IopChannel_i *OldChannel;
				OldChannel = Connect(&Channel);
				if (OldChannel != nullptr) {
					throw InvalidParameter_x(boost::format("Duplicate channel specification for channel %1$03o") % Channel.GetChannelIdx());
				}
			}
			mPeripherals.push_back(Peripheral);
		}
		// Populate the channels that need a tick call
		for (auto &IoChannel : mIoChannels) {
			if (IoChannel != nullptr && IoChannel->NeedsTick()) {
				mIoTickChannels.emplace_back(IoChannel);
			}
		}
		// Load all break-points
		for(const auto &BreakPoint: aConfig.get_child_safe("BreakPoints")) {
			std::string BreakPointType = BreakPoint.second.get<std::string>("Type");
			IopInt_t Addr = FromString<IopInt_t>(BreakPoint.first);
			std::shared_ptr<BreakPoint_i> BreakPointObj(CreateBreakPoint(BreakPointType,BreakPoint.second));
			CRAY_ASSERT(BreakPointObj != nullptr);
			mBreakPoints[Addr].push_back(BreakPointObj);
		}
		// Load all event-points
		mEventPoints.LoadEventPoints(aConfig.get_child_safe("EventPoints"));
		mInstructionBurstSize = aConfig.get<size_t>("InstructionBurstSize", 1)-1;
		// Register ourselves for receving events
		mIopCluster->GetMainframe().GetEventDispatcher().AddHandler(*this);
	}
	catch (...) {
		mPeripherals.clear();
		mBreakPoints.clear();
		throw;
	}
}

void IopCpu_c::GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const {
	if (!mState.InReset) {
		aStatus.put("State", "ON ");
	}
	else {
		aStatus.put("State", "off");
	}
	double InstCntDelta = double(mInstCnt) - double(mLastInstCnt);
	mLastInstCnt = uint64_t(mInstCnt);
	double AvgInstCnt = InstCntDelta / aElapsedTime * 1000.0; // Get instructions per microsecond or virtual MHz...
	std::stringstream SpeedStr;
	SpeedStr << DoublePrinter(AvgInstCnt, 5, 3);
	aStatus.put("MIPS", SpeedStr.str());
}

void IopCpu_c::GetPeripheralStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const {
	for (auto &Peripheral : mPeripherals) {
		if (Peripheral->GetType() == aFilter) {
			StatusReport_c Status;
			Peripheral->GetStatus(Status, aFilter, aLongFormat);
			if (!Status.empty()) {
				aStatus.put_child(Peripheral->GetName(), Status);
			}
		}
		if (Peripheral->GetType() == PeripheralType_e::Expander || Peripheral->GetType() == PeripheralType_e::Bmx) {
			StatusReport_c Status;
			Peripheral->GetStatus(Status, aFilter, aLongFormat);
			if (!Status.empty()) {
				aStatus.put_child(Peripheral->GetName(), Status);
			}
		}
	}
}

void IopCpu_c::RegisterCommands(CommandHooks_t &aHooks) {
	for (auto &Peripheral : mPeripherals) {
		Peripheral->RegisterCommands(aHooks);
	}
}

std::string IopCpu_c::GetName() const {
	std::stringstream Name;
	Name << "IOP" << DecPrinter(mCpuId);
	return Name.str();
}

void IopCpu_c::Dump(size_t aIdent) const {
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "InReset: " << (mState.InReset ? "yes" : "no") << std::endl;
	if (mState.InReset) return;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "A: " << HexPrinter(mState.A) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "C: " << DecPrinter(mState.Carry,1) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "B: " << HexPrinter(mState.B) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "P: " << HexPrinter(mState.P) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "I: " << DecPrinter(mState.I,1) << std::endl;
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "ISet is waiting: " << (mState.ISetIsWaiting ? "yes" : "no") << std::endl;
	for(size_t RegIdx=0;RegIdx<sizeof(mState.OperandRegisters)/sizeof(mState.OperandRegisters[0]);++RegIdx) {
		mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "OR[" << DecPrinter(RegIdx,0) << "]: "
			<< HexPrinter(mState.OperandRegisters[RegIdx]) << " "
			<< std::endl;
	}
	for(size_t i=0;i<mIoChannels.size();++i) {
		if (mIoChannels[i] != nullptr) mIoChannels[i]->Dump(aIdent+1);
	}
}

void IopCpu_c::mShiftRight(IopInt_t aAmount) {
	uint32_t Data = RefC_A;
	if (aAmount > 16) Data = 0; else Data >>= aAmount;
	RefC_A = Data;
}

void IopCpu_c::mRotateRight(IopInt_t aAmount) {
	uint64_t Data = (uint64_t(RefC_A) << 17 * 2) | (uint64_t(RefC_A) << 17) | (RefC_A);
	aAmount &= 0x1f;
	Data >>= aAmount;
	RefC_A = uint32_t(Data);
}

void IopCpu_c::mShiftLeft(IopInt_t aAmount) {
	uint32_t Data = RefC_A;
	if (aAmount > 17) Data = 0; else Data <<= aAmount;
	RefC_A = Data;
}

void IopCpu_c::mRotateLeft(IopInt_t aAmount) {
	uint64_t Data = (uint64_t(RefC_A) << 17 * 2) | (uint64_t(RefC_A) << 17) | (RefC_A);
	aAmount &= 0x1f;
	Data <<= aAmount;
	Data >>= 17;
	RefC_A = uint32_t(Data);
}

void IopCpu_c::Increment(IopInt_t aAmount) {
	uint32_t Data = RefC_A;
	Data += aAmount;
	RefC_A = Data;
}

void IopCpu_c::Decrement(IopInt_t aAmount) {
	uint32_t Data = RefC_A;
	Data += (uint32_t)((uint16_t)~aAmount)+1;
//	Data += ~(uint32_t)aAmount;
//	Data += 1;
	RefC_A = Data;
}

void IopCpu_c::MasterClear() {
	mState.P = 0;
	mState.ChannelPXS.SetE(mState.ChannelPXS.GetE() + 1);
	mState.I = 1;
	mState.ISetIsWaiting = false;
	for(size_t i=0;i<mIoChannels.size();++i) {
		if (mIoChannels[i] != nullptr) mIoChannels[i]->MasterClear();
	}
	mInstCnt = 0;
}

IopInt_t IopCpu_c::Disassemble(IopInt_t aP, std::ostream &aDisassembly, std::ostream &aExplanation, std::ostream &InstFields) {
	IopInt_t OldP = mState.P;

	mState.P = aP;
	IopParcel_t FirstParcel = IoMemAccess<IopParcel_t>(mState.P);
	IopParcel_t SecondParcel = 0;
	SecondParcel = IoMemAccess<IopParcel_t>((mState.P+1) & 0xffff);

	bool DelayISet = false;
	bool BreakBurst;
	IopInt_t Increment = Decode<false>(FirstParcel,SecondParcel,false,aDisassembly,aExplanation,DelayISet,BreakBurst);

	if (Increment == 1) {
		InstFields << HexPrinter(FirstParcel,4) << "-" << "      " << " f:" << OctPrinter(FirstParcel >> 9,4) << " d:" << DecPrinter(FirstParcel & 0x1ff,4);
	} else {
		InstFields << HexPrinter(FirstParcel,4) << "-" << HexPrinter(SecondParcel,4) << " f:" << OctPrinter(FirstParcel >> 9,4) << " d:" << DecPrinter(FirstParcel & 0x1ff,4);
	}

	mState.P = OldP;
	return Increment;
}

void IopCpu_c::Tick() {
	// Advance all I/O channels
/*	for(size_t i=0;i<mIoChannels.size();++i) {
		if (mIoChannels[i] != nullptr) mIoChannels[i]->Tick();
	}*/
	for (auto &IoChannel : mIoTickChannels) {
		IoChannel->Tick();
	}

	if (MultiThreadedSupport) {
		// See if any of the channels wants to stall the IOP
		for (size_t i = 0; i < mIoChannels.size(); ++i) {
			if (mIoChannels[i] != nullptr && mIoChannels[i]->StallIop()) return;
		}
	}

	// Check for interrupts
	if (mState.I != 0) {
		bool HaveInterrupt = false;
		for(size_t i=0;i<mIoChannels.size();++i) {
			if (mIoChannels[i] != nullptr) {
				if (mIoChannels[i]->GetInterrupt() != 0) {
					mLogger << setloglevel(LogLevel_Event) << "================================ INT" << OctPrinter(i,3) << " ================================" << std::endl;
					//cout << "================================ INT" << OctPrinter(i,3) << " to " << "IOP" << DecPrinter(mCpuId) << " ================================" << std::endl;
					HaveInterrupt = true;
					break;
				}
			}
		}
		if (HaveInterrupt) {
			if (mState.InReset) {
				mLogger << setloglevel(LogLevel_Event) << "================================ IOP is starting ================================" << std::endl;
//				cout << "================================ IOP" << DecPrinter(mCpuId) << " is starting ================================" << std::endl;
				mInstCnt = 0;
			}
			mState.InReset = false;
			mState.I = 0;
			mState.ChannelPXS.Push(mState.P);
			mState.P = mState.ChannelPXS.GetAt(0);
		}
	}

	if (!mState.InReset) {
		for(size_t i=0;i<=mInstructionBurstSize;++i) {
			mInstCnt++;
			if (SingleStep()) break;
		}
	}
}

static NullStrm_c NullStrm;

// Return true to break the burst
bool IopCpu_c::SingleStep() {
	IopParcel_t FirstParcel = IoMemAccess<IopParcel_t>(mState.P);
	IopParcel_t SecondParcel = 0;
	SecondParcel = IoMemAccess<IopParcel_t>((mState.P+1) & 0xffff);

	bool DelayISet = false;
	
	if (mBreakPoints.size() > 0) {
		auto BreakPointIt = mBreakPoints.find(mState.P);
		if (BreakPointIt != mBreakPoints.end()) {
			for (auto &BreakPoint : BreakPointIt->second) {
				if (BreakPoint->Test(GetDma())) BreakPoint->Fire(*this);
			}
		}
	}

	bool BreakBurst;
	LogLine_c LogLine = mLogger << setloglevel(LogLevel_InstructionTrace);
	if (LogLine.good()) {
		std::stringstream Disassembly;
		std::stringstream Explanation;
		Decode<false>(FirstParcel,SecondParcel,true,Disassembly,Explanation,DelayISet,BreakBurst);
		LogLine << "exec " << HexPrinter(mState.P) << " " << std::left << std::setw(30) << Disassembly.str() << std::setw(0);
		if (Explanation.str().length() > 0) {
			LogLine << " | " << Explanation.str() << std::endl;
		} else {
			LogLine << std::endl;
		}
	}

	IopInt_t Increment = Decode<true>(FirstParcel,SecondParcel,true,NullStrm,NullStrm,DelayISet,BreakBurst);

	// Implement delayed-enable of interrupts
	if (!DelayISet && mState.ISetIsWaiting) {
		mState.I = 1;
		mState.ISetIsWaiting = false;
	}

	mState.P += Increment;

	return BreakBurst;
}

template <bool aDoExecute, uint16_t aFieldF> IopInt_t IopCpu_c::DecodeJump(
	IopParcel_t aFirstParcel,
	IopParcel_t aSecondParcel,
	bool aInSimulation,
	std::ostream &aDisassembly,
	std::ostream &aExplanation,
	bool &aDelayISet,
	bool &aBreakBurst
) {
	aDelayISet = false;
	aBreakBurst = false;

	uint16_t f = aFieldF;
	uint16_t d = (aFirstParcel & 0x01ff) >> 0;
	uint16_t k = aSecondParcel;

	RefOr_s RefOr(d, *this);

	// special-case branch and call instructions as they have a condition field in them as well
	CRAY_ASSERT(
		((f & 0770) == 0070) ||
		((f & 0740) == 0100)
	);
	uint16_t InstCode = 0xffff;
	const char *ConditionStr = "";
	bool Condition = true;
	switch (f & 0770) {
		case 0070: // Unconditional group
			InstCode = f & 00007;
			ConditionStr = "";
			Condition = true;
		break;
		case 0100:
		case 0110:
		case 0120:
		case 0130:
			switch (f & 3) {
				case 0: // C = 0 group
					InstCode = (f >> 2) & 00007;
					ConditionStr = ", C = 0";
					Condition = RefC == 0;
				break;
				case 1: // C = 1 group
					InstCode = (f >> 2) & 00007;
					ConditionStr = ", C = 1";
					Condition = RefC != 0;
				break;
				case 2: // A = 0 group
					InstCode = (f >> 2) & 00007;
					ConditionStr = ", A = 0";
					Condition = RefA == 0;
				break;
				case 3: // A # 0 group
					InstCode = (f >> 2) & 00007;
					ConditionStr = ", A # 0";
					Condition = RefA != 0;
				break;
				default: CRAY_ASSERT(false);
			}
		break;
		default: CRAY_ASSERT(false);
	}
	switch (InstCode) {
		case 0000: // P = P + d
			if (!aDoExecute) {
				aDisassembly << "P = P + " << DecPrinter(d) << " (" << HexPrinter(GetProgramCounter() + d, 4) << ")" << ConditionStr;
			}
			if (aDoExecute) {
				aDelayISet = true;
				if (Condition) {
					mLogger << setloglevel(LogLevel_Event) << "================================ JUMP =========================" << std::endl;
					mState.P = mState.P + d;
					return 0; // P is already modified, don't modify it any further
				}
			}
			return 1;
		case 0001: // P = P - d
			if (!aDoExecute) {
				aDisassembly << "P = P - " << DecPrinter(d) << " (" << HexPrinter(GetProgramCounter() - d, 4) << ")" << ConditionStr;
			}
			if (aDoExecute) {
				aDelayISet = true;
				if (Condition) {
					mLogger << setloglevel(LogLevel_Event) << "================================ JUMP =========================" << std::endl;
					mState.P = mState.P - d;
					return 0; // P is already modified, don't modify it any further
				}
			}
			return 1;
		case 0002: // R = P + d
			if (!aDoExecute) {
				aDisassembly << "R = P + " << DecPrinter(d) << " (" << HexPrinter(GetProgramCounter() + d, 4) << ")" << ConditionStr;
			}
			if (aDoExecute) {
				aDelayISet = true;
				if (Condition) {
					mLogger << setloglevel(LogLevel_Event) << "================================ CALL =========================" << std::endl;
					IopInt_t Target = mState.P + d;
					mState.P = mState.P + 1;
					//						mLogger.Indent();
					mState.ChannelPXS.Push(mState.P);
					mState.P = Target;
					aBreakBurst = true;
					return 0; // P is already modified, don't modify it any further
				}
			}
			return 1;
		case 0003: // R = P - d
			if (!aDoExecute) {
				aDisassembly << "R = P - " << DecPrinter(d) << " (" << HexPrinter(GetProgramCounter() - d, 4) << ")" << ConditionStr;
			}
			if (aDoExecute) {
				aDelayISet = true;
				if (Condition) {
					mLogger << setloglevel(LogLevel_Event) << "================================ CALL =========================" << std::endl;
					IopInt_t Target = mState.P - d;
					mState.P = mState.P + 1;
					//						mLogger.Indent();
					mState.ChannelPXS.Push(mState.P);
					mState.P = Target;
					aBreakBurst = true;
					return 0; // P is already modified, don't modify it any further
				}
			}
			return 1;
		case 0004: // P = dd
			if (!aDoExecute) {
				aDisassembly << "P = " << RefOr << ConditionStr;
			}
			if (aDoExecute) {
				aDelayISet = true;
				if (Condition) {
					if (RefOr == 0) {
						mLogger << setloglevel(LogLevel_Event) << "================================ PFR REQ =========================" << std::endl;
						mState.ChannelPFR.SetRequestIdx(d);
					}
					mLogger << setloglevel(LogLevel_Event) << "================================ JUMP =========================" << std::endl;
					mState.P = RefOr;
					return 0; // P is already modified, don't modify it any further
				}
			}
			return 1;
		case 0005: // P = dd + k
			if (!aDoExecute) {
				aDisassembly << "P = " << RefOr << "+" << DecPrinter(k) << " (" << HexPrinter(k) << ")" << ConditionStr;
			}
			if (aDoExecute) {
				aDelayISet = true;
				if (Condition) {
					if (RefOr == 0) {
						mLogger << setloglevel(LogLevel_Event) << "================================ PFR REQ =========================" << std::endl;
						mState.ChannelPFR.SetRequestIdx(d);
					}
					mLogger << setloglevel(LogLevel_Event) << "================================ JUMP =========================" << std::endl;
					mState.P = RefOr + k;
					return 0; // P is already modified, don't modify it any further
				}
			}
			return 2;
		case 0006: // R = dd
			if (!aDoExecute) {
				aDisassembly << "R = " << RefOr << ConditionStr;
			}
			if (aDoExecute) {
				aDelayISet = true;
				if (Condition) {
					// TODO: do we save to the stack before raising the interrupt???
					if (RefOr == 0) {
						mLogger << setloglevel(LogLevel_Event) << "================================ PFR REQ =========================" << std::endl;
						mState.ChannelPFR.SetRequestIdx(d);
					}
					mLogger << setloglevel(LogLevel_Event) << "================================ CALL =========================" << std::endl;
					mState.P = mState.P + 1;
					//						mLogger.Indent();
					mState.ChannelPXS.Push(mState.P);
					mState.P = RefOr;
					aBreakBurst = true;
					return 0; // P is already modified, don't modify it any further
				}
			}
			return 1;
		case 0007: // R = dd + k
			if (!aDoExecute) {
				aDisassembly << "R = " << RefOr << "+" << DecPrinter(k) << " (" << HexPrinter(k) << ")" << ConditionStr;
			}
			if (aDoExecute) {
				aDelayISet = true;
				if (Condition) {
					// TODO: do we save to the stack before raising the interrupt???
					if (RefOr == 0) {
						mLogger << setloglevel(LogLevel_Event) << "================================ PFR REQ =========================" << std::endl;
						mState.ChannelPFR.SetRequestIdx(d);
					}
					mLogger << setloglevel(LogLevel_Event) << "================================ CALL =========================" << std::endl;
					mState.P = mState.P + 2;
					//						mLogger.Indent();
					mState.ChannelPXS.Push(mState.P);
					mState.P = RefOr + k;
					aBreakBurst = true;
					return 0; // P is already modified, don't modify it any further
				}
			}
			return 2;
		default: CRAY_ASSERT(false);
	} // switch
	throw Generic_x("Unreachable code");
}

template <bool aDoExecute, uint16_t aFieldF> IopInt_t IopCpu_c::DecodeIo(
	IopParcel_t aFirstParcel,
	IopParcel_t /*aSecondParcel*/,
	bool aInSimulation,
	std::ostream &aDisassembly,
	std::ostream &aExplanation,
	bool &aDelayISet,
	bool &aBreakBurst
) {
	aDelayISet = false;
	aBreakBurst = false;

	uint16_t f = aFieldF;
	uint16_t d = (aFirstParcel & 0x01ff) >> 0;

	RefOr_s RefOr(d, *this);

	// Special-case I/O insrtuctions as they also have special fields in 'f'
	CRAY_ASSERT(
		((f & 0xfff0) == 0140) ||
		((f & 0xfff0) == 0160)
	);
	uint16_t Function = f & 0xf;
	IopIoFunction_t Channel;
	switch (f & 0xfff0) {
		case 0140: // channel defined in d
			if (!aDoExecute) {
				aDisassembly << "io " << ChannelPrinter(mType, d) << ", fn" << OctPrinter(Function, 2);
				aExplanation << IoFunctionNotePrinter(mType, d, Function);
			}
			if (aDoExecute) {
				Channel = d;
				if (mIoChannels[Channel] == nullptr) {
					CRAY_ASSERT(Function == 0 || Function == 6);
					mLogger << setloglevel(LogLevel_Warning) << SideEffectIndent << "WARNING: channel io to non-existent channel" << std::endl;
				}
				else {
					//						stringstream Event;
					//						Event << "IOP" << DecPrinter(GetCpuId()) << " I/O channel" << OctPrinter(Channel) << " fn" << OctPrinter(Function,2);
					//						mMainframe->GetEventDispatcher().Fire(Event.str());
					IopInt_t ReadValue = mIoChannels[Channel]->DoIo(Function, RefA);
					if ((f & 0x000c) == 0x0008) RefC_A = ReadValue;
					aDelayISet = true;
				}
				aBreakBurst = true;
			}
		return 1;
		case 0160: // channel defined in B
			if (!aDoExecute) {
				aDisassembly << "IOB ";
				if (aInSimulation) aDisassembly << ChannelPrinter(mType, RefB) << " ";
				aDisassembly << ", fn" << OctPrinter(Function, 2);
				if (aInSimulation) aExplanation << IoFunctionNotePrinter(mType, RefB, Function);
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				Channel = RefB;
				if (mIoChannels[Channel] == nullptr) {
					LogLine_c LogLine = mLogger << setloglevel((Function != 0 && Function != 6) ? LogLevel_Error : LogLevel_Warning);
					if (Function != 0 && Function != 6) {
						LogLine << "ERROR: ";
					}
					else {
						LogLine << SideEffectIndent << "WARNING: ";
					}
					LogLine << "channel io to non-existent channel (" << OctPrinter(Channel) << ") function: " << OctPrinter(Function) << " at address " << HexPrinter(mState.P) << std::endl;
					//						CRAY_ASSERT(Function == 0 || Function == 6);
				}
				else {
					//						stringstream Event;
					//					Event << "IOP" << DecPrinter(GetCpuId()) << " I/O channel" << OctPrinter(Channel) << " fn" << OctPrinter(Function,2);
					//					mMainframe->GetEventDispatcher().Fire(Event.str());
					IopInt_t ReadValue = mIoChannels[Channel]->DoIo(Function, RefA);
					if ((f & 0x000c) == 0x0008) RefC_A = ReadValue;
					aDelayISet = true;
				}
				aBreakBurst = true;
			}
		return 1;
		default: CRAY_ASSERT(false);
	} // switch
	throw Generic_x("Unreachable code");
}

template <bool aDoExecute, uint16_t aFieldF> IopInt_t IopCpu_c::Decode(
	IopParcel_t aFirstParcel, 
	IopParcel_t aSecondParcel,
	bool aInSimulation,
	std::ostream &aDisassembly,
	std::ostream &aExplanation,
	bool &aDelayISet,
	bool &aBreakBurst
) {
	aDelayISet = false;
	aBreakBurst = false;

	uint16_t f = aFieldF;
	uint16_t d = (aFirstParcel & 0x01ff) >> 0;
	uint16_t k = aSecondParcel;

	RefOr_s RefOr(d, *this);

	// special-case branch and call instructions as they have a condition field in them as well
	CRAY_ASSERT(
		((f & 0770) != 0070) &&
		((f & 0740) != 0100)
	);
	CRAY_ASSERT(
		((f & 0xfff0) != 0140) &&
		((f & 0xfff0) != 0160)
	);

	switch(f) {
		case 0000: // PASS
			if (!aDoExecute) {
				aDisassembly << "PASS";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
		return 1;
		case 0001: // EXIT
			if (!aDoExecute) {
				aDisassembly << "EXIT";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				mLogger << setloglevel(LogLevel_Event) << "================================ RET ==========================" << std::endl;
//					mLogger.UnIndent();
				mState.P = mState.ChannelPXS.Pop();
				aDelayISet = true;
				aBreakBurst = true;
				return 0;
			}
		return 1;
		case 0002: // I = 0
			if (!aDoExecute) {
				aDisassembly << "I = 0";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				mState.I = 0;
				mState.ISetIsWaiting = false;
			}
		return 1;
		case 0003: // I = 1
			if (!aDoExecute) {
				aDisassembly << "I = 1";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				mState.ISetIsWaiting = true;
				aDelayISet = true;
				aBreakBurst = true;
			}
		return 1;
		case 0004: // A = A > d
			if (!aDoExecute) {
				aDisassembly << "A = A > " << DecPrinter(d) << " (" << HexPrinter(d) << ")";
			}
			if (aDoExecute) {
				mShiftRight(d);
			}
		return 1;
		case 0005: // A = A < d
			if (!aDoExecute) {
				aDisassembly << "A = A < " << DecPrinter(d) << " (" << HexPrinter(d) << ")";
			}
			if (aDoExecute) {
				mShiftLeft(d);
			}
		return 1;
		case 0006: // A = A >> d
			if (!aDoExecute) {
				aDisassembly << "A = A >> " << DecPrinter(d) << " (" << HexPrinter(d) << ")";
			}
			if (aDoExecute) {
				mRotateRight(d);
			}
		return 1;
		case 0007: // A = A << d
			if (!aDoExecute) {
				aDisassembly << "A = A << " << DecPrinter(d) << " (" << HexPrinter(d) << ")";
			}
			if (aDoExecute) {
				mRotateLeft(d);
			}
		return 1;
		case 0010: // A = d
			if (!aDoExecute) {
				aDisassembly << "A = " << DecPrinter(d) << " (" << HexPrinter(d) << ")";
			}
			if (aDoExecute) {
				RefC_A = d;
			}
		return 1;
		case 0011: // A = A & d
			if (!aDoExecute) {
				aDisassembly << "A = A & " << DecPrinter(d) << " (" << HexPrinter(d) << ")";
			}
			if (aDoExecute) {
				RefC_A = RefA & d;
			}
		return 1;
		case 0012: // A = A + d
			if (!aDoExecute) {
				aDisassembly << "A = A + " << DecPrinter(d) << " (" << HexPrinter(d) << ")";
			}
			if (aDoExecute) {
				Increment(d);
			}
		return 1;
		case 0013: // A = A - d
			if (!aDoExecute) {
				aDisassembly << "A = A - " << DecPrinter(d) << " (" << HexPrinter(d) << ")";
			}
			if (aDoExecute) {
				Decrement(d);
			}
		return 1;
		case 0014: // A = k
			if (!aDoExecute) {
				aDisassembly << "A = " << DecPrinter(k) << " (" << HexPrinter(k) << ")";
			}
			if (aDoExecute) {
				RefC_A = k;
			}
		return 2;
		case 0015: // A = A & k
			if (!aDoExecute) {
				aDisassembly << "A = A & " << DecPrinter(k) << " (" << HexPrinter(k) << ")";
			}
			if (aDoExecute) {
				RefC_A = RefA & k;
			}
		return 2;
		case 0016: // A = A + k
			if (!aDoExecute) {
				aDisassembly << "A = A + " << DecPrinter(k) << " (" << HexPrinter(k) << ")";
			}
			if (aDoExecute) {
				Increment(k);
			}
		return 2;
		case 0017: // A = A - k
			if (!aDoExecute) {
				aDisassembly << "A = A - " << DecPrinter(k) << " (" << HexPrinter(k) << ")";
			}
			if (aDoExecute) {
				Decrement(k);
			}
		return 2;
		case 0020: // A = dd
			if (!aDoExecute) {
				aDisassembly << "A = " << RefOr;
			}
			if (aDoExecute) {
				RefC_A = RefOr;
			}
		return 1;
		case 0021: // A = A & dd
			if (!aDoExecute) {
				aDisassembly << "A = A & " << RefOr;
			}
			if (aDoExecute) {
				RefC_A = RefA & RefOr;
			}
		return 1;
		case 0022: // A = A + dd
			if (!aDoExecute) {
				aDisassembly << "A = A + " << RefOr;
			}
			if (aDoExecute) {
				Increment(RefOr);
			}
		return 1;
		case 0023: // A = A - dd
			if (!aDoExecute) {
				aDisassembly << "A = A - " << RefOr;
			}
			if (aDoExecute) {
				Decrement(RefOr);
			}
		return 1;
		case 0024: // dd = A
			if (!aDoExecute) {
				aDisassembly << RefOr << " = A";
			}
			if (aDoExecute) {
				RefOr = RefA;
			}
		return 1;
		case 0025: // dd = A + dd
			if (!aDoExecute) {
				aDisassembly << RefOr << " = A + " << RefOr;
			}
			if (aDoExecute) {
				Increment(RefOr);
				RefOr = RefA;
			}
		return 1;
		case 0026: // dd = dd + 1
			if (!aDoExecute) {
				aDisassembly << RefOr << " = " << RefOr << " + 1";
			}
			if (aDoExecute) {
				RefA = 1;
				RefC = 0;
				Increment(RefOr);
				RefOr = RefA;
			}
		return 1;
		case 0027: // dd = dd - 1
			if (!aDoExecute) {
				aDisassembly << RefOr << " = " << RefOr << " - 1";
			}
			if (aDoExecute) {
				RefA = -1;
				RefC = 0;
				Increment(RefOr);
				RefOr = RefA;
			}
		return 1;
		case 0030: // A = (dd)
			if (!aDoExecute) {
				aDisassembly << "A = (" << RefOr << ")";
			}
			if (aDoExecute) {
				RefC_A = ReadMem(RefOr);
			}
		return 1;
		case 0031: // A = A & (dd)
			if (!aDoExecute) {
				aDisassembly << "A = A & (" << RefOr << ")";
			}
			if (aDoExecute) {
				RefC_A = RefA & ReadMem(RefOr);
			}
		return 1;
		case 0032: // A = A + (dd)
			if (!aDoExecute) {
				aDisassembly << "A = A + (" << RefOr << ")";
			}
			if (aDoExecute) {
				Increment(ReadMem(RefOr));
			}
		return 1;
		case 0033: // A = A - (dd)
			if (!aDoExecute) {
				aDisassembly << "A = A - (" << RefOr << ")";
			}
			if (aDoExecute) {
				Decrement(ReadMem(RefOr));
			}
		return 1;
		case 0034: // (dd) = A
			if (!aDoExecute) {
				aDisassembly << "(" << RefOr << ") = A";
			}
			if (aDoExecute) {
				WriteMem(RefOr, RefA);
			}
		return 1;
		case 0035: // (dd) = A + (dd)
			if (!aDoExecute) {
				aDisassembly << "(" << RefOr << ") = A + (" << RefOr << ")";
			}
			if (aDoExecute) {
				Increment(ReadMem(RefOr));
				WriteMem(RefOr, RefA);
			}
		return 1;
		case 0036: // (dd) = (dd) + 1
			if (!aDoExecute) {
				aDisassembly << "(" << RefOr << ") = (" << RefOr << ") + 1";
			}
			if (aDoExecute) {
				RefA = 1;
				RefC = 0;
				Increment(ReadMem(RefOr));
				WriteMem(RefOr, RefA);
			}
		return 1;
		case 0037: // (dd) = (dd) - 1
			if (!aDoExecute) {
				aDisassembly << "(" << RefOr << ") = (" << RefOr << ") - 1";
			}
			if (aDoExecute) {
				RefA = -1;
				RefC = 0;
				Increment(ReadMem(RefOr));
				WriteMem(RefOr, RefA);
			}
		return 1;
		case 0040: // C = 1, iod = DN
			if (!aDoExecute) {
				aDisassembly << RefC << " = 1, io " << ChannelPrinter(mType, d) << " = DN";
			}
			if (aDoExecute) {
				CRAY_ASSERT(mIoChannels[d] != nullptr);
				RefC = mIoChannels[d]->GetDone();
				aDelayISet = true;
				aBreakBurst = true;
			}
		return 1;
		case 0041: // C = 1, iod = BZ
			if (!aDoExecute) {
				aDisassembly << RefC << " = 1, io " << ChannelPrinter(mType, d) << " = BZ";
			}
			if (aDoExecute) {
				CRAY_ASSERT(mIoChannels[d] != nullptr);
				RefC = mIoChannels[d]->GetBusy();
				aDelayISet = true;
				aBreakBurst = true;
			}
		return 1;
		case 0042: // C = 1, IOB = DN
			if (!aDoExecute) {
				aDisassembly << RefC << " = 1, IOB ";
				if (aInSimulation) aDisassembly << ChannelPrinter(mType, RefB) << " ";
				aDisassembly << "= DN";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				CRAY_ASSERT(mIoChannels[d] != nullptr);
				if (mIoChannels[RefB] == nullptr) {
					// Non-existent channels are always 'done'
					mLogger << setloglevel(LogLevel_Warning) << SideEffectIndent << "WARNING: channel io to non-existent channel" << std::endl;
					RefC = 0;
				} else {
					RefC = mIoChannels[RefB]->GetDone();
				}
				aDelayISet = true;
				aBreakBurst = true;
			}
		return 1;
		case 0043: // C = 1, IOB = BZ
			if (!aDoExecute) {
				aDisassembly << RefC << " = 1, IOB ";
				if (aInSimulation) aDisassembly << ChannelPrinter(mType, RefB) << " ";
				aDisassembly << "= BZ";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				CRAY_ASSERT(mIoChannels[d] != nullptr);
				if (mIoChannels[RefB] == nullptr) {
					// Non-existent channels are never 'busy'
					mLogger << setloglevel(LogLevel_Warning) << SideEffectIndent << "WARNING: channel io to non-existent channel" << std::endl;
					RefC = 0;
				} else {
					RefC = mIoChannels[RefB]->GetBusy();
				}
				aDelayISet = true;
				aBreakBurst = true;
			}
		return 1;
		case 0044: // A = A > B
			if (!aDoExecute) {
				aDisassembly << "A = A > B";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				mShiftRight(RefB);
			}
		return 1;
		case 0045: // A = A < B
			if (!aDoExecute) {
				aDisassembly << "A = A < B";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				mShiftLeft(RefB);
			}
		return 1;
		case 0046: // A = A >> B
			if (!aDoExecute) {
				aDisassembly << "A = A >> B";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				mRotateRight(RefB);
			}
		return 1;
		case 0047: // A = A << B
			if (!aDoExecute) {
				aDisassembly << "A = A << B";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				mRotateLeft(RefB);
			}
		return 1;
		case 0050: // A = B
			if (!aDoExecute) {
				aDisassembly << "A = B";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				RefC_A = RefB;
			}
		return 1;
		case 0051: // A = A & B
			if (!aDoExecute) {
				aDisassembly << "A = A & B";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				RefC_A = RefA & RefB;
			}
		return 1;
		case 0052: // A = A + B
			if (!aDoExecute) {
				aDisassembly << "A = A + B";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				Increment(RefB);
			}
		return 1;
		case 0053: // A = A - B
			if (!aDoExecute) {
				aDisassembly << "A = A - B";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				Decrement(RefB);
			}
		return 1;
		case 0054: // B = A
			if (!aDoExecute) {
				aDisassembly << "B = A";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				RefB = RefA;
			}
		return 1;
		case 0055: // B = A + B
			if (!aDoExecute) {
				aDisassembly << "B = A + B";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				Increment(RefB);
				RefB = RefA;
			}
		return 1;
		case 0056: // B = B + 1
			if (!aDoExecute) {
				aDisassembly << "B = B + 1";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				RefA = 1;
				RefC = 0;
				Increment(RefB);
				RefB = RefA;
			}
		return 1;
		case 0057: // B = B - 1
			if (!aDoExecute) {
				aDisassembly << "B = B - 1";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				RefA = -1;
				RefC = 0;
				Increment(RefB);
				RefB = RefA;
			}
		return 1;
		case 0060: // A = (B)
			if (!aDoExecute) {
				aDisassembly << "A = " << RefOrB;
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				RefC_A = RefOrB;
			}
		return 1;
		case 0061: // A = A & (B)
			if (!aDoExecute) {
				aDisassembly << "A = A & " << RefOrB;
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				RefC_A = RefA & RefOrB;
			}
		return 1;
		case 0062: // A = A + (B)
			if (!aDoExecute) {
				aDisassembly << "A = A + " << RefOrB;
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				Increment(RefOrB);
			}
		return 1;
		case 0063: // A = A - (B)
			if (!aDoExecute) {
				aDisassembly << "A = A - " << RefOrB;
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				Decrement(RefOrB);
			}
		return 1;
		case 0064: // (B) = A
			if (!aDoExecute) {
				aDisassembly << RefOrB << " = A";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				RefOrB = RefA;
			}
		return 1;
		case 0065: // (B) = A + (B)
			if (!aDoExecute) {
				aDisassembly << RefOrB << " = A + " << RefOrB;
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				Increment(RefOrB);
				RefOrB = RefA;
			}
		return 1;
		case 0066: // (B) = (B) + 1
			if (!aDoExecute) {
				aDisassembly << RefOrB << " = " << RefOrB << " + 1";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				RefA = 1;
				RefC = 0;
				Increment(RefOrB);
				RefOrB = RefA;
			}
		return 1;
		case 0067: // (B) = (B) - 1
			if (!aDoExecute) {
				aDisassembly << RefOrB << " = " << RefOrB << " - 1";
				if (d != 0) {
					aExplanation << "**** non-standard encoding with D:" << HexPrinter(d) << " ****";
				}
			}
			if (aDoExecute) {
				RefA = -1;
				RefC = 0;
				Increment(RefOrB);
				RefOrB = RefA;
			}
		return 1;
		default:
			CRAY_ASSERT(false);
	}
	throw Generic_x("Unreachable code");
}

#define DECODE(aFieldF) { &IopCpu_c::Decode<true, aFieldF>, &IopCpu_c::Decode<false, aFieldF> }
#define DECODEJUMP(aFieldF) { &IopCpu_c::DecodeJump<true, aFieldF>, &IopCpu_c::DecodeJump<false, aFieldF> }
#define DECODEIO(aFieldF) { &IopCpu_c::DecodeIo<true, aFieldF>, &IopCpu_c::DecodeIo<false, aFieldF> }

const IopCpu_c::InstImplementation_s IopCpu_c::mInstImplementations[] = {
	DECODE(0000), DECODE(0001), DECODE(0002), DECODE(0003), DECODE(0004), DECODE(0005), DECODE(0006), DECODE(0007),
	DECODE(0010), DECODE(0011), DECODE(0012), DECODE(0013), DECODE(0014), DECODE(0015), DECODE(0016), DECODE(0017),
	DECODE(0020), DECODE(0021), DECODE(0022), DECODE(0023), DECODE(0024), DECODE(0025), DECODE(0026), DECODE(0027),
	DECODE(0030), DECODE(0031), DECODE(0032), DECODE(0033), DECODE(0034), DECODE(0035), DECODE(0036), DECODE(0037),
	DECODE(0040), DECODE(0041), DECODE(0042), DECODE(0043), DECODE(0044), DECODE(0045), DECODE(0046), DECODE(0047),
	DECODE(0050), DECODE(0051), DECODE(0052), DECODE(0053), DECODE(0054), DECODE(0055), DECODE(0056), DECODE(0057),
	DECODE(0060), DECODE(0061), DECODE(0062), DECODE(0063), DECODE(0064), DECODE(0065), DECODE(0066), DECODE(0067),
	DECODEJUMP(0070), DECODEJUMP(0071), DECODEJUMP(0072), DECODEJUMP(0073), DECODEJUMP(0074), DECODEJUMP(0075), DECODEJUMP(0076), DECODEJUMP(0077),
	DECODEJUMP(0100), DECODEJUMP(0101), DECODEJUMP(0102), DECODEJUMP(0103), DECODEJUMP(0104), DECODEJUMP(0105), DECODEJUMP(0106), DECODEJUMP(0107),
	DECODEJUMP(0110), DECODEJUMP(0111), DECODEJUMP(0112), DECODEJUMP(0113), DECODEJUMP(0114), DECODEJUMP(0115), DECODEJUMP(0116), DECODEJUMP(0117),
	DECODEJUMP(0120), DECODEJUMP(0121), DECODEJUMP(0122), DECODEJUMP(0123), DECODEJUMP(0124), DECODEJUMP(0125), DECODEJUMP(0126), DECODEJUMP(0127),
	DECODEJUMP(0130), DECODEJUMP(0131), DECODEJUMP(0132), DECODEJUMP(0133), DECODEJUMP(0134), DECODEJUMP(0135), DECODEJUMP(0136), DECODEJUMP(0137),
	DECODEIO(0140), DECODEIO(0141), DECODEIO(0142), DECODEIO(0143), DECODEIO(0144), DECODEIO(0145), DECODEIO(0146), DECODEIO(0147),
	DECODEIO(0150), DECODEIO(0151), DECODEIO(0152), DECODEIO(0153), DECODEIO(0154), DECODEIO(0155), DECODEIO(0156), DECODEIO(0157),
	DECODEIO(0160), DECODEIO(0161), DECODEIO(0162), DECODEIO(0163), DECODEIO(0164), DECODEIO(0165), DECODEIO(0166), DECODEIO(0167),
	DECODEIO(0170), DECODEIO(0171), DECODEIO(0172), DECODEIO(0173), DECODEIO(0174), DECODEIO(0175), DECODEIO(0176), DECODEIO(0177)
};
#undef DECODE

template <bool aDoExecute> IopInt_t IopCpu_c::Decode(
	IopParcel_t aFirstParcel,
	IopParcel_t aSecondParcel,
	bool aInSimulation,
	std::ostream &aDisassembly,
	std::ostream &aExplanation,
	bool &aDelayISet,
	bool &aBreakBurst
) {
	uint16_t f = (aFirstParcel & 0xfe00) >> 9;
	CRAY_ASSERT(f < 128);
	InstImplementation_s InstImpl = mInstImplementations[f];
	if (aDoExecute) {
		return (this->*InstImpl.Exec)(aFirstParcel, aSecondParcel, aInSimulation, aDisassembly, aExplanation, aDelayISet, aBreakBurst);
	} else {
		return (this->*InstImpl.Disasm)(aFirstParcel, aSecondParcel, aInSimulation, aDisassembly, aExplanation, aDelayISet, aBreakBurst);
	}
}

void IopCpu_c::EventFirePoint_c::Fire(IopCpu_c &aIop) {
	std::string IopType;
	switch (aIop.GetType()) {
		case IopType_MIOP: IopType = "MIOP";
		case IopType_BIOP: IopType = "BIOP";
		case IopType_DIOP: IopType = "DIOP";
		case IopType_XIOP: IopType = "XIOP";
		default: CRAY_ASSERT(false);
	}
	std::stringstream IopName;
	IopName << "IOP" << DecPrinter(aIop.GetCpuId());
	DebugEvent_t FiredEvent = Replace(mEvent, "{iop}", IopName.str());
	FiredEvent = Replace(FiredEvent, "{type}", IopType);
	aIop.GetMainframe().GetEventDispatcher().Fire(FiredEvent);
}

class Mainframe_c &IopCpu_c::GetMainframe() { return mIopCluster->GetMainframe(); }
