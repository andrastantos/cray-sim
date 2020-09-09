// Cray-XMP I/O Processor simulator class
#include "sim_iop_con.h"
#include "sim_iop.h"

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

// SimIopConsole_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SimIopConsole_c::SimIopConsole_c(const Configuration_c &aConfig, size_t aConsoleId, SimIopCluster_c &aParent, uint8_t aDev, bool aDisableAutoTerminal) :
	Console_c(
		aConfig,
		mLogger,
		aConfig.get<unsigned short>("Port", aConfig.get<unsigned short>("BasePort", 20000) + aDev),
		!aConfig.get_optional<unsigned short>("Port").is_initialized(),
		aDisableAutoTerminal
	),
	mConsoleId(aConsoleId),
	mLogger(aConfig, "CON", aConsoleId),
	mParent(aParent),
	mDev(aDev)
{
	mLogger.SetParent(aParent.GetLogger());
}

void SimIopConsole_c::GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const {
	aStatus.put("State", mAccepted ? "Conn" : "----");
	if (aLongFormat) {
		std::stringstream Str;
		Str << "<a href=telnet://localhost:" << mPort << ">" << mPort << "</a>";
		aStatus.put("Port", Str.str());
	} else {
		aStatus.put("Port", mPort);
	}
}

std::string SimIopConsole_c::GetName() const {
	std::stringstream Name;
	Name << "CON" << DecPrinter(mConsoleId);
	return Name.str();
}

void SimIopConsole_c::RegisterCommands(CommandHooks_t &aHooks) {}

// SimIopDConsole_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SimIopDConsole_c::HandleTPacket(std::vector<CInt_t> &aPacket) {
	using namespace IosTPacket_n;

	FunctionCodes_e Function = FunctionCodes_e(GetBits(aPacket[0], FunctionRange));
	StatusCodes_e Status = StatusCodes_e(GetBits(aPacket[0], StatusRange));
	uint8_t ByteCnt = uint8_t(GetBits(aPacket[0], ByteCntRange));

	const size_t MaxPayloadSize_c = 5 * sizeof(CInt_t);
	CRAY_ASSERT(ByteCnt <= MaxPayloadSize_c);
	switch (Function) {
		case FunctionCodes_e::PutChars:
			for (size_t i = 0; i < aPacket.size(); ++i) aPacket[i] = SwapBytes(aPacket[i]);
			SendString((char*)(&aPacket[1]), ByteCnt);
			for (size_t i = 0; i < aPacket.size(); ++i) aPacket[i] = SwapBytes(aPacket[i]);
			ReversePacket(aPacket);
			SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
			SetBits(aPacket[0], ByteCntRange, uint64_t(0));
			mParent.SendPacket(aPacket);
		break;
		case FunctionCodes_e::Eol:
			for (size_t i = 0; i < aPacket.size(); ++i) aPacket[i] = SwapBytes(aPacket[i]);
			SendString((char*)(&aPacket[1]), ByteCnt);
			for (size_t i = 0; i < aPacket.size(); ++i) aPacket[i] = SwapBytes(aPacket[i]);
			SendString("\r\n");
			ReversePacket(aPacket);
			SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
			SetBits(aPacket[0], ByteCntRange, uint64_t(0));
			mParent.SendPacket(aPacket);
		break;
		case FunctionCodes_e::Disconnect:
			CRAY_ASSERT(false);
		break;
		case FunctionCodes_e::Connect:
			//Console.SendString("UNICOS TERMINAL\r\n");
			ReversePacket(aPacket);
//			SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::Connect));
			SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
			SetBits(aPacket[0], ByteCntRange, uint64_t(0));
			mParent.SendPacket(aPacket);
		break;
		case FunctionCodes_e::Ioctl:
		{
			// These things seem to correspond nicely to termio.h constants and the termio (or struct termios) structure.
			// See http://man7.org/linux/man-pages/man3/termios.3.html for example for details
			// Or even better, ../termio.txt
			const BitRange_s InputModeRange(0, 15);
			const BitRange_s OutputModeRange(16, 31);
			const BitRange_s ControlModeRange(32, 47);
			const BitRange_s LineDisciplineRange(48, 63);
			uint16_t InputMode = uint16_t(GetBitsReverse(aPacket[1], InputModeRange));
			uint16_t OutputMode = uint16_t(GetBitsReverse(aPacket[1], OutputModeRange));
			uint16_t ControlMode = uint16_t(GetBitsReverse(aPacket[1], ControlModeRange));
			uint16_t LineDiscipline = uint16_t(GetBitsReverse(aPacket[1], LineDisciplineRange));

			// We seem to be getting 0260 in ControlMode which is B0 | CS8 | CREAD
			ReversePacket(aPacket);
			SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
			SetBits(aPacket[0], ByteCntRange, uint64_t(0));
			mParent.SendPacket(aPacket);
		}
		break;
		case FunctionCodes_e::NoTty:
			CRAY_ASSERT(false);
		break;
		case FunctionCodes_e::Logout:
			CRAY_ASSERT(false);
		break;
		default:
			CRAY_ASSERT(false);
		break;
	}
}






bool SimIopDConsole_c::ReceiveHandlerDetail() {
	using namespace IosTPacket_n;

	std::vector<CInt_t> Packet(6);
	std::fill(Packet.begin(), Packet.end(), 0ULL);
	Packet[0] = SetBits(Packet[0], 32, 63, 0x43310054); // Signature
	SetBits(Packet[0], IopNumberRange, uint64_t(mParent.GetId()));
	SetBits(Packet[0], TTyNumberRange, uint64_t(mConsoleId));
	SetBits(Packet[0], FunctionRange, uint64_t(FunctionCodes_e::PutChars));
	SetBits(Packet[0], StatusRange, uint64_t(StatusCodes_e::InputReady));

	char Char = GetChar();

	SetBits(Packet[0], ByteCntRange, uint64_t(1));

	char *Str = (char *)(&Packet[1]);
	Str[0] = Char;

	for (size_t i = 1; i < Packet.size(); ++i) Packet[i] = SwapBytes(Packet[i]);
	mParent.SendPacket(Packet);
	return true;
}



// SimIopEConsole_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SimIopEConsole_c::HandlePacket(std::vector<CInt_t> &aPacket) {
	IopEPacket_c Packet = IopEPacket_c(aPacket);
	HandleIosEPacket(Packet);
}

void SimIopEConsole_c::HandleIosEPacket(IopEPacket_c &aPacket) {
	using namespace IosEZPacket_n;

	RequestCodes_e Request = RequestCodes_e(GetBits(aPacket.Data(0), RequestRange));
	uint16_t CharCount = uint16_t(GetBits(aPacket.Data(0), CharCountRange));
	uint8_t Dev = uint8_t(GetBits(aPacket.Data(0), DevRange));
	uint8_t Flags = uint8_t(GetBits(aPacket.Data(0), FlagsRange));
	mSeqZReceived = uint8_t(GetBits(aPacket.Data(0), SeqRange));
	mSeqZSent = uint8_t(GetBits(aPacket.Data(0), AckRange)); // TODO: how to deal with lost packets? Apparently UNICOS doesn't care about all of our ACKs, and doesn't always increment it's own ACK. In fact, things break if we don't just keep adding 1 to the last ACK-ed packet as our SEQ. So that's what we're doing for now...

	IopEPacket_c Response = mParent.GenerateResponse(aPacket);

	switch (Request) {
		case RequestCodes_e::ZP_DATA:
	//		CRAY_ASSERT(ReadyToSend());
			for (int i = 0; i < (CharCount + 7) / 8; ++i) {
				aPacket.Data(i + 1) = SwapBytes(aPacket.Data(i + 1));
			}
			SendString((char*)(&aPacket.Data(1)), CharCount);

			if (mDisableAutoTerminal) SendTransmitAck();
		break;
		case RequestCodes_e::ZP_DIS:
			if (CharCount != 0) {
				for (int i = 0; i < (CharCount + 7) / 8; ++i) {
					aPacket.Data(i + 1) = SwapBytes(aPacket.Data(i + 1));
				}
				SendString((char*)(&aPacket.Data(1)), CharCount);
	//			SendString("\r\n---- disconnected ----\r\n");
				if (mDisableAutoTerminal) SendTransmitAck();
			}
		break;
		case RequestCodes_e::ZP_CON_RQ:
	//		SendString("**** CONNECT ****\r\n");
			Response.SetSize(1);
			SetBitsInPlace(Response.Data(0), TypeRange, 'z'); // Mark packet as response
			SetBitsInPlace(Response.Data(0), FlagsRange, Flags); // TODO: what to set this to?
			SetBitsInPlace(Response.Data(0), RequestRange, uint64_t(Request));
			SetBitsInPlace(Response.Data(0), CharCountRange, 0);
			SetBitsInPlace(Response.Data(0), DevRange, Dev);
			SetBitsInPlace(Response.Data(0), SeqRange, mSeqZSent + 1);
			SetBitsInPlace(Response.Data(0), AckRange, mSeqZReceived);
			mParent.SendPacket(Response);
		break;
		case RequestCodes_e::ZP_CON_RP:
			Response.SetSize(1);
			SetBitsInPlace(Response.Data(0), TypeRange, 'z'); // Mark packet as response
			SetBitsInPlace(Response.Data(0), FlagsRange, Flags); // TODO: what to set this to?
			SetBitsInPlace(Response.Data(0), RequestRange, uint64_t(Request));
			SetBitsInPlace(Response.Data(0), CharCountRange, 0);
			SetBitsInPlace(Response.Data(0), DevRange, Dev);
			SetBitsInPlace(Response.Data(0), SeqRange, mSeqZSent + 1);
			SetBitsInPlace(Response.Data(0), AckRange, mSeqZReceived);
			mParent.SendPacket(Response);
		break;
		default:
			CRAY_ASSERT(false);
	}
}


bool SimIopEConsole_c::ReceiveHandlerDetail() {
	using namespace IosEZPacket_n;

	// TODO: send as many chars in one message as possible
	IopEPacket_c Response;
	Response.SetSize(2);

	std::vector<uint8_t> Str;
	const size_t MaxSize = 8;
	for (size_t Idx = 0; Idx < MaxSize - 1; ++Idx) {
		if (HasInput()) {
			Str.push_back(GetChar());
			for (size_t PollCnt = 0; PollCnt < 10; ++PollCnt) {
				Poll();
				if (HasInput()) break;
			}
		} else {
			break;
		}
	}
	//for (size_t Idx=0;Idx<Str.size();++Idx) std::cout << " " << HexPrinter(int(Str[Idx]), 2) << std::flush;
	//std::cout << "{" << Str << "}" << std::flush;

	//Response.SetKernelId
	//Response.SetSource
	//Response.SetCluster
	//Response.SetProcess
	//Response.SetLogicalPath
	Response.SetFlags(IopEPacket_c::cFlagAck);

	SetBitsInPlace(Response.Data(0), TypeRange, 'z'); // Mark packet as response
	SetBitsInPlace(Response.Data(0), FlagsRange, cFlagConsole); // TODO: what to set this to?
	SetBitsInPlace(Response.Data(0), RequestRange, uint64_t(RequestCodes_e::ZP_DATA));
	SetBitsInPlace(Response.Data(0), CharCountRange, Str.size());
	SetBitsInPlace(Response.Data(0), DevRange, mDev);
	SetBitsInPlace(Response.Data(0), SeqRange, mSeqZSent + 1);
	SetBitsInPlace(Response.Data(0), AckRange, mSeqZReceived);
	//SetBitsInPlace(Response.Data(1), BitRange_s(56, 63), Char);
	memcpy(&Response.Data(1), &Str[0], Str.size());
	Response.Data(1) = SwapBytes(Response.Data(1));
	mParent.SendPacket(Response);
	return true;
}

void SimIopEConsole_c::SendHandlerDetail() {
	if (mDisableAutoTerminal) return;
	if (mTransmitQueue.empty()) SendTransmitAck();
}

void SimIopEConsole_c::SendTransmitAck() {
	using namespace IosEZPacket_n;

	IopEPacket_c Response = mParent.GenerateResponse();

	Response.SetSize(1);
	SetBitsInPlace(Response.Data(0), TypeRange, 'z'); // Mark packet as response
	SetBitsInPlace(Response.Data(0), FlagsRange, cFlagAck); // TODO: what to set this to?
	SetBitsInPlace(Response.Data(0), RequestRange, uint64_t(RequestCodes_e::ZP_DATA));
	SetBitsInPlace(Response.Data(0), CharCountRange, 0);
	SetBitsInPlace(Response.Data(0), DevRange, mDev);
	SetBitsInPlace(Response.Data(0), SeqRange, mSeqZSent + 1);
	SetBitsInPlace(Response.Data(0), AckRange, mSeqZReceived);
	mParent.SendPacket(Response);
}

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif
