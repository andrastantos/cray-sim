// Cray-XMP I/O Processor simulator class

#include "sim_iop_eth.h"
#include "sim_iop.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include <boost/filesystem.hpp>
//#include <conio.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <boost/asio.hpp>
#include <algorithm>

#if defined(__CYGWIN__) || defined(_WIN32)
#include "vtap_win32.h"
#else
#include "vtap_linux.h"
#endif // _WIN32

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

template <typename tType> class SafeQueue
{
public:
	tType Pop() {
		std::unique_lock<std::mutex> Lock(mMutex);
		while (mQueue.empty()) {
			mNotifier.wait(Lock);
		}
		auto RetVal = mQueue.front();
		mQueue.pop();
		return RetVal;
	}

	void Push(const tType& item) {
		std::unique_lock<std::mutex> Lock(mMutex);
		mQueue.push(item);
		Lock.unlock();
		mNotifier.notify_one();
	}

	void Push(tType&& item) {
		std::unique_lock<std::mutex> Lock(mMutex);
		mQueue.push(std::move(item));
		Lock.unlock();
		mNotifier.notify_one();
	}

	bool Empty() {
		std::unique_lock<std::mutex> Lock(mMutex);
		return mQueue.empty();
	}
protected:
	std::queue<tType> mQueue;
	std::mutex mMutex;
	std::condition_variable mNotifier;
};

typedef std::vector<uint8_t> Packet_c;

class PacketCaptureThread_c : public Thread_c
{
public:
	explicit PacketCaptureThread_c(TapAdapter_c &aAdapter) :
		mAdapter(aAdapter)
	{}
	SafeQueue<Packet_c> mPacketQueue;
	void ReInit() {
// TODO: this needs to be re-worked. I don't know what power-state transitions do to this driver
//		size_t FailCnt = 0;
//		while (mCaptureHandle == nullptr) {
//			try {
//				mCaptureHandle = std::move(std::unique_ptr<pcap_t, decltype(&pcap_close)>(PCapOpenLive(mDevice.c_str(), mPromisc, mTimeout, mSnapLen), pcap_close));
//			}
//			catch (std::exception &) {
//				if (++FailCnt > 100) {
//					throw;
//				}
//				std::this_thread::sleep_for(std::chrono::milliseconds(10));
//			}
//		}
	}
	void SetFilterMac(uint64_t aFilterMac) {
		mFilterMac = aFilterMac;
	}
	void SendEthPacket(std::vector<uint8_t> &aPacket) {
		mAdapter.Send(aPacket);
	}
	void CancelRead() {
		mAdapter.CancelRead();
	}
protected:
	virtual void Thread() override {
		while (!IsTerminating()) {
			try {
				Packet_c Packet = mAdapter.BlockingReceive();
				if (Packet.size() < 24) continue;
				if (IsPaused()) continue;
				uint64_t DstMac = 0;
				for (size_t Idx = 0; Idx < 6; ++Idx) {
					DstMac = SetBits(DstMac, (5 - Idx) * 8, (5 - Idx) * 8 + 7, Packet[Idx]);
				}
				// filter for our MAC, the broadcast and any multi-cast addresses
				if (DstMac == mFilterMac || DstMac == 0xffffffffff || (DstMac & 0x010000000000) != 0) {
					mPacketQueue.Push(Packet);
				}
			} catch (...) {
				if (!IsTerminating()) throw;
			}
		}
	}

	TapAdapter_c &mAdapter;

	uint64_t mFilterMac;
};

//
//std::unique_ptr<pcap_if_t, decltype(&pcap_freealldevs)> AllDevs(PCapFindAllDevs(), pcap_freealldevs);
//
//pcap_if_t *SelectedDev = nullptr;
//for (pcap_if_t *Dev = AllDevs.get(); Dev; Dev = Dev->next) {
//	if (aInterfaceName == Dev->name) {
//		SelectedDev = Dev;
//		break;
//	}
//	if (Dev->description != nullptr && aInterfaceName == Dev->description) {
//		SelectedDev = Dev;
//		break;
//	}
//}
//if (SelectedDev == nullptr) throw Generic_x() << "Can't find requested interface: " << aInterfaceName;
//
//PacketCaptureThread_c Thread(SelectedDev->name, true, 1000);
//
//int LinkType = Thread.GetLinkType();
//if (LinkType != DLT_EN10MB) throw Generic_x() << "Only Ethernet interfaces are supported";
//
//std::cout << "\nlistening on " << SelectedDev->description << "..." << std::endl;
//
//AllDevs = nullptr; // This will free our structure
//
//Thread.Start();
//
//while (!_kbhit()) {
//	if (!Thread.mPacketQueue.Empty()) {
//		Packet_c Packet = Thread.mPacketQueue.Pop();
//		// convert the timestamp to readable format
//		time_t LocalSeconds = Packet.mTimeStamp.tv_sec;
//		struct tm *LocalTime = localtime(&LocalSeconds);
//		char TimeString[16];
//		strftime(TimeString, sizeof TimeString, "%H:%M:%S", LocalTime);
//
//		std::cout << TimeString << "," << Packet.mTimeStamp.tv_usec << " captured bytes: " << Packet.mPacketData.size() << " bytes on the wire: " << Packet.mWireLength << std::endl;
//		for (size_t Ofs = 0; Ofs < std::min(Packet.mPacketData.size(), size_t(512)); ++Ofs) {
//			if (Ofs % 32 == 0) {
//				if (Ofs != 0) std::cout << std::endl;
//				std::cout << "\t";
//			}
//			std::cout << std::noshowbase << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << int(Packet.mPacketData[Ofs]) << " ";
//		}
//		std::cout << std::endl;
//	}
//}
//Thread.Terminate();
//Thread.join();
















void DumpPacket(CLogger_c &aLogger, CAddr_t aAddr, const Mainframe_c &aMainframe, const char *aHeader)
{
	LogLine_c Line = aLogger << setloglevel(LogLevel_IoTrace);
	if (Line.good()) {
		Line << aHeader << std::endl;
		for (size_t Ofs = 0; Ofs < (53 + 7) / 8; ++Ofs) {
			CInt_t Data = aMainframe.MemReadNoWatchpoint<CInt_t>(CAddr_t(aAddr + Ofs));
			Line << HexPrinter(SwapBytes(Data)) << " ";
			if (Ofs % 4 == 3) Line << std::endl;
		}
		Line << std::endl;
	}
}

static void DumpMem(CLogger_c &aLogger, const char *aHeader, CAddr_t aBuffer, size_t aLength, Mainframe_c &aMainframe)
{
	const size_t cLineSize = 4;
	auto TraceLine = aLogger << setloglevel(LogLevel_IoTrace) << aHeader << std::endl;
	CAddr_t End = aBuffer + CAddr_t(aLength);
	for (CAddr_t Addr = aBuffer; Addr < End; Addr += cLineSize) {
		CInt_t Data[cLineSize];
		for (size_t j = 0; j < cLineSize; ++j) {
			if (Addr + j >= End) break;
			Data[j] = aMainframe.MemReadNoWatchpoint<CInt_t>(CAddr_t(Addr + j));
		}
		if (TraceLine.good()) {
			TraceLine << HexPrinter(Addr) << " : ";
			for (size_t j = 0; j < cLineSize; ++j) {
				if (Addr + j >= End) break;
				TraceLine << HexPrinter(SwapBytes(Data[j])) << " ";
			}
			TraceLine << " - ";
			for (size_t j = 0; j < cLineSize; ++j) {
				if (Addr + j >= End) break;
				TraceLine << AsciiDumpPrinter(SwapBytes(Data[j])) << " ";
			}
			TraceLine << std::endl;
		}
	}
	TraceLine << std::endl;
}

void SimIopEth_c::HandleTxPacket(size_t aQueueIdx)
{
	if (mPacketCaptureThread == nullptr) return;

	CAddr_t Base = CAddr_t(CpOutQAddr + 3 + aQueueIdx * cEtherPacketSize / sizeof(CInt_t));
	CInt_t EntryHead = SwapBytes(mParent.GetMainframe().MemReadNoWatchpoint<CInt_t>(Base));
	uint32_t PayloadSize = uint32_t(GetBits(EntryHead, 0, 31));
	uint16_t ErrorCode = uint16_t(GetBits(EntryHead, 32, 47));
	uint16_t Error = uint16_t(GetBits(EntryHead, 48, 63));
	CAddr_t PayloadStart = CAddr_t(Base + 1);

	mLogger << setloglevel(LogLevel_IoTrace) << "Output queue packet # " << DecPrinter(aQueueIdx) << " Error: " << DecPrinter(Error) << " ErrorCode: " << DecPrinter(ErrorCode) << " Size: " << DecPrinter(PayloadSize) << std::endl;
	// 3 words of queue header and 1 word of entry header. There are also 2 bytes of padding at the beginning of the payload and length rouned up to the next word
	DumpMem(mLogger, "ETH PACKET", PayloadStart, (PayloadSize + 2 + 7) / 8, mParent.GetMainframe());
	// For now, let's process and reply to ARP requests
	// 59113654 IOP0.ETH20: Output queue packet # 1 Error: 0 ErrorCode : 0 Size : 60
	// 59113654 IOP0.ETH20 : ETH PACKET
	// 59113654 IOP0.ETH20 : 0x0083DDC7 : 0x0000FFFFFFFFFFFF 0x0000000000000806 0x0001080006040001 0xFEDCBA987654C0A8 - ........ ........ ........ ....vT..
	// 59113654 IOP0.ETH20 : 0x0083DDCB : 0x0001000000000000 0xC0A8006400000000 0x0000000000000000 0x0000000000000000 - ........ ...d.... ........ ........

	// Copy out the packet
	std::vector<uint8_t> TxPacket(PayloadSize);
	for (size_t Idx = 0; Idx < PayloadSize; ++Idx) {
		TxPacket[Idx] = mParent.GetMainframe().MemReadNoWatchpointByType<uint8_t>(CAddr_t(PayloadStart * 8 + 2 + Idx));
	}
	// Patch up src MAC
	for (size_t Idx = 0; Idx < 6; ++Idx) {
		TxPacket[6 + Idx] = uint8_t(GetBits(mSimMacAddr, (5 - Idx) * 8, (5 - Idx) * 8 + 7));
	}
	mPacketsSent++;
	mBytesSent += PayloadSize;
	mPacketCaptureThread->SendEthPacket(TxPacket);
}

void SimIopEth_c::Tick()
{
	if (mPacketCaptureThread != nullptr && !mPacketCaptureThread->IsPaused()) HandleRxPacket();
}

void SimIopEth_c::HandleRxPacket()
{
	using namespace SimIopEth_n;

	if (mPacketCaptureThread == nullptr) return;

	// TODO: how to get hand-shake with the consumer???
	CInt_t ReplyRdIdx = SwapBytes(mParent.GetMainframe().MemReadNoWatchpoint<CInt_t>(CAddr_t(CpInQAddr + 0)));
	CInt_t ReplyWrIdx = SwapBytes(mParent.GetMainframe().MemReadNoWatchpoint<CInt_t>(CAddr_t(CpInQAddr + 1)));
	CInt_t ReplyContSize = SwapBytes(mParent.GetMainframe().MemReadNoWatchpoint<CInt_t>(CAddr_t(CpInQAddr + 2)));
	uint32_t ReplyCont = uint32_t(GetBits(ReplyContSize, 32, 63));
	uint32_t ReplySize = uint32_t(GetBits(ReplyContSize, 0, 31));
	CAddr_t ReplyBase = CAddr_t(CpInQAddr + 3 + ReplyWrIdx * cEtherPacketSize / sizeof(CInt_t));
	CAddr_t ReplyPayloadStart = CAddr_t(ReplyBase + 1);

	CInt_t NextReplyWrIdx = (ReplyWrIdx + 1) % ReplySize; // TODO: this might be a bit sub-optimal for handling full/empty dis-ambiguation
	bool PacketsQueued = false;
	if (!mPacketCaptureThread->mPacketQueue.Empty() && NextReplyWrIdx != ReplyRdIdx) {
		Packet_c Packet = mPacketCaptureThread->mPacketQueue.Pop();
		uint64_t DstMac = 0;
		size_t ReplyPayloadSize = Packet.size();

		mPacketsReceived++;
		mBytesReceived += uint32_t(ReplyPayloadSize);

		if (ReplyPayloadSize <= cEtherPacketSize) {
			mParent.GetMainframe().MemWrite(ReplyBase, SwapBytes(uint64_t(ReplyPayloadSize))); // We signal no error or error-code
			// Create a local array of the payload, rouded up to full QWORD
//			std::vector<uint8_t> Payload(2);
//			Payload[0] = 0; Payload[1] = 0;
//			Payload.insert(Payload.begin() + 2, Packet.begin(), Packet.end());
//			Payload.resize(((Payload.size() + 7) / 8) * 8);
//			for (size_t Idx = 0; Idx < Payload.size() / 8; ++Idx) {
//				mParent.GetMainframe().MemWrite(CAddr_t(ReplyPayloadStart + Idx), *(CInt_t*)(&Payload[Idx]));
//			}
			mParent.GetMainframe().MemWrite(CAddr_t(ReplyPayloadStart * 8 + 0), uint8_t(0));
			mParent.GetMainframe().MemWrite(CAddr_t(ReplyPayloadStart * 8 + 1), uint8_t(0));
			for (size_t Idx = 0; Idx < ReplyPayloadSize; ++Idx) {
				mParent.GetMainframe().MemWrite(CAddr_t(ReplyPayloadStart * 8 + 2 + Idx), Packet[Idx]);
			}
			mLogger << setloglevel(LogLevel_IoTrace) << "Input queue packet # " << DecPrinter(ReplyWrIdx) << " Size: " << DecPrinter(ReplyPayloadSize) << std::endl;
			// 3 words of queue header and 1 word of entry header. There are also 2 bytes of padding at the beginning of the payload and length rouned up to the next word
			DumpMem(mLogger, "ETH REPLY PACKET", ReplyPayloadStart, (ReplyPayloadSize + 2 + 7) / 8, mParent.GetMainframe());

			PacketsQueued = true;
			ReplyWrIdx = NextReplyWrIdx;
			NextReplyWrIdx = (ReplyWrIdx + 1) % ReplySize;
		}
	}
	if (PacketsQueued) {
		mParent.GetMainframe().MemWrite(CAddr_t(CpInQAddr + 1), SwapBytes(ReplyWrIdx));
		IopEPacket_c Response = mParent.GenerateResponse();
		Response.SetSize(7);
		SetBitsInPlace(Response.Data(0), TypeRange, 'm'); // Mark packet as response
		if (mParent.GetIopRevision() == IopRevision_e::IOSE) {
			SetBitsInPlace(Response.Data(1), DestIdRange, 0x4331); // 'C1'
			SetBitsInPlace(Response.Data(1), SourceIdRange, 0x004d); // 'M'
		} else {
			// For IOSD, the reversal will happen during transmission
			SetBitsInPlace(Response.Data(1), DestIdRange, 0x004d); // 'M'
			SetBitsInPlace(Response.Data(1), SourceIdRange, 0x4331); // 'C1'
		}
		//SetBitsInPlace(Response.Data(1), PathOwnerRange, 0);
		SetBitsInPlace(Response.Data(1), IopNumberRange, 0xff); // All 1-s, but why???
		SetBitsInPlace(Response.Data(1), ChannelNumberRange, mChannelId);
		SetBitsInPlace(Response.Data(1), RequestRange, uint8_t(Request_e::MPFC_SLEEP));
		SetBitsInPlace(Response.Data(1), StatusNumberRange, 0);
		SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(Status_e::MPST_OK));
		SetBitsInPlace(Response.Data(2), BufferPtrRange, mBufferPtr);
		SetBitsInPlace(Response.Data(2), DevStructRange, mDevStruct);
		SetBitsInPlace(Response.Data(3), ReqTypeRange, uint32_t(SleepRqType_e::WAKEUP_INPUT));
		SetBitsInPlace(Response.Data(3), CurIndexRange, ReplyWrIdx);

		SendPacket(Response);

		mLogger << setloglevel(LogLevel_IoActivity) << "Ethernet SLEEP sent: WAKEUP_INPUT " << "Current index: " << DecPrinter(ReplyWrIdx) << " MDataP: " << HexPrinter(0) << std::endl;
		mLogger << setloglevel(LogLevel_IoTrace) << "Queues after" << std::endl;
		DumpQueues();
	}
}

void SimIopEth_c::DumpQueues()
{
	DumpMem(mLogger, "IN QUEUE", CpInQAddr, 3, mParent.GetMainframe());
	DumpMem(mLogger, "OUT QUEUE", CpOutQAddr, 3, mParent.GetMainframe());
}

SimIopEth_c::SimIopEth_c(const Configuration_c &aConfig, class SimIopCluster_c &aParent, const std::string &aName):
	mName(aName),
	mLogger(aConfig, mName.c_str()),
	mChannelId(aConfig.get<size_t>("Channel")),
	mParent(aParent),
	mPacketsSent(0),
	mPacketsReceived(0),
	mBytesSent(0),
	mBytesReceived(0),
	mCrayPacketsSent(0),
	mCrayPacketsReceived(0),
	mSimMacAddr(aConfig.get<uint64_t>("SimMacAddr")),
	mIopNumber(aConfig.get<uint8_t>("IopNumber")),
	mThreadStarted(false)
{
	#if defined(__CYGWIN__) || defined(_WIN32)
	TapAdapter_c *SelectedAdapter = nullptr;
	
	std::vector<TapAdapter_c> Adapters = EnumTaps();
	boost::optional<std::string> InterfaceName = aConfig.get_optional<std::string>("InterfaceNameWindows");
	boost::optional<std::string> InterfaceGuid = aConfig.get_optional<std::string>("InterfaceGuid");

	for (auto &Adapter : Adapters) {
		if (InterfaceName.is_initialized() && Adapter.GetName() == InterfaceName.get()) {
			SelectedAdapter = &Adapter;
			break;
		}
		if (InterfaceGuid.is_initialized() && Adapter.GetDeviceGuid() == InterfaceGuid.get()) {
			SelectedAdapter = &Adapter;
			break;
		}
	}
	if (SelectedAdapter == nullptr) {
		if (Adapters.size() == 0) throw Generic_x() << "Can't find appropriate network interface. Please install TAP-windows network driver";
		if (InterfaceName.is_initialized() || InterfaceGuid.is_initialized()) throw Generic_x() << "Can't find specified network interface";
		SelectedAdapter = &Adapters[0];
	}
	mAdapter = std::make_unique<TapAdapter_c>(std::move(*SelectedAdapter));
	#else
	mAdapter = std::make_unique<TapAdapter_c>();
	boost::optional<std::string> InterfaceName = aConfig.get_optional<std::string>("InterfaceNameLinux");
	if (InterfaceName.is_initialized()) mAdapter->SetName(InterfaceName.get().c_str());
	#endif // _WIN32

	try {
		mLogger << setloglevel(LogLevel_IoActivity) << "Opening Ethernet device: " << mAdapter->GetName() << std::endl;
		mAdapter->Open();
		mPacketCaptureThread = std::make_unique<PacketCaptureThread_c>(*mAdapter);
		mPacketCaptureThread->SetFilterMac(mSimMacAddr);
	}
	catch (Generic_x &Ex) {
		mLogger << setloglevel(LogLevel_IoActivity) << "Error opening Ethernet device: " << mAdapter->GetName() << " with error: " << Ex.what() << std::endl;
		mAdapter = nullptr;
		mPacketCaptureThread = nullptr;
	}
	mLogger.SetParent(mParent.GetLogger());
}

SimIopEth_c::~SimIopEth_c()
{
	if (mThreadStarted) {
		//CRAY_ASSERT(mPacketCaptureThread != nullptr);
		mPacketCaptureThread->Terminate();
		mPacketCaptureThread->CancelRead();
		if (mPacketCaptureThread->joinable()) mPacketCaptureThread->join();
		if (mAdapter != nullptr) mAdapter->Close();
	}
}

void SimIopEth_c::GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const
{
	if (mAdapter != nullptr) {
		std::string AdapterName = mAdapter->GetName();
		if (AdapterName.length() < 5 || aLongFormat) {
			aStatus.put(aLongFormat ? "Interface" : "If", AdapterName);
		} else {
			std::string DisplayName(AdapterName.begin(), AdapterName.begin() + 5);
			DisplayName += "...";
			aStatus.put(aLongFormat ? "Interface" : "If", DisplayName);
		}
		aStatus.put(aLongFormat ? "Packets Sent" : "Sp", mPacketsSent);
		aStatus.put(aLongFormat ? "Packets Received" : "Rp", mPacketsReceived);
		aStatus.put(aLongFormat ? "Bytes Sent" : "Sb", mBytesSent);
		aStatus.put(aLongFormat ? "Bytes Received" : "Rb", mBytesReceived);
		aStatus.put(aLongFormat ? "Cray Packets Sent" : "CSp", mCrayPacketsSent);
		aStatus.put(aLongFormat ? "Cray Packets Received" : "CRp", mCrayPacketsReceived);
		aStatus.put(aLongFormat ? "Output Queue Read Idx" : "OQR", SwapBytes(mParent.GetMainframe().MemReadNoWatchpoint<CInt_t>(CAddr_t(CpOutQAddr + 0))));
		aStatus.put(aLongFormat ? "Output Queue Write Idx" : "OQW", SwapBytes(mParent.GetMainframe().MemReadNoWatchpoint<CInt_t>(CAddr_t(CpOutQAddr + 1))));
		aStatus.put(aLongFormat ? "Input Queue Read Idx" : "IQR", SwapBytes(mParent.GetMainframe().MemReadNoWatchpoint<CInt_t>(CAddr_t(CpInQAddr + 0))));
		aStatus.put(aLongFormat ? "Input Queue Write Idx" : "IQW", SwapBytes(mParent.GetMainframe().MemReadNoWatchpoint<CInt_t>(CAddr_t(CpInQAddr + 1))));
	} else {
		aStatus.put(aLongFormat ? "Interface" : "If", "--- N/A ---");
	}
}

std::string SimIopEth_c::GetName() const
{
	return mName;
}

void SimIopEth_c::RegisterCommands(CommandHooks_t &aHooks) {
	class CmdKick_c : public CmdFactoryBase_i {
	public:
		explicit CmdKick_c(SimIopEth_c &aParent) : mParent(aParent) {}
		virtual bool ParseAndExec(TokenStream_t::const_iterator aBegin, TokenStream_t::const_iterator aEnd) override {
			bool DoHelp = false;
			if (aBegin->mValue == "help") {
				DoHelp = true;
				++aBegin; if (aBegin == aEnd) return false;
			}
			if (aBegin->mValue != "kick") return false;
			++aBegin; if (aBegin == aEnd) return false;
			if (aBegin->mValue != mParent.GetName()) return false;
			if (DoHelp) {
				std::cout << "kick " << mParent.GetName() << std::endl;
				std::cout << "    Kicks the ethernet driver to empty its TX queue" << std::endl;
				return true;
			}
			else {
				++aBegin;
				if (aBegin != aEnd && aBegin->mTokenId != TokenTypes_e::EndOfLine) return false;

				try {
					mParent.HandleTxPackets();
					std::cout << mParent.GetName() << " kicked" << std::endl;
				}
				catch (std::exception &Ex) {
					std::cout << "Kick failed" << std::endl;
					std::cout << Ex.what() << std::endl;
				}
				return true;
			}
		}
		virtual std::string GetCommandName() const override { return "kick"; }
		virtual std::string GetDeviceName() const override { return mParent.GetName(); }
	protected:
		SimIopEth_c &mParent;
	};

	aHooks.emplace_back(std::make_unique<CmdKick_c>(*this));
}

void SimIopEth_c::MasterClear()
{
//	mPacketSent = 0;
	mPacketsReceived = 0;
}


bool SimIopEth_c::HandleIosDPacket(std::vector<CInt_t> &aIosDPacket)
{
	IopEPacket_c Packet = GetIosEPacket(aIosDPacket);
	return HandleIosEPacket(Packet);
}

void SimIopEth_c::HandleTxPackets() {
	using namespace SimIopEth_n;

	CInt_t RdIdx = SwapBytes(mParent.GetMainframe().MemReadNoWatchpoint<CInt_t>(CAddr_t(CpOutQAddr + 0)));
	CInt_t WrIdx = SwapBytes(mParent.GetMainframe().MemReadNoWatchpoint<CInt_t>(CAddr_t(CpOutQAddr + 1)));
	CInt_t ContSize = SwapBytes(mParent.GetMainframe().MemReadNoWatchpoint<CInt_t>(CAddr_t(CpOutQAddr + 2)));
	uint32_t Cont = uint32_t(GetBits(ContSize, 32, 63));
	uint32_t Size = uint32_t(GetBits(ContSize, 0, 31));
	bool SendSleep = false;
	for (CInt_t Idx = RdIdx; Idx != WrIdx; Idx = (Idx + 1) % Size) {
		HandleTxPacket(size_t(Idx));
		SendSleep = true;
	}
	mParent.GetMainframe().MemWrite(CAddr_t(CpOutQAddr), SwapBytes(CInt_t(WrIdx)));

	if (SendSleep) {
		IopEPacket_c Response = mParent.GenerateResponse();
		Response.SetSize(7);
		SetBitsInPlace(Response.Data(0), TypeRange, 'm'); // Mark packet as response
		if (mParent.GetIopRevision() == IopRevision_e::IOSE) {
			SetBitsInPlace(Response.Data(1), DestIdRange, 0x4331); // 'C1'
			SetBitsInPlace(Response.Data(1), SourceIdRange, 0x004d); // 'M'
		}
		else {
			// For IOSD, the reversal will happen during transmission
			SetBitsInPlace(Response.Data(1), DestIdRange, 0x004d); // 'M'
			SetBitsInPlace(Response.Data(1), SourceIdRange, 0x4331); // 'C1'
		}
		//SetBitsInPlace(Response.Data(1), PathOwnerRange, 0);
		SetBitsInPlace(Response.Data(1), IopNumberRange, 0xff); // All 1-s, but why???
		SetBitsInPlace(Response.Data(1), ChannelNumberRange, mChannelId);
		SetBitsInPlace(Response.Data(1), RequestRange, uint8_t(Request_e::MPFC_SLEEP));
		SetBitsInPlace(Response.Data(1), StatusNumberRange, 0);
		SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(Status_e::MPST_OK));
		SetBitsInPlace(Response.Data(2), BufferPtrRange, mBufferPtr);
		SetBitsInPlace(Response.Data(2), DevStructRange, mDevStruct);
		SetBitsInPlace(Response.Data(3), ReqTypeRange, uint32_t(SleepRqType_e::SLEEP_OUTPUT));
		SetBitsInPlace(Response.Data(3), CurIndexRange, RdIdx);

		SendPacket(Response);

		mLogger << setloglevel(LogLevel_IoActivity) << "Ethernet SLEEP sent: SLEEP_OUTPUT " << "Current index: " << DecPrinter(RdIdx) << " MDataP: " << HexPrinter(0) << std::endl;
		mLogger << setloglevel(LogLevel_IoTrace) << "Queues after" << std::endl;
		DumpQueues();
	}

}

bool SimIopEth_c::HandleIosEPacket(IopEPacket_c &aPacket)
{
	using namespace SimIopEth_n;

	++mCrayPacketsReceived;

	size_t ChannelNumber = size_t(GetBits(aPacket.Data(1), ChannelNumberRange));
	uint8_t IopNumber = uint8_t(GetBits(aPacket.Data(1), IopNumberRange));
	if (ChannelNumber != mChannelId) return false;
	// For some reason Iop No is allways filled in with all 1-s, so can't really filter on it...
	//	if (IopNumber != mIopNumber) return false;

	std::stringstream EventStr;
	EventStr << "C->I PACKET M request - " << HexPrinter(aPacket.Data(0)) << " " << HexPrinter(aPacket.Data(1)) << " " << HexPrinter(aPacket.Data(2));
	mLogger << setloglevel(LogLevel_EventFire) << EventStr.str() << std::endl;
	mParent.GetMainframe().GetEventDispatcher().Fire(EventStr.str());

	Request_e Request = Request_e(GetBits(aPacket.Data(1), RequestRange));
	mBufferPtr = uint32_t(GetBits(aPacket.Data(2), BufferPtrRange));
	mDevStruct = uint32_t(GetBits(aPacket.Data(2), DevStructRange));

	switch (Request) {
		case Request_e::MPFC_CLS: {
			mLogger << setloglevel(LogLevel_IoActivity) << "Ethernet CLOSE request" << std::endl;
			//			DumpMem(mLogger, "Ethernet DEV STRUCT", DevStruct, 64, GetMainframe());

			IopEPacket_c Response = mParent.GenerateResponse(aPacket);
			Response.SetSize(aPacket.GetLength() - 1);
			for (size_t Idx = 0; Idx < size_t(aPacket.GetLength() - 1); ++Idx) {
				Response.Data(Idx) = aPacket.Data(Idx);
			}
			SetBitsInPlace(Response.Data(0), TypeRange, 'm'); // Mark packet as response
			SetBitsInPlace(Response.Data(1), StatusNumberRange, 0);
			SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(Status_e::MPST_OK));
			SendPacket(Response);
			if (mPacketCaptureThread != nullptr) mPacketCaptureThread->Pause();
		}
		break;
		case Request_e::MPFC_OPN: {
			RingOpAddr = uint32_t(GetBits(aPacket.Data(3), RingOpAddrRange));
			CpInQAddr = uint32_t(GetBits(aPacket.Data(4), CpInQAddrRange));
			CpOutQAddr = uint32_t(GetBits(aPacket.Data(4), CpOutQAddrRange));
			CpQSize = uint32_t(GetBits(aPacket.Data(5), CpQSizeRange));
			Mtu = uint32_t(GetBits(aPacket.Data(5), MtuRange));
			uint64_t MacAddr = GetBits(aPacket.Data(6), MacAddrRange);

			auto Line = mLogger << setloglevel(LogLevel_IoActivity);
			Line << "Ethernet OPEN request: ";
			Line << "RingOpAddr: " << HexPrinter(RingOpAddr) << " ";
			Line << "CpInQAddr: " << HexPrinter(CpInQAddr) << " ";
			Line << "CpOutQAddr: " << HexPrinter(CpOutQAddr) << " ";
			Line << "CpQSize: " << HexPrinter(CpQSize) << " ";
			Line << "MTU: " << DecPrinter(Mtu) << " ";
			Line << std::endl;
			//			DumpMem(mLogger, "Ethernet DEV STRUCT", DevStruct, 64, GetMainframe());

			IopEPacket_c Response = mParent.GenerateResponse(aPacket);
			Response.SetSize(aPacket.GetLength() - 1);
			for (size_t Idx = 0; Idx < size_t(aPacket.GetLength() - 1); ++Idx) {
				Response.Data(Idx) = aPacket.Data(Idx);
			}
			SetBitsInPlace(Response.Data(0), TypeRange, 'm'); // Mark packet as response
			SetBitsInPlace(Response.Data(1), StatusNumberRange, 0);
			if (mPacketCaptureThread != nullptr) {
				SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(Status_e::MPST_OK));
				SetBitsInPlace(Response.Data(6), MacAddrRange, mSimMacAddr);
				SendPacket(Response);
				if (!mThreadStarted) {
					std::cout << "Starting receive thread" << std::endl;
					mPacketCaptureThread->Start();
				}
				mThreadStarted = true;
				mPacketCaptureThread->Resume();
			} else {
				SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(Status_e::MPST_HDW));
				SetBitsInPlace(Response.Data(6), MacAddrRange, mSimMacAddr);
			}
		}
		break;
		case Request_e::MPFC_SLEEP: {
			SleepRqType_e ReqType = SleepRqType_e(GetBits(aPacket.Data(3), ReqTypeRange));
			uint32_t CurIndex = uint32_t(GetBits(aPacket.Data(3), CurIndexRange));
			uint32_t MDataP = uint32_t(GetBits(aPacket.Data(4), MDataPRange));

			auto Line = mLogger << setloglevel(LogLevel_IoActivity);
			Line << "Ethernet SLEEP request: ";
			switch (ReqType) {
			case SleepRqType_e::SLEEP_INPUT:
				Line << "SLEEP_INPUT ";
				break;
			case SleepRqType_e::SLEEP_OUTPUT:
				Line << "SLEEP_OUTPUT ";
				break;
			case SleepRqType_e::WAKEUP_INPUT:
				Line << "WAKEUP_INPUT ";
				break;
			case SleepRqType_e::WAKEUP_OUTPUT:
				Line << "WAKEUP_OUTPUT ";
				break;
			case SleepRqType_e::SLEEP_CLOSED:
				Line << "SLEEP_CLOSED ";
				break;
			}
			Line << "Current index: " << DecPrinter(CurIndex) << " MDataP: " << HexPrinter(MDataP) << std::endl;
			mLogger << setloglevel(LogLevel_IoTrace) << "Queues before" << std::endl;
			DumpQueues();

			bool SendResponse = false;
			IopEPacket_c Response = mParent.GenerateResponse(aPacket);
			Response.SetSize(aPacket.GetLength() - 1);
			for (size_t Idx = 0; Idx < size_t(aPacket.GetLength() - 1); ++Idx) {
				Response.Data(Idx) = aPacket.Data(Idx);
			}
			switch (ReqType) {
			case SleepRqType_e::SLEEP_INPUT:
	//			mParent.GetMainframe().MemWrite(CAddr_t(CpInQAddr + 0), SwapBytes(CInt_t(CurIndex)));
				break;
			case SleepRqType_e::SLEEP_OUTPUT:
				break;
			case SleepRqType_e::WAKEUP_INPUT:
	//			mParent.GetMainframe().MemWrite(CAddr_t(CpInQAddr + 0), SwapBytes(CInt_t(CurIndex)));
	//			SetBitsInPlace(Response.Data(3), ReqTypeRange, uint8_t(SleepRqType_e::SLEEP_INPUT));
	//			SendResponse = true;
	//			mPacketCaptureThread->Resume();
				break;
			case SleepRqType_e::WAKEUP_OUTPUT: {
				HandleTxPackets();
				//SetBitsInPlace(Response.Data(3), ReqTypeRange, uint8_t(SleepRqType_e::SLEEP_OUTPUT));
				//SendResponse = true;
			}
			break;
			case SleepRqType_e::SLEEP_CLOSED:
				break;
			}
			SetBitsInPlace(Response.Data(0), TypeRange, 'm'); // Mark packet as response
			SetBitsInPlace(Response.Data(1), StatusNumberRange, 0);
			if (mPacketCaptureThread != nullptr) {
				SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(Status_e::MPST_OK));
			} else {
				SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(Status_e::MPST_HDW));
			}
			if (SendResponse) SendPacket(Response);
			mLogger << setloglevel(LogLevel_IoTrace) << "Queues after" << std::endl;
			DumpQueues();
		}
		break;
		case Request_e::MPFC_PROBE: {
			mLogger << setloglevel(LogLevel_IoActivity) << "Ethernet PROBE request" << std::endl;
			//			DumpMem(mLogger, "Ethernet DEV STRUCT", DevStruct, 64, GetMainframe());

			IopEPacket_c Response = mParent.GenerateResponse(aPacket);
			Response.SetSize(aPacket.GetLength() - 1);
			for (size_t Idx = 0; Idx < size_t(aPacket.GetLength() - 1); ++Idx) {
				Response.Data(Idx) = aPacket.Data(Idx);
			}
			SetBitsInPlace(Response.Data(0), TypeRange, 'm'); // Mark packet as response
			SetBitsInPlace(Response.Data(1), StatusNumberRange, 0);
			if (mPacketCaptureThread != nullptr) {
				SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(Status_e::MPST_OK));
			} else {
				SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(Status_e::MPST_HDW));
			}
			SendPacket(Response);
		}
		break;
		case Request_e::MPFC_IOCTL: {
			uint32_t Cmd = uint32_t(GetBits(aPacket.Data(3), CmdRange));
			bool IsVoid = (Cmd & 0x20000000) != 0;
			bool IsInput = (Cmd & 0x40000000) != 0;
			bool IsOutput = (Cmd & 0x80000000) != 0;
			size_t ParamLengthInBytes = (Cmd >> 16) & 0x1ff;
			char IoCtlX = (Cmd >> 8) & 0xff;
			uint8_t IoCtlCode = Cmd & 0xff;
			CAddr_t Buffer = CAddr_t(GetBits(aPacket.Data(3), BufferRange));
			uint32_t Length = uint32_t(GetBits(aPacket.Data(4), LengthRange));
			mLogger << setloglevel(LogLevel_IoActivity) << "Ethernet IOCTL request: " << DecPrinter(IoCtlCode) << " with " << DecPrinter(ParamLengthInBytes) << " bytes of parameter" << std::endl;
	//		DumpMem(mLogger, "Parameter dump", Buffer, Length, mParent.GetMainframe());
	//			DumpMem(mLogger, "Ethernet DEV STRUCT", DevStruct, 64, GetMainframe());

			IopEPacket_c Response = mParent.GenerateResponse(aPacket);
			Response.SetSize(aPacket.GetLength() - 1);
			for (size_t Idx = 0; Idx < size_t(aPacket.GetLength() - 1); ++Idx) {
				Response.Data(Idx) = aPacket.Data(Idx);
			}
			SetBitsInPlace(Response.Data(0), TypeRange, 'm'); // Mark packet as response
			SetBitsInPlace(Response.Data(1), StatusNumberRange, 0);
			if (mPacketCaptureThread != nullptr) {
				SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(Status_e::MPST_OK));
			}
			else {
				SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(Status_e::MPST_HDW));
			}
			SendPacket(Response);
		}
		break;
		default:
			CRAY_ASSERT(false);
			break;
	}
	return true;
}

void SimIopEth_c::SendErrorResponse(std::vector<CInt_t> &aIosDPacket, SimIopEth_n::Status_e aStatus)
{
	IopEPacket_c Packet = GetIosEPacket(aIosDPacket);
	SendErrorResponse(Packet, aStatus);
}

void SimIopEth_c::SendErrorResponse(class IopEPacket_c &aPacket, SimIopEth_n::Status_e aStatus)
{
	// Word 0
	const BitRange_s TypeRange(56, 63);
	// Word 1
	const BitRange_s DestIdRange(48, 63);
	const BitRange_s SourceIdRange(32, 47);
	const BitRange_s PathOwnerRange(25, 31);
	const BitRange_s IopNumberRange(22, 24);
	const BitRange_s ChannelNumberRange(16, 21);
	const BitRange_s RequestRange(8, 15);
	const BitRange_s StatusNumberRange(7, 7);
	const BitRange_s StatusRange(0, 6);

	IopEPacket_c Response = mParent.GenerateResponse(aPacket);
	Response.SetSize(aPacket.GetLength() - 1);
	for (size_t Idx = 0; Idx < size_t(aPacket.GetLength() - 1); ++Idx) {
		Response.Data(Idx) = aPacket.Data(Idx);
	}
	SetBitsInPlace(Response.Data(0), TypeRange, 'm'); // Mark packet as response
	SetBitsInPlace(Response.Data(1), StatusNumberRange, 0);
	SetBitsInPlace(Response.Data(1), StatusRange, uint8_t(aStatus));
	SendPacket(Response);
}

IopEPacket_c SimIopEth_c::GetIosEPacket(std::vector<CInt_t> &aIosDPacket)
{
	IopEPacket_c RetVal;
	RetVal.SetSize(aIosDPacket.size() + 1);
	RetVal.SetLength(uint16_t(aIosDPacket.size() + 2));
	for (size_t Idx = 0; Idx < aIosDPacket.size(); ++Idx) {
		RetVal.Data(Idx+1) = aIosDPacket[Idx];
	}
	return RetVal;
}

void SimIopEth_c::SendPacket(IopEPacket_c &aPacket)
{
	++mCrayPacketsSent;
	if (mParent.GetIopRevision() == IopRevision_e::IOSE) {
		mParent.SendPacket(aPacket);
	} else {
		std::vector<CInt_t> IosDPacket(6, 0);
		CRAY_ASSERT(aPacket.GetLength() == 8);
		for (size_t Idx = 0; Idx < size_t(aPacket.GetLength() - 2); ++Idx) IosDPacket[Idx] = aPacket.Data(Idx+1);
		ReversePacket(IosDPacket);
		mParent.SendPacket(IosDPacket);
	}
}

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif
