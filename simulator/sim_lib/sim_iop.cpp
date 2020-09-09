// Cray-XMP I/O Processor simulator class

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
#include <math.h>
#include <memory.h>
#include <string.h>
#include <algorithm>
#include "utils.h"
#include "sim_iop.h"
#include "cray_mainframe.h"

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

bool IopEPacket_c::IsValid() const {
	if (mPacket[0] != mPacket[mPacket.size() - 1]) {
		std::cout << "mPacket[0] != mPacket[mPacket.size() - 1]" << std::endl;
		return false;
	}
	if (GetMagic() != cMagic) {
		std::cout << "GetMagic() != cMagic" << std::endl;
		return false;
	}
	if (GetLength() != mPacket.size() - 1) {
		std::cout << "GetLength() != mPacket.size() - 1" << std::endl;
		return false;
	}
	return true;
}

std::vector<CInt_t> &IopEPacket_c::Finalize() {
	SetMagic(cMagic);
	//		if (mPacket.size() != 1) {
	SetLength(uint16_t(mPacket.size()));
	SetFlags(cFlagAck);
	mPacket.push_back(mPacket[0]);
	//		} else {
	//			SetLength(0);
	//		}
	return			mPacket;
}

std::unique_ptr<SimIopCluster_c::ChannelIToC_c> SimIopCluster_c::CreateIToCChannel(SimIopCluster_c *aThis, const Configuration_c &aConfig) {
	return std::make_unique<ChannelIToC_c>(aConfig.get_child_safe("I2C"), aConfig.get_child_safe("I2C").get<size_t>("Channel"), aThis->GetMainframe(), *aThis);
}

std::unique_ptr<SimIopCluster_c::ChannelCToI_c> SimIopCluster_c::CreateCToIChannel(SimIopCluster_c *aThis, const Configuration_c &aConfig) {
	return std::make_unique<ChannelCToI_c>(aConfig.get_child_safe("C2I"), aConfig.get_child_safe("C2I").get<size_t>("Channel"), aThis->GetMainframe(), *aThis);
}

SimIopCluster_c::SimIopCluster_c(const Configuration_c &aConfig, size_t aIopId, class Mainframe_c &aMainframe, IToCChannelCreator_t aIToCChannelCreator, CToIChannelCreator_t aCToIChannelCreator):
	mMainframe(&aMainframe),
	mLogger(aConfig,"IOP",aIopId), 
	mIopId(aIopId),
	mState(States_e::Reset),
	mMasterCluster(aConfig.get<bool>("MasterCluster", true)),
	mDontTouchExchangePacket(aConfig.get<bool>("DontTouchExchangePacket", false)),
	mPacketsReceived(0),
	mPacketsSent(0)
{
	mLogger.SetParent(aMainframe.GetLogger());
	mIToCChannel = aIToCChannelCreator(this, aConfig);
	mCToIChannel = aCToIChannelCreator(this, aConfig);

	try {
		std::string IopRevision = aConfig.get<std::string>("IopRevision", "IOSE");
		if (IopRevision == "IOSD") {
			mIopRevision = IopRevision_e::IOSD;
		}
		else if (IopRevision == "IOSE") {
			mIopRevision = IopRevision_e::IOSE;
		}
		else {
			throw Generic_x() << "Unknown IOP revision: " << IopRevision;
		}

		for (const auto &DiskConfig : aConfig.get_child_safe("Disks")) {
			std::string DiskId = DiskConfig.first;
			std::unique_ptr<SimIopDisk_c> Disk = std::make_unique<SimIopDisk_c>(DiskConfig.second, DiskId, *this);
			mDisks.push_back(std::move(Disk));
		}
		for (const auto &ConsoleConfig : aConfig.get_child_safe("Consoles")) {
			if (ConsoleConfig.first == "Default") {
				mDefaultConsoleConfig = ConsoleConfig.second;
			} else {
				size_t ConsoleId = FromString<size_t>(ConsoleConfig.first);
				if (mIopRevision == IopRevision_e::IOSE) {
					std::unique_ptr<SimIopConsole_c> Console = std::make_unique<SimIopEConsole_c>(
						ConsoleConfig.second,
						ConsoleId,
						*this,
						uint8_t(mConsoles.size()),
						aMainframe.DoDisableAutoTerminal()
					);
					mConsoles.push_back(std::move(Console));
				} else {
					std::unique_ptr<SimIopConsole_c> Console = std::make_unique<SimIopDConsole_c>(
						ConsoleConfig.second,
						ConsoleId,
						*this,
						uint8_t(mConsoles.size()),
						aMainframe.DoDisableAutoTerminal()
					);
					mConsoles.push_back(std::move(Console));
				}
			}
		}
		for (const auto &EthConfig : aConfig.get_child_safe("EthInterfaces")) {
			std::string Name = EthConfig.first;
			std::unique_ptr<SimIopEth_c> Eth = std::make_unique<SimIopEth_c>(EthConfig.second, *this, Name);
			mEthInterfaces.push_back(std::move(Eth));
		}
		mMainframe->SetChannel(mIToCChannel->GetChannelId() - 8, *mIToCChannel);
		mMainframe->SetChannel(mCToIChannel->GetChannelId() - 8, *mCToIChannel);

		mKernelFileName = aConfig.get_optional<std::string>("KernelFileName");
		mKernelPreamble = aConfig.get<size_t>("KernelPreamble", 8 * 8);
		mParameterFileName = aConfig.get_optional<std::string>("ParameterFileName");
		mRamFsFileName = aConfig.get_optional<std::string>("RamFsFileName");
		mBootCluster = aConfig.get<uint32_t>("BootCluster", 0);
		mBootChannel = aConfig.get<uint32_t>("BootChannel", 0);
		mLogicalMainframeNumber = aConfig.get<uint8_t>("LogicalMainframeNumber", 0);
		mOWSProtocolLevel = aConfig.get<uint8_t>("OWSProtocolLevel", 0);
		mRunLevel = aConfig.get_optional<std::string>("RunLevel");
		if (mRunLevel.is_initialized()) {
			if (mRunLevel.get().length() != 1) throw Generic_x() << "RunLevel must be a single character";
		}
		if (aConfig.get_optional<std::string>("Peripherals").is_initialized() || (mIopRevision == IopRevision_e::IOSD)) {
			mIopPeripheral = std::make_unique<SimIopPeripheral_c>(aConfig.get_child_safe("Peripherals"), *this);
		}
	}
	catch (...) {
		throw;
	}
}

void SimIopCluster_c::GetPeripheralStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const {
	switch (aFilter) {
		case PeripheralType_e::Disk:
			for (auto &Disk : mDisks) {
				StatusReport_c Status;
				Disk->GetStatus(Status, aElapsedTime, aLongFormat);
				if (!Status.empty()) {
					aStatus.put_child(Disk->GetName(), Status);
				}
			}
		break;
		case PeripheralType_e::Console:
			for (auto &Console : mConsoles) {
				StatusReport_c Status;
				Console->GetStatus(Status, aElapsedTime, aLongFormat);
				if (!Status.empty()) {
					aStatus.put_child(Console->GetName(), Status);
				}
			}
		break;
		case PeripheralType_e::Network:
			for (auto &EthInterface : mEthInterfaces) {
				StatusReport_c Status;
				EthInterface->GetStatus(Status, aElapsedTime, aLongFormat);
				if (!Status.empty()) {
					aStatus.put_child(EthInterface->GetName(), Status);
				}
			}
			aStatus.put(aLongFormat ? "MF Packets Received" : "MFR", mPacketsReceived);
			aStatus.put(aLongFormat ? "MF Packets Sent" : "MFS", mPacketsSent);
			aStatus.put(aLongFormat ? "MF Queue Lenngth" : "QL", mIToCChannel->GetQueueLength());
		break;
		default:
		break;
	}
}

void SimIopCluster_c::RegisterCommands(CommandHooks_t &aHooks) {
	for (auto &Disk : mDisks) {
		Disk->RegisterCommands(aHooks);
	}
	for (auto &Console : mConsoles) {
		Console->RegisterCommands(aHooks);
	}
	for (auto &Eth : mEthInterfaces) {
		Eth->RegisterCommands(aHooks);
	}
}

std::string SimIopCluster_c::GetName() const {
	std::stringstream Name;
	Name << "ICL" << DecPrinter(mIopId);
	return Name.str();
}

std::string SimIopCluster_c::GetLongName() const {
	std::stringstream Name;
	Name << "IOP Cluster " << DecPrinter(mIopId);
	return Name.str();
}

void SimIopCluster_c::MasterClear() {
	for (auto &Disk : mDisks) {
		Disk->MasterClear();
	}
	for (auto &Console : mConsoles) {
		Console->Reset();
	}
	mState = States_e::Reset;
}

IopEPacket_c SimIopCluster_c::GenerateResponse(IopEPacket_c &aRequest) const {
	IopEPacket_c Response(aRequest);
	Response.SetSize(0);
	Response.SetAckSeq(mSeqReceived);
	Response.SetSeq(mSeqSent);
	Response.SetFlags(IopEPacket_c::cFlagAck);
	return Response;
}

IopEPacket_c SimIopCluster_c::GenerateResponse() const {
	IopEPacket_c Response;
	Response.SetSize(0);
	Response.SetAckSeq(mSeqReceived);
	Response.SetSeq(mSeqSent);
	Response.SetFlags(IopEPacket_c::cFlagAck); // TODO: do we set this here?
	return Response;
}

void SimIopCluster_c::SendPacket(IopEPacket_c &aPacket) {
//	CRAY_ASSERT(!aReplace);
	aPacket.SetAckSeq(mSeqReceived);
	aPacket.SetSeq(mSeqSent);
	mIToCChannel->QueuePacket(aPacket.Finalize());
	++mSeqSent;
}

void SimIopCluster_c::SendPacket(std::vector<CInt_t> &aPacket) {
	mIToCChannel->QueuePacket(aPacket);
}

MachineTypes_e SimIopCluster_c::GetMachineType() const
{
	return mMainframe->GetMachineType();
}

void SimIopCluster_c::HandleIosEOPacket(IopEPacket_c &aPacket) {
	const BitRange_s TypeRange(56, 63);
	const BitRange_s RunLevelRange(48, 55);
	const BitRange_s RequestRange(40, 47);
	const BitRange_s ResponseRange(32, 39);
	const BitRange_s ClusterRange(24, 31);
	const BitRange_s IopRange(16, 23);

	enum class RequestCodes_e {
		OP_BOOT = 1,    // boot the specified IOP
		OP_DOWN = 2,    // mark the specified IOP as down
		OP_UP = 3,    // mark the specified IOP as up
		OP_KILL = 4,    // master clear and down the specified IOP
		OP_OBIT = 5,    // obituary notice for the specified IOP
		OP_ALIVE = 6,    // existence notice for the specified IOP
		OP_CLOCK = 7,    // send current clock
		OP_PANIC = 8,    // CPU panic message
		OP_DUMP = 9,    // dump given IOP
		OP_ISTOP = 10,   // inform CPU of IOP stop
		OP_IRESTART = 11,   // inform CPU of IOP restart
		OP_CHGLVL = 12,   // Unicos run level changed notice
		OP_RUNLVL = 13,   // change the Unicos run level
		OP_SYSHALT = 14,   // halt the Unicos system
		OP_IABORT = 15,   // inform CPU of IOP abort
		OP_VITAL = 16,   // check health of UNICOS
		OP_FPANIC = 17,   // force a panic
		OP_PREPANIC = 18,   // CPU started panic, may flush
		OP_ERRLOG = 19   // make errlog entry
	};

	const uint8_t cLogicalPath = 7;

	CRAY_ASSERT(aPacket.GetLogicalPath() == cLogicalPath);
	RequestCodes_e Request = RequestCodes_e(GetBits(aPacket.Data(0), RequestRange));

	IopEPacket_c Response = GenerateResponse(aPacket);

	switch (Request) {
		case RequestCodes_e::OP_CLOCK: {
			Response.SetSize(8);
			SetBitsInPlace(Response.Data(0), TypeRange, 'o'); // Mark packet as response
			SetBitsInPlace(Response.Data(0), RequestRange, uint64_t(RequestCodes_e::OP_CLOCK));
			time_t NowTime = time(0);
			struct tm *Now = localtime(&NowTime);
			Response.Data(1) = CInt_t(0);
			Response.Data(2) = CInt_t(Now->tm_mon+1); // Month
			Response.Data(3) = CInt_t(Now->tm_mday); // Day
			Response.Data(4) = CInt_t(Now->tm_year + 1900); // Year
			Response.Data(5) = CInt_t(Now->tm_hour); // Hour
			Response.Data(6) = CInt_t(Now->tm_min); // Minute
			Response.Data(7) = CInt_t(Now->tm_sec); // Second
			SendPacket(Response);
		}
		break;
		case RequestCodes_e::OP_PANIC: {
			MakeConsoles(0);
			mConsoles[0]->SendString("================== KERNEL PANIC ================\r\n");
			size_t CharCount = (aPacket.GetLength() - 2) * 8;
			for (size_t i = 0; i < (CharCount + 7) / 8; ++i) {
				aPacket.Data(i + 2) = SwapBytes(aPacket.Data(i + 2));
			}
			char *Str = (char*)(&aPacket.Data(2));
			mConsoles[0]->SendString(Str, std::min(size_t(CharCount), strlen(Str)));
			mConsoles[0]->SendString("\r\n================================================\r\n");
		}
		break;
		default:
			CRAY_ASSERT(false);
	}
}

void SimIopCluster_c::HandleIosEPPacket(IopEPacket_c &aPacket) {
	// Word 0
	const BitRange_s TypeRange(56, 63);
	const BitRange_s ClusterRange(48, 55);
	const BitRange_s IopRange(40, 47);
	const BitRange_s ChannelRange(32, 39);
	const BitRange_s UnitRange(24, 31);
	const BitRange_s RequestRange(16, 23);
	const BitRange_s ResponseRange(8, 15);
	// OPEN
	///////////////////////////////////
	// Word 1
	const BitRange_s Open_HwTypeRange(32, 63);
	const BitRange_s Open_PacketAddrRange(0, 31);
	// Word 2
	const BitRange_s Open_InReqQueueRange(32, 63);
	const BitRange_s Open_InRspQueueRange(0, 31);
	// Word 3
	const BitRange_s Open_OutReqQueueRange(32, 63);
	const BitRange_s Open_OutRspQueueRange(0, 31);
	// CLOSE
	///////////////////////////////////
	// Word 1
	const BitRange_s Close_PacketAddrRange(0, 31);
	// I/O
	///////////////////////////////////
	// Word 1
	const BitRange_s IO_AalRange(60, 63);
	const BitRange_s IO_FlagsRange(52, 59);
	const BitRange_s IO_LengthMtuRange(32, 51);
	const BitRange_s IO_VpiVciRange(0, 31);
	// Word 2
	const BitRange_s IO_PacketAddrRange(32, 63);
	const BitRange_s IO_SeqAddrRange(0, 31);

	enum class Flags_e {
		PP_TX_VPCI = 0x01,  // vpi/vci is used for transmitting data
		PP_RX_VPCI = 0x02,  // vpi/vci is used for receiving data
	};
	enum class Request_e {
		PP_OPEN       = 0x01,  // open function
		PP_CLOSE      = 0x02,  // close function
		PP_INPUT      = 0x03,  // Input function
		PP_OUTPUT     = 0x04,  // Output function
		PP_ACTIVATE   = 0x05,  // Activate a VCI/VPI pair on the FORE card
		PP_DEACTIVATE = 0x06,  // Deactivate a VCI/VPI pair on the FORE card
		PP_FSTATS     = 0x07,  // Dump FORE statistics to addr <see ioctl>
		PP_DSTATS     = 0x08,  // Dump driver statistics to addr <see ioctl>
		PP_TRACE      = 0x09,  // Dump standard trace buffer
		PP_ATRACE     = 0x0a,  // Dump auxiliary trace buffer
		PP_GQUEUES    = 0x0b,  // Get a copy of all the IOS queue structures
		PP_GOC3CONF   = 0x0c,  // Get OC3 configuration from SUNI chip
		PP_SOC3CONF   = 0x0d,  // Set OC3 configuration on SUNI chip
		PP_DEBUG      = 0x20,  // Toggle IOS debug mode (prints traces, etc)
		PP_CLRTRACE   = 0x21   // Clear the IOS trace and aux trace
	};
	enum class Response_e {
		PPST_OK = 0x00, // ok status
		PPST_HDW = 0x01, // hardware error
		PPST_PROER = 0x02, // protocol error
		PPST_ILCHN = 0x03, // illegal channel
		PPST_ILFNC = 0x04, // illegal function
		PPST_RTO = 0x05, // read timeout
		PPST_WTO = 0x06, // write timeout
		PPST_ILDRV = 0x07, // illegal driver
		PPST_INVAL = 0x08, // invalid argument		(010)
		PPST_FAILED = 0x09, // request failed		(011)
		PPST_TIMER = 0x0a, // Funcion timeout		(012) (0x0a)
		PPST_OVRFLW = 0x0b, // Queue overflow error		(013) (0x0b)
		PPST_CANCEL = 0x0c, // I/O request cancelled	(014) (0x0c)
		PPST_BUSY = 0x0d, // Packet request already in progress (busy)
		PPST_OPENOK = 0xf0	// Open completed succesfully 
	};

	Request_e Request = Request_e(GetBits(aPacket.Data(0), RequestRange));
	switch (Request) {
		case Request_e::PP_OPEN:
		{
			IopEPacket_c Response = GenerateResponse(aPacket);
			Response.SetSize(4);
			for (size_t Idx = 0; Idx < size_t(aPacket.GetLength()-1); ++Idx) {
				Response.Data(Idx) = aPacket.Data(Idx);
			}
			mLogger << setloglevel(LogLevel_IoActivity) << "ATM open" << std::endl;
			//DumpPacket(mLogger, (CAddr_t(GetBits(aPacket.Data(2), IO_PacketAddrRange))), *mMainframe, "packet");

			SetBitsInPlace(Response.Data(0), TypeRange, 'p'); // Mark packet as response
			SetBitsInPlace(Response.Data(0), ResponseRange, uint64_t(Response_e::PPST_OPENOK));
			SendPacket(Response);
		}
		break;
		case Request_e::PP_INPUT:
		{
			IopEPacket_c Response = GenerateResponse(aPacket);
			Response.SetSize(4);
			for (size_t Idx = 0; Idx < size_t(aPacket.GetLength()-1); ++Idx) {
				Response.Data(Idx) = aPacket.Data(Idx);
			}
			mLogger << setloglevel(LogLevel_IoActivity) << "ATM input " <<
				"Length: " << DecPrinter((GetBits(aPacket.Data(1), IO_LengthMtuRange))) << " " <<
				"Vpi/Vci: " << HexPrinter((CAddr_t(GetBits(aPacket.Data(1), IO_VpiVciRange)))) << " " <<
				"Seq: " << HexPrinter((CAddr_t(GetBits(aPacket.Data(2), IO_SeqAddrRange)))) << " " << std::endl;
			//DumpPacket(mLogger, (CAddr_t(GetBits(aPacket.Data(1), Open_PacketAddrRange))), *mMainframe, "packet");

			SetBitsInPlace(Response.Data(0), TypeRange, 'p'); // Mark packet as response
			SetBitsInPlace(Response.Data(0), ResponseRange, uint64_t(Response_e::PPST_OK));
			SendPacket(Response);
		}
		break;
		case Request_e::PP_OUTPUT:
		{
			IopEPacket_c Response = GenerateResponse(aPacket);
			Response.SetSize(4);
			for (size_t Idx = 0; Idx < size_t(aPacket.GetLength()-1); ++Idx) {
				Response.Data(Idx) = aPacket.Data(Idx);
			}
			mLogger << setloglevel(LogLevel_IoActivity) << "ATM output " <<
				"Length: " << DecPrinter((GetBits(aPacket.Data(1), IO_LengthMtuRange))) << " " <<
				"Vpi/Vci: " << HexPrinter((CAddr_t(GetBits(aPacket.Data(1), IO_VpiVciRange)))) << " " <<
				"Seq: " << HexPrinter((CAddr_t(GetBits(aPacket.Data(2), IO_SeqAddrRange)))) << " " << std::endl;
			//DumpPacket(mLogger, (CAddr_t(GetBits(aPacket.Data(1), Open_PacketAddrRange))), *mMainframe, "packet");

			SetBitsInPlace(Response.Data(0), TypeRange, 'p'); // Mark packet as response
			SetBitsInPlace(Response.Data(0), ResponseRange, uint64_t(Response_e::PPST_OK));
			SendPacket(Response);
		}
		break;
		case Request_e::PP_CLOSE:
		{
			IopEPacket_c Response = GenerateResponse(aPacket);
			Response.SetSize(4);
			for (size_t Idx = 0; Idx < size_t(aPacket.GetLength()-1); ++Idx) {
				Response.Data(Idx) = aPacket.Data(Idx);
			}
			mLogger << setloglevel(LogLevel_IoActivity) << "ATM close" << std::endl;
			//DumpPacket(mLogger, (CAddr_t(GetBits(aPacket.Data(2), IO_PacketAddrRange))), *mMainframe, "packet");

			SetBitsInPlace(Response.Data(0), TypeRange, 'p'); // Mark packet as response
			SetBitsInPlace(Response.Data(0), ResponseRange, uint64_t(Response_e::PPST_OK));
			SendPacket(Response);
		}
		break;
		default:
			CRAY_ASSERT(false);
		break;
	}
}

void SimIopCluster_c::HandleIosEDPacket(IopEPacket_c &aPacket) {
	// Word -1
	// EIOP number is stuffed into 'process' GetProcess
	// Word 0
	const BitRange_s TypeRange(56, 63);
	const BitRange_s RequestCodeRange(40, 47);
	const BitRange_s ResponseCodeRange(32, 39);
	const BitRange_s DeviceTypeRange(25, 31);
	const BitRange_s UnitRange(21, 24);
	const BitRange_s ChannelRange(16, 20);
	const BitRange_s RawFunctionParameterRange(0, 15);
	// Word 1
	const BitRange_s IndexRange(32, 63);
	const BitRange_s CurrentBpRange(0, 31);
	// Word 2
	const BitRange_s CylinderRange(32, 63);
	const BitRange_s HeadRange(31, 31);
	const BitRange_s SectorRange(30, 30);
	const BitRange_s SectorCntRange(0, 15);
	// Word 3
	const BitRange_s ErrorRecoveryInProgressRange(59, 59);
	const BitRange_s NextValidPacketRange(58, 58);
	const BitRange_s EnableSpiralRange(57, 57);
	const BitRange_s DisableZeroLatencyRange(56, 56);
	const BitRange_s LogicalSectorSizeRange(32, 47);
	const BitRange_s ChannelBufferDesciptorRange(16, 31);
	const BitRange_s AddressIncrementRange(0, 15);
	// Word 4
	const BitRange_s TargetMemoryTypeRange(48, 50);
	const BitRange_s TargetMemoryAddrRange(0, 47);
	// Word 5
	const BitRange_s ChanneBufferOffsetRange(32, 47);
	const BitRange_s StreamMasksRange(0, 15);

	// Response only fields
	// Word 6
	const BitRange_s GeneralStatusRange(48, 63);
	const BitRange_s M1StatusRange(32, 47);
	const BitRange_s DriveStatusRange(16, 31);
	const BitRange_s ControllerStatus(0, 15);
	// Word 7
	const BitRange_s CylinderInErrorRange(32, 63);
	const BitRange_s HeadInErrorRange(31, 31);
	const BitRange_s SectorInErrorRange(30, 30);
	const BitRange_s SiralOffsetRange(0, 15);
	// Word 8...24
	const BitRange_s WriteBehindCylinderRange(48, 63);
	const BitRange_s WriteBehindHeadRange(32, 47);
	const BitRange_s WriteBehindSectorRange(16, 31);
	const BitRange_s WriteBehindSegmentNumberRange(9, 15);
	const BitRange_s WriteBehindRecoveredFlagRange(8, 8);
	const BitRange_s WriteBehindSectorsInErrorRange(0, 7);
	// 25..70
	// There is more status here up to 70 

	enum class RequestCodes_e {
		// Note: there's quite a bit of repetition here in the list. From what I can tell, this is mostly about more reable names, but same meaning.
		DRQ_ECHO		= 001,		// echo
		DRQ_READ		= 002,		// read data
		DRQ_WRITE		= 003,		// write data
		DRQ_WRITEB		= 004,		// write behind
		DRQ_RID			= 033,		// read ID
		DRQ_RDAECC		= 041,		// read data and ecc - dd6x
		DRQ_READ_TARGET = 042,
		DRQ_WDAECCTP	= 046,		// write data, ecc, toggle parity - dd6x
		DRQ_WDAECC		= 047,		// write data and ecc - dd6x
		DRQ_WID			= 052,		// write ID
		DRQ_WDID		= 053,		// write defective ID
		DRQ_WZECC		= 055,		// write with zero ecc

		DRQ_WRITE_BEHIND				= 004,
		DRQ_CONFIGURE_UP				= 011,
		DRQ_CONFIGURE_DOWN				= 012,
		DRQ_RETURN_STATS				= 021,
		DRQ_READ_SECTOR					= 030,
		DRQ_READ_SECTOR_FROM_BUFFER		= 031,
		DRQ_READ_SECTOR_NO_HW_RA		= 032,
		DRQ_READ_ID						= 033,
		DRQ_READ_ABSOLUTE				= 034,
		DRQ_READ_BUFFER					= 035,
		DRQ_READ_ECC					= 036,
		DRQ_READ_VECTORS				= 037,
		DRQ_READ_TRACK_HEADER			= 040,
		DRQ_WRITE_DATA					= 050,
		DRQ_WRITE_THRU					= 051,
		DRQ_WRITE_ID					= 052,
		DRQ_WRITE_DEFECTIVE_ID			= 053,
		DRQ_WRITE_BUFFER				= 054,
		DRQ_WRITE_DATA_ZERO_ECC			= 055,
		DRQ_WRITE_TRACK_HEADER			= 056,
		DRQ_DIAGNOSTIC					= 060,
		DRQ_RESET						= 061,
		DRQ_CLEAR_FAULTS				= 062,
		DRQ_RTZ							= 063,
		DRQ_SELECT						= 064,
		DRQ_DESELECT					= 065,
		DRQ_DESELECT_OPP_SELECT			= 066,
		DRQ_SEEK						= 067,
		DRQ_SELECT_OFFSET				= 070,
		DRQ_SELECT_STROBE				= 071,
		DRQ_SELECT_HEAD					= 072,
		DRQ_WRITE_STREAMS               = 076,
		DRQ_LOAD_FORMAT					= 077,
		DRQ_RETURN_GENERAL_STATUS		= 0174,
		DRQ_RETURN_SELECT_STATUS		= 0175,
		DRQ_ACCUMULATE_STATUS			= 0176,
		DRQ_RETURN_ACCUMULATED_STATUS	= 0177
	};
	enum class ResponseCodes_e {
		DRS_OK			= 0000,		// ok
		DRS_ERR_MIN		= 0010,
		DRS_ERR			= 0010,		// error
		DRS_DERR		= 0011,		// data error
		DRS_ERR_MAX		= 0077,
		DRS_RERR		= 0301,		// request error
		DRS_PERR		= 0303,		// parameter error
		DRS_NVPERR		= 0322,		// next valid packet error
		
		DRS_RECOVERED				= 0001,
		DRS_CORRECTED				= 0002,
		DRS_SINGLE_BIT				= 0003,
		DRS_IO_WAIT					= 0004,
		DRS_FIRST_FATAL_ERROR_CODE	= 0010,
		DRS_IO_ERROR				= 0010,
		DRS_TRANSFER_ERROR			= 0011,
		DRS_POS_ERROR				= 0012,
		DRS_HEAD_ERROR				= 0013,
		DRS_SELECT_ERROR			= 0014,
		DRS_DESELECT_ERROR			= 0015,
		DRS_RESERVE_ERROR			= 0016,
		DRS_RELEASE_ERROR			= 0017,
		DRS_DOUBLE_BIT				= 0020,
		DRS_STATUS_ERROR			= 0021,
		DRS_TRANSFER_PARAM_ERROR	= 0022,
		DRS_TIMEOUT					= 0023,
		DRS_CHANNEL_ERROR			= 0024,
		DRS_NOT_READY				= 0025,
		DRS_LOAD_POS_ERROR			= 0026,
		DRS_RCP_ERROR				= 0027,
		DRS_RESET_ERROR				= 0030,
		DRS_STROBE_ERROR			= 0031,
		DRS_DIAGNOSTIC_ERROR		= 0032,
		DRS_CLEAR_FAULTS_ERROR		= 0033,
		DRS_OFFSET_ERROR			= 0034,
		DRS_ECHO_ERROR				= 0035,
		DRS_IO_TIMEOUT				= 0040,
		DRS_POS_TIMEOUT				= 0041,
		DRS_HEAD_TIMEOUT			= 0042,
		DRS_SELECT_TIMEOUT			= 0043,
		DRS_DESELECT_TIMEOUT		= 0044,
		DRS_RESERVE_TIMEOUT			= 0045,
		DRS_RELEASE_TIMEOUT			= 0046,
		DRS_STATUS_TIMEOUT			= 0047,
		DRS_LOADPOS_TIMEOUT			= 0050,
		DRS_RCP_TIMEOUT				= 0051,
		DRS_RESET_TIMEOUT			= 0052,
		DRS_STROBE_TIMEOUT			= 0053,
		DRS_DIAG_TIMEOUT			= 0054,
		DRS_CLRFLTS_TIMEOUT			= 0055,
		DRS_OFFSET_TIMEOUT			= 0056,
		DRS_ECHO_TIMEOUT			= 0057,
		DRS_BAD_OPTION				= 0076,
		DRS_OTHER_FATAL_ERROR		= 0077,
		DRS_LAST_FATAL_ERROR_CODE	= 0077,
		DRS_NO_LOCAL_MEMORY			= 0100,
		DRS_NO_CACHE_MEMORY			= 0101,
		DRS_SEGMENT_MEMORY			= 0102,
		DRS_NO_STATISTICS			= 0200,
		DRS_INVALID_REQUEST_CODE	= 0301,
		DRS_UNKNOWN_PACKET_TYPE		= 0302,
		DRS_INVALID_PARAMETER		= 0303,
		DRS_NO_DRIVER				= 0320,
		DRS_ALREADY_CONFIGURED		= 0321,
		DRS_NVP_SEQUENCE_ERROR		= 0322,
		DRS_UNSUPPORTED				= 0323,
		DRS_DEVICE_NOT_CONFIGURED	= 0324,
		DRS_CONFIGURATION_FAILURE	= 0325,
		DRS_UCODE_CHKSUM_FAILURE	= 0326,
		DRS_INCOMPAT_UNICOS			= 0327,
		DRS_NOT_EXECUTED			= 0377
	};
	enum class MemoryTypes_e {
		DTM_CM		= 0,		// central memory
		DTM_SSD		= 2,		// SSD
		DTM_T3D		= 4			// T3D
	};

	uint8_t Channel = uint8_t(GetBits(aPacket.Data(0), ChannelRange));
	uint8_t Unit = uint8_t(GetBits(aPacket.Data(0), UnitRange));
	RequestCodes_e RequestCode = RequestCodes_e(GetBits(aPacket.Data(0), RequestCodeRange));
	size_t Cylinder = size_t(GetBits(aPacket.Data(2), CylinderRange));
	size_t Head = size_t(GetBits(aPacket.Data(2), HeadRange));
	size_t Sector = size_t(GetBits(aPacket.Data(2), SectorRange));
	size_t SectorCnt = size_t(GetBits(aPacket.Data(2), SectorCntRange));
	MemoryTypes_e TargetMemoryType = MemoryTypes_e(GetBits(aPacket.Data(4), TargetMemoryTypeRange));
	uint64_t TargetMemoryAddr = GetBits(aPacket.Data(4), TargetMemoryAddrRange);
	uint8_t IopNumber = aPacket.GetProcess();
	switch (RequestCode) {
		case RequestCodes_e::DRQ_READ:
		{
			bool Found = false;
			for (auto &Disk : mDisks) {
				if (Disk->GetPhysicalDeviceId() == Channel && Disk->GetIopNumber() == IopNumber && Disk->GetUnit() == Unit) {
					Found = true;
					CRAY_ASSERT(Head == 0);
					CRAY_ASSERT(Sector == 0);
					CRAY_ASSERT(TargetMemoryType == MemoryTypes_e::DTM_CM);
					Disk->Read(Cylinder, uint32_t(SectorCnt), CAddr_t(TargetMemoryAddr));

					IopEPacket_c Response = GenerateResponse(aPacket);
					Response.SetSize(68);
					for (size_t Idx = 0; Idx < size_t(aPacket.GetLength()-1); ++Idx) {
						Response.Data(Idx) = aPacket.Data(Idx);
					}
					SetBitsInPlace(Response.Data(0), TypeRange, 'd'); // Mark packet as response
					SetBitsInPlace(Response.Data(0), RequestCodeRange, uint64_t(RequestCodes_e::DRQ_READ));
					SetBitsInPlace(Response.Data(0), ResponseCodeRange, uint64_t(ResponseCodes_e::DRS_OK));
					SendPacket(Response);
				}
			}
			if (!Found) { CRAY_ASSERT(false); }
		}
		break;
		case RequestCodes_e::DRQ_WRITE:
		case RequestCodes_e::DRQ_WRITE_BEHIND:
		{
			bool Found = false;
			for (auto &Disk : mDisks) {
				if (Disk->GetPhysicalDeviceId() == Channel) {
					Found = true;
					CRAY_ASSERT(Head == 0);
					CRAY_ASSERT(Sector == 0);
					CRAY_ASSERT(TargetMemoryType == MemoryTypes_e::DTM_CM);
					Disk->Write(Cylinder, uint32_t(SectorCnt), CAddr_t(TargetMemoryAddr));

					IopEPacket_c Response = GenerateResponse(aPacket);
					Response.SetSize(68);
					for (size_t Idx = 0; Idx < aPacket.GetLength(); ++Idx) {
						Response.Data(Idx) = aPacket.Data(Idx);
					}
					SetBitsInPlace(Response.Data(0), TypeRange, 'd'); // Mark packet as response
					SetBitsInPlace(Response.Data(0), RequestCodeRange, uint64_t(RequestCodes_e::DRQ_READ));
					SetBitsInPlace(Response.Data(0), ResponseCodeRange, uint64_t(ResponseCodes_e::DRS_OK));
					SendPacket(Response);
				}
			}
			if (!Found) { CRAY_ASSERT(false); }
		}
		break;
		case RequestCodes_e::DRQ_CONFIGURE_DOWN:
		case RequestCodes_e::DRQ_CONFIGURE_UP:
		case RequestCodes_e::DRQ_RETURN_ACCUMULATED_STATUS:
		case RequestCodes_e::DRQ_RETURN_GENERAL_STATUS:
		case RequestCodes_e::DRQ_DIAGNOSTIC:
		case RequestCodes_e::DRQ_RESET:
		case RequestCodes_e::DRQ_DESELECT:
		case RequestCodes_e::DRQ_ECHO:
		{
			IopEPacket_c Response = GenerateResponse(aPacket);
			Response.SetSize(68);
			for (size_t Idx = 0; Idx < aPacket.GetLength(); ++Idx) {
				Response.Data(Idx) = aPacket.Data(Idx);
			}
			SetBitsInPlace(Response.Data(0), TypeRange, 'd'); // Mark packet as response
			SetBitsInPlace(Response.Data(0), RequestCodeRange, uint64_t(RequestCode));
			SetBitsInPlace(Response.Data(0), ResponseCodeRange, uint64_t(ResponseCodes_e::DRS_OK));
			SendPacket(Response);
		}
		break;
		default:
			CRAY_ASSERT(false);
			break;
	}
}


void SimIopCluster_c::HandleAPacket(std::vector<CInt_t> &aPacket) {
	enum class FunctionCodes_e {
		Read = 1,
		Write = 2,
		WriteBehind = 12,
		SelectDrive = 040,
		ReleaseDrive = 041,
		ReleaseOpposite = 042
	};
	enum class StatusCodes_e {
		OK = 0,
		RecoveredDataError = 1,
		CorrectedDataError = 11,
		UnrecoveredDataError = 12,
		UnrecoveredHardwareError = 13,
		BadParameterInRequest = 16,
		NoBackdoorConfigured = 17
	};

	uint8_t Diag = uint8_t(GetBitsReverse(aPacket[1], 16, 16));
	uint16_t DiagRequest = uint16_t(GetBitsReverse(aPacket[1], 17, 21));

	// The second word would also contain more info for diagnostics (see apacket.h) but we don't support that for now...
	uint8_t StripeFactor = uint8_t(GetBitsReverse(aPacket[2], 0, 3));
	uint8_t UpperCylinder = uint8_t(GetBitsReverse(aPacket[2], 31, 31));
	uint8_t SequenceNumber = uint8_t(GetBitsReverse(aPacket[2], 40, 47));
	uint16_t MinorDev = uint16_t(GetBitsReverse(aPacket[2], 48, 63));

	uint32_t TargetMem = uint32_t(GetBitsReverse(aPacket[3], 0, 1));
	uint32_t DataAddr = uint32_t(GetBitsReverse(aPacket[3], 2, 31));
	FunctionCodes_e FunctionCode = FunctionCodes_e(GetBitsReverse(aPacket[3], 32, 39));
	StatusCodes_e ReturnStatus = StatusCodes_e(GetBitsReverse(aPacket[3], 40, 47));
	DeviceTypes_e DeviceType = DeviceTypes_e(GetBitsReverse(aPacket[3], 48, 52));
	uint8_t IopNumber = uint8_t(GetBitsReverse(aPacket[3], 53, 54));
	uint8_t UnitNumber = uint8_t(GetBitsReverse(aPacket[3], 55, 56));
	uint8_t PhysicalChannel = uint8_t(GetBitsReverse(aPacket[3], 57, 63));

	uint32_t Cylinder;
	uint32_t Head;
	uint32_t Sector;
	// Device address for HDDs
	if (GetMachineType() == MachineTypes_e::YEL || GetMachineType() == MachineTypes_e::YMP) {
		Cylinder = uint32_t(GetBitsReverse(aPacket[4], 2, 31));
		Head = uint32_t(GetBitsReverse(aPacket[4], 0, 0));
		Sector = uint32_t(GetBitsReverse(aPacket[4], 1, 1));
	} else {
		Cylinder = uint32_t(GetBitsReverse(aPacket[4], 0, 10)) | (UpperCylinder << 11);
		Head = uint32_t(GetBitsReverse(aPacket[4], 11, 15));
		Sector = uint32_t(GetBitsReverse(aPacket[4], 16, 22));
	}
	// Device address for RAM disks
	uint16_t RamAddr = uint16_t(GetBitsReverse(aPacket[4], 0, 31));
	
	uint32_t WordCount = uint32_t(GetBitsReverse(aPacket[4], 32, 63));

	uint16_t ReadAhead = uint16_t(GetBitsReverse(aPacket[5], 32, 47));

	CRAY_ASSERT(Diag == 0);

	std::stringstream EventStr;
	EventStr << "C->I PACKET A request -" <<
		" StripeFactor: " << HexPrinter(StripeFactor) <<
		" SeqNo: " << HexPrinter(SequenceNumber) <<
		" MinorDev: " << HexPrinter(MinorDev) <<
		" TargetMem: " << HexPrinter(TargetMem) <<
		" DataAddr: " << HexPrinter(DataAddr) <<
		" FunctionCode: " << HexPrinter(uint8_t(FunctionCode)) <<
		" RetStatus: " << HexPrinter(uint8_t(ReturnStatus)) <<
		" DeviceType: " << DecPrinter(uint8_t(DeviceType)) <<
		" Iop: " << OctPrinter(IopNumber) <<
		" Unit: " << DecPrinter(UnitNumber) <<
		" PhysicalChannel: " << OctPrinter(PhysicalChannel) <<
		" CHS: " << DecPrinter(Cylinder) << "-" << DecPrinter(Head) << "-" << DecPrinter(Sector) <<
		" WordCount: " << HexPrinter(WordCount) <<
		" ReadAhead: " << DecPrinter(ReadAhead) <<
		std::endl;
	mLogger << setloglevel(LogLevel_EventFire) << EventStr.str();
	GetMainframe().GetEventDispatcher().Fire(EventStr.str());

//	CRAY_ASSERT(StripeFactor == 0);
	CRAY_ASSERT(TargetMem == 0);
	CRAY_ASSERT(FunctionCode == FunctionCodes_e::Read || FunctionCode == FunctionCodes_e::Write || FunctionCode == FunctionCodes_e::WriteBehind);
//	CRAY_ASSERT(DeviceType == DeviceTypes_e::DD29);

	// Look up the drive
	bool Found = false;
	for (auto &Disk : mDisks) {
		if (Disk->GetPhysicalDeviceId() == PhysicalChannel && Disk->GetIopNumber() == IopNumber && Disk->GetUnit() == UnitNumber) {
			Found = true;
			switch(FunctionCode) {
				case FunctionCodes_e::Read:
					Disk->Read(Cylinder, Head, Sector, WordCount * sizeof(CInt_t) / Disk->GetSectorSize(), CAddr_t(DataAddr));
				break;
				case FunctionCodes_e::Write:
				case FunctionCodes_e::WriteBehind:
					Disk->Write(Cylinder, Head, Sector, WordCount * sizeof(CInt_t) / Disk->GetSectorSize(), CAddr_t(DataAddr));
				break;
				default:
					CRAY_ASSERT(false);
			}
			break;
		}
	}
	if (!Found) throw Generic_x() << "Can't find drive for physical channel: " << OctPrinter(PhysicalChannel) << " iop: " << OctPrinter(IopNumber) << " and unit: " << DecPrinter(UnitNumber);

	// Compose a reply
	ReversePacket(aPacket);
	aPacket[3] = SetBitsReverse(aPacket[3], 40, 47, uint64_t(StatusCodes_e::OK));
	mIToCChannel->QueuePacket(aPacket);
}

void SimIopCluster_c::HandleKPacket(std::vector<CInt_t> &aPacket) {
	// Kernel crash: wait for the next packet, print it, and exit
	mState = States_e::KernelPanic;
}

void SimIopCluster_c::Tick() {
	for (auto &Console : mConsoles) Console->Tick();
	for (auto &EthInterfaces : mEthInterfaces) EthInterfaces->Tick();

	switch (mState) {
		case States_e::Reset:
/*  Boot config:
*
*  On Model-E boot, boot info is in registers.
*
          $ELSE
            owslink,0  A5               Boot cluster number
            owschan,0  A6               Boot cluster channel number
            rfsleng,0  S1               Length of root file system 
            scfleng,0  S2               Length of spare chip configuration file
            sysmem,0   S4               LWA+1 of UNICOS binary
            csim,0     S6               CSIM flag from deadstart
            end,0      S7               length of binary + spare chip + par file
            S3         <D'8
            S1         A7
            S1         S1&S3
            logicalmf,0 S1              logical mainframe number
            S1         A7
            S1         S1>D'8
            S1         S1&S3
            owsplvl,0  S1               OWS protocol level
            S1         A7
            S1         S1>D'16
            S1         S1&S3
            S1         S1<D'56
            irunlvl,0  S1               Run level for init
          $ENDIF

*/
			if (mMasterCluster) {
				mSeqSent = 0;
				mSeqReceived = 0;
				for (auto &Console : mConsoles) Console->Reset();
				if (mKernelFileName.is_initialized()) {
					CAddr_t KernelStart = CAddr_t(0);
					mMainframe->LoadImageFile(mKernelFileName.get().c_str(), KernelStart, mKernelPreamble);
					uint64_t KernelSize = boost::filesystem::file_size(mKernelFileName.get());
					CAddr_t DataEnd = KernelStart + CAddr_t((KernelSize + sizeof(CInt_t) - 1) / sizeof(CInt_t));
					if (mParameterFileName.is_initialized()) {
						CAddr_t ParamStart = DataEnd;
						mMainframe->LoadImageFile(mParameterFileName.get().c_str(), ParamStart);
						uint64_t ParamSize = boost::filesystem::file_size(mParameterFileName.get());
						CAddr_t ParamEnd = ParamStart + CAddr_t((ParamSize + sizeof(CInt_t) - 1) / sizeof(CInt_t));
						if (!mDontTouchExchangePacket) {
							mMainframe->MemWrite(CAddr_t(0x00000c), SwapBytes(CInt_t(ParamStart))); // Start address of PARAM file patched into S4 on boot
							mMainframe->MemWrite(CAddr_t(0x00000f), SwapBytes(CInt_t(ParamEnd))); // End address of PARAM file patched into S7 on boot
						}
						DataEnd = ParamEnd;
						if (mRamFsFileName.is_initialized()) {
							CAddr_t RamFsStart = DataEnd;
							mMainframe->LoadImageFile(mRamFsFileName.get().c_str(), RamFsStart);
							uint64_t RamFsSize = boost::filesystem::file_size(mRamFsFileName.get());
							RamFsSize = CAddr_t((RamFsSize + sizeof(CInt_t) - 1) / sizeof(CInt_t));
							CAddr_t RamFsEnd = CAddr_t(RamFsStart + RamFsSize);
							if (!mDontTouchExchangePacket) {
								mMainframe->MemWrite(CAddr_t(0x000009), SwapBytes(CInt_t(RamFsSize))); // End address of RAM FS file patched into S1 on boot
							}
							DataEnd = RamFsEnd;
						}
					}
					if (!mDontTouchExchangePacket) {
						mMainframe->MemWriteNoWatchpointByType<CAddr_t>(CAddr_t(0x000005) * 2 + 1, SwapBytes(CAddr_t(mBootCluster))); // Boot cluster in A5
						mMainframe->MemWriteNoWatchpointByType<CAddr_t>(CAddr_t(0x000006) * 2 + 1, SwapBytes(CAddr_t(mBootChannel))); // Boot channel in A6
					}
					uint8_t RunLevel = 0;
					if (mRunLevel.is_initialized()) {
						RunLevel = mRunLevel.get()[0];
					}
					if (!mDontTouchExchangePacket) {
						CAddr_t A7 = (RunLevel << 16) | (mOWSProtocolLevel << 8) | (mLogicalMainframeNumber); // RunLevel needs to be specified as a 'char' not as an 'int'
						mMainframe->MemWriteNoWatchpointByType<CAddr_t>(CAddr_t(0x000007) * 2 + 1, SwapBytes(A7)); // RunLevel and stuff in A7
					}
				}
				mMainframe->CpuMasterClear(true);
				mMainframe->CpuMasterClear(false);
			}
			if (GetIopRevision() == IopRevision_e::IOSD) {
				time_t NowTime = time(0);
				struct tm *Now = localtime(&NowTime);
				std::vector<CInt_t> Packet(6);
				Packet[0] = CInt_t(Now->tm_mon + 1); // Month
				Packet[1] = CInt_t(Now->tm_mday); // Day
				Packet[2] = 1999; // CInt_t(Now->tm_year + 1900); // Year
				Packet[3] = CInt_t(Now->tm_hour); // Hour
				Packet[4] = CInt_t(Now->tm_min); // Minute
				Packet[5] = CInt_t(Now->tm_sec); // Second (upper 32 bits is TZ offset)
				// TZ offset indeed is in the upper 32 bits, but
				// 1. UNICOS doesn't actually read it
				// 2. Windows doesn't seem to have a tm_gmtoff field
				//Packet[5] = (CInt_t(Now->tm_gmtoff) << 32) | (CInt_t(Now->tm_sec)); // Second (upper 32 bits is TZ offset)
				mIToCChannel->QueuePacket(Packet);
				if (mMasterCluster) {
					mState = States_e::WaitTimeSent;
				} else {
					mState = States_e::WaitTimeSent;
				}
			} else {
				mState = States_e::RunningJ90;
			}
		break;
		case States_e::WaitTimeSent:
			if (mIToCChannel->ReadyToSend()) {
				mLogger << setloglevel(LogLevel_IoTrace) << "###################################################################################" << std::endl;
				mLogger << setloglevel(LogLevel_IoTrace) << "###################################################################################" << std::endl;
				mLogger << setloglevel(LogLevel_IoTrace) << "sending in States_e::WaitTimeSent" << std::endl;
				mLogger << setloglevel(LogLevel_IoTrace) << "###################################################################################" << std::endl;
				mLogger << setloglevel(LogLevel_IoTrace) << "###################################################################################" << std::endl;
				std::vector<CInt_t> Packet(6);
				Packet[0] = 0x4331004900000000; // - C1.I....
				Packet[1] = 0x0000000000000000; // - ........
				Packet[2] = 0x0000000000000000; // - ........
				Packet[3] = 0x0000000000000000; // - ........
				Packet[4] = 0x0000000000000000; // - ........
				Packet[5] = 0x0000000000000000; // - ........
				mIToCChannel->QueuePacket(Packet);
				mState = States_e::WaitISent;
			}
			break;
		case States_e::WaitISent:
			if (mCToIChannel->HasPacket()) {
				mLogger << setloglevel(LogLevel_IoTrace) << "###################################################################################" << std::endl;
				mLogger << setloglevel(LogLevel_IoTrace) << "###################################################################################" << std::endl;
				mLogger << setloglevel(LogLevel_IoTrace) << "receiving in States_e::WaitISent" << std::endl;
				mLogger << setloglevel(LogLevel_IoTrace) << "###################################################################################" << std::endl;
				mLogger << setloglevel(LogLevel_IoTrace) << "###################################################################################" << std::endl;
				std::vector<CInt_t> Packet = mCToIChannel->GetPacket();
				CRAY_ASSERT(Packet.size() == 6);
				CRAY_ASSERT(Packet[0] == 0x0049433100000000); // - .IC1....
				CRAY_ASSERT(Packet[1] == 0x0000000000000000); // - ........
				CRAY_ASSERT(Packet[2] == 0x0000000000000000); // - ........
				CRAY_ASSERT(Packet[3] == 0x0000000000000000); // - ........
				CRAY_ASSERT(Packet[4] == 0x0000000000000000); // - ........
				CRAY_ASSERT(Packet[5] == 0x0000000000000000); // - ........

				Packet[0] = 0x4331004AFFFFFFFF; // - C1.J....
				Packet[1] = 0xFFFFFFFFFFFFFFFF; // - ........
				Packet[2] = 0xFFFFFFFFFFFFFFFF; // - ........
				Packet[3] = 0xFFFFFFFFFFFFFFFF; // - ........
				Packet[4] = 0xFFFFFFFFFFFFFFFF; // - ........
				Packet[5] = 0xFFFFFFFFFFFFFFFF; // - ........
				mIToCChannel->QueuePacket(Packet);

				mState = States_e::WaitJSent;
			}
			break;
		case States_e::WaitJSent:
			if (mCToIChannel->HasPacket()) {
				std::vector<CInt_t> Packet = mCToIChannel->GetPacket();
				CRAY_ASSERT(Packet.size() == 6);
				CRAY_ASSERT(Packet[0] == 0x004A4331FFFFFFFF); // - .JC1....
				CRAY_ASSERT(Packet[1] == 0xFFFFFFFFFFFFFFFF); // - ........
				CRAY_ASSERT(Packet[2] == 0xFFFFFFFFFFFFFFFF); // - ........
				CRAY_ASSERT(Packet[3] == 0xFFFFFFFFFFFFFFFF); // - ........
				CRAY_ASSERT(Packet[4] == 0xFFFFFFFFFFFFFFFF); // - ........
				CRAY_ASSERT(Packet[5] == 0xFFFFFFFFFFFFFFFF); // - ........
															  // Send an error-report (C) packet.
				Packet[0] = 0x4331004300000004; // - C1.C....
				Packet[1] = 0x0000000000000000; // - ........
				Packet[2] = 0x0000000000000000; // - ........
				Packet[3] = 0x0000000000000000; // - ........
				Packet[4] = 0x0000000000000000; // - ........
				Packet[5] = 0x0000000000000000; // - ........
				mIToCChannel->QueuePacket(Packet);

				mState = States_e::RunningYEL;
			}
		break;
		case States_e::RunningYEL:
			if (mCToIChannel->HasPacket()) {
				// TODO: do incoming packet processing
				std::vector<CInt_t> Packet = mCToIChannel->GetPacket();
				CRAY_ASSERT(Packet.size() == 6);
				char PacketType = char(GetBitsReverse(Packet[0], 8, 15));
				switch (PacketType) {
				case 'T': // terminal packet
					HandleTPacket(Packet);
					break;
				case 'A':
					HandleAPacket(Packet);
					break;
				case 'K':
					HandleKPacket(Packet);
					break;
				case 'M': { // Ethernet packet
					bool Found = false;
					for (auto &EthInterface : mEthInterfaces) {
						if (EthInterface->HandleIosDPacket(Packet)) {
							Found = true;
							break;
						}
					}
					if (!Found && !mEthInterfaces.empty()) mEthInterfaces.front()->SendErrorResponse(Packet, SimIopEth_n::Status_e::MPST_ILCHN);
					break;
				}
				case 'F': { // IOP local peripheral packet
					mIopPeripheral->HandleIosDPacket(Packet);
					break;
				}
				default:
					CRAY_ASSERT(false);
					break;
				}
			}
		break;
		case States_e::RunningJ90:
			if (mCToIChannel->HasPacket()) {
				// We have to break the buffer up to multiple messages...
				std::vector<CInt_t> RawPacket = mCToIChannel->GetPacket();
				std::vector<CInt_t>::const_iterator PacketBegin = RawPacket.begin();
				std::vector<CInt_t>::const_iterator PacketEnd = std::find(PacketBegin + 1, RawPacket.cend(), *PacketBegin);
				while (PacketEnd != RawPacket.cend()) {
					IopEPacket_c Packet(PacketBegin,PacketEnd+1);
					CRAY_ASSERT(Packet.IsValid());
					mSeqReceived = Packet.GetSeq();
					mPacketsReceived++;
					if (Packet.GetFlags() & IopEPacket_c::cFlagIni) mSeqSent = 1;
					if (Packet.GetLength() > 1) {
						char PacketType = char(GetBits(Packet.Data(0), 56, 63));
						switch (PacketType) {
							case 'O': // OWS packet
								HandleIosEOPacket(Packet);
								break;
							case 'Z': // Terminal packet
								HandleIosEZPacket(Packet);
								break;
							case 'D': // Disk packet
								HandleIosEDPacket(Packet);
								break;
							case 'P': // ATM packet
								HandleIosEPPacket(Packet);
								break;
							case 'M': { // Ethernet packet
								bool Found = false;
								for (auto &EthInterface : mEthInterfaces) {
									if (EthInterface->HandleIosEPacket(Packet)) {
										Found = true;
										break;
									}
								}
								if (!Found && !mEthInterfaces.empty()) mEthInterfaces.front()->SendErrorResponse(Packet, SimIopEth_n::Status_e::MPST_ILCHN);
								break;
							}
							case 'F': { // Expander packet (????) It seems that it is an IOS-D-style F packet embedded in an IOS-E packet. However the IOS-E style 'f' packet is FTDI. For now, going with an F packet
								if (mIopPeripheral == nullptr) throw Generic_x() << "Can't handle shell driver (F) packet without Peripheral section in IopCluster configuration";
								mIopPeripheral->HandleIosEPacket(Packet);
								break;
							}
							default:
								CRAY_ASSERT(false);
								break;
						}
					}
					PacketBegin = PacketEnd + 1;
					if (PacketBegin == RawPacket.cend()) break;
					PacketEnd = std::find(PacketBegin + 1, RawPacket.cend(), *PacketBegin);
				}
			}
		break;
		case States_e::KernelPanic:
			if (mCToIChannel->HasPacket()) {
				if (mConsoles.size() > 0) {
					SimIopConsole_c &Console = *mConsoles[0];
					std::vector<CInt_t> Packet = mCToIChannel->GetPacket();
					size_t ByteCnt = Packet.size() * sizeof(CInt_t);
					// Simply dump all incoming data to the first console...
					Console.SendString("\r\n");
					bool NewLinePrinted = false;
					for (size_t i = 0; i < ByteCnt; ++i) {
						char Char = ((char*)(&Packet[0]))[(i / 8 * 8) + (7 - (i % 8))];
						switch (Char) {
							case 0:
								if (!NewLinePrinted) {
									Console.SendString("\r\n");
									NewLinePrinted = true;
								}
								break;
							case 0x0a:
							case 0x0d:
								Console.SendString("\r\n");
								break;
							default:
								Console.SendChar(Char);
								NewLinePrinted = false;
								break;
						}
					}
				}
			}
		break;
		default:
			CRAY_ASSERT(false);
		break;
	}
}

// SimIopCluster_c::ChannelIToC_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SimIopCluster_c::ChannelIToC_c::ChannelIToC_c(const Configuration_c &aConfig, size_t aChannelId, Mainframe_c &aMainframe, SimIopCluster_c &aParent):
	Channel_i(aChannelId, aMainframe),
	mLogger(aConfig, "I->C_", aChannelId),
	mParent(aParent),
	mPacketValid(false)
{
	mLogger.SetParent(aParent.GetLogger());
}

void SimIopCluster_c::ChannelIToC_c::ChannelTick() {
	if (!mPacketValid) {
		if (mPacketQueue.empty()) return;
		SendPacket(mPacketQueue.front());
		mPacketQueue.pop();
		return;
	}

	if (IsActive()) {
		while (IsActive()) {
			CInt_t Data = mPacket[mPacketIdx++];
			SetData(Data, mPacketIdx == mPacket.size());
			if (mPacketIdx == mPacket.size()) {
				mPacketValid = false;
				return;
			}
		}
//		mLogger << setloglevel(LogLevel_Error) << "Packet is too large for buffer of Start:" << HexPrinter(mStart) << " Limit: " << HexPrinter(mLimit) << " Size: " << DecPrinter(mLimit - mStart) << std::endl;
		mLogger << setloglevel(LogLevel_Error) << "Packet is too large for buffer" << std::endl;
		mLogger << setloglevel(LogLevel_Error) << "Packet size:" << mPacket.size() << " words" << std::endl;
		for (size_t Idx = 0; Idx < mPacket.size(); ++Idx) {
			mLogger << setloglevel(LogLevel_Error) << HexPrinter(Idx, 4) << " : " << HexPrinter(mPacket[Idx]) << " - " << AsciiDumpPrinter(mPacket[Idx]) << std::endl;
		}
		CRAY_ASSERT(false);
		//Disconnect();
	}
}

bool SimIopCluster_c::ChannelIToC_c::ReadyToSend() const {
	return !mPacketValid;
}

void SimIopCluster_c::ChannelIToC_c::SendPacket(const std::vector<CInt_t> &aPacket, bool aReplace) {
	CRAY_ASSERT(!mPacketValid || aReplace);
	mPacket = aPacket;
	mPacketIdx = 0;
	mPacketValid = true;
	mLogger << setloglevel(LogLevel_IoTrace) << "Priming packet to mainframe of " << mPacket.size() << " words" << std::endl;
}

void SimIopCluster_c::ChannelIToC_c::QueuePacket(const std::vector<CInt_t> &aPacket) {
	++mParent.mPacketsSent;
	if (!mPacketValid) {
		SendPacket(aPacket);
	}
	else {
		mPacketQueue.push(aPacket);
	}
}

// SimIopCluster_c::ChannelCToI_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SimIopCluster_c::ChannelCToI_c::ChannelCToI_c(const Configuration_c &aConfig, size_t aChannelId, Mainframe_c &aMainframe, SimIopCluster_c &aParent) :
	Channel_i(aChannelId, aMainframe),
	mLogger(aConfig, "C->I_", aChannelId),
	mParent(aParent),
	mPacketValid(false),
	mPacketIdx(0)
{
	mLogger.SetParent(aParent.GetLogger());
}

void SimIopCluster_c::ChannelCToI_c::ChannelTick() {
	while (IsActive() && !mPacketValid) {
		CInt_t Data = GetData();
		mPacket.emplace_back(Data);
		if (!IsActive()) { // Channel just got done with transmission due to this word --> mark the packet valid
			mPacketValid = true;
		}
	}
}

bool SimIopCluster_c::ChannelCToI_c::HasPacket() {
	return mPacketValid;
}

std::vector<CInt_t> SimIopCluster_c::ChannelCToI_c::GetPacket() {
	CRAY_ASSERT(mPacketValid);
	mPacketValid = false;
	std::vector<CInt_t> RetVal(std::move(mPacket));
	mPacket.clear();
	return RetVal;
}































// Console handling
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SimIopCluster_c::MakeConsoles(size_t aMaxConsoleId) {
	while (mConsoles.size() <= aMaxConsoleId) {
		if (mIopRevision == IopRevision_e::IOSE) {
			std::unique_ptr<SimIopConsole_c> Console = std::make_unique<SimIopEConsole_c>(
				mDefaultConsoleConfig,
				mConsoles.size(),
				*this,
				uint8_t(mConsoles.size()),
				mMainframe->DoDisableAutoTerminal()
			);
			mConsoles.push_back(std::move(Console));
		}
		else {
			std::unique_ptr<SimIopConsole_c> Console = std::make_unique<SimIopDConsole_c>(
				mDefaultConsoleConfig,
				mConsoles.size(),
				*this,
				uint8_t(mConsoles.size()),
				mMainframe->DoDisableAutoTerminal()
			);
			mConsoles.push_back(std::move(Console));
		}
	}
}

void SimIopCluster_c::HandleIosEZPacket(IopEPacket_c &aPacket) {
	using namespace IosEZPacket_n;

	uint8_t Dev = uint8_t(GetBits(aPacket.Data(0), DevRange));

	MakeConsoles(Dev);
	SimIopConsole_c &Console = *mConsoles[Dev];
	Console.HandlePacket(aPacket.GetRawData());
}

void SimIopCluster_c::HandleTPacket(std::vector<CInt_t> &aPacket) {
	using namespace IosTPacket_n;

	uint8_t IopNumber = uint8_t(GetBits(aPacket[0], IopNumberRange));
	uint8_t TTyNumber = uint8_t(GetBits(aPacket[0], TTyNumberRange));

	CRAY_ASSERT(IopNumber == mIopId);

	MakeConsoles(TTyNumber);
	SimIopConsole_c &Console = *mConsoles[TTyNumber];
	Console.HandlePacket(aPacket);
}



#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif
