#include "iop_concentrator.h"
#include "cray_mainframe.h"
#include <algorithm>
#include <time.h>


/////////////////////////////////////////////////////////
// IopConcentrator_c
/////////////////////////////////////////////////////////
void IopConcentrator_c::StartListen() {
        mLogger << setloglevel(LogLevel_IoActivity) << "Listen on port " << DecPrinter(mPort) << std::endl;
        try {
                mListenSocket = std::make_shared<boost::asio::ip::tcp::acceptor>(mIoService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), mPort), true);
        	mConnectionSocket = std::make_shared<boost::asio::ip::tcp::socket>(mIoService);
        }
        catch (boost::system::system_error &Ex) {
                throw Generic_x() << "Failed to create listen socket: " << Ex.code().message();
        }
}

void IopConcentrator_c::StartAccept() {
        mLogger << setloglevel(LogLevel_IoActivity) << "Start accept on port " << DecPrinter(mPort) << std::endl;
        mListenSocket->async_accept(*mConnectionSocket, boost::bind(&IopConcentrator_c::AcceptHandler, this, boost::asio::placeholders::error));
        mIoService.poll();
}

void IopConcentrator_c::AcceptHandler(const boost::system::error_code& aError) {
	if (aError) {
		mLogger << setloglevel(LogLevel_IoActivity) << "Accept failed: " << aError.message() << std::endl;
		CloseConnSocket();
		return;
	}
	mLogger << setloglevel(LogLevel_IoActivity)
	        << "Connection from " << mConnectionSocket->remote_endpoint( ).address( ).to_string( )
	        << std::endl;
	try {
		mConnectionSocket->set_option(boost::asio::socket_base::keep_alive(true));
		boost::asio::ip::tcp::no_delay option(true);
		mConnectionSocket->set_option(option);
                mAccepted = true;
		ClearBuffers();
		StartReceive();
		mIoService.poll();
	}
	catch (boost::system::system_error &Ex) {
		mLogger << setloglevel(LogLevel_IoActivity) << "Failed to start receive: " << Ex.code().message() << std::endl;
		CloseConnSocket();
	}
}

void IopConcentrator_c::ClearBuffers() {
	ClearReceiveBuffers();
	ClearTransmitBuffers();
}

void IopConcentrator_c::ClearReceiveBuffers() {
	mToCrayData.clear();
	mToCrayIdx = 0;
        mReceiveBuf.consume(mReceiveBuf.size());
}

void IopConcentrator_c::ClearTransmitBuffers() {
	mFromCrayData.clear();
	mTransmitBufIdx = 0;
}

void IopConcentrator_c::CloseConnSocket() {
	if (mAccepted) {
        	mLogger << setloglevel(LogLevel_IoActivity) << "Close connection" << std::endl;
		boost::system::error_code IgnoredError;
		mConnectionSocket->close(IgnoredError);
		mAccepted = false;
	}
	StartAccept();
}

void IopConcentrator_c::ProcessReceivedBytes() {
	size_t i;
	size_t size = mReceiveBuf.size();
	while (size > 0) {
		const uint8_t *data = static_cast<const uint8_t *>(mReceiveBuf.data().data());
		switch (mRcvState) {
			case State_RcvLcpPduLength: // process 4-byte PDU length value
				if (size < 4) return;
				mPduLength = (data[0] << 24)
				           | (data[1] << 16)
				           | (data[2] <<  8)
				           |  data[3];
				mReceiveBuf.consume(4);
				size -= 4;
				mRcvState = State_RcvLcpPduContent;
				mLogger << setloglevel(LogLevel_IoTrace) << "Received LCP PDU length " << DecPrinter(mPduLength) << std::endl;
			break;
			case State_RcvLcpPduContent: // process LCP PDU content
				if (size < mPduLength) return;
				mSubsegmentCount = data[4];
				mLastRcvdMessageCode = data[6];
				i = 0;
				while (mPduLength > 0) {
					CInt_t word = 0;
					for (size_t n = 0; n < 8; n++) word = (word << 8) | data[i++];
					mToCrayData.push_back(word);
					size -= 8;
					mPduLength -= 8;
				}
				mReceiveBuf.consume(i);
				mLogger << setloglevel(LogLevel_IoTrace) << "MC: " << OctPrinter(mLastRcvdMessageCode)
				        << ", NSSG: " << DecPrinter(mSubsegmentCount) << std::endl;
				mLogger << setloglevel(LogLevel_IoTrace) << DecPrinter(mToCrayData.size())
				        << " total upline QWords queued" << std::endl;
				mRcvState = (mSubsegmentCount > 0) ? State_RcvSegPduLength : State_RcvLcpPduLength;
			break;
			case State_RcvSegPduLength: // process 4-byte PDU length value
				if (size < 4) return;
				mPduLength = (data[0] << 24)
				           | (data[1] << 16)
				           | (data[2] <<  8)
				           |  data[3];
				mReceiveBuf.consume(4);
				size -= 4;
				mRcvState = State_RcvSegPduContent;
				mLogger << setloglevel(LogLevel_IoTrace) << "Received Segment PDU length " << DecPrinter(mPduLength) << std::endl;
			break;
			case State_RcvSegPduContent: // process segment PDU content
				if (size < mPduLength) return;
				if (mLastRcvdMessageCode == McLogon) {
					mSegmentSize = ((data[6] << 8) | data[7]) * 8;
					mLogger << setloglevel(LogLevel_IoTrace) << "Subsegment size: "
					        << DecPrinter(mSegmentSize)   << " bytes / "
					        << DecPrinter(mSegmentSize/8) << " QWords"
					        << std::endl;
				}
				i = 0;
				while (mPduLength > 0) {
					CInt_t word = 0;
					for (size_t n = 0; n < 8; n++) word = (word << 8) | data[i++];
					mToCrayData.push_back(word);
					size -= 8;
					mPduLength -= 8;
				}
				mReceiveBuf.consume(i);
				mLogger << setloglevel(LogLevel_IoTrace) << DecPrinter(mToCrayData.size())
				        << " total upline QWords queued" << std::endl;
				mSubsegmentCount -= 1;
				mRcvState = (mSubsegmentCount > 0) ? State_RcvSegPduLength : State_RcvLcpPduLength;
			break;
			default:
				mParent.GetLogger() << setloglevel(LogLevel_Error) << SideEffectIndent
				                    << "ERROR: Invalid receive state" << std::endl;
				MasterClear();
				return;
			break;
		}
	}
}

void IopConcentrator_c::StartReceive() {
       	mLogger << setloglevel(LogLevel_IoActivity) << "Start receive" << std::endl;
	boost::asio::async_read(*mConnectionSocket, mReceiveBuf, boost::asio::transfer_at_least(1),
		boost::bind(&IopConcentrator_c::ReadHandler, this, boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void IopConcentrator_c::ReadHandler(const boost::system::error_code& aError, std::size_t bytesTransferred) {
	if (aError) {
        	mLogger << setloglevel(LogLevel_IoActivity) << "Read failed: " << aError.message() << std::endl;
		CloseConnSocket();
		return;
	}
	mLogger << setloglevel(LogLevel_IoTrace) << DecPrinter(bytesTransferred) << " bytes received" << std::endl;
	StartReceive();
}

void IopConcentrator_c::StartTransmit() {
        mTransmitBufs[mTransmitBufIdx].consume(mTransmitBufs[mTransmitBufIdx].size());
	size_t wordCount = mFromCrayData.size();
	size_t byteCount = wordCount * 8;
	switch (mXmtState) {
		case State_XmtLcpPduStart:
			mTransmitBufs[mTransmitBufIdx].sputc(0);
			mTransmitBufs[mTransmitBufIdx].sputc(0);
			mTransmitBufs[mTransmitBufIdx].sputc(0);
			mTransmitBufs[mTransmitBufIdx].sputc(BytesPerLCP);
			mXmtState = State_XmtLcpPduContent;
			// fall through
		case State_XmtLcpPduContent:
        		for (size_t w = 0; w < wordCount; w++) {
				CInt_t word = mFromCrayData[w];
				size_t shiftCount = 56;
				for (size_t b = 0; b < 8; b++) {
					mTransmitBufs[mTransmitBufIdx].sputc((char)((word >> shiftCount) & 0xff));
					shiftCount -= 8;
				}
			}
			mLastXmitMessageCode = (mFromCrayData[0] >> 8) & 0xff;
			mSubsegmentCount = (mFromCrayData[0] >> 24) & 0xff;
			mXmtState = (mSubsegmentCount > 0) ? State_XmtSegPduStart : State_XmtLcpPduStart;
		break;
		case State_XmtSegPduStart:
			if (mLastXmitMessageCode == McStart) {
				mTransmitBufs[mTransmitBufIdx].sputc((char)((byteCount >> 24) & 0xff));
				mTransmitBufs[mTransmitBufIdx].sputc((char)((byteCount >> 16) & 0xff));
				mTransmitBufs[mTransmitBufIdx].sputc((char)((byteCount >>  8) & 0xff));
				mTransmitBufs[mTransmitBufIdx].sputc((char)(byteCount & 0xff));
			}
			else {
				mTransmitBufs[mTransmitBufIdx].sputc((char)((mSegmentSize >> 24) & 0xff));
				mTransmitBufs[mTransmitBufIdx].sputc((char)((mSegmentSize >> 16) & 0xff));
				mTransmitBufs[mTransmitBufIdx].sputc((char)((mSegmentSize >>  8) & 0xff));
				mTransmitBufs[mTransmitBufIdx].sputc((char)(mSegmentSize & 0xff));
			}
			mBytesTransmitted = 0;
			mXmtState = State_XmtSegPduContent;
			// fall through
		case State_XmtSegPduContent:
        		for (size_t w = 0; w < wordCount; w++) {
				CInt_t word = mFromCrayData[w];
				size_t shiftCount = 56;
				for (size_t b = 0; b < 8; b++) {
					mTransmitBufs[mTransmitBufIdx].sputc((char)((word >> shiftCount) & 0xff));
					shiftCount -= 8;
				}
			}
			mBytesTransmitted += byteCount;
			if (mBytesTransmitted >= mSegmentSize || mLastXmitMessageCode == McStart) {
				mSubsegmentCount -= 1;
				mXmtState = (mSubsegmentCount > 0) ? State_XmtSegPduStart : State_XmtLcpPduStart;
			}
		break;
		default:
			mParent.GetLogger() << setloglevel(LogLevel_Error) << SideEffectIndent
			                    << "ERROR: Invalid transmit state" << std::endl;
			MasterClear();
			return;
		break;
	}
	mFromCrayData.clear();
	mLogger << setloglevel(LogLevel_IoTrace) << "Start transmitting "
	        << DecPrinter(wordCount) << " QWords/" << DecPrinter(byteCount) << " bytes to front end." << std::endl;
	try {
		boost::asio::async_write(*mConnectionSocket, mTransmitBufs[mTransmitBufIdx],
			boost::bind(&IopConcentrator_c::WriteHandler, this, boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		mTransmitBufIdx = (mTransmitBufIdx + 1) % MaxTransmitBufs;
	}
	catch (boost::system::system_error &Ex) {
        	mLogger << setloglevel(LogLevel_IoActivity) << "Failed to start write: " << Ex.code().message() << std::endl;
		CloseConnSocket();
		return;
	}
}

void IopConcentrator_c::WriteHandler(const boost::system::error_code& aError, std::size_t bytesTransferred) {
        if (aError) {
		mLogger << setloglevel(LogLevel_IoActivity) << "Write failed: " << aError.message() << std::endl;
                CloseConnSocket();
                return;
        }
	mLogger << setloglevel(LogLevel_IoTrace) << DecPrinter(bytesTransferred) << " bytes transmitted to front end." << std::endl;
}

void IopConcentrator_c::MasterClear() {
	mLogger << setloglevel(LogLevel_IoActivity) << "Master clear" << std::endl;
	CloseConnSocket();
	ClearBuffers();
	mRcvState = State_RcvLcpPduLength;
	mPduLength = 0;
	mXmtState = State_XmtLcpPduStart;
	mBytesTransmitted = 0;
	mSegmentSize = 0;
}

CInt_t IopConcentrator_c::GetData() {
	CInt_t Data = 0;
	if (mToCrayIdx < mToCrayData.size()) {
		Data = mToCrayData[mToCrayIdx++];
		mLogger << setloglevel(LogLevel_IoActivity) << "Receive " << HexPrinter(Data) << std::endl;
                if (mToCrayIdx >= mToCrayData.size()) {
			mToCrayData.clear();
			mToCrayIdx = 0;
		}
	} else {
		mLogger << setloglevel(LogLevel_IoActivity) << "Sending empty data " << HexPrinter(Data) << std::endl;
	}
	return Data;
}

size_t IopConcentrator_c::GetDataSize() {
	return mToCrayData.size() - mToCrayIdx;
}

bool IopConcentrator_c::HasData() {
	return mToCrayIdx < mToCrayData.size();
}

bool IopConcentrator_c::IsReceptionComplete() {
	return (mRcvState == State_RcvLcpPduLength || mRcvState == State_RcvSegPduLength) && HasData() == false && mPduLength <= 0;
}

void IopConcentrator_c::Poll() {
	mIoService.poll();
}

void IopConcentrator_c::SetData(CInt_t aData) {
	mLogger << setloglevel(LogLevel_IoActivity) << "Transmit " << HexPrinter(aData) << std::endl;
	mFromCrayData.push_back(aData);
}

void IopConcentrator_c::ProcessFromCrayData() {
	mLogger << setloglevel(LogLevel_IoActivity) << "Start transmitting " << DecPrinter(mFromCrayData.size())
	        << " QWords to front end" << std::endl;
	if (mAccepted) {
		StartTransmit();
	}
	else {
		mFromCrayData.clear();
	}
}

void IopConcentrator_c::Tick() {
	ProcessReceivedBytes();
        if (++mPollCnt > 100) {
                Poll();
                mPollCnt = 0;
        }
}

boost::asio::io_context IopConcentrator_c::mIoService;

/////////////////////////////////////////////////////////
// IopConcentratorChannel_c
/////////////////////////////////////////////////////////

IopConcentratorChannel_c::IopConcentratorChannel_c(class IopConcentrator_c &aParent, const Configuration_c &aConfig, size_t aChannelIdx, bool aAllowOut, bool aAllowIn):
	mAllowIn(aAllowIn),
	mAllowOut(aAllowOut),
	mChannelIdx(aChannelIdx),
	mParent(aParent)
{
	CRAY_ASSERT(aAllowOut ^ aAllowIn);
	mTransferDelay = aConfig.get<size_t>("TransferDelay", 10);
	MasterClear();
}

IopInt_t IopConcentratorChannel_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << (mAllowIn?"CONCI":"CONCO") << " DoIo with function code: " << OctPrinter(aFunction) << " data: " << HexPrinter(aData) << std::endl;
	switch (aFunction) {
		case 000: // Clear Channel
			mInActive = false;
			mOutActive = false;
			mDone = false;
		return 0;
		case 001: // Enter Local Memory Address, Start Transfer
			mIoMemoryAddr = aData >> 2; // Clear the lowest two bits
			if (mAllowIn) {
				mInActive = true;
				mOutActive = false;
				mParent.GetLogger() << setloglevel(LogLevel_IoActivity)
				                    << "Start transferring " << DecPrinter(mTransferSize) << " QWords"
				                    << " with " << DecPrinter(mParent.GetDataSize()) << " available"
				                    << " from front end to I/O memory address " << HexPrinter(mIoMemoryAddr,4)
				                    << std::endl;
				mDone = false;
			}
			if (mAllowOut) {
				mInActive = false;
				mOutActive = true;
				mParent.GetLogger() << setloglevel(LogLevel_IoActivity)
				                    << "Start transferring " << DecPrinter(mTransferSize) << " QWords"
				                    << " from I/O memory address " << HexPrinter(mIoMemoryAddr,4)
				                    << " to front end" << std::endl;
				mDone = false;
			}
			mTransferDelayCounter = mTransferDelay;
		return 0;
		case 002: // Enter Parcel Count
			mTransferSize = aData >> 2;
		return 0;
		case 003: // Clear Error Flag
			// Clear parity error bits for CIA and sequence error flags for COA
		return 0;
		case 004: // (CIA) Clear Ready Waiting Flog | (COA) Set/Clear External Control Signals
			if (mAllowIn) {
				mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "CONCI function 4 with value: " << HexPrinter(aData,4) << std::endl;
				mReadyWaiting = false;
			}
			if (mAllowOut) {
				mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "CONCO function 4 with value: " << HexPrinter(aData,4) << std::endl;
				mWriteDisconnect = (aData & (1 << 8)) != 0;
				mHoldDisconnect  = (aData & (1 << 9)) != 0;
/*				if ((aData & (1 << 4)) != 0) {
					// This bit seems to be set once the buffer clear is complete
					mParent.BufferClearComplete();
				}*/
				if (mWriteDisconnect) {
					mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "would be sending disconnect to concentrator" << std::endl;
				}
				if ((aData & (1 << 14)) != 0) mParent.MasterClear();
			}
		return 0;
		case 006: // Clear Interrupt Enable Flag
			mInterruptEnabled = false;
		return 0;
		case 007: // Set Interrupt Enable Flag
			mInterruptEnabled = true;
		return 0;
		case 010: // Read Local Memory Address
			mParent.GetLogger() << setloglevel(LogLevel_IoActivity)
			                    << (mAllowIn ? "CONCI" : "CONCO")
			                    << " function 010 returning: " << HexPrinter(mIoMemoryAddr << 2, 4)
			                    << std::endl;
		return mIoMemoryAddr << 2;
		case 011: // Read Ready Waiting / Error Flags
			if (mAllowIn) aData = mReadyWaiting ? (1 << 15) : 0; // Would return parity error bits in the low-order four bits
			if (mAllowOut) aData = 0; // Would return sequence error and 4-bit channels none of which is supported
			mParent.GetLogger() << setloglevel(LogLevel_IoActivity)
			                    << (mAllowIn ? "CONCI" : "CONCO")
			                    << " function 011 returning: " << HexPrinter(aData,4)
			                    << std::endl;
		return aData;
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
	if (mAllowIn) mParent.Tick(); // Forward ticks to concentrator from the input channel only

	while (mInActive || mOutActive) {
		CRAY_ASSERT(!mDone);
		if (mOutActive) {
			CInt_t Data = SwapBytesInWords(mParent.GetParent().IoMemAccessByType<uint64_t>(mIoMemoryAddr));
			mParent.SetData(SwapBytes(Data));
		}
		if (mInActive) {
			if (mParent.HasData() == false) return;
			CInt_t Data = SwapBytes(mParent.GetData());
			mParent.GetParent().IoMemAccessByType<uint64_t>(mIoMemoryAddr) = SwapBytesInWords(Data);
		}
		if (mIoMemoryAddr != 0x3fff) {
			++mIoMemoryAddr;
		} else {
			mIoMemoryAddr = 0;
		}
		// We're exploiting the fact that this can underflow.
		mTransferSize = (mTransferSize - 1) & 0x3fff;

		if (mTransferSize == 0 || (mAllowIn && mParent.IsReceptionComplete())) {
			if (!mHoldDisconnect && mOutActive) {
				mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "would be sending disconnect to concentrator" << std::endl;
			}
			mInActive = false;
			mOutActive = false;
			mDone = true;
			mParent.GetLogger() << setloglevel(LogLevel_IoActivity) << "Transfer completed" << std::endl;
			if (mAllowOut) mParent.ProcessFromCrayData();
		}
	}
}
