#include "iop_iop2iop.h"
#include "cray_mainframe.h"
#include <algorithm>
#include <time.h>

/////////////////////////////////////////////////////////
// IopIopChannel_c
/////////////////////////////////////////////////////////


IopIopChannel_c::IopIopChannel_c(const Configuration_c &aConfig, IopCpu_c &aSrcIop, IopCpu_c &aDstIop, size_t aAIChannelIdx, size_t aAOChannelIdx) :
	mAI(aConfig, *this, aAIChannelIdx),
	mAO(aConfig, *this, aAOChannelIdx),
	mSrcIop(aSrcIop),
	mDstIop(aDstIop),
	mData(0),
	mThrottle(0),
	mDataValid(false)
{
//	mMultiThreaded = aSrcIop.GetMainframe().IsMultiThreaded();
}

IopInt_t IopIopChannel_c::ChannelAO_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
//	Guard_t Guard(mParent.mLock);
	switch (aFunction) {
		case 000: {
			mParent.mDataValid = false;
			mDone = false;
		} return 0;
		case 001:
			// Here we handle the dead-starting of the other IOP...
			CRAY_ASSERT((aData & (1 << 2)) == 0); // Dead dump is not supported at the moment
			CRAY_ASSERT((aData & 3) != 1); // Master-clear without deadstart isn't supported at the moment either
			if ((aData & 3) == 0 && (mCtrlReg & 3) == 3 ) {
				// This is the falling edge of master clear and dead-start, so let's dead-start the other IOP
//				cout << "Getting IOP" << DecPrinter(mParent.mDstIop.GetCpuId()) << " out of reset by IOP" << DecPrinter(mParent.mSrcIop.GetCpuId()) << std::endl;
				mLogger << setloglevel(LogLevel_IoActivity) << "Getting IOP" << DecPrinter(mParent.mDstIop.GetCpuId()) << " out of reset by IOP" << DecPrinter(mParent.mSrcIop.GetCpuId()) << std::endl;
				mParent.mDstIop.DeadStart();
				mResetStalling = true;
			}
			mCtrlReg = aData;
		return 0;
		case 006:
			mInterruptEnabled = false;
		return 0;
		case 007:
			mInterruptEnabled = true;
		return 0;
		case 014: {
			if (mParent.mDataValid) {
				mLogger << setloglevel(LogLevel_Error) << "ERROR: AO read called with channel data already valid" << std::endl;
			}
			mLogger << setloglevel(LogLevel_Communication) << "at " << CurrentTime() << " AO sending data " << HexPrinter(aData,4) << std::endl;
			mParent.mData = aData;
			mParent.mDataValid = true;
			mDone = false;
		} return 0;
		default:
			mLogger << setloglevel(LogLevel_Error) << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

bool IopIopChannel_c::ChannelAO_c::StallIop() {
	// If we've resetted the target IOP, stall the source IOP until the target started executing instructions.
	// This is needed because - in a multi-threaded environment - the target might take a long time to start and
	// there's a fix delay in the source to change back the boot image in MOS memory (see iop_boot.asm around 0x493D).
	if (mResetStalling) {
		if (mParent.mDstIop.IsInReset()) return true;
//		cout << "Releasing stall on IOP" << DecPrinter(mParent.mSrcIop.GetCpuId()) << std::endl;
		mResetStalling = false;
	}
	// If there's valid data in the register, throttle source IOP execution - this is to avoid HALT 015 (communication buffer exhaustion during boot)
	if (mParent.mDataValid) {
		if (mThrottleCnt == 0) {
			mThrottleCnt = mParent.mThrottle;
		} else {
			--mThrottleCnt;
			return true;
		}
	}
	return false;
}

IopInt_t IopIopChannel_c::ChannelAI_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
//	Guard_t Guard(mParent.mLock); 
	switch (aFunction) {
		case 000: {
			mParent.mDataValid = false;
		} return 0;
		case 006:
			mInterruptEnabled = false;
		return 0;
		case 007:
			mInterruptEnabled = true;
		return 0;
		case 010: {
			if (!mParent.mDataValid) {
				mLogger << setloglevel(LogLevel_Error) << "ERROR: AI read called with channel data invalid" << std::endl;
			}
			mLogger << setloglevel(LogLevel_Communication) << "at " << CurrentTime() << " AI receiving data " << HexPrinter(uint16_t(mParent.mData),4) << " with data valid: " << mParent.mDataValid << std::endl;
			uint16_t Data = mParent.mData;
			mParent.mDataValid = false;
			mParent.mAO.SetDone(true);
			return Data;
		}
		default:
			mLogger << setloglevel(LogLevel_Error) << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

