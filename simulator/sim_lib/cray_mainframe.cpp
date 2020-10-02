// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
//
// To be true to the documentation, octal numbers are extensively used in this file.
// Watch out for the leading 0-s!
//
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!

#include <stdio.h>
#include <stdlib.h>
#include <limits>
#include <math.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "utils.h"
#include "cray_types.h"
#include "cray_mainframe.h"
#include "cray_cpu.h"
#include "cray_softcpu.h"
#include "sys_task_req.h"
#include "iop_cluster.h"

bool Mainframe_c::BreakPointBase_c::Test(const Cpu_c &aCpu, const Mainframe_c &aMainframe) {
	if (mAbsoluteAddr.is_initialized()) {
		if (aCpu.GetAbsoluteProgramCounter() != mAbsoluteAddr.get()) return false;
	}
/***
	for(auto &DataCondition: mDataConditions) {
		if (aMemory.ReadWord(DataCondition.Addr) != DataCondition.Value) {
			//cout << "Breakpoint doesn't hit, memory at address " << HexPrinter(DataCondition.Addr) << " is " << HexPrinter(aMemory.ReadWord(DataCondition.Addr)) << " instead of " << HexPrinter(DataCondition.Value) << endl;
			return false;
		}
	}
***/
	++mPassCnt;
	if (mTriggerCnt > mPassCnt) return false;
	return true;
}
void Mainframe_c::EventFirePoint_c::Fire(Cpu_c &aCpu) {
	std::stringstream CpuName;
	CpuName << "CPU" << DecPrinter(aCpu.GetCpuId());
	DebugEvent_t FiredEvent = Replace(mEvent, "{cpu}", CpuName.str());
	aCpu.GetMainframe().GetEventDispatcher().Fire(FiredEvent);
}
void Mainframe_c::TracePoint_c::Fire(Cpu_c &aCpu) { aCpu.GetLogger() << setloglevel(LogLevel_Event) << mTrace << std::endl; }
void Mainframe_c::CpuDumpPoint_c::Fire(Cpu_c &aCpu) { aCpu.Dump(); }
void Mainframe_c::DumpPoint_c::Fire(Cpu_c &aCpu) { aCpu.GetMainframe().Dump(); aCpu.GetMainframe().DumpMemories(); }
void Mainframe_c::TerminatePoint_c::Fire(Cpu_c &aCpu) { throw Assert_x(mMessage); }
void Mainframe_c::LogOn_c::Fire(Cpu_c &aCpu) { aCpu.GetLogger().SetDisplayLogLevel(LogLevel_All); }
void Mainframe_c::LogOff_c::Fire(Cpu_c &aCpu) { aCpu.GetLogger().SetDisplayLogLevel(LogLevel_None); }
void Mainframe_c::LogLevel_c::Fire(Cpu_c &aCpu) { aCpu.GetLogger().SetDisplayLogLevel(mLogLevel); }
void Mainframe_c::LogLevelPush_c::Fire(Cpu_c &aCpu) { aCpu.GetLogger().PushDisplayLogLevel(mLogLevel); }
void Mainframe_c::LogLevelPop_c::Fire(Cpu_c &aCpu) { aCpu.GetLogger().PopDisplayLogLevel(); }
void Mainframe_c::Dump_c::Fire(Cpu_c &aCpu) { aCpu.Dump(1); }
void Mainframe_c::History_c::Fire(Cpu_c &aCpu) {
	auto LogLine = aCpu.GetLogger() << setloglevel(LogLevel_Dump);
	if (!LogLine.good()) {
		std::cout << "History dump disabled" << std::endl;
		return;
	}
	LogLine << "============================== HISTORY ==============================" << std::endl;
	aCpu.DumpHistory();
	aCpu.ClearHistory();
	aCpu.GetLogger() << setloglevel(LogLevel_Dump) << "============================== END HISTORY ==========================" << std::endl;
}
// Mainframe_c
////////////////////////////////////

std::shared_ptr<Mainframe_c::BreakPoint_i> Mainframe_c::CreateBreakPoint(const std::string &aBreakPointType, const Configuration_c &aConfig) {
	std::unique_ptr<BreakPoint_i> BreakPoint;
	if (aBreakPointType == "LogOn") {
		return std::make_shared<LogOn_c>(aConfig);
	} else if (aBreakPointType == "Terminate") {
		return std::make_shared<TerminatePoint_c>(aConfig);
	} else if (aBreakPointType == "Dump") {
		return std::make_shared<DumpPoint_c>(aConfig);
	} else if (aBreakPointType == "LogOff") {
		return std::make_shared<LogOff_c>(aConfig);
	} else if (aBreakPointType == "LogLevel") {
		return std::make_shared<LogLevel_c>(aConfig);
	} else if (aBreakPointType == "LogLevelPush") {
		return std::make_shared<LogLevelPush_c>(aConfig);
	} else if (aBreakPointType == "LogLevelPop") {
		return std::make_shared<LogLevelPop_c>(aConfig);
	} else if (aBreakPointType == "Trace") {
		return std::make_shared<TracePoint_c>(aConfig);
	} else if (aBreakPointType == "CpuDump") {
		return std::make_shared<CpuDumpPoint_c>(aConfig);
	} else if (aBreakPointType == "Event") {
		return std::make_shared<EventFirePoint_c>(aConfig);
	} else if (aBreakPointType == "DumpCpu") {
		return std::make_shared<Dump_c>(aConfig);
	} else if (aBreakPointType == "History") {
		return std::make_shared<History_c>(aConfig);
	} else {
		throw InvalidParameter_x(boost::format("Unkown channel breakpoint type:") % aBreakPointType);
	}
}

