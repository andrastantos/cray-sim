// Simulation of expander peripherals (essentially F packet processing) for IOSD

#ifndef __SIM_IOP_PERIPH_H__
#define __SIM_IOP_PERIPH_H__

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

class SimIopPeripheral_c {
public:
	SimIopPeripheral_c(const Configuration_c &aConfig, class SimIopCluster_c &aParent);
	~SimIopPeripheral_c() {}
	void GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const;
	std::string GetName() const;
	void RegisterCommands(CommandHooks_t &aHooks);
	void Reset();
	void HandleIosDPacket(std::vector<CInt_t> &aPacket);
	void HandleIosEPacket(IopEPacket_c &aPacket);
protected:
	mutable CLogger_c mLogger;
	SimIopCluster_c &mParent;

	std::fstream mPrinterFile;
	std::fstream mTapeFile;
	std::fstream mDiskFile;
	std::fstream mFile;

	std::string mPrinterFileName;
	std::string mDiskDir;
	std::string mTapeDir;

	uint16_t mLastTapeFile;
	bool mRewindOnClose;
	void Write(
		uint32_t *aAddr,
		uint32_t *aDataLen,
		uint32_t *aTransferLen,
		size_t aNumBuffers,
		std::ostream &aFile
	);
	void Read(
		uint32_t *aAddr,
		uint32_t *aDataLen,
		uint32_t *aTransferLen,
		size_t aNumBuffers,
		std::istream &aFile
	);

	void HandlePacket(std::vector<CInt_t> &aPacket, bool aIosEStyle);
	void SendPacket(std::vector<CInt_t> &aPacket, bool aIosEStyle);
};





namespace IosFPacket_n {
	enum class FunctionCodes_e {
		OPN   = 3,     // open function
		CLS   = 4,     // close function
		RD    = 5,     // read function
		RDHLD = 6,     // read hold function
		RDRD  = 7,     // read read function
		WR    = 8,     // write function
		WRHLD = 9,     // write hold function
		WRWR  = 10,    // write write function
		DRF32 = 32     // driver function (set param)
	};
	enum class StatusCodes_e {
		OK    = 0,     // ok status
		PROER = 3,     // protocol error
		ILCHN = 4,     // illegal channel
		ILFNC = 5,     // illegal function
		ILDRV = 6,     // illegal driver
		DADDR = 7,     // data address error
		DLEN  = 8,     // data length error
	};
	
	extern const char *LPNAME;
	extern const char *EXNAME;
	extern const char *ETNAME;
	extern const char *EFNAME;

	enum class ChannelNumbers_e {
		Printer = 070,
		Disk    = 072,
		Tape    = 071,
		File    = 073
	};

	// Word 0
	const BitRange_s DestinationIdRange(48, 63);
	const BitRange_s SourceIdRange(32, 47);
	const BitRange_s IopNumberRange(22, 24);
	const BitRange_s ChannelNumberRange(16, 21);
	const BitRange_s RequestFunctionRange(8, 15);
	const BitRange_s StatusNumberRange(7, 7); // for multi-part requests the response status number indicates the request part the status applies to 0: part 1; 1: part 2
	const BitRange_s StatusRange(0, 6);

	// Word 1
	//    line printer
	const BitRange_s PrinterDevRange(56, 63);
	//    disk
	const BitRange_s DiskDevRange(0, 23);
	//    superlink
	const BitRange_s SfBpRange(0, 63);
	//    cpu
	// - this is a char[8] string

	// Open request
	//    word 2
	// - this is a driver name string
	//    word 3
	const BitRange_s CoChannelNumberRange(56, 63);
	const BitRange_s ChannelTypeRange(48, 55);
	const BitRange_s TimeoutRange(32, 47); // Timeout measured in 100ms
	//    word 4
	// - not used

	// Close request
	//    word 2
	//    word 3
	//    word 4
	// - not used

	// Read/write requests
	//    word 2
	const BitRange_s AddrReq1Range(32, 63);
	const BitRange_s AddrReq2Range(0, 31);
	//    word 3
	const BitRange_s DataLenReq1Range(32, 63);
	const BitRange_s DataLenReq2Range(0, 31);
	//    word 4
	const BitRange_s TransferLenReq1Range(32, 63);
	const BitRange_s TransferLenReq2Range(0, 31);

	// Driver request
	//    word 2
	//    word 3
	//    word 4
	// - seem to be strings

	// Word 5
	//    line printer
	const BitRange_s PrintPlotRange(63, 63);
	const BitRange_s RotateRange(62, 62);
	const BitRange_s WrapLinesRange(61, 61);
	const BitRange_s ColumnsPerPageRange(56, 60);
	const BitRange_s UnprintableCharRange(48, 55);
	//    disk
	const BitRange_s IoBlockNumberRange(48, 63);
	//    tape (open)
	const BitRange_s ReadWriteRange(56, 63);
	const BitRange_s RewindRange(48, 55);
	const BitRange_s UserRange(32, 47);
	const BitRange_s FileNumberRange(16, 31);
	//    file driver
	//const BitRange_s ReadWriteRange(56, 63); // 0: read; 1: write
	const BitRange_s OverwriteRange(48, 55);
	//    driver reqests
	//  - seems to be a string
};

#endif // __SIM_IOP_PERIPH_H__
