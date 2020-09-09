#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <ostream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <atomic>
#include <map>
#include <memory>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <boost/thread.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/thread/tss.hpp>
#include <boost/date_time.hpp>
#include "utils.h"

class LogLine_c;

class setloglevel {
public:
	explicit setloglevel(size_t aLogLevel) : mLogLevel(aLogLevel) {}
	size_t mLogLevel;
};

FORCEINLINE const class LogLine_c operator << (class LoggerBase_c &aLogger, const setloglevel &aManipulator) FORCEINLINEATTR;

#ifndef CRAY_TURBO
class LogLine_c {
protected:
	LogLine_c(class LoggerBase_c &aParent, bool aMakeBad): mParent(aParent), mBad(aMakeBad) {}
public:
	~LogLine_c() {
		if (good()) Flush();
	}

	bool good() const { return !mBad; }
protected:
	void Flush();

	friend const LogLine_c operator << (LoggerBase_c &aLogger, const setloglevel &aManipulator);
	template <typename tElement> friend const LogLine_c &operator << (const LogLine_c &aLogger, const tElement &aElement);
	friend const LogLine_c &operator << (const LogLine_c &aLogger, std::basic_ostream<char, std::char_traits<char> >& (*aFormatterFn)(std::basic_ostream<char, std::char_traits<char> >&));

	friend class LoggerBase_c;

	class LoggerBase_c &mParent;
	bool mBad;
};
#else // CRAY_TURBO
class LogLine_c {
protected:
	LogLine_c() {}
public:
	~LogLine_c() {}

	bool good() const { return false; }
protected:
	friend const LogLine_c operator << (LoggerBase_c &aLogger, const setloglevel &aManipulator);
	template <typename tElement> friend const LogLine_c &operator << (const LogLine_c &aLogger, const tElement &aElement);
	friend const LogLine_c &operator << (const LogLine_c &aLogger, std::basic_ostream<char, std::char_traits<char> >& (*aFormatterFn)(std::basic_ostream<char, std::char_traits<char> >&));

	friend class LoggerBase_c;
};
#endif // CRAY_TURBO

class LoggerBase_c {
public:
	LoggerBase_c():
		mDisplayLogLevel(0),
		mSelfDisplayLogLevel(0),
		mSelfDisplayLogLevelValid(false),
		mInheritedDisplayLogLevel(0),
		mParent(nullptr)
	{}
	LoggerBase_c(const LoggerBase_c &&aOld) :
		mDisplayLogLevel(aOld.mDisplayLogLevel.load()),
		mSelfDisplayLogLevel(aOld.mSelfDisplayLogLevel),
		mSelfDisplayLogLevelValid(aOld.mSelfDisplayLogLevelValid),
		mInheritedDisplayLogLevel(aOld.mInheritedDisplayLogLevel),
		mChildren(std::move(aOld.mChildren)),
		mParent(aOld.mParent)
	{}
	virtual ~LoggerBase_c();

	friend const LogLine_c operator << (LoggerBase_c &aLogger, const setloglevel &aManipulator);
	friend LogLine_c;

	void SetDisplayLogLevel(size_t aDisplayLogLevel) {
		Guard_t Guard(mHierarchyLock);
		mDisplayLogLevel = aDisplayLogLevel;
		mSelfDisplayLogLevel = aDisplayLogLevel;
		mSelfDisplayLogLevelValid = true;
		// Propagate log level to all children
		for (auto &Child : mChildren) {
			Child->SetInheritedLogLevel(mDisplayLogLevel);
		}
	}
	void PushDisplayLogLevel(size_t aDisplayLogLevel) {
		Guard_t Guard(mHierarchyLock);
		mDisplayLogLevelStack.push_back(DisplayLogLevelStackElement_s(mDisplayLogLevel, mSelfDisplayLogLevel, mSelfDisplayLogLevelValid));
		SetDisplayLogLevel(aDisplayLogLevel);
	}
	void PopDisplayLogLevel() {
		Guard_t Guard(mHierarchyLock);
		if (mDisplayLogLevelStack.empty()) return;
		DisplayLogLevelStackElement_s LastValue = mDisplayLogLevelStack.back();
		mDisplayLogLevelStack.pop_back();
		mDisplayLogLevel = LastValue.DisplayLogLevel;
		mSelfDisplayLogLevel = LastValue.SelfDisplayLogLevel;
		mSelfDisplayLogLevelValid = LastValue.SelfDisplayLogLevelValid;
		// Propagate log level to all children
		for (auto &Child : mChildren) {
			Child->SetInheritedLogLevel(mDisplayLogLevel);
		}
	}
	void NotifyChildren() const {
		Guard_t Guard(mHierarchyLock);
		for (auto &Child : mChildren) {
			Child->ParentChanged();
		}
	}
	void UnsetDisplayLogLevel() {
		Guard_t Guard(mHierarchyLock);
		mSelfDisplayLogLevelValid = false;
		mDisplayLogLevel = mInheritedDisplayLogLevel;
		// Propagate log level to all children
		for (auto &Child : mChildren) {
			Child->SetInheritedLogLevel(mDisplayLogLevel);
		}
	}

	void SetParent(LoggerBase_c &aParent) { SetParent(&aParent); }
	void SetParent(LoggerBase_c *aParent) {
		Guard_t Guard(mHierarchyLock);
		if (mParent != nullptr) {
			// Remove from old parent
			mParent->RemoveChild(*this);
		}
		if (aParent != nullptr) {
			aParent->AddChild(*this);
		}
		mParent = aParent;
		if (aParent != nullptr) {
			SetInheritedLogLevel(mParent->mDisplayLogLevel);
		}
		ParentChanged();
	}
	virtual std::string GetHeader() = 0;
	virtual std::string GetHeaderDelimiter() { return ": "; }
	virtual std::ostream *GetStream() = 0;

	const std::unordered_set<LoggerBase_c *> &GetChildren() const { return mChildren; }
protected:
	bool WouldLog(size_t aLogLevel) {
		return (aLogLevel >= mDisplayLogLevel);
	}
	LoggerBase_c *GetParent() const { return mParent; }
	virtual void ParentChanged() { NotifyChildren(); }
	virtual std::string GetTimeStamp() { return ""; }
//	mutable std::recursive_mutex mHierarchyLock;
//	typedef std::lock_guard<std::recursive_mutex> Guard_t;
	mutable bool mHierarchyLock;
	typedef bool Guard_t;
private:
	bool IsValid() { return (GetStream() != nullptr && GetStream()->good()); }
	void SetInheritedLogLevel(size_t aLogLevel) {
		Guard_t Guard(mHierarchyLock);
		if (!mSelfDisplayLogLevelValid) mDisplayLogLevel = aLogLevel;
		mInheritedDisplayLogLevel = aLogLevel;
		// Propagate log level to all children
		for(auto &Child: mChildren) {
			Child->SetInheritedLogLevel(aLogLevel);
		}
	}
	void RemoveChild(const LoggerBase_c &aChild) {
		Guard_t Guard(mHierarchyLock);
		mChildren.erase((LoggerBase_c *const)(&aChild));
	}
	void AddChild(LoggerBase_c &aChild) {
		Guard_t Guard(mHierarchyLock);
		auto ChildIt = mChildren.find(&aChild);
		if (ChildIt != mChildren.end()) return;
		mChildren.insert(&aChild);
	}

	std::atomic<size_t> mDisplayLogLevel; // Active log level below which messages are swallowed
	size_t mSelfDisplayLogLevel; // Display log level set on this logger instance
	bool mSelfDisplayLogLevelValid; // true if a log level was set on this instance
	size_t mInheritedDisplayLogLevel; // inherited display log level through parent path
	std::unordered_set<LoggerBase_c *>mChildren;
	LoggerBase_c *mParent;

	struct DisplayLogLevelStackElement_s {
		DisplayLogLevelStackElement_s(size_t aDisplayLogLevel, size_t aSelfDisplayLogLevel, bool aSelfDisplayLogLevelValid):
			DisplayLogLevel(aDisplayLogLevel),
			SelfDisplayLogLevel(aSelfDisplayLogLevel),
			SelfDisplayLogLevelValid(aSelfDisplayLogLevelValid)
		{}
		size_t DisplayLogLevel;
		size_t SelfDisplayLogLevel;
		bool SelfDisplayLogLevelValid;
	};
	std::vector<DisplayLogLevelStackElement_s> mDisplayLogLevelStack;

	class ThreadContext_c: public std::stringstream {
	public:
		ThreadContext_c() {}
	};

	ThreadContext_c &GetContext() {
		if (smThreadContext.get() == nullptr) {
			// Make sure we don't leak memory in case 'reset' throws...
			std::unique_ptr<ThreadContext_c> Context = std::make_unique<ThreadContext_c>();
			smThreadContext.reset(Context.get());
			Context.release();
		}
		return *smThreadContext.get();
	}

	static boost::thread_specific_ptr<ThreadContext_c> smThreadContext;

	template <typename tElement> friend const LogLine_c &operator << (const LogLine_c &aLogger, const tElement &aElement);
	friend const LogLine_c &operator << (const LogLine_c &aLogger, std::basic_ostream<char, std::char_traits<char> >& (*aFormatterFn)(std::basic_ostream<char, std::char_traits<char> >&));
};

#ifndef CRAY_TURBO
FORCEINLINE const LogLine_c operator << (LoggerBase_c &aLogger, const setloglevel &aManipulator) {
	return LogLine_c(aLogger, !aLogger.WouldLog(aManipulator.mLogLevel));
}
#else // CRAY_TURBO
inline const LogLine_c operator << (LoggerBase_c &aLogger, const setloglevel &aManipulator) {
	return LogLine_c();
}
#endif // CRAY_TURBO

class SimpleLogger_c: public LoggerBase_c {
public:
	SimpleLogger_c() {}
	explicit SimpleLogger_c(const char *aHeader): mHeader(aHeader==nullptr?"":aHeader), mCombinedHeader(aHeader==nullptr?"":aHeader) {}
	SimpleLogger_c(const SimpleLogger_c &&aOld) :
		LoggerBase_c(std::move(aOld)),
		mHeader(std::move(aOld.mHeader)),
		mCombinedHeader(std::move(aOld.mCombinedHeader))
	{}

	virtual std::string GetHeader() { return mCombinedHeader; }

	void SetHeader(const char *aHeader) {
		mHeader = aHeader;
		UpdateCombinedHeader();
	}
protected:
	std::string mHeader;
	std::string mCombinedHeader;

	void UpdateCombinedHeader() {
		if (GetParent() != nullptr && GetParent()->GetHeader() != "") {
			mCombinedHeader = GetParent()->GetHeader() + "." + mHeader;
		}
		else {
			mCombinedHeader = mHeader;
		}
	}

	virtual void ParentChanged() {
		UpdateCombinedHeader();
		LoggerBase_c::ParentChanged();
	}
};

class StreamLogger_c: public SimpleLogger_c {
public:
	StreamLogger_c(): mLog(nullptr) {}
	explicit StreamLogger_c(const char *aHeader): SimpleLogger_c(aHeader), mLog(nullptr) {}
	StreamLogger_c(std::ostream &aStrm, const char *aHeader): SimpleLogger_c(aHeader), mLog(&aStrm) {}
	StreamLogger_c(const StreamLogger_c &&aOld) : 
		SimpleLogger_c(std::move(aOld)),
		mLog(std::move(aOld.mLog))
	{}
	virtual ~StreamLogger_c() {}

	void SetStream(std::ostream &aStrm) {
		mLog = &aStrm;
		NotifyChildren();
	}
	virtual std::ostream *GetStream() { return mLog; }
protected:
	virtual void ParentChanged() {
		LoggerBase_c *Parent = GetParent();
		if (Parent != nullptr) {
			mLog = Parent->GetStream();
		}
		SimpleLogger_c::ParentChanged();
	}
	std::ostream *mLog;
};

boost::posix_time::time_duration CurrentTime();

#ifndef CRAY_TURBO
inline const LogLine_c &operator << (const LogLine_c &aLogger, std::basic_ostream<char, std::char_traits<char> >& (*aFormatterFn)(std::basic_ostream<char, std::char_traits<char> >&)) {
	if (aLogger.good()) aFormatterFn(aLogger.mParent.GetContext());
	return aLogger;
}
	
template <typename tElement> inline const LogLine_c &operator << (const LogLine_c &aLogger, const tElement &aElement) {
	if (aLogger.good()) aLogger.mParent.GetContext() << aElement;
	return aLogger;
}
#else // CRAY_TURBO

inline const LogLine_c &operator << (const LogLine_c &aLogger, std::basic_ostream<char, std::char_traits<char> >& (*aFormatterFn)(std::basic_ostream<char, std::char_traits<char> >&)) {
	return aLogger;
}

template <typename tElement> inline const LogLine_c &operator << (const LogLine_c &aLogger, const tElement &aElement) {
	return aLogger;
}

#endif // CRAY_TURBO

#endif // __LOGGER_H__