std::unique_ptr<Cpu_c> Mainframe_c::CreateCpu(Mainframe_c *aThis, const std::string aName, std::string aType, const Configuration_c &aCpuConfig, const Configuration_c &aDefaultCpuConfig, BreakPoints_t &aBreakPoints, size_t aCpuId) {
	if (aType == "Soft") {
		return std::make_unique<SoftCpu_c>(aCpuConfig, aDefaultCpuConfig, *aThis, aBreakPoints, aCpuId);
	} else {
		throw InvalidParameter_x(boost::format("Invalid CPU Type specification for CPU %1%: %2%") % aName % aType);
	}
}

std::unique_ptr<IopClusterBase_i> Mainframe_c::CreateIopCluster(Mainframe_c *aThis, std::string aType, const Configuration_c &aConfig, size_t aIopId) {
	if (aType == "SimIopCluster") {
		return std::make_unique<SimIopCluster_c>(
			aConfig,
			aIopId,
			*aThis
		);
	} else if (aType == "IopDCluster") {
		return std::make_unique<IopCluster_c>(
			aConfig,
			aThis->GetLogger(),
			aIopId,
			*aThis
		);
	} else {
		throw Generic_x() << "Unkown IopCluster type: " << aType;
	}
}

Mainframe_c::Mainframe_c(const Configuration_c &aConfig, CLogger_c &aLogger, bool aMultiThreaded, bool aDisableAutoTerminal, CpuCreator_t aCpuCreator, IopClusterCreator_t aIopClusterCreator) :
	mLastCpuIssuedCI(0),
	mRealTimeClock(0),
	mTickCnt(0),
	mIoMasterClear(false),
	mCpuMasterClear(false),
	mMemoryPokesProcessed(false),
	mEventPoints(*this),
	mEventDispatcher(*this),
	mEnableTimeStamp(aConfig.get<bool>("EnableTimeStamp",true)),
	mMultiThreaded(aMultiThreaded),
	mLogger(aConfig),
	mOsType(OsTypes_e::None),
	mUseHostRealTimeClock(aConfig.get<bool>("UseHostRealTimeClock", true)),
	mRealTimeClockIncrement(aConfig.get<CInt_t>("RealTimeClockIncrement", 10)),
	mRealTimeClockChunkLimit(aConfig.get<CInt_t>("RealTimeClockChunkLimit", 5000)),
	mDeltaClockIncrement(aConfig.get<CInt_t>("DeltaClockIncrement", 1)),
	mDisableAutoTerminal(aDisableAutoTerminal)
{
	mLogger.SetParent(aLogger);
	mChannels.resize(aConfig.get<size_t>("ChannelCount", 8));
	mOwnedChannels.resize(mChannels.size());
	try {
		std::string MachineType = aConfig.get<std::string>("MachineType", "J90");
		if (MachineType == "J90") {
			mMachineType = MachineTypes_e::J90;
			mSystemClockPeriod = 10.0;
		}
		else if (MachineType == "YEL" || MachineType == "YMP-el") {
			mMachineType = MachineTypes_e::YEL;
			mSystemClockPeriod = 30.0;
		}
		else if (MachineType == "YMP") {
			mMachineType = MachineTypes_e::YMP;
			mSystemClockPeriod = 6.0;
		}
		else if (MachineType == "SV1") {
			mMachineType = MachineTypes_e::SV1;
			mSystemClockPeriod = 10.0; // The SV1 has a split core/system clock architecture. While the core clock can be up to 500MHz, the system clock was locked at 100MHz.
		}
		else if (MachineType == "XMP1") {
			mMachineType = MachineTypes_e::XMP1xx;
			mSystemClockPeriod = 9.5;
		}
		else if (MachineType == "XMP2") {
			mMachineType = MachineTypes_e::XMP2xx;
			mSystemClockPeriod = 9.5;
		}
		else if (MachineType == "XMP4") {
			mMachineType = MachineTypes_e::XMP4xx;
			mSystemClockPeriod = 9.5;
		}
		else {
			throw Generic_x() << "Unknown machine type: " << MachineType;
		}
		mSystemClockPeriod = aConfig.get<double>("SystemClockPeriod", mSystemClockPeriod);
		if (mUseHostRealTimeClock) {
			mRealTimeTimer.start();
			mRealTimeStart = mRealTimeTimer.elapsed().wall;
		}

		// Create shared memories
		mMemory.resize(aConfig.get("CpuMemorySize",8*1024*1024) * sizeof(CInt_t),0);
		mMemSizeInWords = mMemory.size() / sizeof(CInt_t);
		mMemSizeInParcels = mMemory.size() / sizeof(CParcel_t);
		mFastMemory = &mMemory[0];

		// Load all image files
		for(const auto &Image : aConfig.get_child_safe("ImageFiles")) {
			size_t LoadAddress = FromString<size_t>(Image.first);
			std::string ImageFileName = Image.second.get_value<std::string>();
			LoadImageFile(ImageFileName.c_str(),CAddr_t(LoadAddress));
		}

		// Create IOP Clusters
		for(const auto &IopCluster : aConfig.get_child_safe("IopClusters")) {
			const std::string &Type = IopCluster.first;
			mIopClusters.push_back(aIopClusterCreator(this, Type, IopCluster.second, mIopClusters.size()));
		}

		BreakPoints_t BreakPoints;
		// Load all break-points (need to happen before CPUs are created as CPUs create local copies of the BreakPoint map)
		for (const auto &BreakPoint : aConfig.get_child_safe("BreakPoints")) {
			std::string BreakPointType = BreakPoint.second.get<std::string>("Type");
			CProgramAddr_t Addr = FromString(BreakPoint.first, StringFormat_e::ProgramAddr);
			std::shared_ptr<BreakPoint_i> BreakPointObj(CreateBreakPoint(BreakPointType, BreakPoint.second));
			CRAY_ASSERT(BreakPointObj != nullptr);
			BreakPoints[Addr].push_back(BreakPointObj);
		}
		// Load all watch points
		for (const auto &WatchPoint : aConfig.get_child_safe("WatchPoints")) {
			std::string EventMessage = WatchPoint.second.data();
			CAddr_t Addr = FromString(WatchPoint.first, StringFormat_e::DataAddr);
			mWatchPoints[Addr] = WatchPoint_c(EventMessage);
		}

		// Load all event-points
		if (aConfig.get_optional<std::string>("EventPoints").is_initialized()) {
			mEventPoints.LoadEventPoints(aConfig.get_child("EventPoints"));
		}

		// Create CPUs
		Configuration_c Cpus = aConfig.get_child_safe("Cpus");
		Configuration_c DefaultCpuConfig = Cpus.get_child_safe("Default");
		boost::optional<size_t> CpuCount = aConfig.get_optional<size_t>("CpuCount");
		if (!CpuCount.is_initialized()) {
			size_t MaxCpuId = 0;
			for (const auto &Cpu : aConfig.get_child_safe("Cpus")) {
				if (Cpu.first == "Default") continue;
				size_t CurCpuId = FromString<size_t>(Cpu.first);
				MaxCpuId = std::max(CurCpuId, MaxCpuId);
			}
			CpuCount = MaxCpuId;
		}
		for (size_t CpuIdx = 0; CpuIdx < CpuCount.get(); ++CpuIdx) {
			std::stringstream CpuName;
			CpuName << CpuIdx;
			Configuration_c CpuConfig = Cpus.get_child_safe(CpuName.str().c_str());
			std::string CpuType = CpuConfig.get<std::string>("Type", DefaultCpuConfig.get<std::string>("Type", "Soft"));
			mCpus.push_back(aCpuCreator(this, CpuName.str(), CpuType, CpuConfig, DefaultCpuConfig, BreakPoints, mCpus.size()));
		}

		// Create clusters
		size_t ClusterCnt = aConfig.get<size_t>("ClusterCount", mCpus.size() + 1);
		for(size_t i=0;i<ClusterCnt;++i) mClusters.push_back(std::make_unique<InterCpuCluster_c>());

		// Create channels
		for(const auto &CpuChannel : aConfig.get_child_safe("CpuChannels")) {
			int ChannelIdx = FromString<int>(CpuChannel.first) - 8;
			if (size_t(ChannelIdx) >= mChannels.size() || ChannelIdx < 0) throw InvalidParameter_x(boost::format("Invalid cpu channel index: %1$03o") % (ChannelIdx+8));
			if (mChannels[ChannelIdx] != nullptr) throw InvalidParameter_x(boost::format("Duplicate cpu channel index: %1$03o") % (ChannelIdx+8));
			std::string ChannelType = CpuChannel.second.get<std::string>("Type", "Disconnected");
			if (ChannelType == "Disconnected") {
				mOwnedChannels[ChannelIdx] = std::make_unique<DisconnectedChannel_c>(CpuChannel.second, ChannelIdx + 8, *this, mLogger);
				mChannels[ChannelIdx] = mOwnedChannels[ChannelIdx].get();
			} else if (ChannelType == "Logger") {
				std::string Direction = CpuChannel.second.get<std::string>("Direction", "Auto");
				bool IsInput;
				if (Direction == "Input") { IsInput = true; }
				else if (Direction == "Output") { IsInput = false; }
				else if (Direction == "Auto") { IsInput = (ChannelIdx % 2) == 0; }
				else throw InvalidParameter_x(boost::format("Invalid direction for logger channel %1%: %2%") % (ChannelIdx + 8) % Direction);
				mOwnedChannels[ChannelIdx] = std::make_unique<LoggerChannel_c>(CpuChannel.second, ChannelIdx + 8, *this, mLogger, IsInput);
				mChannels[ChannelIdx] = mOwnedChannels[ChannelIdx].get();
			}else if (ChannelType == "FileInput") {
				mOwnedChannels[ChannelIdx] = std::make_unique<FileInputChannel_c>(CpuChannel.second, ChannelIdx + 8, *this, mLogger);
				mChannels[ChannelIdx] = mOwnedChannels[ChannelIdx].get();
			} else if (ChannelType == "FileOutput") {
				mOwnedChannels[ChannelIdx] = std::make_unique<FileOutputChannel_c>(CpuChannel.second, ChannelIdx + 8, *this, mLogger);
				mChannels[ChannelIdx] = mOwnedChannels[ChannelIdx].get();
			} else {
				throw InvalidParameter_x(boost::format("Invalid CPU channel type specification for channel %1%: %2%") % (ChannelIdx + 8) % ChannelType);
			}
		}
		if (aConfig.get<bool>("LogUnusedChannels")) {
			for (size_t Idx = 0; Idx < mOwnedChannels.size(); ++Idx) {
				if (mChannels[Idx] == nullptr) {
					CRAY_ASSERT(mOwnedChannels[Idx] == nullptr);
					bool IsInput = (Idx % 2) == 0;
					mOwnedChannels[Idx] = std::make_unique<LoggerChannel_c>(aConfig.get_child_safe("DefaultChannel"), Idx + 8, *this, mLogger, IsInput);
					mChannels[Idx] = mOwnedChannels[Idx].get();
				}
			}
		}
		RebuildTickedChannels();

		// Execute pokes to change memory image
		for(const auto &CpuMemoryPoke: aConfig.get_child_safe("CpuMemoryPokes")) {
			MemoryPoke_s Poke;
			Poke.Value = FromString<CInt_t>(CpuMemoryPoke.second.data());
			try {
				Poke.Addr = FromString(CpuMemoryPoke.first, StringFormat_e::ProgramAddr);
				if (size_t(Poke.Addr) * sizeof(CParcel_t) >= GetMemorySize()) throw InvalidParameter_x("Poke address outside of memory");
				Poke.IsParcelPoke = true;
				mMemoryPokes.push_back(Poke);
			}
			catch (StringFormatError_x &) {
				Poke.Addr = FromString(CpuMemoryPoke.first, StringFormat_e::DataAddr);
				if (size_t(Poke.Addr) * sizeof(CInt_t) >= GetMemorySize()) throw InvalidParameter_x("Poke address outside of memory");
				Poke.IsParcelPoke = false;
				mMemoryPokes.push_back(Poke);
			}
		}

		// Set startup CPU
		mStartupCpuIdx = aConfig.get<uint8_t>("StartupCpuIdx",0);

		// Get memory dump file names
		mMainMemoryDumpFileName = aConfig.get_optional<std::string>("MemoryDumpFile");

		// Registering ourselves to receive events
		GetEventDispatcher().AddHandler(*this);

		// Get the OS type
		std::string OsTypeString = aConfig.get<std::string>("OsType", "None");
		if (OsTypeString == "None") {
			mOsType = OsTypes_e::None;
		} else if (OsTypeString == "COS") {
			mOsType = OsTypes_e::COS;
		} else if (OsTypeString == "UNICOS") {
			mOsType = OsTypes_e::UNICOS;
		} else {
			throw InvalidParameter_x("Invalid OS type specified");
		}
		if (mOsType == OsTypes_e::UNICOS) {
			SetupSystaskParser(aConfig);
			try {
				mProcListParser = std::make_unique<UnicosProcList_c>(*this, aConfig);
			}
			catch (std::exception &Ex) {
				mLogger << setloglevel(LogLevel_Error) << "Can't create proc list parser with error: " << Ex.what() << std::endl;
			} // Simply eat the exception and leave the proc-list unconstructed
		}
	}
	catch (...) {
		mCpus.clear();
		mIopClusters.clear();
		mIopIopChannels.clear();
		mClusters.clear();
		mChannels.clear();
		mOwnedChannels.clear();
		mMemory.clear();
		throw;
	}
}

