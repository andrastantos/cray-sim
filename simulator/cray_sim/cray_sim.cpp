#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <signal.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <sys/types.h> // For umask
#include <sys/stat.h>
#include <locale.h>
#endif
#include <boost/thread.hpp>
#include <boost/timer/timer.hpp>
#include <regex>
#include <boost/container/flat_set.hpp>

#include "utils.h"
#include "cray_cpu.h"
#include "cray_iop.h"
#include "curses_win.h"
#include "parser.h"
#include "commands.h"
#include "server.hpp"
#include "cray_softcpu.h"

#define PARTIAL_DEBUG

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

#if 0

class SyslogBuf_c: public std::basic_streambuf<char, std::char_traits<char>>
{
public:
	SyslogBuf_c(const std::string &aIdent, int aFacility, int aOption = LOG_PID);
	SetLogLevel(int aLogLevel) { mLogLevel = aLogLevel; }
protected:
	virtual int sync() overrid;
	virtual int overflow(int aChar) override;

	std::string mBuffer;
	int mLogLevel;
};


SyslogBuf_c::SyslogBuf_c(const std::string &aIdent, int aFacility, int aOption):
	mLogLevel(LOG_DEBUG)
{
	openlog(mIdent.c_str(), aOption, aFacility);
}

int SyslogBuf_c::sync() {
	if (mBuffer.length()) {
		syslog(mLogLevel, mBuffer.c_str());
		mBuffer.erase();
	}
	return 0;
}

int SyslogBuf_c::overflow(int aChar) {
	mBuffer += char(aChar);
	return aChar;
}


#endif

class daemon;
volatile AtomicBool Terminate(false);

void SigIntHandler(int aSignal) {
	Terminate = true;
}

static std::ofstream LogFile;

void BusyThread() {
	while (!Terminate);
}

void CpuThread(Mainframe_c &aMainframe, size_t aCpuIdx) {
	try {
		while (!Terminate) {
			aMainframe.GetCpu(aCpuIdx).Tick();
			if (aCpuIdx == 0) aMainframe.UpdateRealTimeClock();
		}
	}
	catch (std::exception &Ex) {
		RootLogger << setloglevel(LogLevel_Always) << "CPU" << DecPrinter(aCpuIdx) << "THREAD: " << "Can't write memory dump" << std::endl;
		RootLogger << setloglevel(LogLevel_Always) << Ex.what() << std::endl;
		std::cout << "Can't write memory dump" << std::endl;
		std::cout << Ex.what() << std::endl;
		Terminate = true;
	}
}

void AllCpuThread(Mainframe_c &aMainframe) {
	try {
		while (!Terminate) {
			for (size_t i = 0; i < aMainframe.GetCpuCnt(); ++i) {
				aMainframe.GetCpu(i).Tick();
			}
			aMainframe.UpdateRealTimeClock();
		}
	}
	catch (std::exception &Ex) {
		RootLogger << setloglevel(LogLevel_Always) << "CPUTHREAD: " << "Can't write memory dump" << std::endl;
		RootLogger << setloglevel(LogLevel_Always) << Ex.what() << std::endl;
		std::cout << "Can't write memory dump" << std::endl;
		std::cout << Ex.what() << std::endl;
		Terminate = true;
	}
}

void GetCpuStatus(StatusReport_c &aStatus, const Mainframe_c &aMainframe, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) {
	size_t CpuCnt = aMainframe.GetCpuCnt();
	for (size_t CpuIdx = 0; CpuIdx < CpuCnt; ++CpuIdx) {
		StatusReport_c CpuStatus;
		aMainframe.GetCpu(CpuIdx).GetStatus(CpuStatus, aElapsedTime, aLongFormat);
		if (!CpuStatus.empty()) {
			aStatus.put_child(aMainframe.GetCpu(CpuIdx).GetName(), CpuStatus);
		}
	}
	size_t IopCnt = aMainframe.GetIopClusterCnt();
	for (size_t IopIdx = 0; IopIdx < IopCnt; ++IopIdx) {
		StatusReport_c IopStatus;
		aMainframe.GetIopCluster(IopIdx).GetCpuStatus(IopStatus, aElapsedTime, aLongFormat);
		if (!IopStatus.empty()) {
			aStatus.put_child(aMainframe.GetIopCluster(IopIdx).GetLongName(), IopStatus);
		}
	}

}

void GetPeripheralStatus(StatusReport_c &aStatus, const Mainframe_c &aMainframe, PeripheralType_e aFilter, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) {
	size_t IopCnt = aMainframe.GetIopClusterCnt();
	for (size_t IopIdx = 0; IopIdx < IopCnt; ++IopIdx) {
		StatusReport_c IopStatus;
		aMainframe.GetIopCluster(IopIdx).GetPeripheralStatus(IopStatus, aFilter, aElapsedTime, aLongFormat);
		if (!IopStatus.empty()) {
			aStatus.put_child(aMainframe.GetIopCluster(IopIdx).GetLongName(), IopStatus);
		}
	}
}

std::string StatusIdent(size_t aLevel) {
	std::string RetVal;
	for (size_t i = 0; i < aLevel; ++i) RetVal += " ";
	return RetVal;
}

