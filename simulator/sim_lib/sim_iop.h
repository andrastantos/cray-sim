#ifndef __SIM_IOP_H__
#define __SIM_IOP_H__

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
#include "iop_cluster_if.h"
#include "sim_iop_disk.h"
#include "sim_iop_con.h"
#include "sim_iop_eth.h"
#include "sim_iop_periph.h"

enum class DeviceTypes_e {
	DD00 = 0,           // Invalid entry (???)
	DD19 = 1,           // DD19 disk drive
	DD29 = 2,           // DD29 disk drive
	DD49 = 3,           // DD49 disk drive
	DD39 = 4,           // DD39 disk drive
	DD10 = 5,           // DD10 disk drive
	DD40 = 6,           // DD40 disk drive
	DD50 = 7,           // DD50 disk drive
	DD11 = 8,           // DD11 disk drive
	DD41 = 9,           // DD41 disk drive
	DD60 = 10,          // DD60 disk drive
	DD61 = 11,          // DD61 disk drive
	DD62 = 12,          // DD62 disk drive
	DD42 = 13,          // DD42 disk drive
	DA62 = 14,          // DA62 disk drive
	DA60 = 15,          // DA60 disk drive
	DD301 = 16,         // DD301 disk drive
	DA301 = 17,         // DA301 disk drive
	DD302 = 18,         // DD302 disk drive
	DA302 = 19,         // DA302 disk drive
	DDESDI = 64,        // old esdi drive
	DD3 = 65,           // new esdi drive
	DDLDAS = 66,        // old DAS
	DDAS2 = 67,         // new DAS
	DD4 = 68,           // ipi + sabre 7
	RD1 = 69,           // removable esdi
	DDIMEM = 70,        // ironics+memory vme boards
	DD5S = 71,          // DD5S SCSI drive
	DD5I = 72,          // DD5I IPI drive
	DD_U = 73,          // DD_U Generic SCSI drive
	DD6S = 74,          // DD6S SCSI drive
	DD7S = 75,          // DD7S SCSI drive
	DD318 = 76,         // DD318 SCSI drive
	DD501 = 77,         // DD501 SCSI drive
};

inline uint64_t GetBitsReverse(uint64_t aData, const BitRange_s aRange) {
	return GetBitsReverse(aData, aRange.Start, aRange.End);
}

inline void SetBitsReverse(uint64_t &aData, const BitRange_s aRange, uint64_t aValue) {
	aData = SetBitsReverse(aData, aRange.Start, aRange.End, aValue);
}

inline void SetBits(uint64_t &aData, const BitRange_s aRange, uint64_t aValue) {
	aData = SetBits(aData, aRange.Start, aRange.End, aValue);
}

inline uint64_t GetBits(uint64_t aData, const BitRange_s aRange) {
	return GetBits(aData, aRange.Start, aRange.End);
}

inline void SetBitsInPlace(uint64_t &aData, const BitRange_s aRange, uint64_t aValue) {
	aData = SetBits(aData, aRange.Start, aRange.End, aValue);
}

//enum class MachineTypes_e : int;

class IopEPacket_c {
public:
	explicit IopEPacket_c(const std::vector<CInt_t> &aPacket) : mPacket(aPacket) {}
	IopEPacket_c(const std::vector<CInt_t>::const_iterator &aPacketBegin,const std::vector<CInt_t>::const_iterator &aPacketEnd): mPacket(aPacketBegin, aPacketEnd) {}
	IopEPacket_c(const IopEPacket_c &aOri) : mPacket(aOri.mPacket) {}
	IopEPacket_c() {}
	void SetSize(size_t aSize) {
		mPacket.resize(aSize + 1, 0);
		SetLength(uint16_t(aSize + 1));
	}
	static const uint8_t cMagic = 45;

	static const uint8_t cFlagRet = 0200; // Retransmit packet request
	static const uint8_t cFlagAck = 0100; // Acknowledge packet
	static const uint8_t cFlagIni = 040; // Restart IOP sequence numbers at 1

	bool IsValid() const;

	std::vector<CInt_t> &Finalize();
	std::vector<CInt_t> &GetRawData() { return mPacket; }
	const std::vector<CInt_t> &GetRawData() const { return mPacket; }

	uint8_t GetMagic() const { return uint8_t(GetBits(mPacket[0], 58, 63)); }
	uint16_t GetLength() const { return uint16_t(GetBits(mPacket[0], 48, 57)); }
	uint8_t GetKernelId() const { return uint8_t(GetBits(mPacket[0], 45, 47)); }
	uint8_t GetSource() const { return uint8_t(GetBits(mPacket[0], 41, 44)); }
	uint8_t GetCluster() const { return uint8_t(GetBits(mPacket[0], 37, 40)); }
	uint8_t GetProcess() const { return uint8_t(GetBits(mPacket[0], 32, 36)); }
	uint8_t GetFlags() const { return uint8_t(GetBits(mPacket[0], 24, 31)); }
	uint8_t GetLogicalPath() const { return uint8_t(GetBits(mPacket[0], 16, 23)); }
	uint8_t GetSeq() const { return uint8_t(GetBits(mPacket[0], 8, 15)); }
	uint8_t GetAckSeq() const { return uint8_t(GetBits(mPacket[0], 0, 7)); }

	void SetMagic(uint8_t aValue) { mPacket[0] = SetBits(mPacket[0], 58, 63, aValue); }
	void SetLength(uint16_t aValue) { mPacket[0] = SetBits(mPacket[0], 48, 57, aValue); }
	void SetKernelId(uint8_t aValue) { mPacket[0] = SetBits(mPacket[0], 45, 47, aValue); }
	void SetSource(uint8_t aValue) { mPacket[0] = SetBits(mPacket[0], 41, 44, aValue); }
	void SetCluster(uint8_t aValue) { mPacket[0] = SetBits(mPacket[0], 37, 40, aValue); }
	void SetProcess(uint8_t aValue) { mPacket[0] = SetBits(mPacket[0], 32, 36, aValue); }
	void SetFlags(uint8_t aValue) { mPacket[0] = SetBits(mPacket[0], 24, 31, aValue); }
	void SetLogicalPath(uint8_t aValue) { mPacket[0] = SetBits(mPacket[0], 16, 23, aValue); }
	void SetSeq(uint8_t aValue) { mPacket[0] = SetBits(mPacket[0], 8, 15, aValue); }
	void SetAckSeq(uint8_t aValue) { mPacket[0] = SetBits(mPacket[0], 0, 7, aValue); }

	CInt_t &Data(size_t aIdx) { return mPacket[aIdx + 1]; }
	CInt_t Data(size_t aIdx) const { return mPacket[aIdx + 1]; }
protected:
	std::vector<CInt_t> mPacket;
};

// Simulated I/O processor class

inline void ReversePacket(std::vector<CInt_t> &aPacket) {
	const BitRange_s DIDRange(0, 15);
	const BitRange_s SIDRange(16, 31);

	// Swap source and destination
	uint16_t DID = uint16_t(GetBitsReverse(aPacket[0], DIDRange));
	uint16_t SID = uint16_t(GetBitsReverse(aPacket[0], SIDRange));
	SetBitsReverse(aPacket[0], DIDRange, SID);
	SetBitsReverse(aPacket[0], SIDRange, DID);
}

class SimIopCluster_c: public IopClusterBase_i {
protected:
	class ChannelIToC_c : public Channel_i {
	public:
		ChannelIToC_c(const Configuration_c &aConfig, size_t aChannelId, Mainframe_c &aMainframe, class SimIopCluster_c &aParent);
		~ChannelIToC_c() {}
		virtual void ChannelTick() override;
		virtual bool NeedsChannelTick() const override { return true; }
		virtual CLogger_c &GetLogger() const override { return mLogger; }
		bool ReadyToSend() const;
		void QueuePacket(const std::vector<CInt_t> &aPacket);
		virtual void HandleActivation() override {
			mLogger << setloglevel(LogLevel_IoActivity) << " primed to transmit " << DecPrinter(BufferLeft()) << " words at address " << HexPrinter(GetAddress()) << std::endl;
		}
		virtual void HandleDeactivation() override {
			mLogger << setloglevel(LogLevel_IoActivity) << " went inactive" << std::endl;
		}
		size_t GetQueueLength() const { return mPacketQueue.size(); }
		virtual void SetAddress(CAddr_t aAddress) override {
			mLogger << setloglevel(LogLevel_IoTrace) << "======== Channel address is set to " << HexPrinter(aAddress) << std::endl;
			Channel_i::SetAddress(aAddress);
		}
	protected:
		void SendPacket(const std::vector<CInt_t> &aPacket, bool aReplace = false);
		std::queue<std::vector<CInt_t>> mPacketQueue;
		std::vector<CInt_t> mPacket;
		size_t mPacketIdx;
		bool mPacketValid;
		mutable CLogger_c mLogger;
		SimIopCluster_c &mParent;
	};

	class ChannelCToI_c : public Channel_i {
	public:
		ChannelCToI_c(const Configuration_c &aConfig, size_t aChannelId, Mainframe_c &aMainframe, class SimIopCluster_c &aParent);
		~ChannelCToI_c() {}
		virtual void ChannelTick() override;
		virtual bool NeedsChannelTick() const override { return true; }
		virtual CLogger_c &GetLogger() const override { return mLogger; }
		bool HasPacket();
		std::vector<CInt_t> GetPacket();
		virtual void HandleActivation() override {
			mLogger << setloglevel(LogLevel_IoActivity) << " primed to transmit " << DecPrinter(BufferLeft()) << " words at address " << HexPrinter(GetAddress()) << std::endl;
		}
		virtual void HandleDeactivation() override {
			mLogger << setloglevel(LogLevel_IoActivity) << " went inactive" << std::endl;
		}
		virtual void SetAddress(CAddr_t aAddress) override {
			mLogger << setloglevel(LogLevel_IoTrace) << "======== Channel address is set to " << HexPrinter(aAddress) << std::endl;
			Channel_i::SetAddress(aAddress);
		}
	protected:
		std::vector<CInt_t> mPacket;
		size_t mPacketIdx;
		bool mPacketValid;
		mutable CLogger_c mLogger;
		SimIopCluster_c &mParent;
	};

public:
	typedef std::unique_ptr<ChannelIToC_c> (*IToCChannelCreator_t)(SimIopCluster_c *aThis, const Configuration_c &aConfig);
	typedef std::unique_ptr<ChannelCToI_c> (*CToIChannelCreator_t)(SimIopCluster_c *aThis, const Configuration_c &aConfig);
	explicit SimIopCluster_c(const Configuration_c &aConfig, size_t aIopId, class Mainframe_c &aMainframe, IToCChannelCreator_t aIToCChannelCreator = CreateIToCChannel, CToIChannelCreator_t aCToIChannelCreator = CreateCToIChannel);

	virtual void RegisterCommands(CommandHooks_t &aHooks) override;
	virtual void GetPeripheralStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const override;
	virtual void GetCpuStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const override {}
	virtual std::string GetName() const override;
	virtual std::string GetLongName() const override;
	
	CLogger_c &GetLogger() const { return mLogger; }
	class Mainframe_c &GetMainframe() { return *mMainframe; }

	virtual void MasterClear() override;

	virtual void Tick() override;

	ChannelIToC_c &GetIToCChannel() { return *mIToCChannel; }
	ChannelCToI_c &GetCToIChannel() { return *mCToIChannel; }

	IopEPacket_c GenerateResponse(IopEPacket_c &aRequest) const;
	IopEPacket_c GenerateResponse() const;
	void SendPacket(IopEPacket_c &aPacket);
	void SendPacket(std::vector<CInt_t> &aPacket);
	MachineTypes_e GetMachineType() const;
	IopRevision_e GetIopRevision() const { return mIopRevision; }
	size_t GetId() const { return mIopId; }
	virtual void Dump(size_t aIdent = 0) const  override {}
	virtual void DeadStart() override {}
protected:
	static std::unique_ptr<ChannelIToC_c> CreateIToCChannel(SimIopCluster_c *aThis, const Configuration_c &aConfig);
	static std::unique_ptr<ChannelCToI_c> CreateCToIChannel(SimIopCluster_c *aThis, const Configuration_c &aConfig);
	enum class States_e {
		Reset,
		WaitTimeSent,
		WaitISent,
		WaitJSent,
		RunningJ90,
		RunningYEL,
		KernelPanic
	} mState;
	IopRevision_e mIopRevision;
	bool mMasterCluster;
	mutable CLogger_c mLogger;
	class Mainframe_c *mMainframe;
	size_t mIopId;
	uint8_t mSeqSent;
	uint8_t mSeqReceived;
	size_t mPacketsReceived;
	size_t mPacketsSent;

	std::vector<std::unique_ptr<class SimIopDisk_c>> mDisks;
	std::vector<std::unique_ptr<class SimIopConsole_c>> mConsoles;
	boost::property_tree::ptree mDefaultConsoleConfig;
	std::vector<std::unique_ptr<class SimIopEth_c>> mEthInterfaces;
	std::unique_ptr<class SimIopPeripheral_c> mIopPeripheral;

	std::unique_ptr<ChannelIToC_c> mIToCChannel;
	std::unique_ptr<ChannelCToI_c> mCToIChannel;

	void HandleTPacket(std::vector<CInt_t> &aPacket);
	void HandleAPacket(std::vector<CInt_t> &aPacket);
	void HandleKPacket(std::vector<CInt_t> &aPacket);

	void HandleIosEOPacket(IopEPacket_c &aPacket);
	void HandleIosEZPacket(IopEPacket_c &aPacket);
	void HandleIosEDPacket(IopEPacket_c &aPacket);
	void HandleIosEPPacket(IopEPacket_c &aPacket);

	void MakeConsoles(size_t aMaxConsoleId);

	boost::optional<std::string> mKernelFileName;
	size_t mKernelPreamble;
	boost::optional<std::string> mParameterFileName;
	boost::optional<std::string> mRamFsFileName;
	uint32_t mBootCluster;
	uint32_t mBootChannel;
	uint8_t mLogicalMainframeNumber;
	uint8_t mOWSProtocolLevel;
	boost::optional<std::string> mRunLevel;
	bool mDontTouchExchangePacket;
};

#endif // __SIM_IOP_H__