Mainframe_c::Mainframe_c(size_t aMemSize, MachineTypes_e aMachineType) :
	mLastCpuIssuedCI(0),
	mRealTimeClock(0),
	mTickCnt(0),
	mIoMasterClear(false),
	mCpuMasterClear(false),
	mMemoryPokesProcessed(false),
	mEventPoints(*this),
	mEventDispatcher(*this),
	mMultiThreaded(false),
	mMachineType(aMachineType),
	mLogger(Configuration_c::GetEmptyConfig()),
	mDisableAutoTerminal(false)
{
	try {
		// Create shared memories
		uint64_t MemorySize = aMemSize;
		const size_t MaxMemSize = 128 * 1024 * 1024;
		if (MemorySize > MaxMemSize) throw Generic_x("Can't allocate that much memory!");

		mMemory.resize((size_t)MemorySize * sizeof(CInt_t), 0);
		mMemSizeInWords = mMemory.size() / sizeof(CInt_t);
		mMemSizeInParcels = mMemory.size() / sizeof(CParcel_t);

		// Create a CPU
		mCpus.push_back(std::make_unique<SoftCpu_c>(*this));
	}
	catch (...) {
		mCpus.clear();
		mIopClusters.clear();
		mIopIopChannels.clear();
		mClusters.clear();
		mChannels.clear();
		mOwnedChannels.clear();
		mMemory.clear();
		throw;
	}
}

void Mainframe_c::LoadImageFile(const char *aFileName, CAddr_t aLoadAddr, size_t aPreamble, size_t aImageSize) {
	CRAY_ASSERT(mMemory.size() % sizeof(CInt_t) == 0);
	CAddr_t MemSize = CAddr_t(mMemory.size()/sizeof(CInt_t));
	if (aLoadAddr > MemSize) throw ImageLoadFailure_x(aFileName,"load address is beyond memory size");
	std::ifstream File(aFileName, std::ios_base::in | std::ios_base::binary);
	if (File.fail()) throw ImageLoadFailure_x(aFileName, "can't open file");
	File.seekg(aPreamble);
	uintmax_t FileSize = std::min(boost::filesystem::file_size(aFileName) - aPreamble, uintmax_t(aImageSize));
	//if (FileSize % sizeof(CInt_t) != 0) throw ImageLoadFailure_x(aFileName,"file size is not modulo 8");
	if ((FileSize + sizeof(CInt_t) - 1) / sizeof(CInt_t) + size_t(aLoadAddr) > size_t(MemSize)) throw ImageLoadFailure_x(aFileName, "file doesn't fit in memory");
	File.read((char*)(&mMemory[(size_t(aLoadAddr) * sizeof(CInt_t))]), FileSize);
	size_t BytesRead = size_t(File.gcount());
	if (BytesRead != FileSize || File.fail()) throw ImageLoadFailure_x(aFileName,"can't read file");
}

Mainframe_c::~Mainframe_c() {
}

void Mainframe_c::RebuildTickedChannels() {
	mTickedChannels.clear();
	for (auto &Channel : mChannels) {
		if (Channel == nullptr) continue;
		if (Channel->NeedsChannelTick()) mTickedChannels.push_back(Channel);
	}
}

void Mainframe_c::IoMasterClear(bool aMasterClear) {
	// Resetting the mainframe channels only
	if (mIoMasterClear && !aMasterClear) {
		// Resetting all IO channels
		for (auto &Channel : GetChannels()) if (Channel != nullptr) Channel->MainframeSideMasterClear();
	}
	mIoMasterClear = aMasterClear;
}

void Mainframe_c::CpuMasterClear(bool aMasterClear) {
	if (mCpuMasterClear != aMasterClear) {
		if (aMasterClear) {
			mLogger << setloglevel(LogLevel_IoActivity) << "setting CPU Master Clear ON" << std::endl;
			for(size_t i=0;i<GetCpuCnt();++i) {
				mCpus[i]->MasterClear();
			}
		}
		else {
			mLogger << setloglevel(LogLevel_IoActivity) << "setting CPU Master Clear OFF" << std::endl;
			CRAY_ASSERT(mStartupCpuIdx < mCpus.size());
			if (!mMemoryPokesProcessed) {
				// Process memory pokes only once, when the first main CPU gets out of reset
				for(size_t i=0;i<mMemoryPokes.size();++i) {
					if (mMemoryPokes[i].IsParcelPoke) {
						mLogger << setloglevel(LogLevel_Dump) << "Overriding address " << InstAddr(mMemoryPokes[i].Addr) << " with value " << HexPrinter(CParcel_t(mMemoryPokes[i].Value)) << std::endl;
						MemWriteNoWatchpointByType<CParcel_t>(mMemoryPokes[i].Addr, SwapBytes(CParcel_t(mMemoryPokes[i].Value)));
					} else {
						mLogger << setloglevel(LogLevel_Dump) << "Overriding address " << Addr(mMemoryPokes[i].Addr) << " with value " << HexPrinter(mMemoryPokes[i].Value) << std::endl;
						MemWriteNoWatchpointByType<CInt_t>(mMemoryPokes[i].Addr, SwapBytes(mMemoryPokes[i].Value));
					}
				}
				mMemoryPokesProcessed = true;
			}
			mCpus[mStartupCpuIdx]->DeadStart();
		}
	}
	mCpuMasterClear = aMasterClear;
}

void Mainframe_c::RegisterCommands(CommandHooks_t &aHooks) {
	class ProcList_c : public CmdFactoryBase_i {
	public:
		explicit ProcList_c(Mainframe_c &aParent) : mParent(aParent) {}
		virtual bool ParseAndExec(TokenStream_t::const_iterator aBegin, TokenStream_t::const_iterator aEnd) override {
			bool DoHelp = false;
			if (aBegin->mValue == "help") {
				DoHelp = true;
				++aBegin; if (aBegin == aEnd) return false;
			}
			if (aBegin->mValue != "ps") return false;
			++aBegin; if (aBegin == aEnd) return false;
			if (DoHelp) {
				std::cout << "ps" << std::endl;
				std::cout << "    Lists running processes" << std::endl;
				return true;
			}
			else {
				++aBegin;
				if (aBegin != aEnd && aBegin->mTokenId != TokenTypes_e::EndOfLine) return false;

				// TODO: add warning about existing file
				try {
					if (mParent.mProcListParser != nullptr) {
						std::cout << mParent.mProcListParser->Dump() << std::endl;
					} else {
						std::cout << "Can't do ps: did you forget to set ProcTableBase or ProcTableLength?" << std::endl;
					}
				}
				catch (std::exception &Ex) {
					std::cout << "ps failed" << std::endl;
					std::cout << Ex.what() << std::endl;
				}
				return true;
			}
		}
		virtual std::string GetCommandName() const override { return "ps"; }
		virtual std::string GetDeviceName() const override { return "mainframe"; }
	protected:
		Mainframe_c &mParent;
	};
	class LoadUseHist_c : public CmdFactoryBase_i {
	public:
		explicit LoadUseHist_c(Mainframe_c &aParent) : mParent(aParent) {}
		virtual bool ParseAndExec(TokenStream_t::const_iterator aBegin, TokenStream_t::const_iterator aEnd) override {
			bool DoHelp = false;
			if (aBegin->mValue == "help") {
				DoHelp = true;
				++aBegin; if (aBegin == aEnd) return false;
			}
			if (aBegin->mValue != "loadusehist") return false;
			++aBegin; if (aBegin == aEnd) return false;
			if (DoHelp) {
				std::cout << "loadusehist" << std::endl;
				std::cout << "    Reports load-to-use histogram" << std::endl;
				return true;
			}
			else {
				++aBegin;
				if (aBegin != aEnd && aBegin->mTokenId != TokenTypes_e::EndOfLine) return false;

				try {
					std::cout << mParent.ReportHist() << std::endl;
				}
				catch (std::exception &Ex) {
					std::cout << "loadusehist failed" << std::endl;
					std::cout << Ex.what() << std::endl;
				}
				return true;
			}
		}
		virtual std::string GetCommandName() const override { return "loadusehist"; }
		virtual std::string GetDeviceName() const override { return "mainframe"; }
	protected:
		Mainframe_c & mParent;
	};
	class BBHist_c : public CmdFactoryBase_i {
	public:
		explicit BBHist_c(Mainframe_c &aParent) : mParent(aParent) {}
		virtual bool ParseAndExec(TokenStream_t::const_iterator aBegin, TokenStream_t::const_iterator aEnd) override {
			bool DoHelp = false;
			if (aBegin->mValue == "help") {
				DoHelp = true;
				++aBegin; if (aBegin == aEnd) return false;
			}
			if (aBegin->mValue != "bbhist") return false;
			++aBegin; if (aBegin == aEnd) return false;
			if (DoHelp) {
				std::cout << "bbhist" << std::endl;
				std::cout << "    Reports basic-block histogram" << std::endl;
				return true;
			}
			else {
				++aBegin;
				if (aBegin != aEnd && aBegin->mTokenId != TokenTypes_e::EndOfLine) return false;

				try {
					std::cout << mParent.ReportBBHist() << std::endl;
				}
				catch (std::exception &Ex) {
					std::cout << "bbhist failed" << std::endl;
					std::cout << Ex.what() << std::endl;
				}
				return true;
			}
		}
		virtual std::string GetCommandName() const override { return "bbhist"; }
		virtual std::string GetDeviceName() const override { return "mainframe"; }
	protected:
		Mainframe_c & mParent;
	};


	if (mOsType == OsTypes_e::UNICOS) {
		aHooks.emplace_back(std::make_unique<ProcList_c>(*this));
#ifdef COLLECT_PERF
		aHooks.emplace_back(std::make_unique<LoadUseHist_c>(*this));
		aHooks.emplace_back(std::make_unique<BBHist_c>(*this));
#endif
	}
}

std::string Mainframe_c::ReportHist() const {
	std::stringstream Strm;
#ifdef COLLECT_PERF
	std::array<uint64_t, Cpu_c::mHistSize> Hist;
	std::fill(Hist.begin(), Hist.end(), 0);
	uint64_t CycleCount = 0;

	for (auto &Cpu : mCpus) {
		if (!Cpu->IsInReset()) {
			std::array<uint64_t, Cpu_c::mHistSize> CurrentHist = Cpu->GetLoadUseHist();
			for (size_t i = 0; i < CurrentHist.size(); ++i) {
				CycleCount += CurrentHist[i];
				Hist[i] += CurrentHist[i];
			}
		}
	}
	for (size_t i = 0; i < Hist.size(); ++i) {
		Strm << DecPrinter(i, 3) << ": " << DecPrinter(int(double(Hist[i]) / double(CycleCount) * 100.0), 3) << "%" << std::endl;
	}
#else
	Strm << "This build doesn't have histogram functions enabled." << std::endl;
#endif
	return Strm.str();
}

std::string Mainframe_c::ReportBBHist() const {
	std::stringstream Strm;
#ifdef COLLECT_PERF
	std::array<uint64_t, Cpu_c::mHistSize> BBHist;
	std::fill(BBHist.begin(), BBHist.end(), 0);
	uint64_t JumpCount = 0;
	uint64_t JumpsTaken = 0;
	uint64_t JumpsNotTaken = 0;

	for (auto &Cpu : mCpus) {
		if (!Cpu->IsInReset()) {
			std::array<uint64_t, Cpu_c::mHistSize> CurrentHist = Cpu->GetBBHist();
			for (size_t i = 0; i < CurrentHist.size(); ++i) {
				JumpCount += CurrentHist[i];
				BBHist[i] += CurrentHist[i];
			}
			JumpsTaken += Cpu->GetJumpsTaken();
			JumpsNotTaken += Cpu->GetJumpsNotTaken();
		}
	}
	for (size_t i = 0; i < BBHist.size(); ++i) {
		Strm << DecPrinter(i, 3) << ": " << DecPrinter(int(double(BBHist[i]) / double(JumpCount) * 100.0), 3) << "%" << std::endl;
	}
	Strm << std::endl;
	Strm << "Jumps taken:     " << DecPrinter(JumpsTaken) << " " << DecPrinter(int(double(JumpsTaken) / double(JumpCount) * 100.0), 3) << "%" << std::endl;
	Strm << "Jumps not taken: " << DecPrinter(JumpsNotTaken) << " " << DecPrinter(int(double(JumpsNotTaken) / double(JumpCount) * 100.0), 3) << "%" << std::endl;
	Strm << "Total jumps:     " << DecPrinter(JumpsTaken + JumpsNotTaken) << " " << DecPrinter(JumpCount) << std::endl;
#else
	Strm << "This build doesn't have histogram functions enabled." << std::endl;
#endif
	return Strm.str();
}



void Mainframe_c::DeadStart() {
	for (auto &Cpu : GetCpus()) Cpu.MasterClear();
	for (auto &Channel : GetChannels()) if (Channel != nullptr) Channel->MainframeSideMasterClear();
	for (auto &Iop: GetIopClusters()) Iop.MasterClear();

	for (auto &Iop : GetIopClusters()) Iop.DeadStart();
}

void Mainframe_c::ChannelTick() {
	++mTickCnt;
	for (auto &Channel : mTickedChannels) { CRAY_ASSERT(Channel != nullptr); Channel->ChannelTick(); }
}

void Mainframe_c::RouteChannelInterrupt() {
//	mLogger << setloglevel(LogLevel_IoActivity) << "**************** SETTING IOI INTERRUPT ***********" << std::endl;
	// We have an external interrupt, route it to the appropriate CPU
	// This procedure is detailed in HR-0097 2-15
	// 1. Route the interrupt to the CPU who has the Select External Interrupt mode set
	for (auto &Cpu : GetCpus()) {
		if (Cpu.IsSelectedForExternalInterrupts()) {
			Cpu.SetIoInterrupt();
			//mLogger << setloglevel(LogLevel_Event) << "**************** SETTING IOI INTERRUPT ***********" << std::endl;
			return;
		}
	}
	// 2. If no CPU has it set, route it to the CPU stalling on a test-and-set
	for (auto &Cpu : GetCpus()) {
		if (Cpu.IsTsBlocked()) {
			Cpu.SetIoInterrupt();
			//mLogger << setloglevel(LogLevel_Event) << "**************** SETTING IOI INTERRUPT ***********" << std::endl;
			return;
		}
	}
	// 3. If not CPU is in a test-end-set route to the CPU that las issued a CI,Aj instruction
	//    (if none, than simply route to the first CPU)
	mCpus[mLastCpuIssuedCI]->SetIoInterrupt();
	//mLogger << setloglevel(LogLevel_Event) << "**************** SETTING IOI INTERRUPT ***********" << std::endl;
}

void Mainframe_c::HandleDeadLock(uint16_t aCluster) {
	CRAY_ASSERT(aCluster != 0);
	for(size_t Cpu=0;Cpu<GetCpuCnt();++Cpu) {
		uint8_t Cluster = mCpus[Cpu]->GetCluster();
		if (Cluster == aCluster) {
			mCpus[Cpu]->SetDeadLockInterrupt();
		}
	}
}

void Mainframe_c::Dump(size_t aIdent) const {
	for(size_t Cpu=0;Cpu<mCpus.size();++Cpu) {
		if (mCpus[Cpu] == nullptr) continue;
		mLogger << setloglevel(LogLevel_Dump) << "Cpu" << DecPrinter(Cpu,0) << std::endl;
		mCpus[Cpu]->Dump(aIdent+1);
	}
	for(size_t Cluster=0;Cluster<mClusters.size();++Cluster) {
		if (mClusters[Cluster] == nullptr) continue;
		mLogger << setloglevel(LogLevel_Dump) << "Cluster" << DecPrinter(Cluster,0) << std::endl;
		mClusters[Cluster]->Dump(mLogger,aIdent+1);
	}
	for(size_t Channel=0;Channel<mChannels.size();++Channel) {
		if (mChannels[Channel] == nullptr) continue;
		mLogger << setloglevel(LogLevel_Dump) << "Channel" << DecPrinter(Channel + 8, 0) << std::endl;
		mChannels[Channel]->Dump(aIdent + 1);
	}
	for(size_t Iop=0;Iop<mIopClusters.size();++Iop) {
		if (mIopClusters[Iop] == nullptr) continue;
		mLogger << setloglevel(LogLevel_Dump) << "Iop" << DecPrinter(Iop,0) << std::endl;
		mIopClusters[Iop]->Dump(aIdent+1);
	}
	std::stringstream Strm;
	DumpSyscallStats(Strm, *this, mOsType);
	mLogger << setloglevel(LogLevel_Dump) << Strm.str();
}

void Mainframe_c::DumpHistory() {
	for (size_t Cpu = 0; Cpu<mCpus.size(); ++Cpu) {
		if (mCpus[Cpu] == nullptr) continue;
		auto LogLine = mCpus[Cpu]->GetLogger() << setloglevel(LogLevel_Dump);
		if (!LogLine.good()) {
			std::cout << "History dump disabled" << std::endl;
			return;
		}
		LogLine << "============================== CPU " << DecPrinter(Cpu) << " HISTORY ==============================" << std::endl;
		mCpus[Cpu]->DumpHistory();
		mCpus[Cpu]->ClearHistory();
		mCpus[Cpu]->GetLogger() << setloglevel(LogLevel_Dump) << "============================== END HISTORY ==========================" << std::endl;
	}
}

void Mainframe_c::DumpMemories() const {
	if (mMainMemoryDumpFileName.is_initialized()) {
		std::ofstream MemDump(mMainMemoryDumpFileName.get(), std::ios::out | std::ios::trunc | std::ios::binary);
		MemDump.write((char*)(&mMemory[0]),mMemory.size());
		MemDump.close();
	}
}

void Mainframe_c::CheckWatchPointsInternal(CAddr_t aAddr, bool aWrite, CInt_t aData) {
	auto WatchPoint = mWatchPoints.find(aAddr);
	if (WatchPoint != mWatchPoints.end()) {
		std::stringstream EventStr;
		if (aWrite) {
			EventStr << "Watchpoint at address " << Addr(aAddr) << " hit #:" << WatchPoint->second.mHitCnt << " written with: " << HexPrinter(SwapBytes(aData)) << " " << WatchPoint->second.mMessage << std::endl;
		} else {
			EventStr << "Watchpoint at address " << Addr(aAddr) << " hit #:" << WatchPoint->second.mHitCnt << " read " << WatchPoint->second.mMessage << std::endl;
		}
		WatchPoint->second.mHitCnt++;
		GetEventDispatcher().Fire(EventStr.str());
		std::cout << EventStr.str();
	}
}

// Host <---> Sim communication works on a high level, like a FIFO.
// The simulated target can send 32-bit values to the host, using SimToHost. On return, the low-order 32 bits are unchanged, and the MSB is set if the write DID NOT take place due to the FIFO being full.
CInt_t Mainframe_c::SimToHost(CInt_t aValue) {
	// For now, the only communication that's implemented is to terminate the simulation.
	if ((aValue & 0xffffffff) == 0x65786974) { // 'exit' in hex
		throw Terminate_x() << " requested from SIM";
	}
	return (CInt_t(1) << 63) | aValue;
}

// The simulated target can read 32-bit values to the host, using HostToSim. On return, the low-order 32 bits are from the FIFO, and the MSB is set if the read DID NOT take place due to the FIFO being empty.
CInt_t Mainframe_c::HostToSim() {
	return CInt_t(1) << 63;
}

CInt_t Mainframe_c::GetRealTimeClock() const {
	if (mUseHostRealTimeClock) {
		boost::timer::nanosecond_type DeltaTime = mRealTimeTimer.elapsed().wall - mRealTimeStart;
		CInt_t DeltaClocks = CInt_t(double(DeltaTime) / mSystemClockPeriod);
		CInt_t NewRTC = mRealTimeClock + DeltaClocks;
		if (NewRTC > mLastRealTimeReading + mRealTimeClockChunkLimit) {
			mLastRealTimeReading = NewRTC;
			return NewRTC;
		} else {
			NewRTC = mLastRealTimeReading + mRealTimeClockIncrement;
			mLastRealTimeReading = NewRTC;
			return NewRTC;
		}
	} else {
		return mRealTimeClock;
	}
}
