#include "iop_concentrator.h"
#include "cray_mainframe.h"
#include <algorithm>
#include <time.h>


/////////////////////////////////////////////////////////
// IopConcentrator_c
/////////////////////////////////////////////////////////
void IopConcentrator_c::MasterClear(bool aValue) {
	if (mState != State_InMasterClear && aValue) {
		mLogger << setloglevel(LogLevel_IoActivity) << "Going in to master clear" << std::endl;
		mState = State_InMasterClear;
	}
	if (mState == State_InMasterClear && !aValue) {
		// Falling edge on master clear: reset state...
		mLogger << setloglevel(LogLevel_IoActivity) << "Comming out of master clear" << std::endl;
		mState = State_ClearingBuffers;
		mFromCrayData.clear();
		mToCrayData.clear();
		mToCrayIdx = 0;
		mToCrayLast = true;
		mToCrayPacketIdx = 0;
	}
}

bool IopConcentrator_c::HasData() {
	switch (mState) {
		case State_InMasterClear: return false;
		case State_ClearingBuffers: return false;
		case State_Operational: return mToCrayData.size() > 0 && mToCrayIdx < mToCrayData.size();
		default: CRAY_ASSERT(false);
	}
	throw Generic_x("Unreachable code");
}

CInt_t IopConcentrator_c::GetData() {
	CInt_t Data = 0;
	if(mToCrayIdx < mToCrayData.size()) {
		Data = mToCrayData[mToCrayIdx];
		++mToCrayIdx;
/*		if (mToCrayIdx == mToCrayData.size()) {
			if (!mToCrayLast) {
				SetupNextTransfer();
			} else {
				mToCrayData.clear();
				mToCrayIdx = 0;
				mToCrayLast = true;
			}
		}*/
		mLogger << setloglevel(LogLevel_IoActivity) << "Sending data: " << HexPrinter(Data) << std::endl;
	} else {
		mLogger << setloglevel(LogLevel_IoActivity) << "Sending empty data: " << HexPrinter(Data) << std::endl;
	}
	return Data;
}

void IopConcentrator_c::ProcessFromCrayLcp() {
	mFromCrayLcp = FrontEndLcp_c(mFromCrayData);
	mLogger << setloglevel(LogLevel_IoActivity) << "Processing CRAY message: " << OctPrinter(mFromCrayLcp.GetMessageCode()) << std::endl;
	switch (mFromCrayLcp.GetMessageCode()) {
		case FrontEndLcp_c::MessageCode_Restart: {
/*			FrontEndLcp_c Control;
			Control.SetSourceId(mFrontEndId);
			Control.SetDestinationId(mMainframeId);
			Control.SetNumberOfSubsegments(0);
			Control.SetMessageNumber(1);
			Control.SetMessageCode(FrontEndLcp_c::MessageCode_Control);
			Control.SetDataBitsInSegment(0000);
			mLogger << setloglevel(LogLevel_IoActivity) << "Sending reply of: " << DecPrinter(Control.GetData().size()) << " QWORDS" << std::endl;
			SetToCrayData(Control.GetData(), true);*/
/*		In reality we only send a logon message once a client is connected, not right after the restart. The logic is this:
		- Upon power-up, restart initializes the concentrator and opens it up for network connections
		- Upon connection of a client, logon is sent (this is only accepted on a fully booted system
		At least this is what the logic *should be*. However if I don't send a logon, I get a timeout. If I send a control message, I get some other error...
*/
			//mMainframeId = mFromCrayLcp.GetSourceId();
			FrontEndLcp_c Logon;
			Logon.SetSourceId(mFrontEndId);
			Logon.SetDestinationId(mMainframeId);
			Logon.SetNumberOfSubsegments(1);
			Logon.SetMessageNumber(++mMessageNumber);
			Logon.SetMessageCode(FrontEndLcp_c::MessageCode_Logon);
			Logon.SetDataBitsInSegment(0200); // It is 0200 (2x64) in the documentation, thought the message actually supposed to have 6 QWORDS in it...
			mLogger << setloglevel(LogLevel_IoActivity) << "Sending reply of: " << DecPrinter(Logon.GetData().size()) << " QWORDS" << std::endl;
			SetToCrayData(Logon.GetData(), false);
		}
		break;
		case FrontEndLcp_c::MessageCode_MessageError: {
			std::cout << "Front-end error received: " << OctPrinter(mFromCrayLcp.GetMessageSubcode()) << " for message: " << DecPrinter(mFromCrayLcp.GetMessageNumber()) << std::endl;
			mLogger << setloglevel(LogLevel_IoActivity) << "Front-end error received: " << OctPrinter(mFromCrayLcp.GetMessageSubcode()) << " for message: " << DecPrinter(mFromCrayLcp.GetMessageNumber()) << std::endl;
			if (mFromCrayLcp.GetMessageSubcode() == 0251) {
				std::cout << "Front-end re-sending logon..." << std::endl;
				// Resend logon...
				FrontEndLcp_c Logon;
				Logon.SetSourceId(mFrontEndId);
				Logon.SetDestinationId(mMainframeId);
				Logon.SetNumberOfSubsegments(1);
				Logon.SetMessageNumber(++mMessageNumber);
				Logon.SetMessageCode(FrontEndLcp_c::MessageCode_Logon);
				Logon.SetDataBitsInSegment(0200); // It is 0200 (2x64) in the documentation, thought the message actually supposed to have 6 QWORDS in it...
				mLogger << setloglevel(LogLevel_IoActivity) << "Sending reply of: " << DecPrinter(Logon.GetData().size()) << " QWORDS" << std::endl;
				SetToCrayData(Logon.GetData(), false);
			}
		}
		break;
		default: CRAY_ASSERT(false);
	}
}

void IopConcentrator_c::SetupNextTransfer() {
	switch (mFromCrayLcp.GetMessageCode()) {
		case FrontEndLcp_c::MessageCode_Restart: {
			std::vector<CInt_t> Data;
			Data.resize(6,0);
			//        STYP        DBSZ        CKSZ        MIST         MOST         MAST         MSSG           SSGZ
			Data[0] = 2ULL << 0 | 0ULL << 2 | 0ULL << 8 | 1ULL << 16 | 1ULL << 24 | 2ULL << 32 | 255ULL << 40 | 255ULL << 48;
			//        MRE         SLOT        ISLT        VARS         FRCE         MTE           DSZ
			Data[1] = 1ULL << 0 | 1ULL << 8 | 1ULL << 9 | 1ULL << 10 | 0ULL << 15 | 15ULL << 16 | 0ULL << 40;
			//        STOP
			//Data[4] = 0ULL; //TODO: This should be the password, but what is it???
			mLogger << setloglevel(LogLevel_IoActivity) << "Sending data segment of : " << DecPrinter(Data.size()) << " QWORDS" << std::endl;
			SetToCrayData(Data, true);
		}
		break;
		default: CRAY_ASSERT(false);
	}
}

void IopConcentrator_c::SetData(CInt_t aData) {
	mLogger << setloglevel(LogLevel_IoActivity) << "Receiving data: " << HexPrinter(aData) << std::endl;
	switch (mState) {
		case State_ClearingBuffers:
			if ((aData & 0xffff000000000000ULL) == 0x0000000000000000ULL) {
				BufferClearComplete();
			}
		break;
		case State_Operational:
			mFromCrayData.push_back(aData);
		break;
		default: break;
	}
}

void IopConcentrator_c::ProcessFromCrayData() {
	mLogger << setloglevel(LogLevel_IoActivity) << "Processing CRAY data of " << DecPrinter(mFromCrayData.size()) << " QWORDs" << std::endl;
	if (mFromCrayData.size() == FrontEndLcp_c::LcpSize) ProcessFromCrayLcp();
	mFromCrayData.clear();
}

void IopConcentrator_c::TransferCompleted() {
	if (!mToCrayLast) {
		mLogger << setloglevel(LogLevel_IoActivity) << "Setting up next transfer" << std::endl;
		SetupNextTransfer();
	} else {
/*		mLogger << setloglevel(LogLevel_IoActivity) << "Setting up poll reply" << std::endl;

		FrontEndLcp_c PollReply;
		PollReply.SetSourceId(mFrontEndId);
		PollReply.SetDestinationId(mMainframeId);
		PollReply.SetNumberOfSubsegments(0);
		PollReply.SetMessageNumber(0);
		PollReply.SetMessageCode(0);
		PollReply.SetDataBitsInSegment(0);
		SetToCrayData(PollReply.GetData(), true);*/
	}
}

void IopConcentrator_c::BufferClearComplete() {
	mLogger << setloglevel(LogLevel_IoActivity) << "State transition to State_Operational" << std::endl;
	CRAY_ASSERT(mState == State_ClearingBuffers);
	mState = State_Operational;
	mFromCrayData.clear();
	mToCrayData.clear();
	mToCrayIdx = 0;
	mToCrayLast = true;
	mToCrayPacketIdx = 0;
}

void IopConcentrator_c::LogState(const char *aHeader) const {
	switch (mState) {
		case State_InMasterClear:   mLogger << setloglevel(LogLevel_IoActivity) << aHeader << " State: State_InMasterClear" << std::endl; break;
		case State_ClearingBuffers: mLogger << setloglevel(LogLevel_IoActivity) << aHeader << " State: State_ClearingBuffers" << std::endl; break;
		case State_Operational:     mLogger << setloglevel(LogLevel_IoActivity) << aHeader << " State: State_Operational" << std::endl; break;
		default: CRAY_ASSERT(false);
	}
}

void IopConcentrator_c::Tick() {
/*	if (mState == State_Operational) {
		++mPollTimer;
		if (mPollTimer == mPollDelay) {
			mPollTimer = 0;
			if (!HasData()) {
				mLogger << setloglevel(LogLevel_IoActivity) << "Setting up poll reply" << std::endl;

				FrontEndLcp_c PollReply;
				PollReply.SetSourceId(mFrontEndId);
				PollReply.SetDestinationId(mMainframeId);
				PollReply.SetNumberOfSubsegments(0);
				PollReply.SetMessageNumber(0);
				PollReply.SetMessageCode(0);
				PollReply.SetDataBitsInSegment(0);
				SetToCrayData(PollReply.GetData(), true);
			} else {
				mLogger << setloglevel(LogLevel_IoActivity) << "Not setting up poll reply mToCrayData.size()=" << DecPrinter(_ToCrayData.size()) << " _ToCrayIdx=" << DecPrinter(_ToCrayIdx) << std::endl;
			}
		}
	}*/
}

/////////////////////////////////////////////////////////
// IopConcentratorChannel_c
/////////////////////////////////////////////////////////

IopConcentratorChannel_c::IopConcentratorChannel_c(class IopConcentrator_c &aParent, const Configuration_c &aConfig, size_t aChannelIdx, bool aAllowOut, bool aAllowIn):
	mParent(aParent),
	mAllowOut(aAllowOut),
	mAllowIn(aAllowIn),
	mChannelIdx(aChannelIdx),
	mChannelActive(false)
{
	CRAY_ASSERT(aAllowOut ^ aAllowIn);
	mTransferDelay = aConfig.get<size_t>("TransferDelay", 10);
	MasterClear();
}

IopInt_t IopConcentratorChannel_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	mChannelActive = true; // TODO: this is not what should happen: this is set from the other side, when the destination address is written
	//mParent.LogState("DoIo begin");
	mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << (mAllowIn?"CONCI":"CONCO") << " DoIo with function code: " << OctPrinter(aFunction) << " data: " << HexPrinter(aData) << std::endl;
	switch (aFunction) {
		case 000:
			mInActive = false;
			mOutActive = false;
			mDone = false;
		return 0;
		case 001:
			mIoMemoryAddr = aData >> 2; // Clear the lowest two bits
			if (mAllowIn) {
				if (mParent.HasData()) {
					mInActive = true;
					mOutActive = false;
					mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "Initiating transfer of " << DecPrinter(mTransferSize) << " QWords" << " from Concentrator to I/O memory address " << HexPrinter(mIoMemoryAddr,4) << std::endl;
					mDone = false;
				} else {
					mInActive = false;
					mOutActive = false;
					mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "Data transfer request with no data pending: " << DecPrinter(mTransferSize) << " QWords" << " from Concentrator to I/O memory address " << HexPrinter(mIoMemoryAddr,4) << std::endl;
					mDone = false;
				}
			}
			if (mAllowOut) {
				mInActive = false;
				mOutActive = true;
				mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "Initiating transfer of " << DecPrinter(mTransferSize) << " QWords" << " from I/O memory address " << HexPrinter(mIoMemoryAddr,4) << " to Concentrator" << std::endl;
				mDone = false;
			}
			mTransferDelayCounter = mTransferDelay;
		return 0;
		case 002:
			mTransferSize = aData >> 2;
		return 0;
		case 003:
			// Clear parity error bits for CIA and sequence error flags for COA
		return 0;
		case 004:
			// Would set active to false, but that's not supported yet
			if (mAllowIn) {
				mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "CONCI function 4 with value: " << HexPrinter(aData,4) << std::endl;
				//mChannelActive = false;
			}
			if (mAllowOut) {
				mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "CONCO function 4 with value: " << HexPrinter(aData,4) << std::endl;
/*				if ((aData & (1 << 4)) != 0) {
					// This bit seems to be set once the buffer clear is complete
					mParent.BufferClearComplete();
				}*/
				if ((aData & (1 << 8)) != 0) {
					mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "would be sending disconnect to concentrator!" << std::endl;
				}
				mHoldDisconnect = ((aData & (1 << 9)) != 0);
				mParent.MasterClear((aData & (1 << 14)) != 0);
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
			if (mAllowIn) aData = mParent.HasData() ? (1 << 15) : 0; // Would return parity error bits in the low-order four bits - for now we won't signal ready waiting as it is an endless loop during initialization
			if (mAllowOut) aData = 0; // Would return sequence error and 4-bit channels none of which is supported
			if (mAllowIn) {
				mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "CONCI function 011 returning: " << HexPrinter(aData,4) << std::endl;
			}
			if (mAllowOut) {
				mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "CONCO function 011 returning: " << HexPrinter(aData,4) << std::endl;
			}
			return aData;
		CRAY_ASSERT(false);
		return 0;
		default:
			mParent.GetLogger() << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

IopBit_t IopConcentratorChannel_c::GetInterrupt() {
	if (mInterruptEnabled && mDone) mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "Sending interrupt" << std::endl;
	return mInterruptEnabled && mDone;
}

void IopConcentratorChannel_c::Tick() {
/*	if ((mInActive || mOutActive) && mChannelActive) {
		if (mTransferDelayCounter > 0) {
			--mTransferDelayCounter;
			return;
		}
		mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "Transfer resumed" << std::endl;
	}*/
	if (mAllowIn) mParent.Tick(); // Forward ticks to concentrator, but only once per tick (i.e. from the input channel only)

	while ((mInActive || mOutActive) && mChannelActive) {
		CRAY_ASSERT(!mDone);
		if (mOutActive) {
			CInt_t Data = SwapBytesInWords(mParent.GetParent().IoMemAccessByType<uint64_t>(mIoMemoryAddr));
			mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "Sending data " << HexPrinter(Data) << " to concentrator!" << std::endl;
			mParent.SetData(SwapBytes(Data));
		}
		if (mInActive) {
			CInt_t Data = SwapBytes(mParent.GetData());
			mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "Receiving data " << HexPrinter(Data) << " from concentrator!" << std::endl;
			mParent.GetParent().IoMemAccessByType<uint64_t>(mIoMemoryAddr) = SwapBytesInWords(Data);
		}
		if (mIoMemoryAddr != 0x3fff) {
			++mIoMemoryAddr;
		} else {
			mIoMemoryAddr = 0;
		}
		// We're exploiting the fact that this can underflow.
		mTransferSize = (mTransferSize - 1) & 0x3fff;

		if (mTransferSize == 0 || (mInActive && !mParent.HasData())) {
			if (!mHoldDisconnect && mOutActive) {
				mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "would be sending disconnect to concentrator!" << std::endl;
			}
			mInActive = false;
			mOutActive = false;
			mDone = true;
			mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "Transfer completed" << std::endl;
			if (mAllowOut) mParent.ProcessFromCrayData();
			if (mAllowIn) mParent.TransferCompleted();
		}
	}
}
