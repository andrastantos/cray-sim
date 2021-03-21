#include "console.h"

#if defined(__CYGWIN__) || defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX
#include <windows.h>
#endif

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

// Console_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Console_c::Console_c(const Configuration_c &aConfig, CLogger_c &aLogger, uint16_t aPort, bool aAutoPort, bool aDisableAutoTerminal) :
	mLogger(aLogger),
	mAccepted(false),
	mConnectionSocket(nullptr),
	mPort(aPort),
	mAutoPort(aAutoPort),
#if defined(_WIN32)
	mConsoleCommand(aConfig.get_optional<std::string>("ConsoleCommandWindows")),
#else
	mConsoleCommand(aConfig.get_optional<std::string>("ConsoleCommandLinux")),
#endif
	mNewTerminalCommand(aConfig.get_optional<std::string>("NewTerminalCommand")),
	mReceiveFull(false),
	mPollCnt(0),
	mTelnetState(TelnetStates_e::WaitIAC),
	mTelnetAction(TelnetCodes_e::IAC),
	mEatLF(aConfig.get<bool>("EatLF", true)),
	mEatNull(aConfig.get<bool>("EatNull", true)),
	mMapBackspace(aConfig.get<bool>("MapBackspace", true)),
	mBackspaceChar(aConfig.get<int>("BackspaceChar", 127) & 0xff),
	mDisableAutoTerminal(aDisableAutoTerminal),
	mLogInitialized(false)
{
	if (!mConsoleCommand.is_initialized()) {
		mConsoleCommand = aConfig.get_optional<std::string>("ConsoleCommand");
	}
	for (auto &TelnetOption : mTelnetOptions) {
		TelnetOption.Do = false;
		TelnetOption.Will = false;
		TelnetOption.OutstandingDoDont = false;
		TelnetOption.OutstandingWillWont = false;
	}
	do {
		try {
			mServerSocket = std::make_shared<boost::asio::ip::tcp::acceptor>(mIoService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), mPort), false);
			break;
		}
		catch (boost::system::system_error &Ex) {
			switch (Ex.code().value()) {
				case EADDRINUSE:
				case 10048: //case WSAEADDRINUSE:
					if (mAutoPort) {
						++mPort;
						break;
					}
				// Deliberate fall-through in case of no mAutoPort
				default:
					throw Generic_x() << "Can't open console port " << DecPrinter(mPort) << " with error: " << Ex.code().message();
			}
		}
	} while (true);
	mLogFileName = aConfig.get<std::string>("LogFile", "");
}

Console_c::~Console_c() {
	Flush();
}

bool Console_c::HasInput() const { return mReceiveFull; }

char Console_c::GetChar() {
	CRAY_ASSERT(mReceiveFull);
	volatile char RetVal = mReceiveBuf[0];
	mReceiveFull = false;
	PrimeForReceive();
	return RetVal;
}

void Console_c::SendString(std::string &aData) {
	if (!mLogInitialized) {
		if (!mLogFileName.empty()) {
			if (mLogFileName == "auto") {
				mLogFileName = GetName();
			}
			std::ofstream LogFile(mLogFileName.c_str(), std::ios_base::out);
		}
		mLogInitialized = true;
	}
	if (!mLogFileName.empty()) {
		std::ofstream LogFile(mLogFileName.c_str(), std::ios_base::out | std::ios_base::app);
		LogFile << aData;
	}
	SendStringInternal(aData, true);
}

void Console_c::SendFromQueue() {
	if (mTransmitQueue.empty()) return;

	AcceptIfNeeded(false); // This might recursively call SendFromQueue, so re-test the empty condition afterwards

	if (mTransmitQueue.empty()) return;
	if (mAccepted) {
		CRAY_ASSERT(mConnectionSocket != nullptr);
		try {
			// Start an (async) write
			TransmitQueueEntry_s &Head = mTransmitQueue.front();
			boost::asio::async_write(
				*mConnectionSocket,
				boost::asio::buffer(Head.mData),
				boost::bind(&Console_c::WriteHandler, this, boost::asio::placeholders::error)
				);
			//for (int i = 0; i < 100; ++i) {
			//	mIoService.reset();
			//	mIoService.poll();
			//}
		}
		catch (boost::system::system_error &) {
			CloseSocket();
		}
	}
}

void Console_c::SendOptionString(TelnetCodes_e aAction, TelnetOptions_e aOption, bool aRequest) {
	char Str[4];
	Str[0] = char(TelnetCodes_e::IAC);
	Str[1] = char(aAction);
	Str[2] = char(aOption);
	Str[3] = 0;
//	std::cout << (aRequest ? " Request<" : " Reply<" ) << DecPrinter(int(aAction) & 0xff, 2) << ", " << DecPrinter(int(aOption) & 0xff, 2) << ">" << std::flush;
	std::string OptStr(Str);
	SendStringInternal(OptStr, false);
	switch (aAction) {
		case TelnetCodes_e::DO:
		case TelnetCodes_e::DONT:
			mTelnetOptions[size_t(aOption)].OutstandingDoDont = aRequest;
			mTelnetOptions[size_t(aOption)].OutstandingWillWont = false;
		break;
		case TelnetCodes_e::WILL:
		case TelnetCodes_e::WONT:
			mTelnetOptions[size_t(aOption)].OutstandingDoDont = false;
			mTelnetOptions[size_t(aOption)].OutstandingWillWont = aRequest;
		break;
		default:
			CRAY_ASSERT(false);
		break;
	}
//    std::cout << " STAT:<" << DecPrinter(int(aOption) & 0xff, 2) << ", " << (mTelnetOptions[size_t(aOption)].OutstandingWillWont ? "OW " : "-- ") << (mTelnetOptions[size_t(aOption)].OutstandingDoDont ? "OD " : "-- ") << ">" << std::flush;
}

void Console_c::SendStringInternal(std::string &aData, bool aDoNotify) {
	bool SendNow = mTransmitQueue.empty();

	//if (aDoNotify) std::cout << aData << std::flush;
	mTransmitQueue.push(TransmitQueueEntry_s(aData, aDoNotify));

	if (SendNow) SendFromQueue();
}

void Console_c::Flush() {
	while (!mTransmitQueue.empty()) Poll();
}

void Console_c::SendString(const char *aData, size_t aLen) {
	std::string Data(aData, aLen);
	SendString(Data);
}

void Console_c::SendString(const char *aData) {
	std::string Data(aData);
	SendString(Data);
}

void Console_c::SendChar(char aData) {
	char Data[2];
	Data[0] = aData;
	Data[1] = 0;
	SendString(Data);
}

void Console_c::Reset() {
	ResetDetail();
}

void Console_c::Tick() {
	if (++mPollCnt > 1000) {
		while (HasInput()) {
			if (!ReceiveHandlerDetail()) break;
			for (size_t Idx = 0; Idx < 10; ++Idx) {
				Poll();
				if (HasInput()) break;
			}
		}
		Poll();
		mPollCnt = 0;
	}
}

void Console_c::Poll() {
	mIoService.reset();
	mIoService.poll();
}

void Console_c::CloseSocket() {
	boost::system::error_code IgnoredError;
	mConnectionSocket->close(IgnoredError);
	mConnectionSocket = nullptr;
	mReceiveBuf[0] = 0;
	mAccepted = false;
	AcceptIfNeeded(false);
}

void Console_c::PrimeForReceive() {
	// Prime the socket to receive chars
	boost::asio::async_read(
		*mConnectionSocket,
		boost::asio::buffer(mReceiveBuf),
		boost::bind(&Console_c::ReadHandler, this, boost::asio::placeholders::error)
	);
}

void Console_c::AcceptHandler(const boost::system::error_code& aError) {
//	std::cout << "A" << std::flush;
	if (aError) {
		CloseSocket();
		return;
	}
	try {
		mAccepted = true;
		mConnectionSocket->set_option(boost::asio::socket_base::keep_alive(true));
		PrimeForReceive();
		// If there's a pending transmission, send it
		ConnectDetail();
//		SendOptionString(TelnetCodes_e::DONT, TelnetOptions_e::OPT_ECHO, true);
		SendOptionString(TelnetCodes_e::WILL, TelnetOptions_e::OPT_ECHO, true);
		SendOptionString(TelnetCodes_e::WILL, TelnetOptions_e::SGA, true);
		SendOptionString(TelnetCodes_e::WONT, TelnetOptions_e::LINEMODE, true);
//		SendOptionString(TelnetCodes_e::DO, TelnetOptions_e::SGA, true);
		SendFromQueue();
//		mIoService.poll();
	}
	catch (boost::system::system_error &) {
		CloseSocket();
	}
}

void Console_c::ReadHandler(const boost::system::error_code& aError) {
//	std::cout << "r" << std::flush;
	if (aError) {
		CloseSocket();
		return;
	}
	CRAY_ASSERT(!mReceiveFull);
	bool ReceiveFull = true;
//	mLogger << setloglevel(LogLevel_IoTrace) << "Input char: " << DecPrinter(uint32_t(mReceiveBuf[0]) & 0xff) << " in state: " << int(mTelnetState) << std::endl;
//	std::cout << " " << DecPrinter(uint32_t(mReceiveBuf[0]) & 0xff) << std::flush;
//    std::cout << " RS:<" << DecPrinter(int(1) & 0xff, 2) << ", " << (mTelnetOptions[size_t(1)].OutstandingWillWont ? "OW " : "-- ") << (mTelnetOptions[size_t(1)].OutstandingDoDont ? "OD " : "-- ") << ">" << std::flush;
//    std::cout << " RS:<" << DecPrinter(int(3) & 0xff, 2) << ", " << (mTelnetOptions[size_t(3)].OutstandingWillWont ? "OW " : "-- ") << (mTelnetOptions[size_t(3)].OutstandingDoDont ? "OD " : "-- ") << ">" << std::flush;
	switch (mTelnetState) {
		case TelnetStates_e::WaitIAC:
//			std::cout << "-" << HexPrinter(int(mReceiveBuf[0]), 2) << "-" << std::flush;
			if (mReceiveBuf[0] == char(TelnetCodes_e::IAC)) {
				mTelnetState = TelnetStates_e::WaitAction;
				ReceiveFull = false;
			}
			else {
				// Eat LF from CRLF
				if (mEatLF && mReceiveBuf[0] == 0x0a) {
					ReceiveFull = false;
				}
				if (mEatNull && mReceiveBuf[0] == 0x00) {
					ReceiveFull = false;
				}
				// Convert ^H to back-space
				if (mMapBackspace && (mReceiveBuf[0] == 0x08 || mReceiveBuf[0] == 0x7f)) mReceiveBuf[0] = mBackspaceChar;
				mTelnetState = TelnetStates_e::WaitIAC;
			}
		break;
		case TelnetStates_e::WaitAction:
			mTelnetAction = TelnetCodes_e(mReceiveBuf[0]);
			switch (mTelnetAction) {
				case TelnetCodes_e::DO:
				case TelnetCodes_e::DONT:
				case TelnetCodes_e::WILL:
				case TelnetCodes_e::WONT:
					mTelnetState = TelnetStates_e::WaitOption;
					ReceiveFull = false;
				break;
				case TelnetCodes_e::NOP: // Eat keep-alive
					ReceiveFull = false;
					mTelnetState = TelnetStates_e::WaitIAC;
				break;
				default:
					mTelnetState = TelnetStates_e::WaitIAC;
				break;
			}
		break;
		case TelnetStates_e::WaitOption:
			TelnetOptions_e TelnetOption = TelnetOptions_e(mReceiveBuf[0]);
			ReceiveFull = false;
//			std::cout << " In:<" << DecPrinter(int(mTelnetAction) & 0xff, 2) << ", " << DecPrinter(int(TelnetOption) & 0xff, 2) << ", " << (mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont ? "OW " : "-- ") << (mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont ? "OD " : "-- ") << ">" << std::flush;
			switch (mTelnetAction) {
				case TelnetCodes_e::DO: {
					switch (TelnetOption) {
						case TelnetOptions_e::OPT_ECHO:
						case TelnetOptions_e::SGA:
							if (!mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont) SendOptionString(TelnetCodes_e::WILL, TelnetOption, false);
							mTelnetOptions[size_t(TelnetOption)].Do = true;
							mTelnetOptions[size_t(TelnetOption)].Will = true;
							mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont = false;
						break;
						default:
							if (!mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont) SendOptionString(TelnetCodes_e::WONT, TelnetOption, false);
							mTelnetOptions[size_t(TelnetOption)].Do = true;
							mTelnetOptions[size_t(TelnetOption)].Will = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont = false;
						break;
					}
				}
				break;
				case TelnetCodes_e::DONT: {
					switch (TelnetOption) {
						case TelnetOptions_e::OPT_ECHO:
						case TelnetOptions_e::SGA:
							if (!mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont) SendOptionString(TelnetCodes_e::WILL, TelnetOption, false);
							mTelnetOptions[size_t(TelnetOption)].Do = false;
							mTelnetOptions[size_t(TelnetOption)].Will = true;
							mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont = false;
						break;
						default:
							if (!mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont) SendOptionString(TelnetCodes_e::WONT, TelnetOption, false);
							mTelnetOptions[size_t(TelnetOption)].Do = false;
							mTelnetOptions[size_t(TelnetOption)].Will = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont = false;
						break;
					}
				}
				break;
				case TelnetCodes_e::WILL: {
					switch (TelnetOption) {
						case TelnetOptions_e::OPT_ECHO:
						case TelnetOptions_e::SGA:
							if (!mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont) SendOptionString(TelnetCodes_e::DO, TelnetOption, false);
							mTelnetOptions[size_t(TelnetOption)].Do = true;
							mTelnetOptions[size_t(TelnetOption)].Will = true;
							mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont = false;
						break;
						default:
							if (!mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont) SendOptionString(TelnetCodes_e::DONT, TelnetOption, false);
							mTelnetOptions[size_t(TelnetOption)].Do = false;
							mTelnetOptions[size_t(TelnetOption)].Will = true;
							mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont = false;
						break;
					}
				}
				break;
				case TelnetCodes_e::WONT: {
					switch (TelnetOption) {
						case TelnetOptions_e::OPT_ECHO:
						case TelnetOptions_e::SGA:
							if (!mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont) SendOptionString(TelnetCodes_e::DO, TelnetOption, false);
							mTelnetOptions[size_t(TelnetOption)].Do = true;
							mTelnetOptions[size_t(TelnetOption)].Will = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont = false;
						break;
						default:
							if (!mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont) SendOptionString(TelnetCodes_e::DONT, TelnetOption, false);
							mTelnetOptions[size_t(TelnetOption)].Do = false;
							mTelnetOptions[size_t(TelnetOption)].Will = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingWillWont = false;
							mTelnetOptions[size_t(TelnetOption)].OutstandingDoDont = false;
						break;
					}
				}
				break;
				default:
					CRAY_ASSERT(false);
				break;
			}
			mTelnetState = TelnetStates_e::WaitIAC;
		break;
	}
	mReceiveFull = ReceiveFull;
	if (!mReceiveFull) PrimeForReceive();
}


void Console_c::WriteHandler(const boost::system::error_code& aError) {
//	std::cout << "w" << std::flush;
	if (aError) {
		CloseSocket();
		return;
	}
	if (!mTransmitQueue.empty()) {
		bool DoNotify = mTransmitQueue.front().mDoNotify;
		mTransmitQueue.pop();
		SendFromQueue();
		if (DoNotify) SendHandlerDetail();
	}
}

void Console_c::AcceptIfNeeded(bool aSilent) {
	if (mConnectionSocket == nullptr) {
		if (!aSilent && !mDisableAutoTerminal) {
			if (!mConsoleCommand.is_initialized()) {
				std::cout << "please connect terminal to localhost:" << DecPrinter(mPort) << std::endl;
			} else {
#if !defined(__CYGWIN__) && !defined(_WIN32)
				std::stringstream PortStr;
				PortStr << DecPrinter(mPort);

				std::string CmdLine = mNewTerminalCommand.get_value_or("{cmd} &");
				CmdLine = Replace(CmdLine, "{cmd}", mConsoleCommand.get());
				CmdLine = Replace(CmdLine, "{host}", "localhost");
				CmdLine = Replace(CmdLine, "{port}", PortStr.str());
				if (!aSilent) std::cout << "Starting console application: " << CmdLine << std::endl;
				int RetVal = system(CmdLine.c_str());
#else
				std::string CmdLine = Replace(mConsoleCommand.get(), "{host}", "localhost");
				std::stringstream PortStr;
				PortStr << DecPrinter(mPort);
				CmdLine = Replace(CmdLine, "{port}", PortStr.str());
				if (!aSilent) std::cout << "Starting console application: " << CmdLine << std::endl;

				char *CCmdLine = new char[CmdLine.length() + 1];
				memcpy(CCmdLine, CmdLine.c_str(), CmdLine.length() + 1);
				PROCESS_INFORMATION processInformation;
				STARTUPINFOA startupInfo;
				memset(&processInformation, 0, sizeof(processInformation));
				memset(&startupInfo, 0, sizeof(startupInfo));
				CreateProcessA(NULL, CCmdLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInformation);
				CloseHandle(processInformation.hProcess);
				CloseHandle(processInformation.hThread);
				delete[] CCmdLine;
#endif
			}
		}
		try {
//			std::cout << " CLEAR" << std::flush;
			for (auto &TelnetOption : mTelnetOptions) {
				TelnetOption.Do = false;
				TelnetOption.Will = false;
				TelnetOption.OutstandingDoDont = false;
				TelnetOption.OutstandingWillWont = false;
			}
			mConnectionSocket = std::make_shared<boost::asio::ip::tcp::socket>(mIoService);
			mAccepted = false;
			mServerSocket->async_accept(*mConnectionSocket, boost::bind(&Console_c::AcceptHandler, this, boost::asio::placeholders::error));
			mIoService.poll();
		}
		catch (boost::system::system_error &) {
			CloseSocket();
		}
	}
}

boost::asio::io_service Console_c::mIoService;

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif
