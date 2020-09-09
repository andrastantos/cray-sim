#include "logger.h"

#ifndef CRAY_TURBO
void LogLine_c::Flush() {
	if (!good()) return;
	// Put things into the logger on distruction
	std::string Buffer = mParent.GetContext().str();

	if (Buffer.length() == 0) return; // Nothing to log, nothing to do...

	std::string Header = mParent.GetTimeStamp() + mParent.GetHeader() + mParent.GetHeaderDelimiter();

	std::string NewLineHeader = "\n" + Header;

	bool HadEndl = false;
	if (*(--Buffer.end()) == '\n') {
		Buffer.erase(--Buffer.end());
		HadEndl = true;
	}
	boost::replace_all(Buffer, "\n", NewLineHeader);
	std::ostream *Strm = mParent.GetStream();
	if (Strm != nullptr) {
		*Strm << Header;
		*Strm << Buffer;
		if (HadEndl) *Strm << std::endl;
	}
	mParent.GetContext().str(std::string());
}
#endif // CRAY_TURBO

LoggerBase_c::~LoggerBase_c() {
	// Let's get removed from the parent
	if (mParent != nullptr) mParent->RemoveChild(*this);
	// Re-parent all children to our parent (might be nullptr, but that's fine)
	// NOTE: SetParent will call our 'RemoveChild' method, which in turn will modify the list, so we can't use an iterator here.
	//       At the same time, the list should shrink by each call, so just let's loop until it's empty
	while (!mChildren.empty()) {
		(*mChildren.begin())->SetParent(mParent);
	}
	mParent = nullptr;
}

static class StartTime_c {
	bool mValid;
	boost::posix_time::ptime mStart;
	std::recursive_mutex mLock;
	typedef std::lock_guard<std::recursive_mutex> Guard_t;
//	bool mLock;
//	typedef bool Guard_t;
public:
	boost::posix_time::ptime StartTime() {
		if (!mValid) {
			Guard_t Guard(mLock);
			if (!mValid) {
				mStart = boost::posix_time::microsec_clock::local_time();
				mValid = true;
			}
		}
		return mStart;
	}
	boost::posix_time::time_duration SinceStart() {
		boost::posix_time::ptime Now = boost::posix_time::microsec_clock::local_time();
		return Now - StartTime();
	}
} StartTime;

boost::posix_time::time_duration CurrentTime() {
	return StartTime.SinceStart();
}

boost::thread_specific_ptr<LoggerBase_c::ThreadContext_c> LoggerBase_c::smThreadContext;


