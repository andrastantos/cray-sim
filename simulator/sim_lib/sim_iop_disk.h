#ifndef __SIM_IOP_DISK_H__
#define __SIM_IOP_DISK_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <boost/asio.hpp>
#include "utils.h"
#include "cray_types.h"
#include "cray_logger.h"
#include "cray_channels.h"
#include "debug_events.h"
#include <stdint.h>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/timer/timer.hpp>
#include "config_file.h"
#include "ui.h"
#include "cray_mainframe.h"
#include <queue>

class SimIopDisk_c {
public:
	SimIopDisk_c(const Configuration_c &aConfig, const std::string &aDiskId, class SimIopCluster_c &aParent);
	~SimIopDisk_c() {}
	void Read(std::streamoff aCylinder, std::streamoff aHead, std::streamoff aSector, uint32_t aSectorCnt, CAddr_t aMainframeAddr);
	void Write(std::streamoff aCylinder, std::streamoff aHead, std::streamoff aSector, uint32_t aSectorCnt, CAddr_t aMainframeAddr);
	void Read(std::streamoff aSectorIdx, uint32_t aSectorCnt, CAddr_t aMainframeAddr);
	void Write(std::streamoff aSectorIdx, uint32_t aSectorCnt, CAddr_t aMainframeAddr);
	void GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const;
	std::string GetName() const;
	void RegisterCommands(CommandHooks_t &aHooks);
	void MasterClear();
	uint32_t GetPhysicalDeviceId() const { return mPysicalDeviceId; }
	uint8_t GetIopNumber() const { return mIopNumber; }
	uint8_t GetUnit() const { return mUnit; }
	uint32_t GetSectorSize() const { return mSectorSize; }
protected:
	std::string mDiskId;
	CLogger_c mLogger;
	SimIopCluster_c &mParent;
	std::string mImageFileName;
	uint8_t mIopNumber;
	uint8_t mUnit;
	uint32_t mPysicalDeviceId;
	std::streamoff mNumHeads;
	std::streamoff mNumSectors;
	std::streamoff mNumCylinders;
	uint32_t mSectorSize;

	// Statistics
	std::streamoff mCurrentHead;
	std::streamoff mCurrentSector;
	std::streamoff mCurrentCylinder;

	uint32_t mReads;
	uint32_t mWrites;
};


#endif // __SIM_IOP_DISK_H__
