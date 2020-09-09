#ifndef __IOP_DISK_H__
#define __IOP_DISK_H__

#include "cray_iop.h"
#include <fstream>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/variant.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/timer/timer.hpp>
#include "cray_channels.h"
#ifndef _WIN32
#include <unistd.h>
#endif //_WIN32
#include "config_file.h"

class IopDiskDD29_c: public IopSimplePeripheral_i {
public:
	explicit IopDiskDD29_c(const Configuration_c &aConfig, IopCpu_c &aParent);
	virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
	virtual IopBit_t GetBusy() override { return mOperation != Operation_Idle; }
	virtual IopBit_t GetDone() override { return mDone; }
	virtual IopBit_t GetInterrupt() override { return mInterruptEnabled && mDone; }
	virtual void MasterClear() override { mDone = false; mOperation = Operation_Idle; mInterruptEnabled = true; mIoMemoryAddr = 0; mCurrentHead = 0; mCurrentSector = 0; mCurrentTrack = 0; mMounted = false; mStatusResponseRegister = 0; mBufferEchoMode = true; }
	virtual size_t GetChannelIdx() const override { return mChannelIdx; }
	void DeadStart() { MasterClear(); }
	virtual void Tick() override {}
	virtual bool NeedsTick() override { return false; }
	virtual void Dump(size_t aIdent=0) override {}
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Disk; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override;
	virtual void RegisterCommands(CommandHooks_t &aHooks) override {}
	virtual std::string GetName() const override;
protected:
	IopCpu_c &mParent;

	// Configuration parameters
	bool mReadOnly;
	std::string mImageFileName;
	size_t mNumHeads;
	size_t mNumSectors;
	size_t mNumTracks;
	size_t mSectorSize;
	// Operational parameters
	IopInt_t mIoMemoryAddr;
	size_t mCurrentHead;
	size_t mCurrentSector;
	size_t mCurrentTrack;
	size_t mChannelIdx;
	bool mDone;
	enum Operations_e {
		Operation_Read,
		Operation_Write,
		Operation_Format,
		Operation_Idle
	} mOperation;
	bool mInterruptEnabled;
	bool mMounted;
	bool mBufferEchoMode;
	IopInt_t mStatusResponseRegister;
	IopInt_t mBufferAB[512];
	CLogger_c mLogger;

	// Statistics counters
	size_t mReads;
	size_t mWrites;

	std::string mName;

	void ExecOperation();
};

#endif // __IOP_DISK_H__
