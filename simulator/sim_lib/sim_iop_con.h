// TODO: there are at least two things to figure out:
// - How to specify the mDev member. Right now it's just consequtive, but that's probably wrong. It als impacts how SimIopCluster_c should select the right terminal, which is also strictly index-based
// - Now the sequence codes are generated per terminal, not globally: is that good, bad, indifferent?

#ifndef __SIM_IOP_CON_H__
#define __SIM_IOP_CON_H__

#include <boost/asio.hpp>
#include "utils.h"
#include "cray_types.h"
#include "cray_logger.h"
#include "cray_channels.h"
#include "debug_events.h"
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/timer/timer.hpp>
#include "config_file.h"
#include "ui.h"
#include "cray_mainframe.h"
#include <queue>
#include "console.h"

class SimIopConsole_c: public Console_c {
public:
	SimIopConsole_c(const Configuration_c &aConfig, size_t aConsoleId, class SimIopCluster_c &aParent, uint8_t aDev, bool aDisableAutoTerminal);
	void GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const;
	virtual std::string GetName() const override;
	void RegisterCommands(CommandHooks_t &aHooks);
	virtual void HandlePacket(std::vector<CInt_t> &aPacket) = 0;
protected:
	size_t mConsoleId;
	mutable CLogger_c mLogger;
	SimIopCluster_c &mParent;
	uint8_t mDev;
};





namespace IosTPacket_n {
	enum class FunctionCodes_e {
		PutChars = 1,
		Eol = 2,
		Disconnect = 3,
		Connect = 4,
		Ioctl = 5,
		NoTty = 6,
		Logout = 7
	};
	enum class StatusCodes_e {
		OK = 1,
		InputReady = 2,
		Eol = 3,
		Disconnect = 4,
		Connect = 5
	};

	const BitRange_s IopNumberRange(24, 31);
	const BitRange_s TTyNumberRange(16, 23);
	const BitRange_s FunctionRange(12, 15);
	const BitRange_s StatusRange(8, 11);
	const BitRange_s ByteCntRange(0, 7);
};


class SimIopDConsole_c : public SimIopConsole_c {
public:
	SimIopDConsole_c(const Configuration_c &aConfig, size_t aConsoleId, class SimIopCluster_c &aParent, uint8_t aDev, bool aDisableAutoTerminal) : SimIopConsole_c(aConfig, aConsoleId, aParent, aDev, aDisableAutoTerminal) {}
	virtual void HandlePacket(std::vector<CInt_t> &aPacket) override { HandleTPacket(aPacket); }
protected:
	void HandleTPacket(std::vector<CInt_t> &aPacket);

	virtual bool ReceiveHandlerDetail() override;
	virtual void SendHandlerDetail() override {}
	virtual void ResetDetail() override {}
};


namespace IosEZPacket_n {
	const BitRange_s TypeRange(56, 63);
	const BitRange_s FlagsRange(48, 55);
	const BitRange_s RequestRange(40, 47);
	const BitRange_s ResponseRange(32, 39);
	const BitRange_s CharCountRange(23, 31); // character count
	const BitRange_s DevRange(16, 22);  // device identification
	const BitRange_s SeqRange(8, 15); // sequence number of this packet
	const BitRange_s AckRange(0, 7); // seq. number of last packet received

	enum class RequestCodes_e {
		ZP_CON_RQ = 1, // connect request
		ZP_CON_RP = 2, // connect reply
		ZP_DIS = 3, // disconnect
		ZP_DATA = 4, // data with piggybacked acknowledgement
		ZP_WINCH = 5  // window size change information
	};

	const uint8_t cFlagCanWinCh = 0x01; // will support ZP_WINCH
	const uint8_t cFlagConsole = 0x02; // from system console
	const uint8_t cFlagLog = 0x04; // log message flag
	const uint8_t cFlagAck = 0x08; // acknowledge previously sent packet
	const uint8_t cFlagHeartBeat = 0x10; // connection heartbeat

	enum class FunctionCodes_e {
		PutChars = 1,
		Eol = 2,
		Disconnect = 3,
		Connect = 4,
		Ioctl = 5,
		NoTty = 6,
		Logout = 7
	};
	enum class StatusCodes_e {
		OK = 1,
		InputReady = 2,
		Eol = 3,
		Disconnect = 4,
		Connect = 5
	};

	const BitRange_s IopNumberRange(24, 31);
	const BitRange_s TTyNumberRange(16, 23);
	const BitRange_s FunctionRange(12, 15);
	const BitRange_s StatusRange(8, 11);
	const BitRange_s ByteCntRange(0, 7);

};


class SimIopEConsole_c : public SimIopConsole_c {
public:
	SimIopEConsole_c(const Configuration_c &aConfig, size_t aConsoleId, class SimIopCluster_c &aParent, uint8_t aDev, bool aDisableAutoTerminal) : SimIopConsole_c(aConfig, aConsoleId, aParent, aDev, aDisableAutoTerminal) {}
	virtual void HandlePacket(std::vector<CInt_t> &aPacket) override;
protected:
	uint8_t mSeqZSent;
	uint8_t mSeqZReceived;
	bool mHadEscape;
	std::string mReceivedStr;

	void HandleIosEPacket(class IopEPacket_c &aPacket);
	virtual bool ReceiveHandlerDetail() override;
	virtual void SendHandlerDetail() override;
	virtual void ResetDetail() override {
		mSeqZSent = 0;
		mSeqZReceived = 0;
	}

	void SendTransmitAck();
};

#endif // __SIM_IOP_CON_H__
