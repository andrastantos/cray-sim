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

IopInt_t IopCpu_c::ChannelIOR_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	if (aFunction != 010) {
		mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
	for(size_t i=0;i<mParent.mIoChannels.size();++i) {
		if (mParent.mIoChannels[i] != nullptr) {
			if (mParent.mIoChannels[i]->GetInterrupt() != 0) {
				mLogger << setloglevel(LogLevel_Event) << " IOR sending interrupt from channel " << DecPrinter(i) << std::endl;
				return IopInt_t(i);
			}
		}
	}
	return 0;
}

IopInt_t IopCpu_c::ChannelPFR_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	switch (aFunction) {
		case 000:
			mRequestFlag = false;
		return 0;
		case 006:
			mInterruptEnabled = false;
		return 0;
		case 007:
			mInterruptEnabled = true;
		return 0;
		case 010:
		return mRequestIdx;
		default:
			mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

IopInt_t IopCpu_c::ChannelPXS_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	switch (aFunction) {
		case 000:
			mBundaryFlag = false;
		return 0;
		case 006:
			mInterruptEnabled = false;
		return 0;
		case 007:
			mInterruptEnabled = true;
		return 0;
		case 010:
		return GetE();
		case 011:
		return GetAtE();
		case 014:
			SetE(uint8_t(aData));
		return 0;
		case 015:
			SetAtE(aData);
		return 0;
		default:
			mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

void IopCpu_c::ChannelPXS_c::Dump(size_t aIdent) {
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "E: " << DecPrinter(mE) << std::endl;
	for(size_t Idx=0;Idx<sizeof(mProgramExitStack)/sizeof(mProgramExitStack[0]);++Idx) {
		mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Program Exit Stack[" << DecPrinter(Idx,2) << "]: " << HexPrinter(mProgramExitStack[Idx]) << std::endl;
	}
	mLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Interrupt Enabled: " << (mInterruptEnabled ? "yes":"no") << std::endl;
}

std::string IopCpu_c::ChannelPXS_c::StackDump(bool aSingleLine) {
	std::stringstream Str;
	if (aSingleLine) {
		for(size_t Idx=mE;Idx>=1;--Idx) {
			Str << "[" << DecPrinter(Idx,2) << "]: " << HexPrinter(mProgramExitStack[Idx]) << "  ";
		}
	} else {
		Str << "E: " << DecPrinter(mE) << std::endl;
		for(size_t Idx=0;Idx<sizeof(mProgramExitStack)/sizeof(mProgramExitStack[0]);++Idx) {
			Str << "Program Exit Stack[" << DecPrinter(Idx,2) << "]: " << HexPrinter(mProgramExitStack[Idx]) << std::endl;
		}
		Str << "Interrupt Enabled: " << (mInterruptEnabled ? "yes":"no") << std::endl;
	}
	return Str.str();
}

IopInt_t IopCpu_c::ChannelLME_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	switch (aFunction) {
		case 000:
		return 0;
		case 006:
		return 0;
		case 007:
		return 0;
		case 010:
		return 0;
		default:
			mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}


// ChannelRTC_c
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IopInt_t IopCpu_c::ChannelRTC_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	switch (aFunction) {
		case 000:
			mInterrupt = false;
		return 0;
		case 006:
			mInterruptEnabled = false;
		return 0;
		case 007:
			mInterruptEnabled = true;
		return 0;
		case 010:
		return (TimeToTick(mTimer.elapsed().wall) >> 1) & 0xffff;
		default:
			mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

void IopCpu_c::ChannelRTC_c::Tick() {
	if (++mTickCnt > 50) {
		mTickCnt = 0;
		uint64_t TickCnt = TimeToTick(mTimer.elapsed().wall);
		if (TickCnt - mLastIntTick > mTimerLimit) {
			mInterrupt = true;
			mLastIntTick = TickCnt;
		}
	}
}

// ChannelMOS_c
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IopInt_t IopCpu_c::ChannelMOS_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	switch (aFunction) {
		case 000:
			mItoBActive = false;
			mBtoIActive = false;
			mDone = false;
		return 0;
		case 001:
			mIoMemoryAddr = aData >> 2; // Clear the lowest two bits
		return 0;
		case 002:
			mBufferMemoryAddr = (uint32_t(mBufferMemoryAddr) & 0x000001ff) | (uint32_t(aData & 0x7fff) << 9);
		return 0;
		case 003:
			mBufferMemoryAddr = (uint32_t(mBufferMemoryAddr) & 0xfffffe00) | uint32_t(aData & 0x01ff);
		return 0;
		case 004:
			mTransferSize = aData & 0x3fff; // Top two bits are ignored as transfer sizes are measured in QWORDS
			mBtoIActive = true;
			mDone = false;
			// Do the actual transfer right here and now
			DoTransfer();
		return 0;
		case 005:
			mTransferSize = aData & 0x3fff; // Top two bits are ignored as transfer sizes are measured in QWORDS
			mItoBActive = true;
			mDone = false;
			// Do the actual transfer right here and now
			DoTransfer();
		return 0;
		case 006:
			mInterruptEnabled = false;
		return 0;
		case 007:
			mInterruptEnabled = true;
		return 0;
		case 014:
		return 0;
		default:
			mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

void IopCpu_c::ChannelMOS_c::DoTransfer() {
	CRAY_ASSERT(!(mItoBActive && mBtoIActive));

	// Using the if here to properly scope the stringstream object which is slow to construct
	// TODO: do we really need to do this? I don't think DoTransfer is called all that often any more as Tick is optimized out...
	if (mItoBActive || mBtoIActive) {
		std::stringstream EventStr;
		if (mItoBActive) {
			EventStr << "MOS Transferring " << DecPrinter(mTransferSize) << " QWords" << " from I/O memory address " << HexPrinter(mIoMemoryAddr * 4, 4) << " to Buffer memory address " << HexPrinter(mBufferMemoryAddr, 6);
		}
		if (mBtoIActive) {
			EventStr << "MOS Transferring " << DecPrinter(mTransferSize) << " QWords" << " from Buffer memory address " << HexPrinter(mBufferMemoryAddr, 6) << " to I/O memory address " << HexPrinter(mIoMemoryAddr * 4, 4);
		}
		mLogger << setloglevel(LogLevel_IoActivity) << EventStr.str() << std::endl;
		EventStr << std::endl;
		mParent.GetMainframe().GetEventDispatcher().Fire(EventStr.str());
	}

	LogLine_c TraceLine = mLogger << setloglevel(LogLevel_IoTrace);
	while (mItoBActive || mBtoIActive) {
		CRAY_ASSERT(!mDone);
		if (mItoBActive) {
			mParent.BufferMemAccessByType<uint64_t>(mBufferMemoryAddr) = SwapBytesInWords(mParent.IoMemAccessByType<uint64_t>(mIoMemoryAddr));
			TraceLine << "MOS Transferred " ;
			for(int i=0;i<8;++i) {
				TraceLine << HexPrinter(mParent.IoMemAccessByType<uint8_t>(mIoMemoryAddr * 8 + (i & 6) + (1 - (i & 1))), 2) << " ";
			}
			TraceLine << " - ";
			for(int i=0;i<8;++i) {
				TraceLine << PrintableChar(mParent.IoMemAccessByType<uint8_t>(mIoMemoryAddr * 8 + (i & 6) + (1 - (i & 1))));
			}
			TraceLine << std::endl;
		}
		if (mBtoIActive) {
			mParent.IoMemAccessByType<uint64_t>(mIoMemoryAddr) = SwapBytesInWords(mParent.BufferMemAccessByType<uint64_t>(mBufferMemoryAddr));
			TraceLine << "MOS Transferred ";
			for(int i=0;i<8;++i) {
				TraceLine << HexPrinter(mParent.IoMemAccessByType<uint8_t>(mIoMemoryAddr * 8 + (i & 6) + (1 - (i & 1))), 2) << " ";
			}
			TraceLine << " - ";
			for(int i=0;i<8;++i) {
				TraceLine << PrintableChar(mParent.IoMemAccessByType<uint8_t>(mIoMemoryAddr * 8 + (i & 6) + (1 - (i & 1))));
			}
			TraceLine << std::endl;
		}
		if (mIoMemoryAddr != 0x3fff) {
			++mIoMemoryAddr;
		} else {
			mIoMemoryAddr = 0;
		}

		++mBufferMemoryAddr;

		// We're exploiting the fact that this can underflow.
		mTransferSize = (mTransferSize - 1) & 0x3fff;

		if (mTransferSize == 0) {
			mItoBActive = false;
			mBtoIActive = false;
			mDone = true;
		}
	}
}

