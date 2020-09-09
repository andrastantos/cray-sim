#include "iop_host_memory.h"
#include "cray_mainframe.h"
#include <algorithm>
#include <time.h>

/////////////////////////////////////////////////////////
// ChannelHostMemory_c
/////////////////////////////////////////////////////////

IopInt_t ChannelHostMemory_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	switch (aFunction) {
		case 000:
			mItoCActive = false;
			mCtoIActive = false;
			mDone = false;
		return 0;
		case 001:
			mIoMemoryAddr = aData >> 2; // Clear the lowest two bits
		return 0;
		case 002:
			mCentralMemoryAddr = (uint32_t(mCentralMemoryAddr) & 0x000001ff) | (uint32_t(aData & 0x7fff) << 9);
		return 0;
		case 003:
			mCentralMemoryAddr = (uint32_t(mCentralMemoryAddr) & 0xfffffe00) | uint32_t(aData & 0x01ff);
		return 0;
		case 004:
			if (mAllowCtoI) {
				mTransferSize = aData & 0x3fff; // Top two bits are ignored as transfer sizes are measured in QWORDS
				mCtoIActive = true;
				mItoCActive = false;
				mDone = false;
			} else {
				mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel: " << OctPrinter(aFunction,3) << std::endl;
			}
			DoTransfer();
		return 0;
		case 005:
			if (mAllowItoC) {
				mTransferSize = aData & 0x3fff; // Top two bits are ignored as transfer sizes are measured in QWORDS
				mItoCActive = true;
				mCtoIActive = false;
				mDone = false;
			} else {
				mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel:" << OctPrinter(aFunction,3) << std::endl;
			}
			DoTransfer();
		return 0;
		case 006:
			mInterruptEnabled = false;
		return 0;
		case 007:
			mInterruptEnabled = true;
		return 0;
		case 014:
			CRAY_ASSERT(false); // TODO: test modes are not yet supported
		return 0;
		default:
			mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << OctPrinter(aFunction,3) << std::endl;
		return 0;
	}
}

void ChannelHostMemory_c::DoTransfer() {
	if (mItoCActive) {
		mLogger << setloglevel(LogLevel_Communication) << "at " << CurrentTime() << " Transferring " << DecPrinter(mTransferSize) << " QWords" << " from I/O memory address " << HexPrinter(mIoMemoryAddr,4) << " to Central memory address " << HexPrinter(mCentralMemoryAddr,6) << std::endl;
		std::stringstream EventStr;
		EventStr << "HO " << "Transferring " << DecPrinter(mTransferSize) << " QWords" << " from I/O memory address " << HexPrinter(mIoMemoryAddr,4) << " to Central memory address " << HexPrinter(mCentralMemoryAddr,6) << std::endl;
		mParent.GetMainframe().GetEventDispatcher().Fire(EventStr.str());
	}
	if (mCtoIActive) {
		mLogger << setloglevel(LogLevel_Communication) << "at " << CurrentTime() << " Transferring " << DecPrinter(mTransferSize) << " QWords" << " from Central memory address " << HexPrinter(mCentralMemoryAddr,6) << " to I/O memory address " << HexPrinter(mIoMemoryAddr,4) << std::endl;
		std::stringstream EventStr;
		EventStr << "HO " << "Transferring " << DecPrinter(mTransferSize) << " QWords" << " from Central memory address " << HexPrinter(mCentralMemoryAddr,6) << " to I/O memory address " << HexPrinter(mIoMemoryAddr,4) << std::endl;
		mParent.GetMainframe().GetEventDispatcher().Fire(EventStr.str());
	}
	while (mItoCActive || mCtoIActive) {
		CRAY_ASSERT(!mDone);
		CInt_t *CentralMemoryLocation = (CInt_t*)(&mCentralMemory[size_t(mCentralMemoryAddr)*sizeof(CInt_t)]);
		if (mItoCActive) {
			*CentralMemoryLocation = SwapBytesInWords(mParent.IoMemAccessByType<uint64_t>(mIoMemoryAddr));
//			*CentralMemoryLocation = mParent.IoMemAccessByType<uint64_t>(mIoMemoryAddr);
		}
		if (mCtoIActive) {
			mParent.IoMemAccessByType<uint64_t>(mIoMemoryAddr) = SwapBytesInWords(*CentralMemoryLocation);
//			mParent.IoMemAccessByType<uint64_t>(mIoMemoryAddr) = *CentralMemoryLocation;
		}

		LogLine_c LogLine = mLogger << setloglevel(LogLevel_IoTrace);
		LogLine << "Transfered ";
		for(int i=0;i<8;++i) {
			LogLine << HexPrinter(((uint8_t*)CentralMemoryLocation)[i],2) << " ";
		}
		LogLine << " - ";
		for(int i=0;i<8;++i) {
			LogLine << PrintableChar(((uint8_t*)CentralMemoryLocation)[i]);
		}
		LogLine << std::endl;

		if (mIoMemoryAddr != 0x3fff) {
			++mIoMemoryAddr;
		} else {
			mIoMemoryAddr = 0;
		}

		++mCentralMemoryAddr;

		// We're exploiting the fact that this can underflow.
		mTransferSize = (mTransferSize - 1) & 0x3fff;

		if (mTransferSize == 0) {
			mItoCActive = false;
			mCtoIActive = false;
			mDone = true;
		}
	}
}

void ChannelHostMemory_c::Tick() {}