void DisplayStatus(const StatusReport_c &aStatus, std::ostream &aStrm, bool &aNeedEndl, size_t aLevel) {
	// This is a sub-tree, recurse into it
	for (const auto & Elem : aStatus) {
		if (Elem.second.empty()) { // Leaf node
			if (aLevel == 0) {
				if (aNeedEndl) aStrm << std::endl;
				if (Elem.first.length() != 0) aStrm << Elem.first << " - ";
				aNeedEndl = true;
			}
			else {
				if (Elem.first.length() != 0) aStrm << Elem.first << ": ";
				if (Elem.second.data().length() != 0) aStrm << Elem.second.data() << " ";
				aNeedEndl = true;
			}
		} else {
			if (aNeedEndl) aStrm << std::endl;
			aNeedEndl = true;
			aStrm << StatusIdent(aLevel) << Elem.first << " - ";
			DisplayStatus(Elem.second, aStrm, aNeedEndl, aLevel + 1);
			if (aNeedEndl) aStrm << std::endl;
			aNeedEndl = false;
		}
	}
}

void DisplayStatus(const StatusReport_c &aStatus, std::ostream &aStrm) {
	bool NeedEndl = false;
	DisplayStatus(aStatus, aStrm, NeedEndl, 0);
}

class NoReportException_x : public std::exception {};

void GetLoggerNames(const LoggerBase_c &aLogger, std::string &aNames) {
	for (auto &Logger : aLogger.GetChildren()) {
		if (!aNames.empty()) aNames.append(" ");
		aNames.append(Logger->GetHeader());
		GetLoggerNames(*Logger, aNames);
	}
}

std::string GetLoggerNames() {
	std::string Names;
	GetLoggerNames(RootLogger, Names);
	return Names;
}

LoggerBase_c *GetLogger(const LoggerBase_c &aLogger, std::string &aName) {
	for (auto &Logger : aLogger.GetChildren()) {
		if (Logger->GetHeader() == aName) return Logger;
		LoggerBase_c *RetVal = GetLogger(*Logger, aName);
		if (RetVal != nullptr) return RetVal;
	}
	return nullptr;
}

LoggerBase_c *GetLogger(std::string &aName) {
	return GetLogger(RootLogger, aName);
}


void ResizeWindows(
	int aConsoleHeight,
	Curses::Frame_c &aCpuStatusWindow,
	Curses::Frame_c &aIoStatusWindow,
	Curses::Frame_c &aCoutWindow,
	Curses::Session_c &aCursesSession,
	const Mainframe_c &aMainframe
) {
	try {
		aCursesSession.ResizeTerm();
	}
	catch (Generic_x &) {
	}

	int LeftWindowWidth = aCursesSession.GetMaxX() / 2;
	int RightWindowWidth = aCursesSession.GetMaxX() - LeftWindowWidth;
	if (LeftWindowWidth < 3 || RightWindowWidth < 3) return;
	int PeripheralWindowHeight = aCursesSession.GetMaxY() - aConsoleHeight;
	if (PeripheralWindowHeight < 3) {
		PeripheralWindowHeight = -1;
		aConsoleHeight = aCursesSession.GetMaxY();
		if (aConsoleHeight < 3) {
			return;
		}
	}
	if (PeripheralWindowHeight < 0) {
		aIoStatusWindow.Hide();
		aCpuStatusWindow.Hide();
	} else {
		aIoStatusWindow.Show();
		aIoStatusWindow.Resize(RightWindowWidth, PeripheralWindowHeight);
		aIoStatusWindow.MoveWindow(LeftWindowWidth, 0);
		aCpuStatusWindow.Show();
		aCpuStatusWindow.Resize(LeftWindowWidth, PeripheralWindowHeight);
		aCpuStatusWindow.MoveWindow(0, 0);
	}

	aCoutWindow.Resize(aCursesSession.GetMaxX(), aConsoleHeight);
	aCoutWindow.MoveWindow(0, aCursesSession.GetMaxY() - aConsoleHeight);

	Curses::Panel_c::Update();
}

void DumpMainframe(Mainframe_c &aMainframe) {
	RootLogger << setloglevel(LogLevel_Always) << "=====================================================================" << std::endl;
	RootLogger << setloglevel(LogLevel_Always) << "Mainframe state:" << std::endl;
	RootLogger << setloglevel(LogLevel_Always) << "=====================================================================" << std::endl;
	aMainframe.Dump();
	aMainframe.DumpHistory();
	aMainframe.DumpMemories();
}

class StatusContentHandler_c : public http::server::request_handler {
public:
	StatusContentHandler_c(const StatusContentHandler_c&) = delete;
	StatusContentHandler_c& operator=(const StatusContentHandler_c&) = delete;

