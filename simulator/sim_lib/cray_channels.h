#ifndef __CRAY_CHANNELS_H__
#define __CRAY_CHANNELS_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "utils.h"
#include "cray_types.h"
#include <stdint.h>
#include <string.h>
#include <vector>
#include <map>
#include <limits.h>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <queue>
#include "cray_logger.h"
#include "config_file.h"
#include <thread>
#include <mutex>

class Mainframe_c; // Declared in cray_mainframe.h, but we need to break the dependency loop

// Abstract base for CPU I/O channels. Derivatives of this class can handle actual I/O, by overriding the Tick() function
class Channel_i {
public:
	Channel_i(size_t aChannelId, Mainframe_c &aMainframe):
		mAddress(0),
		mLimit(0),
		mInterruptPending(false),
		mError(false),
		mActive(false),
		mChannelId(aChannelId),
		mMainframe(aMainframe)
	{}
	virtual ~Channel_i() {}

	// CPU interface
	virtual void SetAddress(CAddr_t aAddress) {
		mAddress = aAddress;
		if (mAddress == mLimit && IsActive()) {
			GetLogger() << setloglevel(LogLevel_IoActivity) << "Force disconnecting channel by setting Address to Limit, while channel active" << std::endl;
		}
		SetActive(mAddress != mLimit);
		mError = false;
	}
	void SetLimit(CAddr_t aLimit) { mLimit = aLimit; }
	CAddr_t GetAddress() const { return mAddress; }
	virtual bool GetInterruptPending() const { return mInterruptPending; }
	bool GetError() const { return mError; }
	void ClearInterrupt() { SetInterruptPending(false); }
	void ClearError() { mError = false; }

	// Resets the mainframe size of the channel.
	// If there is an IOP side to the channel as well, that doesn't get reset.
	// Called for:
	// Instruction MC:
	//   - For output channels, the master-clear is set. That is to say that it is possible
	//     for an output channel to stop mid-transmission and go silent by executing an MC
	//   - For input channels, the ready flag is cleared. We don't have a ready flag, but
	//     theoretically it should mean that an input channel can stop accepting data
	//     mid-way by executing an MC instruction.
	// For IOPD IopCrayChannel_c (COA) channel:
	//   - The documentation isn't quite clear on this. The original Cray-1S documentation
	//     States that function 4, bit 14 sets/clears the I/O master clear signal. Bit 15 is
	//     used to reset the mainframe processors.
	//   - For the IOS model C/D manual, COA function only mentions bit-14 as the master-clear
	//     signal, but here, the COA channel is described as one connecting to the fornt-end.
	//     For the very similar LOA channel, that connects to the mainframe, also only bit-14
	//     is mentioned as the mainframe master-clear signal.
	//   - The actual behavior seems to be that during mainframe initialization, first both
	//     bit 15 and 14 are set, then bit 14 is released, followed by the release of bit 15.
	//     This is actually consistent with the Cray-1S description and not the IOS C/D manual.
	virtual void MainframeSideMasterClear() {
		mAddress = 0;
		mLimit = 0;
		SetInterruptPending(false);
		mError = false;
		SetActive(false);
	}

	// Plumbing interface
	virtual void ChannelTick() = 0;
	virtual CLogger_c &GetLogger() const = 0;
	virtual bool NeedsChannelTick() const = 0;
	size_t GetChannelId() const { return mChannelId; }

	// Debug functions
	void Dump(size_t aIdent=0) const;
	CAddr_t BufferLeft() const { return mLimit - mAddress; }
protected:
	// Peripheral interface
	virtual void SetInterrupt() { SetInterruptPending(true); }
	virtual void SetError();
	virtual CInt_t GetData();
	virtual void SetData(CInt_t aData, bool aWithDisconnect = false);
	virtual void Disconnect();
	bool IsActive() const { return mActive; }

	virtual void HandleActivation() {}
	virtual void HandleDeactivation() {}
private:
	void SetInterruptPending(bool aInterruptPending);
	void SetActive(bool aActive) {
		if (mActive != aActive) {
			mActive = aActive;
			if (mInterruptPending && mActive) {
				GetLogger() << setloglevel(LogLevel_IoActivity) << "Channel going active with pending interrupt..." << std::endl;
			}
			if (mActive) {
				HandleActivation();
			} else {
				HandleDeactivation();
			}
		}
	}
protected:
	CAddr_t mAddress;
	CAddr_t mLimit;
	bool mInterruptPending;
	bool mError;
	AtomicBool mActive;
	size_t mChannelId;

	Mainframe_c &mMainframe;

	std::recursive_mutex mInterruptLock;
};

class LoggerChannel_c : public Channel_i {
public:
	LoggerChannel_c(const Configuration_c &aConfig, size_t aChannelId, Mainframe_c &aMainframe, CLogger_c &aLogger, bool aInput) :
		Channel_i(aChannelId, aMainframe),
		mLogger(aConfig, "CLOG", aChannelId),
		mInput(aInput)
	{
		mLogger.SetParent(aLogger);
	}
	virtual void ChannelTick() override {}
	virtual CLogger_c &GetLogger() const override { return mLogger; }
	virtual void HandleActivation() override;
	virtual bool NeedsChannelTick() const override { return false; }
protected:
	mutable CLogger_c mLogger;
	bool mInput;
};

class DisconnectedChannel_c: public Channel_i {
public:
	DisconnectedChannel_c(const Configuration_c &aConfig, size_t aChannelId, Mainframe_c &aMainframe, CLogger_c &aLogger): 
	Channel_i(aChannelId, aMainframe), mLogger(aConfig,"CDIS",aChannelId)
	{
		mLogger.SetParent(aLogger);
	}
	virtual void ChannelTick() override {}
	virtual CLogger_c &GetLogger() const override { return mLogger; }
	virtual bool NeedsChannelTick() const override { return false; }
protected:
	mutable CLogger_c mLogger;
};

class FileInputChannel_c: public Channel_i {
public:
/*	FileInputChannel_c(const char *aFileName, std::vector<uint8_t> &aMemory, size_t aChannelId, CLogger_c &aLogger): Channel_i(aMemory, aChannelId), mFileName(aFileName), mLogger("CFI",aChannelId) {
		mLogger.SetParent(aLogger);
		MasterClear();
	}*/
	FileInputChannel_c(const Configuration_c &aConfig, size_t aChannelId, Mainframe_c &aMainframe, CLogger_c &aLogger):
	Channel_i(aChannelId, aMainframe), mFileName(aConfig.get<std::string>("FileName")), mLogger(aConfig, "CFI",aChannelId)
	{
		mLogger.SetParent(aLogger);
		mFile.open(mFileName, std::ios::in | std::ios::binary);
	}
	~FileInputChannel_c() {}
	virtual void ChannelTick() override;
	virtual CLogger_c &GetLogger() const override { return mLogger; }
	virtual bool NeedsChannelTick() const override { return true; }
protected:
	std::string mFileName;
	std::ifstream mFile;
	mutable CLogger_c mLogger;
};

class FileOutputChannel_c: public Channel_i {
public:
/*	FileOutputChannel_c(const char *aFileName, std::vector<uint8_t> &aMemory, size_t aChannelId, CLogger_c &aLogger): Channel_i(aMemory, aChannelId), mFileName(aFileName), mLogger("CFO",aChannelId) {
		mLogger.SetParent(aLogger);
	}*/
	FileOutputChannel_c(const Configuration_c &aConfig, size_t aChannelId, Mainframe_c &aMainframe, CLogger_c &aLogger):
	Channel_i(aChannelId, aMainframe), mFileName(aConfig.get<std::string>("FileName")), mLogger(aConfig,"CFO",aChannelId)
	{
		mLogger.SetParent(aLogger);
	}
	~FileOutputChannel_c() {}
	virtual void ChannelTick() override;
	virtual CLogger_c &GetLogger() const override { return mLogger; }
	virtual bool NeedsChannelTick() const override { return true; }
protected:
	std::string mFileName;
	mutable CLogger_c mLogger;
};

#endif // __CRAY_CHANNELS_H__