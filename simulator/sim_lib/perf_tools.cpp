#include "perf_tools.h"

namespace PerfTools {

string Percentage(uint32_t aCount, uint32_t aTotal) {
	stringstream StrStrm;
	StrStrm << DoublePrinter(100.0 * double(aCount)/double(aTotal), 5, 2, true) << "%";
	return StrStrm.str();
}

void Pipeline_c::Tick(uint32_t aAmount) {
	mTickCount += aAmount;
	for (auto It=mResources.begin(); It != mResources.end(); ++It) (*It)->Tick(aAmount);
}

void Pipeline_c::Stall(uint32_t aAmount) {
	mStallCount += aAmount;
	Tick(aAmount);
}

void Pipeline_c::Print(ostream &aStream) const {
	aStream << "Pipeline summary: " << endl;
	aStream << "===========================" << endl;
	aStream << "    Number of cycles: " << DecPrinter(mTickCount) << endl;
	aStream << "    Number of stalls: " << DecPrinter(mStallCount) << endl;
	aStream << "    Average IPC:      " << DoublePrinter(double(mTickCount-mStallCount)/double(mTickCount),4,4,true) << endl;
}

void Pipeline_c::AddResource(Resource_i *aResource) {
	mResources.push_back(aResource);
}







void InstFetch_c::Tick(uint32_t aAmount) {
	if (!mInternalStall) mPrefetchAmount = max(mPrefetchBufferSize/2, mPrefetchAmount + aAmount);
}

void InstFetch_c::Fetch(bool aHidden) {
//	CRAY_ASSERT(mInstructionFetches <= 7020670); // Terminate after a set amount of instructions for easy benchmarking
	ExtraFetch(aHidden);
	++mCurrentBasicBlockLength;
	++mInstructionFetches;
}

void InstFetch_c::ExtraFetch(bool aHidden) {
	if (mPrefetchAmount == 0) {
		uint32_t Stall = mMemoryModel.ReadLatency();
		mInternalStall = true;
		mParent.Stall(Stall);
		mStallCount += Stall;
		mInternalStall = false;
		if (aHidden) mPrefetchAmount = 1;
	}
	if (!aHidden) mParent.Tick();
	CRAY_ASSERT(mPrefetchAmount > 0);
	--mPrefetchAmount;
	++mFetchCount;
	mPostExecAmount = max(mPrefetchAmount/2, mPostExecAmount+1);
}

void InstFetch_c::TargetCalcStall(uint32_t aAmount) {
	mStallCount += aAmount;
	mTargetCallStallCount += aAmount;
	mParent.Stall(aAmount);
}

void InstFetch_c::ShortJump(int32_t aOffset) {
	if (mBasicBlockLengths.size() <= mCurrentBasicBlockLength) mBasicBlockLengths.resize(mCurrentBasicBlockLength+1);
	++(mBasicBlockLengths[mCurrentBasicBlockLength]);
	mCurrentBasicBlockLength = 0;

	++mShortJumpCount;
	if ((aOffset > 0 && aOffset <= (int32_t)mPrefetchAmount) || (aOffset < 0 && -aOffset <= (int32_t)mPostExecAmount)) {
		return;
	}
	++mOutOfBufferShortJumpCount;
	mPrefetchAmount = 0;
	mPostExecAmount = 0;
}

void InstFetch_c::LongJump() {
	++mLongJumpCount;
	mPrefetchAmount = 0;
	mPostExecAmount = 0;

	if (mBasicBlockLengths.size() <= mCurrentBasicBlockLength) mBasicBlockLengths.resize(mCurrentBasicBlockLength+1);
	++(mBasicBlockLengths[mCurrentBasicBlockLength]);
	mCurrentBasicBlockLength = 0;
}

void InstFetch_c::Interrupt() {
	++mInterruptCount;
	mPrefetchAmount = 0;
	mPostExecAmount = 0;
}

void InstFetch_c::Print(ostream &aStream) const {
	aStream << "Instruction buffer summary: " << endl;
	aStream << "===========================" << endl;
	aStream << "    Number of instruction fetches:       " << DecPrinter(mInstructionFetches) << endl;
	aStream << "    Number of fetches:                   " << DecPrinter(mFetchCount) << endl;
	aStream << "    Number of stalls:                    " << DecPrinter(mStallCount) << endl;
	aStream << "    Number of target calc stalls:        " << DecPrinter(mTargetCallStallCount) << endl;
	aStream << "    Number of short jumps:               " << DecPrinter(mShortJumpCount) << endl;
	aStream << "    Number of out of buffer short jumps: " << DecPrinter(mOutOfBufferShortJumpCount) << endl;
	aStream << "    Number of long jumps:                " << DecPrinter(mLongJumpCount) << endl;
	aStream << "    Number of interrupts:                " << DecPrinter(mInterruptCount) << endl;
	aStream << "    Basic block length histogram:        " << endl;
	uint32_t Sum = 0;
	for(size_t i=0;i<mBasicBlockLengths.size();++i) {
		Sum += mBasicBlockLengths[i];
	}

	uint32_t Sum2 = 0;
	uint32_t TerminateSum = Sum * 99 / 100;
	uint32_t Terminate = mBasicBlockLengths.size()-1;
	for(size_t i=0;i<mBasicBlockLengths.size();++i) {
		Sum2 += mBasicBlockLengths[i];
		if (Sum2 >= TerminateSum) {
			Terminate = i;
			break;
		}
	}
	Sum2 = 0;
	for(size_t i=0;i<=Terminate;++i) {
		Sum2 += mBasicBlockLengths[i];
		aStream << "        " << DecPrinter(i) << " " << Percentage(mBasicBlockLengths[i],Sum) << " - cumulative: " << Percentage(Sum2,Sum) << endl;
	}
}






void ComputeResource_c::Tick(uint32_t aAmount) {
	if (aAmount < mUseMap.size()) {
		for(size_t Idx = 0; Idx < mUseMap.size() - aAmount; ++Idx) {
			mUseMap[Idx] = mUseMap[Idx+aAmount];
		}
		for(size_t Idx = mUseMap.size() - aAmount; Idx < mUseMap.size(); ++Idx) {
			mUseMap[Idx] = false;
		}
	} else {
		for(size_t Idx = 0; Idx < mUseMap.size(); ++Idx) {
			mUseMap[Idx] = false;
		}
	}
}

void ComputeResource_c::Use(uint32_t aTimeInFuture, uint32_t aDuration) {
	CRAY_ASSERT(aDuration > 0);
	// First let's see if the requested block is available
	size_t TrueStartTime = aTimeInFuture;
	for(size_t Idx=std::min(size_t(mUseMap.size()),size_t(aTimeInFuture+aDuration))-1; Idx>=aTimeInFuture; --Idx) {
		if (mUseMap[Idx]) {
			TrueStartTime = Idx;
			break;
		}
	}
	CRAY_ASSERT(TrueStartTime >= aTimeInFuture);
	// Record the new use-time, resizing the array if needed
	size_t TrueEndTime = TrueStartTime + aDuration - 1;
	if (TrueEndTime >= mUseMap.size()) mUseMap.resize(TrueEndTime+1);
	for(size_t Idx=TrueStartTime; Idx <= TrueEndTime; ++Idx) {
		mUseMap[Idx] = true;
	}
	// Figure out if we needed to stall, if we did, report the stall to parent
	size_t Stall = TrueStartTime-aTimeInFuture;
	if (Stall > 0) {
		mParent.Stall(Stall);
		mStallCount += Stall;
	}
	mUseCount += aDuration;
}

void ComputeResource_c::Print(ostream &aStream) const {
	aStream << "Compute resource: " << mName << endl;
	aStream << "===========================" << endl;
	aStream << "    Number of stalls:               " << DecPrinter(mStallCount) << endl;
	aStream << "    Number of active cycles (uses): " << DecPrinter(mUseCount) << endl;
}





void StorageResource_c::Tick(uint32_t aAmount) {
	mNextRead = max((int)(mNextRead)-(int)(aAmount), 0);
	mNextWrite = max((int)(mNextWrite)-(int)(aAmount), 0);
}

void StorageResource_c::Read(uint32_t aTimeInFuture) {
	CRAY_ASSERT(aTimeInFuture > 0);
	uint32_t TrueReadTime = aTimeInFuture;

	// Look for read-after-read hazards
	if (aTimeInFuture < mNextRead) {
		++mReadAfterReadCount;
		// We don't have to order reads of the same resource, so this doesn't generate any stalls
	}

	// Look for read-after-write hazards
	if (aTimeInFuture <= mNextWrite) {
		size_t Stall = mNextWrite - aTimeInFuture+1;
		mParent.Stall(Stall);
		++mReadAfterWriteCount;
		mReadAfterWriteStallCount += Stall;
		mStallCount += Stall;
		TrueReadTime = mNextWrite + 1;
		CRAY_ASSERT(TrueReadTime == aTimeInFuture);
	}
	++mReadCount;
	mNextRead = max(mNextRead, TrueReadTime);
}

void StorageResource_c::Write(uint32_t aTimeInFuture) {
	CRAY_ASSERT(aTimeInFuture > 0);
	uint32_t TrueWriteTime = aTimeInFuture;

	// Look for write-after-write hazards
	if (TrueWriteTime <= mNextWrite) {
		size_t Stall = mNextWrite - TrueWriteTime+1;
		mParent.Stall(Stall);
		++mWriteAfterWriteCount;
		mWriteAfterWriteStallCount += Stall;
		mStallCount += Stall;
		TrueWriteTime = mNextWrite + 1;
		CRAY_ASSERT(TrueWriteTime == aTimeInFuture);
	}

	// Look for write-after-read hazards
	if (TrueWriteTime <= mNextRead) {
		size_t Stall = mNextRead - TrueWriteTime+1;
		mParent.Stall(Stall);
		++mWriteAfterReadCount;
		mWriteAfterReadStallCount += Stall;
		mStallCount += Stall;
		TrueWriteTime = mNextRead + 1;
		CRAY_ASSERT(TrueWriteTime == aTimeInFuture);
	}
	++mWriteCount;
	mNextWrite = max(mNextWrite, TrueWriteTime);
}

void StorageResource_c::Print(ostream &aStream) const {
	aStream << "Storage resource: " << mName << endl;
	aStream << "===========================" << endl;
	aStream << "    Number of stalls:                                  " << DecPrinter(mStallCount) << endl;
	aStream << "    Number of reads:                                   " << DecPrinter(mReadCount) << endl;
	aStream << "    Number of writes:                                  " << DecPrinter(mWriteCount) << endl;
	aStream << "    Number of read-after-read hazards:                 " << DecPrinter(mReadAfterReadCount) << endl;
	aStream << "    Number of read-after-write hazards:                " << DecPrinter(mReadAfterWriteCount) << endl;
	aStream << "    Number of write-after-read hazards:                " << DecPrinter(mWriteAfterReadCount) << endl;
	aStream << "    Number of write-after-write hazards:               " << DecPrinter(mWriteAfterWriteCount) << endl;
	aStream << "    Number of stalls due to read-after-read hazards:   " << DecPrinter(mReadAfterReadStallCount) << endl;
	aStream << "    Number of stalls due to read-after-write hazards:  " << DecPrinter(mReadAfterWriteStallCount) << endl;
	aStream << "    Number of stalls due to write-after-read hazards:  " << DecPrinter(mWriteAfterReadStallCount) << endl;
	aStream << "    Number of stalls due to write-after-write hazards: " << DecPrinter(mWriteAfterWriteStallCount) << endl;
}



void MemoryModel_c::Tick(uint32_t aAmount) {
}

uint32_t MemoryModel_c::ReadLatency(uint32_t aAddr) {
	// Test for disabled caches
	if (mTag.size() == 0) return ReadLatency();

	++mReadCount;
	uint32_t TagAddr = GetTagAddr(aAddr);
	uint32_t TagIdx = GetTagIdx(aAddr);
	TagEntry_s *HitEntry = nullptr;
	TagEntry_s *EntryToAllocate = &mTag[0][TagIdx];
	// Update timestamps
	for(uint32_t Way=0;Way<mNumWays;++Way) {
		TagEntry_s &Entry = mTag[Way][TagIdx];
		if (Entry.Valid) {
			// Age all ways
			++Entry.TimeStamp;
			if (Entry.Addr == TagAddr) {
				CRAY_ASSERT(HitEntry == nullptr);
				// We have a hit
				HitEntry = &Entry;
			}
			// Get the least-recently-used entry as the one to allocate, unless there's an invalid one
			if ((Entry.TimeStamp > EntryToAllocate->TimeStamp) && EntryToAllocate->Valid) {
				EntryToAllocate = &Entry;
			}
		} else {
			EntryToAllocate = &Entry;
		}
	}
	// Handle hits
	if (HitEntry != nullptr) {
		CRAY_ASSERT(HitEntry->Valid);
		CRAY_ASSERT(HitEntry->Addr == TagAddr);
		HitEntry->TimeStamp = 0; // Reset timestamp for the way that hit
		mReadDelayCount += mTagLatency;
		return mTagLatency;
	}
	uint32_t Delay = mTagLatency;
	// Handle misses
	if (EntryToAllocate->Valid) {
		++mEvictionCount;
		if (EntryToAllocate->Dirty) {
			CRAY_ASSERT(mWriteBack);
			Delay += mWriteLatency + mLineSize / mBurstReadRate;
		}
//		cout << "Evicting cache line: " << HexPrinter(aAddr) << endl;
	}
	EntryToAllocate->Valid = true;
	EntryToAllocate->Dirty = false;
	EntryToAllocate->Addr = TagAddr;
	EntryToAllocate->TimeStamp = 0;
	++mMissCount;
	Delay += mReadLatency + mLineSize / mBurstReadRate;
	mReadDelayCount += Delay;
	return Delay;
}

uint32_t MemoryModel_c::WriteLatency(uint32_t aAddr) {
	// We implement write-through, so we only invalidate the hitting line
	++mWriteCount;

	if (mTag.size() == 0) {
		mWriteDelayCount += mWriteLatency;
		return mWriteLatency;
	}
	if (!mInvalidateOnWrite & !mAllocateOnWrite & !mWriteBack) {
		mWriteDelayCount += mWriteLatency;
		return mWriteLatency;
	}

	uint32_t TagAddr = GetTagAddr(aAddr);
	uint32_t TagIdx = GetTagIdx(aAddr);
	TagEntry_s *HitEntry = nullptr;
	TagEntry_s *EntryToAllocate = &mTag[0][TagIdx];

	if (mAllocateOnWrite || mWriteBack) {
		for(uint32_t Way=0;Way<mNumWays;++Way) {
			TagEntry_s &Entry = mTag[Way][TagIdx];

			if (Entry.Valid) {
				// Age all ways
				++Entry.TimeStamp;
				if (Entry.Addr == TagAddr) {
					CRAY_ASSERT(HitEntry == nullptr);
					// We have a hit
					HitEntry = &Entry;
					break;
				}
				// Get the least-recently-used entry as the one to allocate, unless there's an invalid one
				if ((Entry.TimeStamp > EntryToAllocate->TimeStamp) && EntryToAllocate->Valid) {
					EntryToAllocate = &Entry;
				}
			} else {
				EntryToAllocate = &Entry;
			}
		}
		if (HitEntry != nullptr) {
			if (mWriteBack) {
				HitEntry->Dirty = true;
				uint32_t Delay = mTagLatency;
				mWriteDelayCount += Delay;
				return Delay;
			} else {
				// In case of write allocate, we have a hit, but still have to do the write
				uint32_t Delay = mWriteLatency;
				mWriteDelayCount += Delay;
				return Delay;
			}
		} else {
			if (EntryToAllocate->Valid) {
				++mEvictionCount;
		//		cout << "Evicting cache line: " << HexPrinter(aAddr) << endl;
			}
			EntryToAllocate->Valid = true;
			EntryToAllocate->Addr = TagAddr;
			EntryToAllocate->TimeStamp = 0;
			++mWriteAllocateCount;
			if (mWriteBack) {
				EntryToAllocate->Dirty = true;
				uint32_t Delay = mTagLatency + mReadLatency + mLineSize / mBurstReadRate;
				mWriteDelayCount += Delay;
				return Delay;
			} else {
				uint32_t Delay = max(mWriteLatency, mTagLatency) + mReadLatency + mLineSize / mBurstReadRate;
				mWriteDelayCount += Delay;
				return Delay;
			}
		}
	}


	if (mInvalidateOnWrite) {
		for(uint32_t Way=0;Way<mNumWays;++Way) {
			TagEntry_s &Entry = mTag[Way][TagIdx];
			if (Entry.Valid) {
				if (Entry.Addr == TagAddr) {
					++mWriteInvalidateCount;
					Entry.Valid = false;
				}
			}
		}
		mWriteDelayCount += mWriteLatency;
		return mWriteLatency;
	}

	CRAY_ASSERT(false);
}

void MemoryModel_c::Print(ostream &aStream) const {
	aStream << "Memory model" << endl;
	aStream << "================" << endl;
	aStream << "    Read latency:                 " << mReadLatency << endl;
	aStream << "    Write latency:                " << mWriteLatency << endl;
	aStream << "    Tag access latency:           " << mTagLatency << endl;
	aStream << "    Cache size:                   " << mCacheSize << endl;
	aStream << "    Line size:                    " << mLineSize << endl;
	aStream << "    Line size bits:               " << mLineSizeBits << endl;
	aStream << "    Tag size:                     " << mTagSize << endl;
	aStream << "    Tag size bits:                " << mTagSizeBits << endl;
	aStream << "    Ways:                         " << mNumWays << endl;
	aStream << "    Invalidate on write:          " << mInvalidateOnWrite << endl;
	aStream << "    Allocate on write:            " << mAllocateOnWrite << endl;
	aStream << "    Number of uncached reads:     " << mUncachedReadCount << endl;
	aStream << "    Number of misses:             " << mMissCount << endl;
	aStream << "    Number of reads:              " << mReadCount << endl;
	aStream << "    Number of writes:             " << mWriteCount << endl;
	aStream << "    Number of read delay cycles:  " << mReadDelayCount << endl;
	aStream << "    Number of write delay cycles: " << mWriteDelayCount << endl;
	aStream << "    Number of write invalidates:  " << mWriteInvalidateCount << endl;
	aStream << "    Number of write allocations:  " << mAllocateOnWrite << endl;
	aStream << "    Number of read evictions:     " << mEvictionCount << endl;
	aStream << "    Miss rate:                    " << Percentage(mMissCount, mReadCount) << endl;
}


} // namespace PerfTools