	/// Construct with a directory containing files to be served.
	explicit StatusContentHandler_c(const Configuration_c &aConfig, const std::string& doc_root) : request_handler(doc_root) {
		for (auto Pattern : aConfig) {
			ResourcePattern_s ResourcePattern;
			ResourcePattern.Pattern = Pattern.first;
			std::string StrStatus = Pattern.second.data();
			if (StrStatus == "ok") {
				ResourcePattern.Status = http::server::reply::ok;
			} else if (StrStatus == "created") {
				ResourcePattern.Status = http::server::reply::created;
			} else if (StrStatus == "accepted") {
				ResourcePattern.Status = http::server::reply::accepted;
			} else if (StrStatus == "no_content") {
				ResourcePattern.Status = http::server::reply::no_content;
			} else if (StrStatus == "multiple_choices") {
				ResourcePattern.Status = http::server::reply::multiple_choices;
			} else if (StrStatus == "moved_permanently") {
				ResourcePattern.Status = http::server::reply::moved_permanently;
			} else if (StrStatus == "moved_temporarily") {
				ResourcePattern.Status = http::server::reply::moved_temporarily;
			} else if (StrStatus == "not_modified") {
				ResourcePattern.Status = http::server::reply::not_modified;
			} else if (StrStatus == "bad_request") {
				ResourcePattern.Status = http::server::reply::bad_request;
			} else if (StrStatus == "unauthorized") {
				ResourcePattern.Status = http::server::reply::unauthorized;
			} else if (StrStatus == "forbidden") {
				ResourcePattern.Status = http::server::reply::forbidden;
			} else if (StrStatus == "not_found") {
				ResourcePattern.Status = http::server::reply::not_found;
			} else if (StrStatus == "internal_server_error") {
				ResourcePattern.Status = http::server::reply::internal_server_error;
			} else if (StrStatus == "not_implemented") {
				ResourcePattern.Status = http::server::reply::not_implemented;
			} else if (StrStatus == "bad_gateway") {
				ResourcePattern.Status = http::server::reply::bad_gateway;
			} else if (StrStatus == "service_unavailable") {
				ResourcePattern.Status = http::server::reply::service_unavailable;
			} else {
				throw Generic_x() << "Unknwon pattern status: " << StrStatus;
			}
			mResourcePatterns.push_back(ResourcePattern);
		}
	}
	void UpdateStatus(const char *aStatusName, const StatusReport_c &aStatus) {
		mStatus[aStatusName] = ConvertStatus(aStatus);
	}
private:
	virtual http::server::reply::status_type PostProcessRequest(const http::server::request& aRequest, const std::string &aRequestPath) override {
		if (aRequestPath != "/index.html") std::cout << "Web server processing request: " << aRequestPath << std::endl;
		for (auto &ResourcePattern : mResourcePatterns) {
			if (std::regex_match(aRequestPath, ResourcePattern.Pattern)) return ResourcePattern.Status;
		}
		return http::server::reply::not_found;
	}
	virtual void PostProcessResponse(http::server::reply& aResponse) override {
		if (aResponse.headers["Content-Type"] == "text/html") {
			for (auto &Status : mStatus) {
				std::string Marker = "{";
				Marker.append(Status.first);
				Marker.append("}");
				aResponse.content = Replace(aResponse.content, Marker, Status.second);
			}
		}
	}

	void ConvertStatus(const StatusReport_c &aStatus, std::ostream &aStrm, bool &aNeedEndl, size_t aLevel) {
		// This is a sub-tree, recurse into it
		aStrm << StatusIdent(aLevel) << "<ul>" << std::endl;
		for (const auto & Elem : aStatus) {
			if (Elem.second.empty()) { // Leaf node
				if (aLevel == 0) {
					if (aNeedEndl) aStrm << std::endl << StatusIdent(aLevel) << " <li>";
					if (Elem.first.length() != 0) aStrm << Elem.first << " - ";
					aNeedEndl = true;
				}
				else {
					if (Elem.first.length() != 0) aStrm << Elem.first << ": ";
					if (Elem.second.data().length() != 0) aStrm << Elem.second.data() << " ";
					aNeedEndl = true;
				}
			}
			else {
				if (aNeedEndl) aStrm << std::endl;
				aNeedEndl = true;
				aStrm << StatusIdent(aLevel) << StatusIdent(aLevel) << " <li>" << Elem.first << " - ";
				ConvertStatus(Elem.second, aStrm, aNeedEndl, aLevel + 1);
				if (aNeedEndl) aStrm << std::endl;
				aNeedEndl = false;
			}
		}
		aStrm << StatusIdent(aLevel) << "</ul>" << std::endl;
	}

	std::string ConvertStatus(const StatusReport_c &aStatus) {
		std::stringstream Strm;
		bool NeedEndl = false;
		if (aStatus.empty()) return "&lt;none&gt;";
		ConvertStatus(aStatus, Strm, NeedEndl, 0);
		return Strm.str();
	}

	std::map<std::string, std::string> mStatus;

	struct ResourcePattern_s {
		std::regex Pattern;
		http::server::reply::status_type Status;
	};
	std::vector<ResourcePattern_s> mResourcePatterns;
};

class WebServerThread_c {
public:
	WebServerThread_c(const Configuration_c &aConfig):
		mHttpStatus(aConfig.get_child_safe("ResourcePatterns"), aConfig.get<std::string>("Docroot", "")),
		mServer(mHttpStatus),
		mValid(false)
	{
		if (aConfig.empty()) return;

		mValid = true;
		mServerPort = aConfig.get<std::string>("Port");
		mServerAddress = aConfig.get<std::string>("Address");

		mServer.setup(mServerAddress, mServerPort);
	}
	~WebServerThread_c() { Stop(); }
	void Start() {
		if (!mValid) return;
		mWebServerThread = std::make_unique<std::thread>(std::bind(&http::server::server::run, &mServer));
//		std::cout << "Web server started with ID: " << mWebServerThread->get_id() << " - " << (mWebServerThread->joinable() ? "joinable" : "NOT JOINABLE") << std::endl;
	}
	void Stop() {
		if (!mValid) return;
		mServer.stop();
		if (mWebServerThread != nullptr) {
			if (mWebServerThread->joinable()) {
				mWebServerThread->join();
			}
		}
		mWebServerThread = nullptr;
	}
	bool IsValid() const { return mValid; }
	void UpdateStatus(const char *aStatusName, const StatusReport_c &aStatus) {
		mHttpStatus.UpdateStatus(aStatusName, aStatus);
	}
protected:
	std::string mServerPort;
	std::string mServerAddress;

	StatusContentHandler_c mHttpStatus; // Member order is important: server need mHttpStatus constructed
	http::server::server mServer;
	std::unique_ptr<std::thread> mWebServerThread;
	bool mValid;
};

int PrintUsage(const char *aExecName, const char *ErrorStr = nullptr) {
	if (ErrorStr != nullptr) std::cout << "Error: " << ErrorStr << std::endl;
	std::cout << "Usage: " << aExecName << " [options] <config file name>" << std::endl;
	std::cout << std::endl;
	std::cout << "\t" << "Options:" << std::endl;
#if !defined(_WIN32)
	std::cout << "\t" << "-d:            Start the simulator in daemon mode" << std::endl;
	std::cout << "\t" << "-l <log file>: Redirect stdout to log file in daemon mode" << std::endl;
#endif
	std::cout << "\t" << "-a:            Disables auto-start of terminal applications" << std::endl;
	std::cout << "\t" << "-h:            Show this help screen" << std::endl;
	return 1;
}

void MulTest() {
	CFloat_t S5, S2, S3;
	S3.Value = 0x0000000005000000ull;
	S2.Value = 0x0000000037000000ull;
	S5 = S2 * S3;
	std::cout << "S5: " << HexPrinter(S5) << std::endl;
	S5 = S3 * S2;
	std::cout << "S5: " << HexPrinter(S5) << std::endl;
}

void RecipTest() {
	CFloat_t S1, S6, S3;
	S1.Value = 0x4001800000000000ull;
	S6.Value = 0x4000ffffffff8000ull;
	S3 = ReciprocIterate(S1, S6);
	std::cout << "S3: " << HexPrinter(S3) << std::endl;
//		SW:  0x4001800000004000 
//		RTL: 0x4000000000008000 
}

int main(int argc, const char* argv[])
{
//	MulTest();
//	RecipTest();
//	return 1;
    
	Configuration_c Config;
	int RetVal = 0;

	bool DaemonMode = false;
	bool DisableAutoTerminal = false;
	bool WaitOnTermination = true;
	std::string ConfigFileName;
	std::string StdoutFileName;
	std::ofstream StdoutFile;
	
	CommandLine_c CommandLine(argc, argv);
	try {
		while (CommandLine.HasMoreParams()) {
			std::string CurParam = CommandLine.GetNextParam();
			if (CurParam.length() == 0) continue;
#if !defined(_WIN32)
			if (CurParam == "-d") {
				DaemonMode = true;
			} else if (CurParam == "-l") {
				if (!StdoutFileName.empty()) throw Generic_x() << "-l can only be specified once";
				StdoutFileName = CommandLine.GetNextParam();
			} else
#endif
			if (CurParam == "-a") {
				DisableAutoTerminal = true;
			}
			else if (CurParam == "-h") {
				return PrintUsage(argv[0]);
			}
			else {
				if (ConfigFileName.empty()) ConfigFileName = CurParam; else throw Generic_x("Unkown command line parameter");
			}
		}
		if (ConfigFileName.empty()) throw Generic_x() << "Config file name must be specified";
	}
	catch(std::exception &Ex) {
		return PrintUsage(argv[0], Ex.what());
	}
	
	if (!DaemonMode) signal(SIGINT, SigIntHandler);

	auto OldCoutBuf = std::cout.rdbuf();
#if !defined(_WIN32)
	setlocale(LC_ALL, "");
	try {
		if (DaemonMode) {
			umask(0);
			if (!StdoutFileName.empty()) {
				StdoutFile.open(StdoutFileName.c_str(), std::ios_base::out | std::ios_base::app);
				if (StdoutFile.bad()) throw Generic_x() << "Can't open daemon log file: " << StdoutFileName;
			}
			std::cout.rdbuf(StdoutFile.rdbuf()); // Redirect stdout to file
			// TODO: we might want another command-line flag to fork into another process. For now, however the scripts assume the deamon doesn't do it
			//daemon(TRUE, FALSE); // Don't CD to root, but close all standard streams
		}
	}
	catch(std::exception &Ex) {
		std::cout << Ex.what();
		return 1;
	}
#endif

	std::unique_ptr<Curses::Session_c> CursesSession;
	if (!DaemonMode) CursesSession = std::make_unique<Curses::Session_c>();

	try {
		// Set up a status windows for the mainframe
		std::unique_ptr<Curses::Frame_c> CpuStatusWindow;
		std::unique_ptr<Curses::Frame_c> IoStatusWindow;
		std::unique_ptr<Curses::Frame_c> CoutWindow;
		if (!DaemonMode) {
			CpuStatusWindow = std::make_unique<Curses::Frame_c>("Processors", *CursesSession, 0, 0);
			IoStatusWindow = std::make_unique<Curses::Frame_c>("Peripherals", *CursesSession, 0, 0);
			CpuStatusWindow->Hide();
			IoStatusWindow->Hide();
			
			// Set up console window
			CoutWindow = std::make_unique<Curses::Frame_c>("Console", *CursesSession, 0, 0);

			CoutWindow->ScrollOk(true);
			OldCoutBuf = std::cout.rdbuf(CoutWindow->rdbuf()); // Redirect cout to be inside the window

//			std::cout << "!!!!!!!" << std::endl;
			Curses::Panel_c::Update();

		}

		RootLogger.SetStream(std::cout);

		try {
			Config.Read(ConfigFileName.c_str());
		}
		catch (std::exception &Ex) {
			RootLogger.ReadConfig(Config);
			std::cout << "Can't read configuration file" << std::endl;
			std::cout << Ex.what() << std::endl;
			std::cout << "Press any key to terminate application" << std::endl;
			while (getch() == ERR);
			RetVal = 1;
			throw NoReportException_x();
		}

		bool MultiThreaded = false;
		bool SingleThreadCpus = false;

		if (!DaemonMode) {
#if !defined(__CYGWIN__) && defined(_WIN32)
			// Console resizing is not supported on Linux or under Cygwin.
			try {
				boost::optional<size_t> Width = Config.get_optional<size_t>("WindowWidth");
				boost::optional<size_t> Height = Config.get_optional<size_t>("WindowHeight");
				// Attempt to resize terminal to fit status display
				if (Width.is_initialized() && Height.is_initialized()) {
					CursesSession->ResizeTerm(Width.get(), Height.get());
				}
			}
			catch (std::exception &Ex) {
				std::cout << "Can't resize console: " << Ex.what() << std::endl;
			}
#endif
			try {
				CpuStatusWindow->Resize(CursesSession->GetMaxX(), CursesSession->GetMaxY());
				CpuStatusWindow->MoveWindow(0, 0);
				IoStatusWindow->Resize(CursesSession->GetMaxX(), CursesSession->GetMaxY());
				IoStatusWindow->MoveWindow(0, 0);
				CoutWindow->Resize(CursesSession->GetMaxX(), CursesSession->GetMaxY());
				CoutWindow->MoveWindow(0, 0);

				Curses::Panel_c::Update();
			}
			catch (std::exception &Ex) {
				if (OldCoutBuf != nullptr) std::cout.rdbuf(OldCoutBuf);
				OldCoutBuf = nullptr;
				std::cout << "Can't setup windows with exception: " << Ex.what() << std::endl;
				RetVal = 1;
				throw NoReportException_x();
			}
		}

		size_t CpuThreadStepping;
		bool UseThreadAffinity;
		try {
			// Set up log file, if specified
			boost::optional<std::string> LogFileName = Config.get_optional<std::string>("LogFileName");
			if (LogFileName.is_initialized()) {
				LogFile.open(LogFileName.get().c_str(), std::ios::out | std::ios::trunc);
				RootLogger.SetStream(LogFile);
			}
			// Read other global parameters
			if (MultiThreadedSupport) {
				MultiThreaded = Config.get<bool>("MultiThreaded", false);
				SingleThreadCpus = Config.get<bool>("SingleThreadCpus", false);
				CpuThreadStepping = Config.get<size_t>("CpuThreadStepping", 1);
				UseThreadAffinity = Config.get<bool>("UseThreadAffinity", false);
			}
		}
		catch (std::exception &Ex) {
			RootLogger << setloglevel(LogLevel_Always) << "Can't set up initial log state" << std::endl;
			RootLogger << setloglevel(LogLevel_Always) << Ex.what() << std::endl;
			std::cout << "Can't set up initial log state" << std::endl;
			std::cout << Ex.what() << std::endl;
			std::cout << "Press any key to terminate application" << std::endl;
			while (getch() == ERR);
			RetVal = 1;
			throw NoReportException_x();
		}

		try {
			WaitOnTermination = Config.get<bool>("WaitOnTermination", true);
			Mainframe_c Mainframe(Config, RootLogger, MultiThreaded, DaemonMode || DisableAutoTerminal);
			WebServerThread_c WebServerThread(Config.get_child_safe("WebServer"));
			if (WebServerThread.IsValid()) {
				WebServerThread.Start();
			}

			boost::optional<int64_t> TerminateTime = Config.get_optional<int64_t>("TerminateTime");
			if (TerminateTime.is_initialized()) TerminateTime.get() = TerminateTime.get() * 1000 * 1000 * 1000;

			int IntendedConsoleHeight = Config.get<int>("ConsoleHeight", 10);

			if (!DaemonMode) {
				CpuStatusWindow->Show();
				IoStatusWindow->Show();
				ResizeWindows(
					IntendedConsoleHeight,
					*CpuStatusWindow,
					*IoStatusWindow,
					*CoutWindow,
					*CursesSession,
					Mainframe
				);
			}

			RootLogger.SetTimeStampGenerator(&Mainframe);
			RootLogger.NotifyChildren();
			Mainframe.DeadStart();
			std::vector<std::thread> CpuThreadHandles(Mainframe.GetCpuCnt());
			std::vector<std::thread> BusyThreadHandles;
			if (MultiThreaded) {
				// Affinitize the main thread to core 0
				if (UseThreadAffinity) {
					SetThreadAffinity(0);
				}
				// Create CPU threads and affinitize them to cores in stepping
				if (SingleThreadCpus) {
					CpuThreadHandles[0] = std::thread(std::bind(AllCpuThread, std::ref(Mainframe)));
					if (UseThreadAffinity) {
						SetThreadAffinity(CpuThreadHandles[0], CpuThreadStepping);
					}
				} else {
					size_t CoreIdx = CpuThreadStepping;
					for (size_t i = 0; i < Mainframe.GetCpuCnt(); ++i) {
						CpuThreadHandles[i] = std::thread(std::bind(CpuThread, std::ref(Mainframe), i));
						if (UseThreadAffinity) {
							SetThreadAffinity(CpuThreadHandles[i], CoreIdx);
							CoreIdx += CpuThreadStepping;
						}
					}
				}
			}

			// Register all UI handlers
			CommandHooks_t CommandHooks;
			Mainframe.RegisterCommands(CommandHooks);
			for (auto &IopCluster : Mainframe.GetIopClusters()) {
				IopCluster.RegisterCommands(CommandHooks);
			}
			// TODO: check for duplicate triggers...
			/*		SmallWnd.MoveWindow(5, 5);
			Curses::Panel_c::Update();*/
			boost::timer::cpu_timer Timer;
			Timer.start();
			boost::timer::cpu_times LastTick = Timer.elapsed();
			boost::timer::cpu_times StartTick = LastTick;
			size_t KeyCheck = 0;

			try {
				std::vector<std::string> InputHistory;
				size_t HistoryIdx = 0;
				std::string InputLine;
				size_t InputPos = 0;
				bool ProcessLine = false;
				//bool OverrideMode = false;
				if (!DaemonMode) CoutWindow->Timeout(0);
				size_t TimerTestCnt = 0;
				while (!Terminate) {
					for (auto &IopCluster : Mainframe.GetIopClusters()) { IopCluster.Tick(); }
					if (!MultiThreaded) {
						for (auto &Cpu : Mainframe.GetCpus()) { Cpu.Tick(); }
						Mainframe.UpdateRealTimeClock();
					}
					Mainframe.ChannelTick();

//					if (WebServerThread.IsValid()) WebServer.poll();

					if (KeyCheck == 0 && !DaemonMode) {
						int Key = CoutWindow->GetCh();
						if (Key != ERR) {
							switch (Key) {
							case 3: // CTRL-C
								Terminate = true;
							break;
							case KEY_RESIZE:
								ResizeWindows(
									IntendedConsoleHeight,
									*CpuStatusWindow,
									*IoStatusWindow,
									*CoutWindow,
									*CursesSession,
									Mainframe
								);
							break;
							case KEY_UP:
								if (HistoryIdx > 0) {
									--HistoryIdx;
									InputLine = InputHistory[HistoryIdx];
									InputPos = InputLine.length();
								} else {
									beep();
								}
							break;
							case KEY_DOWN:
								if (HistoryIdx + 1 < InputHistory.size()) {
									++HistoryIdx;
									InputLine = InputHistory[HistoryIdx];
									InputPos = InputLine.length();
								}
								else if (HistoryIdx + 1 == InputHistory.size()) {
									++HistoryIdx;
									InputLine = "";
									InputPos = 0;
								} else {
									beep();
								}
							break;
							case KEY_LEFT:
								if (InputPos != 0) --InputPos; else beep();
							break;
							case KEY_RIGHT:
								if (InputPos < InputLine.length()) ++InputPos; else beep();
							break;
							case KEY_HOME:
								InputPos = 0;
							break;
							case KEY_END:
								InputPos = InputLine.length();
							break;
							case 2: // Ctrl-b
//								BusyThreadHandles.push_back(std::thread(BusyThread));
								Mainframe.RouteChannelInterrupt();
							break;
							case 0x7f:
							case KEY_BACKSPACE:
							case '\b':
								if (InputPos != 0) {
									--InputPos;
									InputLine.erase(InputPos,1);
								}
								else {
									beep();
								}
							break;
							case '\n':
							case '\r':
								ProcessLine = true;
								CoutWindow->Move(int(InputLine.length()), CoutWindow->GetCurY());
								std::cout << std::endl;
								InputPos = 0;
							break;

							case '\t': // TAB: repeat buffer
								std::cout << std::endl << InputLine << std::flush;
							break;
							default:
								if (Key >= ' ' && Key < 0x7f) {
									if (int(InputPos) < CoutWindow->GetMaxX() - 3) {
										InputLine.insert(InputPos, 1, Key);
										InputPos++;
									}
									else {
										beep();
									}
								} else {
//									std::cout << "Char: " << HexPrinter(Key) << std::endl;
								}
							break;
							}
							// Print the input line over the current line
							CoutWindow->Move(0, CoutWindow->GetCurY());
							CoutWindow->AddStr(InputLine);
							CoutWindow->ClearToEol();
							CoutWindow->Move(int(InputPos), CoutWindow->GetCurY());
							
							//std::cout << "Key pressed: " << Key << std::endl;
							/*for (auto &UIHook : UIHooks) {
							if (UIHook.TriggerKey == Key) {
							UIHook.Callback();
							}
							}*/
						} else {
							KeyCheck = 500;
						}
					} else {
						--KeyCheck;
					}
					if (ProcessLine) {
						try {
							TokenStream_t Tokens = Tokenize(InputLine);
							TokenStream_t::const_iterator Token = Tokens.cbegin();
							TokenStream_t::const_iterator TokenEnd = Tokens.cend();
							// Test for help with no parameters
							if (Tokens.size() > 1) {
								if (Token->mValue == "help" && Tokens.size() == 2) {
									std::cout << "Available system commands: exit setloglevel consoleheight dump" << std::endl;
									std::cout << "Available device commands: ";
									boost::container::flat_set<std::string> Commands;
									for (const auto &Command : CommandHooks) {
										Commands.insert(Command->GetCommandName());
									}
									for (const auto &Command : Commands) {
										std::cout << Command << " ";
									}
									std::cout << std::endl;
									std::cout << "Each device command needs a device name. Available device names: ";
									boost::container::flat_set<std::string> Devices;
									for (const auto &Command : CommandHooks) {
										Devices.insert(Command->GetDeviceName());
									}
									for (const auto &Device : Devices) {
										std::cout << Device << " ";
									}
									std::cout << std::endl;
									std::cout << "Type 'help <command> <device>' for more detailed help on individual device commands or 'help <command>' for help on system commands " << std::endl;
								}
								else if (Token->mValue == "help" && Tokens.size() == 3) {
									Token++;
									if (Token->mValue == "exit") {
										std::cout << "exit" << std::endl << "    terminates the simulation" << std::endl;
									}
									else if (Token->mValue == "setloglevel") {
										std::cout << "setloglevel <logger> <log level>" << std::endl << "    Sets the log level on the specified logger" << std::endl;
										std::string Loggers = GetLoggerNames();
										std::cout << "    Available loggers: " << Loggers << std::endl;
										std::cout << "    Available log levels: " << GetLogLevels() << std::endl;
									}
									else if (Token->mValue == "consoleheight") {
										std::cout << "consoleheight <height>" << std::endl << "    Sets the console height. Possible values are 'normal' and 'tall' or an integer number" << std::endl;
									}
									else if (Token->mValue == "dump") {
										std::cout << "dump" << std::endl << "    Dumps current state of memories and registers into files" << std::endl;
									}
									else {
										std::cout << "Unkown command: no help is available" << std::endl;
									}
								}
								else if (Token->mValue == "exit" && Tokens.size() == 2) {
									Terminate = true;
								}
								else if (Token->mValue == "setloglevel" && Tokens.size() == 4) {
									do {
										Token++;
										std::string LoggerName = Token->mValue;
										Token++;
										std::string LogLevelStr = Token->mValue;
										LoggerBase_c *Logger = GetLogger(LoggerName);
										if (Logger == nullptr) {
											std::cout << "Unknown logger" << std::endl;
											break;
										}
										try {
											LogLevels_e LogLevel = FromString_LogLevels_e(LogLevelStr);
											Logger->SetDisplayLogLevel(LogLevel);
											std::cout << "Log level set" << std::endl;
										}
										catch (std::exception &Ex) {
											std::cout << "Can't set log level due to: " << Ex.what() << std::endl;
										}
									} while (false);
								}
								else if (Token->mValue == "consoleheight" && Tokens.size() == 3) {
									Token++;
									int Height;
									if (Token->mValue == "normal") Height = 10;
									else if (Token->mValue == "tall") Height = CursesSession->GetMaxY() * 2 / 3;
									else {
										try {
											Height = std::stoi(Token->mValue);
										}
										catch (...) { Height = 0; } // This will be checked later and an error will be reported
									}
									if (Height < 5 || Height > CursesSession->GetMaxY() - 10) {
										std::cout << "invlid height specified" << std::endl;
									}
									else {
										IntendedConsoleHeight = Height;
										ResizeWindows(
											IntendedConsoleHeight,
											*CpuStatusWindow,
											*IoStatusWindow,
											*CoutWindow,
											*CursesSession,
											Mainframe
										);
									}
								}
								else if (Token->mValue == "dump" && Tokens.size() == 2) {
									try {
										DumpMainframe(Mainframe);
										std::cout << "Dump complete" << std::endl;
									}
									catch (std::exception &Ex) {
										RootLogger << setloglevel(LogLevel_Always) << "Can't write memory dump" << std::endl;
										RootLogger << setloglevel(LogLevel_Always) << Ex.what() << std::endl;
										std::cout << "Can't write memory dump" << std::endl;
										std::cout << Ex.what() << std::endl;
									}
								}
								else {
									bool Found = false;
									for (const auto &Command : CommandHooks) {
										Found = Command->ParseAndExec(Token, TokenEnd);
										if (Found) break;
									}
									if (!Found) {
										// None of the commands managed to parse the current token
										throw Generic_x() << "Invalid command. Try 'help' for available commands";
									}
								}
							}
						}
						catch (std::exception &Ex) {
							std::cout << Ex.what() << std::endl;
						}
						InputHistory.push_back(InputLine);
						HistoryIdx = InputHistory.size();
						InputLine.clear();
						ProcessLine = false;
					}
					if (++TimerTestCnt == 1000) {
						boost::timer::cpu_times CurrentTime = Timer.elapsed();
//						boost::timer::nanosecond_type DeltaTime = CurrentTime.user - LastTick.user;
						boost::timer::nanosecond_type DeltaWallTime = CurrentTime.wall - LastTick.wall;
						boost::timer::nanosecond_type WallTime = CurrentTime.wall - StartTick.wall;
						// Update status info four times a second
						if (DeltaWallTime > boost::timer::nanosecond_type(1000 * 1000 * 250)) {
							LastTick = CurrentTime;

							if (TerminateTime.is_initialized()) {
								if (TerminateTime.get() <= WallTime) Terminate = true;
							}

							WallTime /= 1000000ULL; // Turn into milliseconds
							uint64_t Millseconds = WallTime % 1000;
							uint64_t Seconds = ((WallTime - Millseconds) / 1000) % 60;
							uint64_t Minutes = ((WallTime - Millseconds - Seconds * 1000) / (1000 * 60)) % 60;
							uint64_t Hours = ((WallTime - Millseconds - Seconds * 1000 - Minutes * 1000 * 60) / (1000 * 60 * 60)) % 24;
							uint64_t Days = ((WallTime - Millseconds - Seconds * 1000 - Minutes * 1000 * 60 - Hours * 1000 * 60 * 60) / (1000 * 60 * 60 * 24));
							std::stringstream WallTimeStr;
							WallTimeStr << Days << " days " << Hours << ":" << std::setw(2) << std::setfill('0') << Minutes << ":" << std::setw(2) << std::setfill('0') << Seconds << "." << std::setw(3) << std::setfill('0') << Millseconds;
							StatusReport_c WallTimeStatus;
							WallTimeStatus.put("Uptime", WallTimeStr.str());

							if (!DaemonMode) {
								CpuStatusWindow->Move(0, 0);
								CpuStatusWindow->clear();
								StatusReport_c CpuStatus;
								CpuStatus.put_child("Wall time", WallTimeStatus);
								GetCpuStatus(CpuStatus, Mainframe, DeltaWallTime, false);
								DisplayStatus(CpuStatus, *CpuStatusWindow);
								CpuStatusWindow->ClearToBot();
							}
							if (WebServerThread.IsValid()) {
								StatusReport_c CpuStatus;
								CpuStatus.put_child("Wall time", WallTimeStatus);
								GetCpuStatus(CpuStatus, Mainframe, DeltaWallTime, true);
								WebServerThread.UpdateStatus("Cpu", CpuStatus);
							}

							if (!DaemonMode) {
								IoStatusWindow->Move(0, 0);
								IoStatusWindow->clear();
							}
							static const PeripheralType_e StatusDisplays[] = { PeripheralType_e::Disk, PeripheralType_e::Tape, PeripheralType_e::Network, PeripheralType_e::Console, PeripheralType_e::Printer };
							for (auto PeripheralType : StatusDisplays) {
								if (!DaemonMode) {
									StatusReport_c Status;
									GetPeripheralStatus(Status, Mainframe, PeripheralType, DeltaWallTime, false);
									if (!Status.empty()) {
										DisplayStatus(Status, *IoStatusWindow);
										*IoStatusWindow << std::endl;
									}
								}
								if (WebServerThread.IsValid()) {
									StatusReport_c Status;
									GetPeripheralStatus(Status, Mainframe, PeripheralType, DeltaWallTime, true);
									WebServerThread.UpdateStatus(PeripheralTypeNames[size_t(PeripheralType)], Status);
								}
							}
							if (!DaemonMode) {
								IoStatusWindow->ClearToBot();
							}
						}
						TimerTestCnt = 0;
					}
				}
				if (MultiThreaded) {
					if (SingleThreadCpus) {
						CpuThreadHandles[0].join();
					} else {
						for (size_t i = 0; i < Mainframe.GetCpuCnt(); ++i) {
							CpuThreadHandles[i].join();
						}
					}
				}
				for (auto &BusyThreadHandle : BusyThreadHandles) BusyThreadHandle.join();
				RootLogger << setloglevel(LogLevel_Always) << "Simulation was terminated by user" << std::endl;
				std::cout << "Simulation was terminated by user" << std::endl;
			}
			catch (Terminate_x &Ex) {
				RootLogger << setloglevel(LogLevel_Always) << "=====================================================================" << std::endl;
				RootLogger << setloglevel(LogLevel_Always) << "Simulation terminated from sim" << std::endl;
				RootLogger << setloglevel(LogLevel_Always) << Ex.what() << std::endl;
				std::cout << "=====================================================================" << std::endl;
				std::cout << "Simulation terminated from sim" << std::endl;
				std::cout << Ex.what() << std::endl;
				RetVal = 1;
			}
			catch (std::exception &Ex) {
				RootLogger << setloglevel(LogLevel_Always) << "=====================================================================" << std::endl;
				RootLogger << setloglevel(LogLevel_Always) << "Simulation terminated with exception" << std::endl;
				RootLogger << setloglevel(LogLevel_Always) << Ex.what() << std::endl;
				std::cout << "=====================================================================" << std::endl;
				std::cout << "Simulation terminated with exception" << std::endl;
				std::cout << Ex.what() << std::endl;
				RetVal = 1;
			}
			WebServerThread.Stop();
			try {
				DumpMainframe(Mainframe);
			}
			catch (std::exception &Ex) {
				RootLogger << setloglevel(LogLevel_Always) << "Can't write memory dump" << std::endl;
				RootLogger << setloglevel(LogLevel_Always) << Ex.what() << std::endl;
				std::cout << "Can't write memory dump" << std::endl;
				std::cout << Ex.what() << std::endl;
				RetVal = 1;
			}
		}
		catch (std::exception &Ex) {
			std::cout << "Simulation couldn't start with exception" << std::endl;
			std::cout << Ex.what() << std::endl;
			RetVal = 1;
		}

		if ((!Terminate || RetVal != 0) && WaitOnTermination) {
			std::cout << "Press any key to terminate application" << std::endl;
			while (getch() == ERR);
		}
	}
	catch (NoReportException_x &) {
		if (OldCoutBuf != nullptr) std::cout.rdbuf(OldCoutBuf);
		OldCoutBuf = nullptr;
	}
	catch (std::exception &Ex) {
		if (OldCoutBuf != nullptr) std::cout.rdbuf(OldCoutBuf);
		OldCoutBuf = nullptr;
		RootLogger << setloglevel(LogLevel_Always) << "Execption during execution:" << std::endl;
		RootLogger << setloglevel(LogLevel_Always) << Ex.what() << std::endl;
		std::cout << "Execption during execution:" << std::endl;
		std::cout << Ex.what() << std::endl;
		if (WaitOnTermination) {
			std::cout << "Press any key to terminate application" << std::endl;
			while (getch() == ERR);
		}
		RetVal = 1;
	}
	if (OldCoutBuf != nullptr) std::cout.rdbuf(OldCoutBuf);
	OldCoutBuf = nullptr;
	return RetVal;
}

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif
