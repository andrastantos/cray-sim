#ifndef __SIM_IOP_ETH_H__
#define __SIM_IOP_ETH_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <boost/asio.hpp>
#include "utils.h"
#include "cray_types.h"
#include "cray_logger.h"
#include "cray_channels.h"
#include "debug_events.h"
#include <stdint.h>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/timer/timer.hpp>
#include "config_file.h"
#include "ui.h"
#include "cray_mainframe.h"
#include <queue>

// Definition is in epackm.h (for IOS_E) and mpacket.h (for IOS_D). They're essentially the same, the IOS_E simply packing the IOS_D structure into the new header format.
// There are some new status codes, but that's about it

namespace SimIopEth_n {
	// Word 0
	const BitRange_s TypeRange = BitRange_s(56, 63);
	// Word 1
	const BitRange_s DestIdRange(48, 63);
	const BitRange_s SourceIdRange(32, 47);
	const BitRange_s PathOwnerRange(25, 31);
	const BitRange_s IopNumberRange(22, 24);
	const BitRange_s ChannelNumberRange(16, 21);
	const BitRange_s RequestRange(8, 15);
	const BitRange_s StatusNumberRange(7, 7);
	const BitRange_s StatusRange(0, 6);
	// Word 2
	const BitRange_s BufferPtrRange(32, 63); // TODO: check if swapping is necessary
	const BitRange_s DevStructRange(0, 31); // TODO: check if swapping is necessary
	// Open parameters
	/////////////////////
	// Word 4
	const BitRange_s RingOpAddrRange(32, 63); // TODO: check if swapping is necessary
	// Word 5
	const BitRange_s CpInQAddrRange(32, 63);
	const BitRange_s CpOutQAddrRange(0, 31);
	// Word 6
	const BitRange_s CpQSizeRange(32, 63);
	const BitRange_s MtuRange(0, 31);
	// Word 8
	const BitRange_s MacAddrRange(16, 63);
	// Sleep/wakeup parameters
	/////////////////////
	// Word 4
	const BitRange_s ReqTypeRange(32, 63);
	const BitRange_s CurIndexRange(0, 31);
	// Word 5
	const BitRange_s MDataPRange(32, 63); // TODO: check if swapping is necessary
	// IOCTL parameters
	/////////////////////
	// Word 4
	const BitRange_s CmdRange(32, 63); // TODO: check if swapping is necessary
	const BitRange_s BufferRange(0, 31); // TODO: check if swapping is necessary
	// Word 5
	const BitRange_s LengthRange(32, 63); // TODO: check if swapping is necessary

	enum class Request_e
	{
		MPFC_OPN = 1, // open function
		MPFC_CLS = 2, // close function
		MPFC_PROBE = 3, // probe function
		MPFC_SLEEP = 5, // sleep function
		MPFC_IOCTL = 9  // ioctl function
	};
	enum class SleepRqType_e
	{
		SLEEP_INPUT = 0,
		SLEEP_OUTPUT = 1,
		WAKEUP_INPUT = 2,
		WAKEUP_OUTPUT = 3,
		SLEEP_CLOSED = 4
	};
	enum class Status_e {
		MPST_OK = 0, // ok status
		MPST_HDW = 1, // hardware error
		MPST_PROER = 2, // protocol error
		MPST_ILCHN = 3, // illegal channel
		MPST_ILFNC = 4, // illegal function
		MPST_RTO = 5, // read timeout
		MPST_WTO = 6, // write timeout
		MPST_ILDRV = 7, // illegal driver IOS_E only
		MPST_INVAL = 8, // invalid argument IOS_E only
		MPST_FAILED = 9, // request failed IOS_E only
	};
};

class SimIopEth_c {
public:
	SimIopEth_c(const Configuration_c &aConfig, class SimIopCluster_c &aParent, const std::string &aName);
	virtual ~SimIopEth_c();
	void GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const;
	std::string GetName() const;
	void RegisterCommands(CommandHooks_t &aHooks);
	void MasterClear();
	bool HandleIosEPacket(class IopEPacket_c &aPacket);
	bool HandleIosDPacket(std::vector<CInt_t> &aIosDPacket);
	void HandleTxPacket(size_t aQueueIdx);
	void HandleRxPacket();
	void HandleTxPackets();
	void Tick();
	void SendErrorResponse(class IopEPacket_c &aPacket, SimIopEth_n::Status_e aStatus);
	void SendErrorResponse(std::vector<CInt_t> &aIosDPacket, SimIopEth_n::Status_e aStatus);
protected:
	std::string mName;
	CLogger_c mLogger;
	SimIopCluster_c &mParent;
	size_t mChannelId;
	uint8_t mIopNumber;

	// Statistics
	uint32_t mPacketsSent;
	uint32_t mPacketsReceived;
	uint32_t mBytesSent;
	uint32_t mBytesReceived;
	uint32_t mCrayPacketsSent;
	uint32_t mCrayPacketsReceived;

	// Driver state
	uint32_t RingOpAddr;
	uint32_t CpInQAddr;
	uint32_t CpOutQAddr;
	uint32_t CpQSize;
	uint32_t Mtu;
	CAddr_t mBufferPtr;
	CAddr_t mDevStruct;

	const size_t cEtherPacketSize = 1536; // In bytes

	uint64_t mSimMacAddr;

	void DumpQueues();

	std::unique_ptr<class PacketCaptureThread_c> mPacketCaptureThread;
	std::unique_ptr<class TapAdapter_c> mAdapter;

	class IopEPacket_c GetIosEPacket(std::vector<CInt_t> &aIosDPacket);
	void SendPacket(IopEPacket_c &aPacket);
	bool mThreadStarted;



};


#endif // __SIM_IOP_ETH_H__
