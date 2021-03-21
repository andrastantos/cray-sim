#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <stdio.h>
#include <stdlib.h>
#include <boost/asio.hpp>
#include "utils.h"
#include "cray_logger.h"
#include "debug_events.h"
#include <stdint.h>
#include <vector>
#include "config_file.h"
#include <queue>


enum class TelnetCodes_e : uint8_t {
	IAC = 255,
	DONT = 254,
	DO = 253,
	WONT = 252,
	WILL = 251,
	NOP = 241
};

enum class TelnetOptions_e : uint8_t {
	BINARY = 0, // 8-bit data path
	OPT_ECHO = 1, // echo
	RCP = 2, // prepare to reconnect
	SGA = 3, // suppress go ahead
	NAMS = 4, // approximate message size
	STATUS = 5, // give status
	TM = 6, // timing mark
	RCTE = 7, // remote controlled transmission and echo
	NAOL = 8, // negotiate about output line width
	NAOP = 9, // negotiate about output page size
	NAOCRD = 10, // negotiate about CR disposition
	NAOHTS = 11, // negotiate about horizontal tabstops
	NAOHTD = 12, // negotiate about horizontal tab disposition
	NAOFFD = 13, // negotiate about formfeed disposition
	NAOVTS = 14, // negotiate about vertical tab stops
	NAOVTD = 15, // negotiate about vertical tab disposition
	NAOLFD = 16, // negotiate about output LF disposition
	XASCII = 17, // extended ascic character set
	LOGOUT = 18, // force logout
	BM = 19, // byte macro
	DET = 20, // data entry terminal
	SUPDUP = 21, // supdup protocol
	SUPDUPOUTPUT = 22, // supdup output
	SNDLOC = 23, // send location
	TTYPE = 24, // terminal type
	EOR = 25, // end or record
	TUID = 26, // TACACS user identification
	OUTMRK = 27, // output marking
	TTYLOC = 28, // terminal location number
	OPT3270REGIME = 29, // 3270 regime
	X3PAD = 30, // X.3 PAD
	NAWS = 31, // window size
	TSPEED = 32, // terminal speed
	LFLOW = 33, // remote flow control
	LINEMODE = 34, // Linemode option
	XDISPLOC = 35, // X Display Location
	OLD_ENVIRON = 36, // Old - Environment variables
	AUTHENTICATION = 37, // Authenticate
	ENCRYPT = 38, // Encryption option
	NEW_ENVIRON = 39, // New - Environment variables
	TN3270E = 40, // TN3270 enhancements
	XAUTH = 41,
	CHARSET = 42, // Character set
	RSP = 43, // Remote serial port
	COM_PORT_OPTION = 44, // Com port control
	SLE = 45, // Suppress local echo
	STARTTLS = 46, // Start TLS
	KERMIT = 47, // Automatic Kermit file transfer
	SEND_URL = 48,
	FORWARD_X = 49,
	PRAGMA_LOGON = 138,
	SSPI_LOGON = 139,
	PRAGMA_HEARTBEAT = 140,
	EXOPL = 255  // extended-options-list
};

class Console_c {
public:
	Console_c(const Configuration_c &aConfig, CLogger_c &aLogger, uint16_t aPort, bool aAutoPort, bool aDisableAutoTerminal);
	virtual ~Console_c();
	bool HasInput() const;
	char GetChar();
	void SendString(std::string &aData);
	void SendString(const char *aData, size_t aLen);
	void SendString(const char *aData);
	void SendChar(char aData);
	void Reset();
	void Tick();
protected:
	CLogger_c &mLogger;
	uint8_t mDev;

	uint16_t mPort;
	bool mAutoPort;
	boost::optional<std::string> mConsoleCommand;
	boost::optional<std::string> mNewTerminalCommand;

	static boost::asio::io_service mIoService;
	std::shared_ptr<boost::asio::ip::tcp::acceptor> mServerSocket;
	std::shared_ptr<boost::asio::ip::tcp::socket> mConnectionSocket;
	boost::array<char, 1> mReceiveBuf;
	bool mReceiveFull;
	struct TransmitQueueEntry_s {
		TransmitQueueEntry_s(std::string aData, bool aDoNotify) : mData(aData), mDoNotify(aDoNotify) {}

		std::string mData;
		bool mDoNotify;
	};
	std::queue<TransmitQueueEntry_s> mTransmitQueue;
	// std::string mTransmitBuf;
	// bool mTransmitFull;
	bool mAccepted;

	void CloseSocket();
	void AcceptHandler(const boost::system::error_code& aError);
	void ReadHandler(const boost::system::error_code& aError);
	void WriteHandler(const boost::system::error_code& aError);
	void AcceptIfNeeded(bool aSilent);
	void Poll();
	size_t mPollCnt;

	void SendStringInternal(std::string &aData, bool aDoNotify);
	void SendFromQueue();

	virtual bool ReceiveHandlerDetail() = 0;
	virtual void SendHandlerDetail() = 0;
	virtual void ResetDetail() = 0;
	virtual void ConnectDetail() {}
	virtual std::string GetName() const = 0;

	void Flush();
	void PrimeForReceive();
	void SendOptionString(TelnetCodes_e aAction, TelnetOptions_e aOption, bool aRequest);

	struct TelnetOptions_s {
		bool Do;
		bool Will;
		bool OutstandingDoDont;
		bool OutstandingWillWont;
	};
	std::array<TelnetOptions_s, 256> mTelnetOptions;

	enum class TelnetStates_e {
		WaitIAC,
		WaitAction,
		WaitOption
	} mTelnetState;
	TelnetCodes_e mTelnetAction;

	bool mEatLF;
	bool mEatNull;
	bool mMapBackspace;
	char mBackspaceChar;
	bool mDisableAutoTerminal;
	std::string mLogFileName;
	bool mLogInitialized;
};

#endif // __CONSOLE_H__
