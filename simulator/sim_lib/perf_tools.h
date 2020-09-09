#ifndef __PERF_TOOLS_H__
#define __PERF_TOOLS_H__

#include "utils.h"
#include "cray_types.h"
#include "cray_logger.h"
#include <vector>
#include <iostream>

using namespace std;

namespace PerfTools {

class Resource_i {
public:
	virtual void Tick(uint32_t aAmount=1) = 0;
};

class Pipeline_c {
public:
	Pipeline_c(): mStallCount(0), mTickCount(0) {}

	void Tick(uint32_t aAmount=1);
	void Stall(uint32_t aAmount);

	void Print(ostream &aStream) const;
	void AddResource(Resource_i *aResource);
protected:
	uint32_t mStallCount;
	uint32_t mTickCount;
	vector<Resource_i*> mResources;
};

inline ostream & operator << (ostream &aStream, const Pipeline_c &aItem) { 
	if (!aStream.good()) return aStream;
	aItem.Print(aStream); 
	return aStream; 
}






class MemoryModel_c: public Resource_i {
public:
	MemoryModel_c(
		uint32_t aReadLatency, 
		uint32_t aWriteLatency, 
		uint32_t aTagLatency,
		uint32_t aCacheSize,
		uint32_t aLineSize,
		uint32_t aNumWays,
		uint32_t aBurstReadRate,
		bool aInvalidateOnWrite,
		bool aAllocateOnWrite,
		bool aWriteBack,
		Pipeline_c &aParent
	):
		mReadLatency(aReadLatency),
		mWriteLatency(aWriteLatency),
		mTagLatency(aTagLatency),
		mCacheSize(aCacheSize),
		mLineSize(aLineSize),
		mNumWays(aNumWays),
		mUncachedReadCount(0),
		mMissCount(0),
		mReadCount(0),
		mWriteCount(0),
		mReadDelayCount(0),
		mWriteDelayCount(0),
		mWriteInvalidateCount(0),
		mInvalidateOnWrite(aInvalidateOnWrite),
		mAllocateOnWrite(aAllocateOnWrite),
		mBurstReadRate(aBurstReadRate),
		mWriteBack(aWriteBack),
		mEvictionCount(0),
		mWriteAllocateCount(0),
		mParent(aParent)
	{
		mTagSize = aNumWays == 0 ? 0 : aCacheSize/aNumWays/aLineSize;
		CRAY_ASSERT(!(mInvalidateOnWrite && mAllocateOnWrite));
		CRAY_ASSERT(!(mInvalidateOnWrite && mWriteBack));
		mParent.AddResource(this);
		for(mLineSizeBits = 0; mLineSizeBits < 32; ++mLineSizeBits) {
			if ((1UL << mLineSizeBits) >= mLineSize) break;
		}
		for(mTagSizeBits = 0; mTagSizeBits < 32; ++mTagSizeBits) {
			if ((1UL << mTagSizeBits) >= mTagSize) break;
		}
		mTag.resize(mNumWays);
		for(uint32_t Way=0;Way<mNumWays;++Way) {
			mTag[Way].resize(mTagSize);
		}
	}
	virtual void Tick(uint32_t aAmount=1) override;
	uint32_t ReadLatency() { ++mUncachedReadCount; mReadDelayCount += mReadLatency; return mReadLatency; }
//	uint32_t WriteLatency() { return mWriteLatency; }
	uint32_t ReadLatency(uint32_t aAddr);
	uint32_t WriteLatency(uint32_t aAddr);
	void Print(ostream &aStream) const;
protected:
	uint32_t GetTagIdx(uint32_t aAddr) { return (aAddr >> mLineSizeBits) & ((1 << mTagSizeBits) - 1); }
	uint32_t GetTagAddr(uint32_t aAddr) { return aAddr >> mLineSizeBits >> mTagSizeBits; }
	struct TagEntry_s {
		bool     Valid;
		bool     Dirty;
		uint32_t Addr;
		uint32_t TimeStamp;
		TagEntry_s(): Valid(false) {}
	};
	typedef vector<TagEntry_s> WayTag_t;
	typedef vector<WayTag_t> Tag_t;
	Tag_t mTag;

	uint32_t mReadLatency;
	uint32_t mWriteLatency;
	uint32_t mTagLatency;
	uint32_t mCacheSize;
	uint32_t mLineSize;
	uint32_t mLineSizeBits;
	uint32_t mTagSize;
	uint32_t mTagSizeBits;
	uint32_t mNumWays;
	bool mInvalidateOnWrite;
	bool mAllocateOnWrite;
	bool mWriteBack;
	uint32_t mBurstReadRate;

	uint32_t mUncachedReadCount;
	uint32_t mMissCount;
	uint32_t mReadCount;
	uint32_t mWriteCount;
	uint32_t mReadDelayCount;
	uint32_t mWriteDelayCount;
	uint32_t mWriteInvalidateCount;
	uint32_t mWriteAllocateCount;
	uint32_t mEvictionCount;

	Pipeline_c &mParent;
};

inline ostream & operator << (ostream &aStream, const MemoryModel_c &aItem) { 
	if (!aStream.good()) return aStream;
	aItem.Print(aStream); 
	return aStream; 
}




class InstFetch_c: public Resource_i {
public:
	InstFetch_c(uint32_t aPrefetchBufferSize, Pipeline_c &aParent, MemoryModel_c &aMemoryModel):
		mPrefetchBufferSize(aPrefetchBufferSize),
		mPrefetchAmount(0),
		mPostExecAmount(0),
		mParent(aParent),
		mInternalStall(false),
		mFetchCount(0),
		mShortJumpCount(0),
		mOutOfBufferShortJumpCount(0),
		mLongJumpCount(0),
		mInterruptCount(0),
		mStallCount(0),
		mTargetCallStallCount(0),
		mCurrentBasicBlockLength(0),
		mInstructionFetches(0),
		mMemoryModel(aMemoryModel)
	{
		mParent.AddResource(this);
	}

	virtual void Tick(uint32_t aAmount=1) override;
	void Fetch(bool aHidden = false);
	void ExtraFetch(bool aHidden = false);
	void TargetCalcStall(uint32_t aAmount);
	void ShortJump(int32_t aOffset);
	void LongJump();
	void Interrupt();
	void Print(ostream &aStream) const;
protected:
	uint32_t mPrefetchBufferSize;
	uint32_t mPrefetchAmount;
	uint32_t mPostExecAmount;
	Pipeline_c &mParent;
	bool mInternalStall;
	uint32_t mFetchCount;
	uint32_t mShortJumpCount;
	uint32_t mOutOfBufferShortJumpCount;
	uint32_t mLongJumpCount;
	uint32_t mInterruptCount;
	uint32_t mStallCount;
	uint32_t mTargetCallStallCount;
	uint32_t mInstructionFetches;
	MemoryModel_c &mMemoryModel;
	uint32_t mCurrentBasicBlockLength;
	vector<uint32_t> mBasicBlockLengths;
};

inline ostream & operator << (ostream &aStream, const InstFetch_c &aItem) { 
	if (!aStream.good()) return aStream;
	aItem.Print(aStream); 
	return aStream; 
}






class ComputeResource_c: public Resource_i {
public:
	ComputeResource_c(const string &aName, size_t aMaxPipelineLength, Pipeline_c &aParent): mName(aName), mParent(aParent), mStallCount(0), mUseCount(0) {
		mUseMap.resize(aMaxPipelineLength);
		mParent.AddResource(this);
	}
	virtual void Tick(uint32_t aAmount=1) override;
	void Use(uint32_t aTimeInFuture, uint32_t aDuration = 1);

	void Print(ostream &aStream) const;
protected:
	string mName;
	vector<bool> mUseMap;
	Pipeline_c &mParent;
	size_t mStallCount;
	size_t mUseCount;
};

inline ostream & operator << (ostream &aStream, const ComputeResource_c &aItem) { 
	if (!aStream.good()) return aStream;
	aItem.Print(aStream); 
	return aStream; 
}










class StorageResource_c: public Resource_i {
public:
	StorageResource_c(const string &aName, Pipeline_c &aParent):
		mName(aName),
		mNextRead(0),
		mNextWrite(0),
		mParent(aParent),
		mStallCount(0),
		mReadCount(0),
		mWriteCount(0),
		mReadAfterReadCount(0),
		mReadAfterWriteCount(0),
		mWriteAfterReadCount(0),
		mWriteAfterWriteCount(0),
		mReadAfterReadStallCount(0),
		mReadAfterWriteStallCount(0),
		mWriteAfterReadStallCount(0),
		mWriteAfterWriteStallCount(0)
	{
		mParent.AddResource(this);
	}
	virtual void Tick(uint32_t aAmount=1) override;
	void Read(uint32_t aTimeInFuture);
	void Write(uint32_t aTimeInFuture);

	void Print(ostream &aStream) const;
protected:
	string mName;
	uint32_t mNextRead;
	uint32_t mNextWrite;
	Pipeline_c &mParent;
	size_t mStallCount;
	size_t mReadCount;
	size_t mWriteCount;
	size_t mReadAfterReadCount;
	size_t mReadAfterWriteCount;
	size_t mWriteAfterReadCount;
	size_t mWriteAfterWriteCount;
	size_t mReadAfterReadStallCount;
	size_t mReadAfterWriteStallCount;
	size_t mWriteAfterReadStallCount;
	size_t mWriteAfterWriteStallCount;
};

inline ostream & operator << (ostream &aStream, const StorageResource_c &aItem) { 
	if (!aStream.good()) return aStream;
	aItem.Print(aStream); 
	return aStream; 
}










} // namespace PerfToos

#endif // __PERF_TOOLS_H__
