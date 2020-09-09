#include "iop_iop2cray.h"
#include "cray_mainframe.h"
#include <algorithm>
#include <time.h>

/////////////////////////////////////////////////////////
// IopCrayChannel_c
/////////////////////////////////////////////////////////


IopInt_t IopCrayChannel_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	switch (aFunction) {
		case 000:
			mItoCActive = false;
			mCtoIActive = false;
			mDone = false;
		return 0;
		case 001:
			mIoMemoryAddr = aData >> 2; // Clear the lowest two bits
			if (mAllowCtoI) {
				mCtoIActive = true;
				mItoCActive = false;
				mLogger << setloglevel(LogLevel_Communication) << "at " << CurrentTime() << " " << (!IsActive() ? " Attempting to transfer " : "Transferring ") << DecPrinter(mTransferSize) << " QWords" << " from Central memory to I/O memory address " << HexPrinter(mIoMemoryAddr,4) << std::endl;
				mDone = false;
			}
			if (mAllowItoC) {
				mCtoIActive = false;
				mItoCActive = true;
				mLogger << setloglevel(LogLevel_Communication) << "at " << CurrentTime() << " " << (!IsActive() ? " Attempting to transfer " : "Transferring ") << DecPrinter(mTransferSize) << " QWords" << " from I/O memory address " << HexPrinter(mIoMemoryAddr,4) << " to Central memory" << std::endl;
				mDone = false;
			}
			DoTransfer();
		return 0;
		case 002:
			mTransferSize = aData >> 2;
		return 0;
		case 003:
			// Clear parity error bits for CIA and sequence error flags for COA
		return 0;
		case 004:
			if (mAllowCtoI) {
				// Description from the documentation:
				//   Clears the ready-waiting flag. The channel sets a
				//   ready-waiting flag when it is inactive and receives a
				//   Ready signal from the CRI mainframe. The LIA : 4
				//   function discards the Ready signal by clearing the
				//   ready-waiting flag. The Ready signal must be discarded
				//   to resynchronize the transfer. This function does not
				//   affect the busy and done flags.
				// The 'ready-waiting' bit is queried by function code 011 below.
				// It appears that these channels had a dual-DMA functionality and I
				// managed to mix things up a little here: the READY bit is what the
				// GetActive() method call returns. The ready-waiting is something else,
				// somewhat of a latch: it gets set if the mainframe side is ready to send
				// but the IOP side is not ready to receive. This is the flag that gets cleared
				// here, not the active flag.
				//
				// Since I don't have the equivalent functionality implemented, let's at least
				// not clear the mainframe-side flag, which is by all accounts the wrong thing
				// to do here.
				mReadyWaiting = false;
				// SetActive(false);
			}
			if (mAllowItoC) {
				if ((aData & (1 << 8)) != 0) {
					mLogger << setloglevel(LogLevel_IoActivity) << "sending disconnect to mainframe!" << std::endl;
					Disconnect();
				}
				mHoldDisconnect = ((aData & (1 << 9)) != 0);
				CRAY_ASSERT((aData & (1 << 11)) == 0); // This is the DEAD DUMP bit, which we don't support
				CRAY_ASSERT((aData & (1 << 12)) == 0); // This is the RTC INTERRUPT bit, which we don't support
				CRAY_ASSERT((aData & (1 << 13)) == 0); // This is the ERROR CHANNEL RESUME bit, which we don't support
				// Master clear bits are actually meaningful in a SW emulation on the falling edge...
				if ((aData & (1 << 14)) != 0) { // This is the I/O MASTER CLEAR bit, which we don't support
					mLogger << setloglevel(LogLevel_IoActivity) << "setting master clear to I/O!" << std::endl;
					mMainframe.IoMasterClear(true);
				}
				if ((aData & (1 << 14)) == 0) { // This is the I/O MASTER CLEAR bit, which we don't support
					mLogger << setloglevel(LogLevel_IoActivity) << "clearing master clear to I/O!" << std::endl;
					mMainframe.IoMasterClear(false);
				}
				if ((aData & (1 << 15)) != 0) { // This is the CPU MASTER CLEAR bit
					mLogger << setloglevel(LogLevel_IoActivity) << "setting master clear to mainframe!" << std::endl;
					mMainframe.CpuMasterClear(true);
				}
				if ((aData & (1 << 15)) == 0) { // This is the CPU MASTER CLEAR bit
					mLogger << setloglevel(LogLevel_IoActivity) << "clearing master clear to mainframe!" << std::endl;
					mMainframe.CpuMasterClear(false);
				}
			}
		return 0;
		case 006:
			mInterruptEnabled = false;
		return 0;
		case 007:
			mInterruptEnabled = true;
		return 0;
		case 010:
		return mIoMemoryAddr << 2;
		case 011:
			if (mAllowCtoI) return mReadyWaiting ? (1 << 15) : 0; // Would return parity error bits in the low-order four bits
			if (mAllowItoC) return 0; // Would return sequence error and 4-bit channels none of which is supported
		CRAY_ASSERT(false);
		default:
			mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

void IopCrayChannel_c::HandleActivation() {
	// If the mainframe side went active ('ready') and the IOP side is not active,
	// set the ready-waiting flag. This indicate that there's something wrong...
	if (!mCtoIActive) mReadyWaiting = true;
	DoTransfer();
}

void IopCrayChannel_c::SetAddress(CAddr_t aAddress) {
	Channel_i::SetAddress(aAddress);
	mLogger << setloglevel(LogLevel_Communication) << "at " << CurrentTime() << " mainframe CPU sending " << DecPrinter(BufferLeft()) << " QWORDS of data from address " << HexPrinter(GetAddress()) << std::endl;
	//Guard_t IopGuard(mIopLock);
	//DoTransfer();
}

void IopCrayChannel_c::DoTransfer() {
	if ((mItoCActive || mCtoIActive) && IsActive()) {
		mLogger << setloglevel(LogLevel_Communication) << "at " << CurrentTime() << " Transfer resumed from between mainframe address: " << HexPrinter(GetAddress()) << " and IOP address: " << HexPrinter(mIoMemoryAddr) << std::endl;
	}
	while ((mItoCActive || mCtoIActive) && IsActive()) {
		CRAY_ASSERT(!mDone);
		if (mItoCActive) {
			CInt_t Data = SwapBytes(SwapBytesInWords(mParent.IoMemAccessByType<uint64_t>(mIoMemoryAddr)));
			LogLine_c LogLine = mLogger << setloglevel(LogLevel_IoTrace);
			if (LogLine.good()) {
				LogLine << "I->C " << HexPrinter(Data) << " - ";
				for (size_t ByteIdx = 0; ByteIdx < 8; ++ByteIdx) {
					uint8_t Byte = uint8_t(Data >> (8 * (7 - ByteIdx)) & 0xff);
					LogLine << PrintableChar(Byte);
				}
				LogLine << std::endl;
			}
			mMsgBuffer.push_back(Data);
			SetData(Data);
		}
		if (mCtoIActive) {
			CInt_t Data = GetData();
			LogLine_c LogLine = mLogger << setloglevel(LogLevel_IoTrace);
			if (LogLine.good()) {
				LogLine << "C->I " << HexPrinter(Data) << " - ";
				for (size_t ByteIdx = 0; ByteIdx < 8; ++ByteIdx) {
					uint8_t Byte = uint8_t(Data >> (8 * (7 - ByteIdx)) & 0xff);
					LogLine << PrintableChar(Byte);
				}
				LogLine << std::endl;
			}
			mMsgBuffer.push_back(Data);
			mParent.IoMemAccessByType<uint64_t>(mIoMemoryAddr) = SwapBytesInWords(SwapBytes(Data));
		}
		if (mIoMemoryAddr != 0x3fff) {
			++mIoMemoryAddr;
		} else {
			mIoMemoryAddr = 0;
		}
		// We're exploiting the fact that this can underflow.
		mTransferSize = (mTransferSize - 1) & 0x3fff;

		if (mTransferSize == 0) {
			if (!mHoldDisconnect && mItoCActive) {
				Disconnect();
			}
			mItoCActive = false;
			mCtoIActive = false;
			mDone = true;
			DecodeBuffer();
			mMsgBuffer.clear();
			mLogger << setloglevel(LogLevel_IoActivity) << "Transfer completed" << std::endl;
		}
	}
}

void IopCrayChannel_c::DecodeBuffer() {
	uint16_t DID = uint16_t(GetBitsReverse(mMsgBuffer[0], 0, 15));
	uint8_t DTID = uint8_t(GetBits(uint32_t(DID), 0, 7));
	uint8_t DPT = uint8_t(GetBits(uint32_t(DID), 8, 15));
	uint16_t SID = uint16_t(GetBitsReverse(mMsgBuffer[0], 16, 31));
	uint8_t STID = uint8_t(GetBits(uint32_t(SID), 0, 7));
	uint8_t SPT = uint8_t(GetBits(uint32_t(SID), 8, 15));
	if (STID == '1' && SPT == 'C') {
		switch (DTID) {
			case 'A': {
				uint8_t CHK = uint8_t(GetBitsReverse(mMsgBuffer[1], 32, 32));
				uint16_t SBK = uint16_t(GetBitsReverse(mMsgBuffer[1], 33, 47));
				uint8_t PST = uint8_t(GetBitsReverse(mMsgBuffer[1], 48, 49));
				uint16_t PID = uint16_t(GetBitsReverse(mMsgBuffer[1], 50, 63));
				uint32_t EQT = uint32_t(GetBitsReverse(mMsgBuffer[2], 40, 63));
				uint32_t DA = uint32_t(GetBitsReverse(mMsgBuffer[3], 0, 31));
				uint8_t FCT = uint8_t(GetBitsReverse(mMsgBuffer[3], 32, 39));
				uint8_t STS = uint8_t(GetBitsReverse(mMsgBuffer[3], 40, 47));
				uint8_t DT = uint8_t(GetBitsReverse(mMsgBuffer[3], 48, 52));
				uint8_t PN = uint8_t(GetBitsReverse(mMsgBuffer[3], 53, 54));
				uint8_t CHN = uint8_t(GetBitsReverse(mMsgBuffer[3], 55, 63));
				uint16_t CYL = uint16_t(GetBitsReverse(mMsgBuffer[4], 0, 10));
				uint8_t HD = uint8_t(GetBitsReverse(mMsgBuffer[4], 11, 15));
				uint8_t SEC = uint8_t(GetBitsReverse(mMsgBuffer[4], 16, 22));
				uint16_t OFF = uint16_t(GetBitsReverse(mMsgBuffer[4], 23, 31));
				uint32_t WL = uint32_t(GetBitsReverse(mMsgBuffer[4], 32, 63));
				uint32_t MOS = uint32_t(GetBitsReverse(mMsgBuffer[5], 0, 31));
				uint16_t RAC = uint16_t(GetBitsReverse(mMsgBuffer[5], 32, 47));
				uint16_t RES = uint16_t(GetBitsReverse(mMsgBuffer[5], 48, 63));
				std::stringstream EventStr;
				EventStr << "C->I PACKET A request -" <<
					" PST: " << DecPrinter(PST) <<
					" PID: " << DecPrinter(PID) <<
					" EQT: " << HexPrinter(EQT) <<
					" DA: " << HexPrinter(DA) <<
					" FCT: " << HexPrinter(FCT) <<
					" STS: " << HexPrinter(STS) <<
					" DT: " << DecPrinter(DT) <<
					" CHN: " << OctPrinter(CHN) <<
					" CHS: " << DecPrinter(CYL) << "-" << DecPrinter(HD) << "-" << DecPrinter(SEC) <<
					" OFF: " << HexPrinter(OFF) <<
					" WL: " << HexPrinter(WL) <<
					" MOS: " << HexPrinter(MOS) <<
					" RAC: " << DecPrinter(RAC) <<
					std::endl;
				mLogger << setloglevel(LogLevel_EventFire) << EventStr.str();
				mParent.GetMainframe().GetEventDispatcher().Fire(EventStr.str());
			}
			break;
			default:
			break;
		}
	}
	if (DTID == '1' && DPT == 'C') {
		switch (STID) {
			case 'A': {
				uint8_t CHK = uint8_t(GetBitsReverse(mMsgBuffer[1], 32, 32));
				uint16_t SBK = uint16_t(GetBitsReverse(mMsgBuffer[1], 33, 47));
				uint8_t PST = uint8_t(GetBitsReverse(mMsgBuffer[1], 48, 49));
				uint16_t PID = uint16_t(GetBitsReverse(mMsgBuffer[1], 50, 63));
				uint32_t EQT = uint32_t(GetBitsReverse(mMsgBuffer[2], 40, 63));
				uint32_t DA = uint32_t(GetBitsReverse(mMsgBuffer[3], 0, 31));
				uint8_t FCT = uint8_t(GetBitsReverse(mMsgBuffer[3], 32, 39));
				uint8_t STS = uint8_t(GetBitsReverse(mMsgBuffer[3], 40, 47));
				uint8_t DT = uint8_t(GetBitsReverse(mMsgBuffer[3], 48, 52));
				uint8_t PN = uint8_t(GetBitsReverse(mMsgBuffer[3], 53, 54));
				uint8_t CHN = uint8_t(GetBitsReverse(mMsgBuffer[3], 55, 63));
				uint16_t CYL = uint16_t(GetBitsReverse(mMsgBuffer[4], 0, 10));
				uint8_t HD = uint8_t(GetBitsReverse(mMsgBuffer[4], 11, 15));
				uint8_t SEC = uint8_t(GetBitsReverse(mMsgBuffer[4], 16, 22));
				uint16_t OFF = uint16_t(GetBitsReverse(mMsgBuffer[4], 23, 31));
				uint32_t WL = uint32_t(GetBitsReverse(mMsgBuffer[4], 32, 63));
				uint32_t MOS = uint32_t(GetBitsReverse(mMsgBuffer[5], 0, 31));
				uint16_t RAC = uint16_t(GetBitsReverse(mMsgBuffer[5], 32, 47));
				uint16_t RES = uint16_t(GetBitsReverse(mMsgBuffer[5], 48, 63));
				std::stringstream EventStr;
				EventStr << "I->C PACKET A response -" <<
					" PST: " << DecPrinter(PST) <<
					" PID: " << DecPrinter(PID) <<
					" EQT: " << HexPrinter(EQT) <<
					" DA: " << HexPrinter(DA) <<
					" FCT: " << HexPrinter(FCT) <<
					" STS: " << HexPrinter(STS) <<
					" DT: " << DecPrinter(DT) <<
					" CHN: " << OctPrinter(CHN) <<
					" CHS: " << DecPrinter(CYL) << "-" << DecPrinter(HD) << "-" << DecPrinter(SEC) <<
					" OFF: " << HexPrinter(OFF) <<
					" WL: " << HexPrinter(WL) <<
					" MOS: " << HexPrinter(MOS) <<
					" RAC: " << DecPrinter(RAC) <<
					std::endl;
				mLogger << setloglevel(LogLevel_EventFire) << EventStr.str();
				mParent.GetMainframe().GetEventDispatcher().Fire(EventStr.str());
			}
			break;
			default:
			break;
		}
	}
}

void IopCrayChannel_c::Tick() {}
bool IopCrayChannel_c::NeedsTick() { return false; }
