#ifndef __CRAY_MAINFRAME_H__
#define __CRAY_MAINFRAME_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mutex>
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
#include "sim_iop.h"
#include "iop_iop2iop.h"
#include "cray_channels.h"
#include "debug_events.h"
#include "config_file.h"
#include "sys_task_req.h"
#include "unicos_proc_list.h"
#include "iop_cluster_if.h"

// Cray Mainframe: this variant doesn't uses host-implemented IOPs instead of simulated ones

// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
//
// To be true to the documentation, octal numbers are extensively used in this file.
// Watch out for the leading 0-s!
//
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!

class SimIopCluster_c;

class Lock_c {
public:
	Lock_c(std::atomic_flag &aFlag): mFlag(aFlag) {
		while (mFlag.test_and_set());
	}
	~Lock_c() { mFlag.clear(); }
protected:
	std::atomic_flag &mFlag;
};

class Terminate_x : public Generic_x {
public:
	Terminate_x() { mErrorStrm << "*** TERMINATE ***"; }
};

class InterCpuCluster_c {
public:
	InterCpuCluster_c() :
		TestAndSetBlockedCnt(0),
		ClusterCpuCnt(0)
	{
		ClusterMutex.clear();
	}
	void Dump(CLogger_c &aLogger, size_t aIdent=0) const {
		for(size_t Idx=0;Idx<sizeof(SB)/sizeof(SB[0]);++Idx) {
			aLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "SB" << DecPrinter(Idx,0) << ": " << HexPrinter(SB[Idx],6) << std::endl;
		}
		for(size_t Idx=0;Idx<sizeof(ST)/sizeof(ST[0]);++Idx) {
			aLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "ST" << DecPrinter(Idx,0) << ": " << HexPrinter(ST[Idx],6) << std::endl;
		}
//		aLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "SM: " << HexPrinter(SM,8) << " (";
		aLogger << setloglevel(LogLevel_Dump) << Indent(aIdent) << "SM: (";
		for(size_t Idx=0;Idx<32;++Idx) {
//			aLogger << setloglevel(LogLevel_Dump) << (((SM & (0x80000000 >> Idx)) == 0) ? '0' : '1');
			aLogger << setloglevel(LogLevel_Dump) << (SM[Idx] ? '0' : '1');
		}
		aLogger << setloglevel(LogLevel_Dump) << ')' << std::endl;
	}

	CAddr_t SB[8];
	CInt_t ST[8];
	std::array<std::atomic_bool, 32> SM;
	size_t TestAndSetBlockedCnt;
	size_t ClusterCpuCnt;
	//std::mutex ClusterMutex;
	std::atomic_flag ClusterMutex;
};

class Cpu_c;
class Mainframe_c;



class Mainframe_c: public TimeStampGenerator_i {
public:
	class BreakPoint_i {
	public:
		BreakPoint_i() {}
		virtual ~BreakPoint_i() {}
		virtual void Fire(Cpu_c &aCpu) = 0;
		virtual bool Test(const Cpu_c &aCpu, const Mainframe_c &aMainframe) = 0;
	};
	class BreakPointBase_c: public BreakPoint_i {
	public:
		explicit BreakPointBase_c(const Configuration_c &aConfig) {
	/***
			try {
				for(const auto &DataCondition: aConfig.get_child_safe("DataConditions")) {
					IopAddr_t Addr = FromString<IopAddr_t>(DataCondition.first);
					IopInt_t Value = DataCondition.second.get_value<IopInt_t>();
					mDataConditions.push_back(DataCondition_s(Addr,Value));
				}
			}
			catch(exception &Ex) {
				mDataConditions.clear();
				throw Ex;
			}
	****/
			mTriggerCnt = aConfig.get<size_t>("TriggerCnt", 0);
			boost::optional<std::string> AbsoluteAddr = aConfig.get_optional<std::string>("AbsoluteAddr");
			if (AbsoluteAddr.is_initialized()) {
				mAbsoluteAddr = FromString(AbsoluteAddr.get(), StringFormat_e::ProgramAddr);
			}
			mPassCnt = 0;
		};
		virtual bool Test(const Cpu_c &aCpu, const Mainframe_c &aMainframe) override;
	protected:
	/***
		struct DataCondition_s {
			IopAddr_t Addr;
			IopInt_t Value;
			DataCondition_s(IopAddr_t aAddr, IopInt_t aValue): Addr(aAddr), Value(aValue) {}
		};
		vector<DataCondition_s> mDataConditions;
	****/
		size_t mPassCnt;
		size_t mTriggerCnt;
		boost::optional<CProgramAddr_t> mAbsoluteAddr;
	};
	class EventFirePoint_c: public BreakPointBase_c {
		public:
			EventFirePoint_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig), mEvent(aConfig.get<DebugEvent_t>("Event")) {}
			virtual void Fire(Cpu_c &aCpu) override;
		protected:
			DebugEvent_t mEvent;
	};
	class TracePoint_c : public BreakPointBase_c {
	public:
		TracePoint_c(const Configuration_c &aConfig) : BreakPointBase_c(aConfig), mTrace(aConfig.get<std::string>("Message")) {}
		virtual void Fire(Cpu_c &aCpu) override;
	protected:
		std::string mTrace;
	};
	class DumpPoint_c : public BreakPointBase_c {
	public:
		DumpPoint_c(const Configuration_c &aConfig) : BreakPointBase_c(aConfig) {}
		virtual void Fire(Cpu_c &aCpu) override;
	protected:
		std::string mTrace;
	};
	class CpuDumpPoint_c: public BreakPointBase_c {
	public:
		CpuDumpPoint_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {}
		virtual void Fire(Cpu_c &aCpu) override;
	};
	class TerminatePoint_c: public BreakPointBase_c {
	public:
		TerminatePoint_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig), mMessage(aConfig.get<std::string>("Message")) {}
		virtual void Fire(Cpu_c &aCpu) override;
	protected:
		std::string mMessage;
	};
	class LogOn_c: public BreakPointBase_c {
	public:
		LogOn_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {}
		virtual void Fire(Cpu_c &aCpu) override;
	};
	class LogOff_c: public BreakPointBase_c {
	public:
		LogOff_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {}
		virtual void Fire(Cpu_c &aCpu) override;
	};
	class LogLevel_c: public BreakPointBase_c {
	public:
		LogLevel_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {
			std::string Level = aConfig.get<std::string>("Level");
			mLogLevel = FromString_LogLevels_e(Level);
		}
		virtual void Fire(Cpu_c &aCpu) override;
	protected:
		LogLevels_e mLogLevel;
	};
	class LogLevelPush_c: public BreakPointBase_c {
	public:
		LogLevelPush_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {
			std::string Level = aConfig.get<std::string>("Level");
			mLogLevel = FromString_LogLevels_e(Level);
		}
		virtual void Fire(Cpu_c &aCpu) override;
	protected:
		LogLevels_e mLogLevel;
	};
	class LogLevelPop_c: public BreakPointBase_c {
	public:
		LogLevelPop_c(const Configuration_c &aConfig): BreakPointBase_c(aConfig) {}
		virtual void Fire(Cpu_c &aCpu) override;
	};
	class Dump_c : public BreakPointBase_c {
	public:
		Dump_c(const Configuration_c &aConfig) : BreakPointBase_c(aConfig) {}
		virtual void Fire(Cpu_c &aCpu) override;
	};
	class History_c : public BreakPointBase_c {
	public:
		History_c(const Configuration_c &aConfig) : BreakPointBase_c(aConfig) {}
		virtual void Fire(Cpu_c &aCpu) override;
	};

	typedef std::map<CProgramAddr_t, std::vector<std::shared_ptr<BreakPoint_i>>> BreakPoints_t;

	class ImageLoadFailure_x: public std::exception {
	public:
		ImageLoadFailure_x(const char *aFileName, const char *aReason) {
			std::stringstream Strm;
			Strm << "Image file: " << aFileName << " cannot be loaded due to: " << aReason;
			mErrorStr = Strm.str();
		}
		ImageLoadFailure_x(const char *aFileName) {
			std::stringstream Strm;
			Strm << "Image file: " << aFileName << " cannot be loaded";
			mErrorStr = Strm.str();
		}
		virtual const char *what() const throw() override { return mErrorStr.c_str(); }
	protected:
		std::string mErrorStr;
	};

	typedef std::unique_ptr<Cpu_c> (*CpuCreator_t)(Mainframe_c *aThis, const std::string aName, const std::string aType, const Configuration_c &aCpuConfig, const Configuration_c &aDefaultCpuConfig, BreakPoints_t &aBreakPoints, size_t aCpuId);
	typedef std::unique_ptr<IopClusterBase_i>(*IopClusterCreator_t)(Mainframe_c *aThis, std::string aType, const Configuration_c &aConfig, size_t aIopId);

	explicit Mainframe_c(const Configuration_c &aConfig, CLogger_c &aLogger, bool aMultiThreaded, bool aDisableAutoTerminal, CpuCreator_t aCpuCreator = CreateCpu, IopClusterCreator_t aIopClusterCreator = CreateIopCluster);
	explicit Mainframe_c(size_t aMemSize, MachineTypes_e aMachineType);
	virtual ~Mainframe_c() override;

	size_t GetChannelCnt() const { return mChannels.size(); }
	typedef Channel_i *ChannelPtr_t;
	ChannelPtr_t &GetChannel(size_t aIdx) {
		CRAY_ASSERT(aIdx < mChannels.size());
		return mChannels[aIdx];
	}
	const ChannelPtr_t &GetChannel(size_t aIdx) const {
		CRAY_ASSERT(aIdx < mChannels.size());
		return mChannels[aIdx];
	}

	void SetChannel(size_t aIdx, Channel_i &aChannel) {
		CRAY_ASSERT(aIdx < mChannels.size());
		if (mChannels[aIdx] != nullptr) {
			CRAY_ASSERT(mOwnedChannels[aIdx] != nullptr);
			mLogger << setloglevel(LogLevel_Warning) << "WARNING: replacing channel at channel number: " << DecPrinter(aIdx + 8) << std::endl;
			mOwnedChannels[aIdx] = nullptr; // Remove the possibly default channel
		}
		mChannels[aIdx] = &aChannel;
		RebuildTickedChannels();
	}

	typedef IteratorWrapper_t<Mainframe_c, ChannelPtr_t, &Mainframe_c::GetChannelCnt, &Mainframe_c::GetChannel> ChannelIteratorWrapper_c;
	const ChannelIteratorWrapper_c GetChannels() { return ChannelIteratorWrapper_c(*this); }
	typedef ConstIteratorWrapper_t<Mainframe_c, ChannelPtr_t, &Mainframe_c::GetChannelCnt, &Mainframe_c::GetChannel> ConstChannelIteratorWrapper_c;
	const ConstChannelIteratorWrapper_c GetChannels() const { return ConstChannelIteratorWrapper_c(*this); }

	size_t GetCpuCnt() const { return mCpus.size(); }
	Cpu_c &GetCpu(size_t aIdx) {
		CRAY_ASSERT(aIdx < mCpus.size());
		return *mCpus[aIdx];
	}
	const Cpu_c &GetCpu(size_t aIdx) const {
		CRAY_ASSERT(aIdx < mCpus.size());
		return *mCpus[aIdx];
	}

	typedef IteratorWrapper_t<Mainframe_c, Cpu_c, &Mainframe_c::GetCpuCnt, &Mainframe_c::GetCpu> CpuIteratorWrapper_c;
	const CpuIteratorWrapper_c GetCpus() { return CpuIteratorWrapper_c(*this); }
	typedef ConstIteratorWrapper_t<Mainframe_c, Cpu_c, &Mainframe_c::GetCpuCnt, &Mainframe_c::GetCpu> ConstCpuIteratorWrapper_c;
	const ConstCpuIteratorWrapper_c GetCpus() const { return ConstCpuIteratorWrapper_c(*this); }

	size_t GetIopClusterCnt() const { return mIopClusters.size(); }
	IopClusterBase_i &GetIopCluster(size_t aIdx) {
		CRAY_ASSERT(aIdx < mIopClusters.size());
		return *mIopClusters[aIdx];
	}
	const IopClusterBase_i &GetIopCluster(size_t aIdx) const {
		CRAY_ASSERT(aIdx < mIopClusters.size());
		return *mIopClusters[aIdx];
	}

	typedef IteratorWrapper_t<Mainframe_c, IopClusterBase_i, &Mainframe_c::GetIopClusterCnt, &Mainframe_c::GetIopCluster> IopIteratorWrapper_c;
	const IopIteratorWrapper_c GetIopClusters() { return IopIteratorWrapper_c(*this); }
	typedef ConstIteratorWrapper_t<Mainframe_c, IopClusterBase_i, &Mainframe_c::GetIopClusterCnt, &Mainframe_c::GetIopCluster> ConstIopIteratorWrapper_c;
	const ConstIopIteratorWrapper_c GetIopClusters() const { return ConstIopIteratorWrapper_c(*this); }

	size_t GetClusterCnt() const { return mClusters.size(); }
	InterCpuCluster_c &GetCluster(size_t aIdx) {
		CRAY_ASSERT(aIdx < mClusters.size());
		return *mClusters[aIdx];
	}
	const InterCpuCluster_c &GetCluster(size_t aIdx) const {
		CRAY_ASSERT(aIdx < mClusters.size());
		return *mClusters[aIdx];
	}

	typedef IteratorWrapper_t<Mainframe_c, InterCpuCluster_c, &Mainframe_c::GetClusterCnt, &Mainframe_c::GetCluster> ClusterIteratorWrapper_c;
	const ClusterIteratorWrapper_c GetClusters() { return ClusterIteratorWrapper_c(*this); }
	typedef ConstIteratorWrapper_t<Mainframe_c, InterCpuCluster_c, &Mainframe_c::GetClusterCnt, &Mainframe_c::GetCluster> ConstClusterIteratorWrapper_c;
	ConstClusterIteratorWrapper_c GetClusters() const { return ConstClusterIteratorWrapper_c(*this); }

	std::vector<uint8_t> &GetMemory() { return mMemory; }
	const std::vector<uint8_t> &GetMemory() const { return mMemory; }
	size_t GetMemorySize() { return mMemory.size(); }
	size_t GetMemorySizeInWords() const { return mMemSizeInWords; }
	size_t GetMemorySizeInParcels() const { return mMemSizeInParcels; }
	size_t mMemSizeInWords;
	size_t mMemSizeInParcels;

	void SetLastCpuIssuedCI(size_t aCpuId) { mLastCpuIssuedCI = aCpuId; }

	void IoMasterClear(bool aMasterClear);
	void CpuMasterClear(bool aMasterClear);
	void DeadStart();
	void ChannelTick();

	virtual CInt_t GetRealTimeClock() const;
	void SetRealTimeClock(CInt_t aValue) {
		if (mUseHostRealTimeClock) {
			mRealTimeStart = mRealTimeTimer.elapsed().wall;
			mLastRealTimeReading = 0;
		}
		mRealTimeClock = aValue;
	}
	void UpdateRealTimeClock() { if (!mUseHostRealTimeClock) mRealTimeClock += mRealTimeClockIncrement; }

	void Dump(size_t aIdent=0) const;
	void DumpMemories() const;
	void DumpHistory();
	CLogger_c &GetLogger() const { return mLogger; }

	virtual uint64_t GetTimeStamp() const override { return mEnableTimeStamp ? mTickCnt : 0ULL; }
	virtual uint64_t GetResolution() const override { return 105000000ULL; } // TODO: make this configurable

	const DebugEventDispatcher_c &GetEventDispatcher() const { return mEventDispatcher; }
	DebugEventDispatcher_c &GetEventDispatcher() { return mEventDispatcher; }

	void HandleDebugEvent(const DebugEvent_t &aEvent) { mEventPoints.Fire(aEvent); }
	bool IsMultiThreaded() const { return mMultiThreaded; }
	OsTypes_e GetOsType() const { return mOsType; }

	void LoadImageFile(const char *aFileName, CAddr_t aLoadAddr, size_t aPreamble = 0, size_t aImageSize = std::numeric_limits<size_t>::max());

	MachineTypes_e GetMachineType() const { return mMachineType; }
	void HandleDeadLock(uint16_t aCluster);
	void RouteChannelInterrupt(); // Route the channel interrupt to the appropriate CPU
	bool DoDisableAutoTerminal() const { return mDisableAutoTerminal; }
	CInt_t SimToHost(CInt_t aValue);
	CInt_t HostToSim();
	void RegisterCommands(CommandHooks_t &aHooks);
protected:
	MachineTypes_e mMachineType;
	bool mMultiThreaded;
	bool mDisableAutoTerminal;
	mutable CInt_t mLastRealTimeReading;
	CInt_t mDeltaClockIncrement;

	class MainFrameEventDispatcher_c: public DebugEventDispatcher_c {
	public:
		MainFrameEventDispatcher_c(Mainframe_c &aParent): mParent(aParent) {}
		virtual void Fire(const DebugEvent_t &aEvent) const override {
			mParent.GetLogger() << setloglevel(LogLevel_EventFire) << aEvent << std::endl;
			DebugEventDispatcher_c::Fire(aEvent);
		}
	protected:
		Mainframe_c &mParent;
	} mEventDispatcher;


	struct MemoryPoke_s {
		CAddr_t Addr;
		CInt_t Value;
		bool IsParcelPoke;
	};
	std::vector<MemoryPoke_s> mMemoryPokes;
	bool mMemoryPokesProcessed;

	std::vector<std::unique_ptr<Cpu_c>> mCpus;
	std::vector<std::unique_ptr<IopClusterBase_i>> mIopClusters;
	std::vector<std::unique_ptr<IopIopChannel_c>> mIopIopChannels;

	size_t mLastCpuIssuedCI; // Index to the last CPU that issued a clear interrupt

	std::vector<std::unique_ptr<InterCpuCluster_c>> mClusters;
	std::vector<Channel_i*> mChannels;
	std::vector<Channel_i*> mTickedChannels;
	std::vector<std::unique_ptr<Channel_i>> mOwnedChannels;
	std::vector<uint8_t> mMemory;
	uint8_t *mFastMemory;
	uint8_t mStartupCpuIdx;

	void RebuildTickedChannels();

	boost::timer::cpu_timer mRealTimeTimer;
	boost::timer::nanosecond_type mRealTimeStart;
	double mSystemClockPeriod;
	CInt_t mRealTimeClock;
	CInt_t mRealTimeClockIncrement;
	CInt_t mRealTimeClockChunkLimit;
	bool mUseHostRealTimeClock;

	struct WatchPoint_c {
		WatchPoint_c() {}
		explicit WatchPoint_c(const std::string &aMessage): mHitCnt(0), mMessage(aMessage) {}
		size_t mHitCnt;
		std::string mMessage;
	};
	std::map<CAddr_t, WatchPoint_c> mWatchPoints;

	static std::shared_ptr<BreakPoint_i> CreateBreakPoint(const std::string &aBreakPointType, const Configuration_c &aConfig);

	class EventPoints_c: public EventPoints_t<BreakPoint_i> {
	public:
		explicit EventPoints_c(Mainframe_c &aParent): mParent(aParent) {}
	protected:
		virtual void FireBreakPoint(BreakPoint_i *aBreakPoint) override { if (aBreakPoint->Test(mParent.GetCpu(0), mParent)) aBreakPoint->Fire(mParent.GetCpu(0)); } // TODO: which CPU to pass on to Fire???
		virtual std::shared_ptr<BreakPoint_i> CreateBreakPoint(const std::string &aBreakPointType, const Configuration_c &aConfig) override { return Mainframe_c::CreateBreakPoint(aBreakPointType, aConfig); }
		Mainframe_c &mParent;
	};
	EventPoints_c mEventPoints;

	bool mIoMasterClear;
	bool mCpuMasterClear;

	uint64_t mTickCnt;

	bool mEnableTimeStamp;

	OsTypes_e mOsType;

	void CheckWatchPointsInternal(CAddr_t aAddr, bool aWrite, CInt_t aData);

	void CheckWatchPoints(CAddr_t aAddr, CInt_t aData) {
#ifndef CRAY_TURBO
		if (mProcListParser != nullptr) mProcListParser->TestMemAccess(aAddr, aData);
#endif
		if (mWatchPoints.empty()) return;
		CheckWatchPointsInternal(aAddr, true, aData);
	}

	void CheckWatchPoints(CAddr_t aAddr) {
		if (mWatchPoints.empty()) return;
		CheckWatchPointsInternal(aAddr, false, 0);
	}

	std::string ReportHist() const;
	std::string ReportBBHist() const;

public:
	struct ReadOutOfBoundsError_x : public Generic_x { ReadOutOfBoundsError_x() : Generic_x("Data read out of bounds") {} };

	template <typename aType> aType MemReadNoWatchpoint(const CAddr_t aAddr) const { size_t Addr = size_t(aAddr) * 8; if (Addr >= mMemory.size()) throw ReadOutOfBoundsError_x(); return *(aType *)(&mMemory[Addr]); } // Default addresses are by QWORDs
	template <typename aType> aType MemReadNoWatchpointByType(const CAddr_t aAddr) const { size_t Addr = size_t(aAddr) * sizeof(aType); if (Addr >= mMemory.size()) throw ReadOutOfBoundsError_x(); return *(aType *)(&mMemory[Addr]); }
	template <typename aType> aType MemReadNoWatchpointByByte(const CAddr_t aAddr) const { size_t Addr = size_t(aAddr); if (Addr >= mMemory.size()) throw ReadOutOfBoundsError_x(); return *(aType *)(&mMemory[Addr]); }

//	template <typename aType> void MemWriteNoWatchpoint(const CAddr_t aAddr, aType aVal) { CheckWatchPoints(aAddr, aVal); *(aType *)(&mMemory[aAddr * 8]) = aVal; } // Default addresses are by QWORDs
	template <typename aType> void MemWriteNoWatchpointByType(const CAddr_t aAddr, aType aVal) { *(aType *)(&mMemory[aAddr * sizeof(aType)]) = aVal; }
//	template <typename aType> void MemWriteNoWatchpointByByte(const CAddr_t aAddr, aType aVal) { *(aType *)(&mMemory[size_t(aAddr)]) = aVal; }

//	template <typename aType> aType *MemAccess(const CAddr_t aAddr) { CheckWatchPoints(aAddr); return (aType *)(&mMemory[aAddr * 8]); } // Default addresses are by QWORDs
	template <typename aType> aType *MemAccessByType(const CAddr_t aAddr) { CheckWatchPoints(aAddr * sizeof(aType) / 8); return (aType *)(&mMemory[aAddr * sizeof(aType)]); }
//	template <typename aType> aType *MemAccessByByte(const CAddr_t aAddr) { CheckWatchPoints(aAddr / 8); return (aType *)(&mMemory[size_t(aAddr)]); }

	template <typename aType> aType MemRead(const CAddr_t aAddr) { CheckWatchPoints(aAddr); return *(aType *)(&mMemory[aAddr * 8]); } // Default addresses are by QWORDs
	template <typename aType> aType MemReadByType(const CAddr_t aAddr) { CheckWatchPoints(aAddr * sizeof(aType) / 8); return *(aType *)(&mMemory[aAddr * sizeof(aType)]); }
	template <typename aType> aType MemReadByByte(const CAddr_t aAddr) { CheckWatchPoints(aAddr / 8); return *(aType *)(&mMemory[size_t(aAddr)]); }

//	template <typename aType> void MemWrite(const CAddr_t aAddr, aType aVal) { CheckWatchPoints(aAddr); *(aType *)(&mMemory[aAddr * 8]) = aVal; } // Default addresses are by QWORDs
//	template <typename aType> void MemWriteByType(const CAddr_t aAddr, aType aVal) { CheckWatchPoints(aAddr * sizeof(aType) / 8); *(aType *)(&mMemory[aAddr * sizeof(aType)]) = aVal; }

	void MemWrite(const CAddr_t aAddr, uint8_t aVal) { CheckWatchPoints(aAddr / 8); mMemory[size_t(aAddr)] = aVal; }
	void MemWrite(const CAddr_t aAddr, CInt_t aVal) { CheckWatchPoints(aAddr, aVal); *(CInt_t *)(&mMemory[aAddr * 8]) = aVal; } // Default addresses are by QWORDs
protected:

	boost::optional<std::string> mMainMemoryDumpFileName;

	mutable CLogger_c mLogger;
	std::unique_ptr<UnicosProcList_c> mProcListParser;

	// A function that needs to be implemented to create various CPU types inside the final applcation.
	// If not, then the application will not link.

	static std::unique_ptr<Cpu_c> CreateCpu(Mainframe_c *aThis, const std::string aName, const std::string aType, const Configuration_c &aCpuConfig, const Configuration_c &aDefaultCpuConfig, BreakPoints_t &aBreakPoints, size_t aCpuId);
	static std::unique_ptr<IopClusterBase_i> CreateIopCluster(Mainframe_c *aThis, std::string aType, const Configuration_c &aConfig, size_t aIopId);
};


#endif // __CRAY_MAINFRAME_H__
