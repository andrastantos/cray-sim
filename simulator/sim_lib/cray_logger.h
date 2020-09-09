#ifndef __CRAY_LOGGER_H__
#define __CRAY_LOGGER_H__

#include "utils.h"
#include "logger.h"
#include "config_file.h"

/////////////////////////////////////////////////////////////////////
// Logger infrastrcture

class TimeStampGenerator_i {
public:
	virtual ~TimeStampGenerator_i() {}
	virtual uint64_t GetTimeStamp() const = 0;
	virtual uint64_t GetResolution() const = 0;
};

enum LogLevels_e {
	LogLevel_All = 0,
	LogLevel_Debug,
	LogLevel_SideEffects,
	LogLevel_InstructionTrace,
	LogLevel_Event,
	LogLevel_IoTrace,
	LogLevel_IoActivity,
	LogLevel_Communication,
	LogLevel_Syscall,
	LogLevel_EventFire,
	LogLevel_Interrupt,
	LogLevel_Warning,
	LogLevel_Error,
	LogLevel_Dump = 0xffffffff,
	LogLevel_None = 0xffffffff,
	LogLevel_Always = LogLevel_None
};

inline LogLevels_e FromString_LogLevels_e(const std::string &aStr) {
	if (aStr == "All") return LogLevel_All;
	if (aStr == "Debug") return LogLevel_Debug;
	if (aStr == "SideEffects") return LogLevel_SideEffects;
	if (aStr == "InstructionTrace") return LogLevel_InstructionTrace;
	if (aStr == "Event") return LogLevel_Event;
	if (aStr == "IoActivity") return LogLevel_IoActivity;
	if (aStr == "Communication") return LogLevel_Communication;
	if (aStr == "EventFire") return LogLevel_EventFire;
	if (aStr == "Syscall") return LogLevel_Syscall;
	if (aStr == "Interrupt") return LogLevel_Interrupt;
	if (aStr == "IoTrace") return LogLevel_IoTrace;
	if (aStr == "Warning") return LogLevel_Warning;
	if (aStr == "Error") return LogLevel_Error;
	if (aStr == "None") return LogLevel_None;
	auto RawLevel = IntTranslator_c<size_t>().get_value(aStr);
	if (!RawLevel.is_initialized()) throw Generic_x() << "Invalid log level string: " << aStr;
	return (LogLevels_e)RawLevel.get();
}

inline std::string GetLogLevels() {
	std::string RetVal;
	RetVal.append("All");
	RetVal.append(" Debug");
	RetVal.append(" SideEffects");
	RetVal.append(" InstructionTrace");
	RetVal.append(" Event");
	RetVal.append(" IoActivity");
	RetVal.append(" Communication");
	RetVal.append(" Syscall");
	RetVal.append(" EventFire");
	RetVal.append(" Interrupt");
	RetVal.append(" IoTrace");
	RetVal.append(" Warning");
	RetVal.append(" Error");
	RetVal.append(" None");
	return RetVal;
}

static std::string ModuleNameFormatter(const char *aModuleName, size_t aInstance, size_t aBase = 10) {
	std::stringstream Str;
	Str << aModuleName;
	switch (aBase) {
	case 16: Str << std::hex; break;
	case 10: Str << std::dec; break;
	case 8:  Str << std::oct; break;
	default:
		CRAY_ASSERT(false);
	}
	Str << aInstance;
	return Str.str();
}

class CLogger_c : public StreamLogger_c {
private:
	CLogger_c(): StreamLogger_c(nullptr), mIndentCnt(0), mTimeStampGenerator(nullptr), mLocalTimeStampGenerator(false) {}
	CLogger_c(const CLogger_c &&aOld) :
		mIndentCnt(aOld.mIndentCnt),
		mTimeStampGenerator(aOld.mTimeStampGenerator),
		mLocalTimeStampGenerator(aOld.mLocalTimeStampGenerator)
	{}
public:
/*	explicit CLogger_c(const char *aModuleName): StreamLogger_c(aModuleName), mIndentCnt(0), mTimeStampGenerator(nullptr), mLocalTimeStampGenerator(false) {}
	CLogger_c(const char *aModuleName, size_t aInstance, size_t aBase=10): StreamLogger_c(ModuleNameFormatter(aModuleName,aInstance,aBase).c_str()), mIndentCnt(0), mTimeStampGenerator(nullptr), mLocalTimeStampGenerator(false) {}
	CLogger_c(std::ostream &aStrm, const char *aModuleName): StreamLogger_c(aStrm, aModuleName), mIndentCnt(0), mTimeStampGenerator(nullptr), mLocalTimeStampGenerator(false) {}
	CLogger_c(std::ostream &aStrm, const char *aModuleName, size_t aInstance, size_t aBase=10): StreamLogger_c(aStrm, ModuleNameFormatter(aModuleName,aInstance,aBase).c_str()), mIndentCnt(0), mTimeStampGenerator(nullptr), mLocalTimeStampGenerator(false) {}*/
	explicit CLogger_c(const Configuration_c &aConfig) : StreamLogger_c(nullptr), mIndentCnt(0), mTimeStampGenerator(nullptr), mLocalTimeStampGenerator(false) { ReadConfig(aConfig); }
	CLogger_c(const Configuration_c &aConfig, const char *aModuleName) : StreamLogger_c(aModuleName), mIndentCnt(0), mTimeStampGenerator(nullptr), mLocalTimeStampGenerator(false) { ReadConfig(aConfig); }
	CLogger_c(const Configuration_c &aConfig, const char *aModuleName, size_t aInstance, size_t aBase = 10) : StreamLogger_c(ModuleNameFormatter(aModuleName, aInstance, aBase).c_str()), mIndentCnt(0), mTimeStampGenerator(nullptr), mLocalTimeStampGenerator(false) { ReadConfig(aConfig); }
	CLogger_c(const Configuration_c &aConfig, std::ostream &aStrm, const char *aModuleName) : StreamLogger_c(aStrm, aModuleName), mIndentCnt(0), mTimeStampGenerator(nullptr), mLocalTimeStampGenerator(false) { ReadConfig(aConfig); }
	CLogger_c(const Configuration_c &aConfig, std::ostream &aStrm, const char *aModuleName, size_t aInstance, size_t aBase = 10) : StreamLogger_c(aStrm, ModuleNameFormatter(aModuleName, aInstance, aBase).c_str()), mIndentCnt(0), mTimeStampGenerator(nullptr), mLocalTimeStampGenerator(false) { ReadConfig(aConfig); }
	~CLogger_c() {}
	void ReadConfig(const Configuration_c &aConfig) {
		boost::optional<std::string> DefaultLogLevel = aConfig.get_optional<std::string>("DefaultLogLevel");
		if (DefaultLogLevel.is_initialized()) {
			SetDisplayLogLevel(FromString_LogLevels_e(DefaultLogLevel.get()));
		}
	}
	void SetTimeStampGenerator(const TimeStampGenerator_i *aTimeStampGenerator) {
		mTimeStampGenerator = aTimeStampGenerator;
		mLocalTimeStampGenerator = true;
		NotifyChildren();
	}
	void UnsetTimeStampGenerator() {
		mTimeStampGenerator = nullptr;
		mLocalTimeStampGenerator = false;
		NotifyChildren();
	}
	virtual std::string GetTimeStamp() override {
		if (mTimeStampGenerator != nullptr) {
			uint64_t TimeStamp = mTimeStampGenerator->GetTimeStamp();
			std::stringstream TimeStampStr;
/*			uint64_t ClockRate = mTimeStampGenerator->GetResolution();
			uint32_t Seconds = (uint32_t)(TimeStamp / ClockRate);
			uint32_t Ticks = (uint32_t)(TimeStamp % ClockRate);
			uint32_t Minutes = Seconds / 60;
			Seconds %= 60;
			uint32_t Hours = Minutes / 60;
			Minutes %= 60;
			TimeStampStr << setw(2) << setfill('0') << Hours << ':' << setw(2) << setfill('0') << Minutes << ':' << setw(2) << setfill('0') << Seconds << '.' << setw(8) << setfill('0') << Ticks << ' ';*/
			TimeStampStr << DecPrinter(TimeStamp,8);
			return TimeStampStr.str() + ' ';
		} else {
			return StreamLogger_c::GetTimeStamp();
		}
	}
	virtual std::string GetHeaderDelimiter() override {
		std::string RetVal = ":";
		for(int i=0;i<mIndentCnt;++i) RetVal += '\t';
		RetVal += " ";
		return RetVal;
	}
	void Indent(int aCnt = 1) { mIndentCnt += aCnt; if (mIndentCnt < 0) mIndentCnt = 0; }
	void UnIndent(int aCnt = 1) { mIndentCnt -= aCnt; if (mIndentCnt < 0) mIndentCnt = 0; }
protected:
	const TimeStampGenerator_i *GetLocalTimeStampGenerator() const { return mTimeStampGenerator; }
	const TimeStampGenerator_i *GetInheritedTimeStampGenerator() const {
		CLogger_c *Parent = (CLogger_c*)(GetParent());
		if (Parent == nullptr) return nullptr;
		return Parent->GetTimeStampGenerator();
	}
	const TimeStampGenerator_i *GetTimeStampGenerator() const {
		if (mLocalTimeStampGenerator) return GetLocalTimeStampGenerator();
		return GetInheritedTimeStampGenerator();
	}
	virtual void ParentChanged() override {
		if (!mLocalTimeStampGenerator) mTimeStampGenerator = GetInheritedTimeStampGenerator();
		StreamLogger_c::ParentChanged();
	}

	int mIndentCnt;
	const TimeStampGenerator_i *mTimeStampGenerator;
	bool mLocalTimeStampGenerator;

public:
	static CLogger_c mRootLogger;
};

extern CLogger_c &RootLogger;

#endif // __CRAY_LOGGER_H__
