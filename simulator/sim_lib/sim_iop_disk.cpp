// Cray-XMP I/O Processor simulator class

#include "sim_iop_disk.h"
#include "sim_iop.h"

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

// SimIopCluster_c::IopDisk_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DiskGeometry_s {
	const char *Name;
	DeviceTypes_e DeviceType;
	uint32_t SectorSize;
	std::streamoff Sectors;
	std::streamoff Heads;
	std::streamoff Cylinders;
	uint32_t StripeFactor;
};

const DiskGeometry_s cDiskGeometry[] = {
	{"DD00",   DeviceTypes_e::DD00,    512, 42,  8,       890, 1},
	{"DD19",   DeviceTypes_e::DD19,    512, 18, 10,       411, 0},
	{"DD29",   DeviceTypes_e::DD29,    512, 18, 10,       823, 0},
	{"DD49",   DeviceTypes_e::DD49,    512, 42,  8,       890, 8},
	{"DD39",   DeviceTypes_e::DD39,    512, 24,  5,       842, 0},
	{"DD10",   DeviceTypes_e::DD10,    512, 12, 19,      1420, 0},
	{"DD40",   DeviceTypes_e::DD40,    512, 48, 19,      1420, 8},
	{"DD50",   DeviceTypes_e::DD50,    512, 42, 16,       890, 8},
	{"DD11",   DeviceTypes_e::DD11,    512, 12, 15,      1635, 0},
	{"DD41",   DeviceTypes_e::DD41,    512, 48, 15,      1635, 8},
	{"DD60",   DeviceTypes_e::DD60,   2048, 23,  2,      2611, 2},
	{"DD61",   DeviceTypes_e::DD61,    512, 11, 19,      2611,11},
	{"DD62",   DeviceTypes_e::DD62,    512, 28,  9,      2655, 6},
	{"DD42",   DeviceTypes_e::DD42,    512, 48, 19,      2611, 8},
	{"DA62",   DeviceTypes_e::DA62,   2048, 28,  9,      2655, 6},
	{"DA60",   DeviceTypes_e::DA60,   8192, 23,  2,      2611, 2},
	{"DD301",  DeviceTypes_e::DD301,   512, 25,  7,      1930, 5},
	{"DA301",  DeviceTypes_e::DA301,  2048, 25,  7,      1930, 5},
	{"DD302",  DeviceTypes_e::DD302,   512, 28,  7,      2251, 4},
	{"DA302",  DeviceTypes_e::DA302,  2048, 28,  7,      2251, 4},
	{"DDESDI", DeviceTypes_e::DDESDI,  512,  1,  1,    170100, 1},
	{"DD3",	   DeviceTypes_e::DD3,     512,  1,  1,    334200, 1},
	{"DDLDAS", DeviceTypes_e::DDLDAS,  512,  1,  1,   1269114, 1},
	{"DDAS2",  DeviceTypes_e::DDAS2,   512,  1,  1,   2502000, 1},
	{"DD4",    DeviceTypes_e::DD4,     512,  1,  1,    653000, 1},
	{"RD-1",   DeviceTypes_e::RD1,     512,  1,  1,    334200, 1},
	{"DDIMEM", DeviceTypes_e::DDIMEM,  512,  1,  1,     32768, 1},
	{"DD5S",   DeviceTypes_e::DD5S,    512,  1,  1,    781000, 1},
	{"DD5I",   DeviceTypes_e::DD5I,    512,  1,  1,    723000, 1},
	{"DD_U",   DeviceTypes_e::DD_U,    512,  1,  1, 0xfffffff, 1},
	{"DD6S",   DeviceTypes_e::DD6S,    512,  1,  1,   2389000, 1},
	{"DD314",  DeviceTypes_e::DD7S,    512,  1,  1,   1102000, 1},
	{"DD318",  DeviceTypes_e::DD318,   512,  1,  1,   2342634, 1},
	{"DD501",  DeviceTypes_e::DD501,   512,  1,  1,   6019481, 1}
};

SimIopDisk_c::SimIopDisk_c(const Configuration_c &aConfig, const std::string &aDiskId, SimIopCluster_c &aParent):
	mDiskId(aDiskId),
	mLogger(aConfig, std::string("DSK_").append(aDiskId).c_str()),
	mParent(aParent),
	mCurrentHead(0),
	mCurrentSector(0),
	mCurrentCylinder(0),
	mReads(0),
	mWrites(0) 
{
	mLogger.SetParent(aParent.GetLogger());
	mImageFileName = aConfig.get<std::string>("ImageFileName");
	mPysicalDeviceId = aConfig.get<uint32_t>("PysicalDeviceId");
	mIopNumber = aConfig.get<uint8_t>("IopNumber");
	mUnit = aConfig.get<uint8_t>("Unit");
	boost::optional<std::string> DeviceType = aConfig.get_optional<std::string>("DeviceType");
	boost::optional<std::streamoff> SectorCnt = aConfig.get_optional<std::streamoff>("SectorCnt");
	if (DeviceType.is_initialized()) {
		bool Found = false;
		for (auto &DiskGeometry : cDiskGeometry) {
			if (DeviceType.get() == DiskGeometry.Name) {
				Found = true;
				mNumHeads = DiskGeometry.Heads;
				mNumSectors = DiskGeometry.Sectors;
				mNumCylinders = DiskGeometry.Cylinders;
				mSectorSize = DiskGeometry.SectorSize * sizeof(CInt_t);
				break;
			}
		}
		if (!Found) throw InvalidParameter_x("Unknown DeviceType");
	} else if (SectorCnt.is_initialized()) {
		mNumHeads = 1;
		mNumSectors = 1;
		mNumCylinders = SectorCnt.get();
		if (mNumCylinders == std::streamoff(0)) throw InvalidParameter_x("Cylinders must be greater than 0");
		mSectorSize = aConfig.get<uint32_t>("SectorSize", 4096);
		if (mSectorSize == 0) throw InvalidParameter_x("SectorSize must be greater than 0");
		if (mSectorSize % 1024 != 0) throw InvalidParameter_x("SectorSize must be a multiple of 1024");
	} else {
		mNumHeads = aConfig.get<std::streamoff>("Heads");
		if (mNumHeads == std::streamoff(0)) throw InvalidParameter_x("Heads must be greater than 0");
		mNumSectors = aConfig.get<std::streamoff>("Sectors");
		if (mNumSectors == std::streamoff(0)) throw InvalidParameter_x("Sectors must be greater than 0");
		mNumCylinders = aConfig.get<std::streamoff>("Cylinders");
		if (mNumCylinders == std::streamoff(0)) throw InvalidParameter_x("Cylinders must be greater than 0");
		mSectorSize = aConfig.get<uint32_t>("SectorSize", 4096);
		if (mSectorSize == 0) throw InvalidParameter_x("SectorSize must be greater than 0");
		if (mSectorSize % 1024 != 0) throw InvalidParameter_x("SectorSize must be a multiple of 1024");
	}
	MasterClear();

	if (!boost::filesystem::exists(mImageFileName)) {
		std::fstream File;

		std::cout << "Creating disk image file: " << mImageFileName << "..." << std::flush;
		File.open(mImageFileName, std::ios::out | std::ios::binary);
		CRAY_ASSERT(!File.bad());
		//size_t DiskSize = mNumHeads * mNumSectors * mNumCylinders * mSectorSize;
		//File.seekp(DiskSize - 1);
		CRAY_ASSERT(!File.bad());
		char Data = 0;
		File.write(&Data, sizeof(Data));
		CRAY_ASSERT(!File.bad());
		File.close();
		std::cout << " done" << std::endl;
	}
}

void SimIopDisk_c::Read(std::streamoff aCylinder, std::streamoff aHead, std::streamoff aSector, uint32_t aSectorCnt, CAddr_t aMainframeAddr) {
	mCurrentCylinder = aCylinder;
	mCurrentHead = aHead;
	mCurrentSector = aSector;
	std::streamoff ImageOffset = mSectorSize * (aCylinder * mNumHeads * mNumSectors + aHead * mNumSectors + aSector);

	mReads += mSectorSize / 1024 * aSectorCnt;

	std::stringstream EventStr;
	EventStr << "Reading sector " << "CHS:" << DecPrinter(aCylinder) << "-" << DecPrinter(aHead) << "-" << DecPrinter(aSector) << " (offset: " << HexPrinter(ImageOffset) << ") " << " to memory address " << HexPrinter(aMainframeAddr, 4) << " sector count: " << DecPrinter(aSectorCnt);

	mLogger << setloglevel(LogLevel_IoActivity) << EventStr.str() << std::endl;
	mParent.GetMainframe().GetEventDispatcher().Fire(EventStr.str());

	std::fstream File(mImageFileName, std::ios::in | std::ios::binary);
	CRAY_ASSERT(!File.bad());
	File.seekg(ImageOffset);
	CRAY_ASSERT(!File.bad());

	LogLine_c TraceLine = mLogger << setloglevel(LogLevel_IoTrace);

	std::streamoff AccessSize = std::streamoff(mSectorSize) * std::streamoff(aSectorCnt) / sizeof(CInt_t);
	const size_t cLineSize = 4;
	CRAY_ASSERT(AccessSize % cLineSize == 0);
	AccessSize = AccessSize / cLineSize;
	for (std::streamoff i = 0; i < AccessSize; ++i) {
		CInt_t Data[cLineSize];
		if (!File.eof()) {
			File.read((char*)(&Data), sizeof(Data));
			CRAY_ASSERT(!File.bad());
		} else {
			// If we're at EOF, simply 0-fill the data
			std::fill(Data, Data + cLineSize, 0);
		}
		if (TraceLine.good()) {
			TraceLine << HexPrinter(aMainframeAddr + i * cLineSize) << " : ";
			for (size_t j = 0; j < cLineSize; ++j) {
				TraceLine << HexPrinter(SwapBytes(Data[j])) << " ";
			}
			TraceLine << " - ";
			for (size_t j = 0; j < cLineSize; ++j) {
				TraceLine << AsciiDumpPrinter(SwapBytes(Data[j])) << " ";
			}
			TraceLine << std::endl;
		}
		for (size_t j = 0; j < cLineSize; ++j) {
			mParent.GetMainframe().MemWrite(CAddr_t(aMainframeAddr + i * cLineSize + j), Data[j]);
		}
	}
	TraceLine << std::endl;
	File.close();
}

void SimIopDisk_c::Read(std::streamoff aSectorIdx, uint32_t aSectorCnt, CAddr_t aMainframeAddr) {
	std::streamoff Sector = aSectorIdx % mNumSectors;
	aSectorIdx = aSectorIdx / mNumSectors;
	std::streamoff Head = aSectorIdx % mNumHeads;
	aSectorIdx = aSectorIdx / mNumHeads;
	std::streamoff Cylinder = aSectorIdx;
	mLogger << setloglevel(LogLevel_IoActivity) << "Reading sector " << DecPrinter(aSectorIdx) << " CHS:" << DecPrinter(Cylinder) << "-" << DecPrinter(Head) << "-" << DecPrinter(Sector) << " mNumCylinder: " << DecPrinter(mNumCylinders) << " to memory address " << HexPrinter(aMainframeAddr, 4) << " sector count: " << DecPrinter(aSectorCnt) << std::endl;
	CRAY_ASSERT(Cylinder < mNumCylinders);
	Read(Cylinder, Head, Sector, aSectorCnt, aMainframeAddr);
}

void SimIopDisk_c::Write(std::streamoff aCylinder, std::streamoff aHead, std::streamoff aSector, uint32_t aSectorCnt, CAddr_t aMainframeAddr) {
	mCurrentCylinder = aCylinder;
	mCurrentHead = aHead;
	mCurrentSector = aSector;
	std::streamoff ImageOffset = mSectorSize * (aCylinder * mNumHeads * mNumSectors + aHead * mNumSectors + aSector);

	mWrites += mSectorSize / 1024 * aSectorCnt;

	std::stringstream EventStr;
	EventStr << "Writing sector " << "CHS:" << DecPrinter(aCylinder) << "-" << DecPrinter(aHead) << "-" << DecPrinter(aSector) << " (offset: " << HexPrinter(ImageOffset) << ") " << " from memory address " << HexPrinter(aMainframeAddr, 4) << " sector count: " << DecPrinter(aSectorCnt);

	mLogger << setloglevel(LogLevel_IoActivity) << EventStr.str() << std::endl;
	mParent.GetMainframe().GetEventDispatcher().Fire(EventStr.str());

	std::fstream File;
	if (boost::filesystem::exists(mImageFileName)) {
		File.open(mImageFileName, std::ios::out | std::ios::in | std::ios::binary);
	} else {
		File.open(mImageFileName, std::ios::out | std::ios::binary);
	}
	CRAY_ASSERT(!File.bad());
	File.seekp(ImageOffset);
	CRAY_ASSERT(File.good());
	CRAY_ASSERT(!File.bad());

	LogLine_c TraceLine = mLogger << setloglevel(LogLevel_IoTrace);

	std::streamoff AccessSize = std::streamoff(mSectorSize) * std::streamoff(aSectorCnt) / sizeof(CInt_t);
	const size_t cLineSize = 4;
	CRAY_ASSERT(AccessSize % cLineSize == 0);
	AccessSize = AccessSize / cLineSize;
	for (std::streamoff i = 0; i < AccessSize; ++i) {
		CInt_t Data[cLineSize];
		for (size_t j = 0; j < cLineSize; ++j) {
			Data[j] = mParent.GetMainframe().MemRead<CInt_t>(CAddr_t(aMainframeAddr + i * cLineSize + j));
		}

		File.write((char*)(&Data), sizeof(Data));
		CRAY_ASSERT(!File.bad());

		if (TraceLine.good()) {
			TraceLine << HexPrinter(aMainframeAddr + i * cLineSize) << " : ";
			for (size_t j = 0; j < cLineSize; ++j) {
				TraceLine << HexPrinter(SwapBytes(Data[j])) << " ";
			}
			TraceLine << " - ";
			for (size_t j = 0; j < cLineSize; ++j) {
				TraceLine << AsciiDumpPrinter(SwapBytes(Data[j])) << " ";
			}
			TraceLine << std::endl;
		}
	}
	TraceLine << std::endl;
	File.close();
}

void SimIopDisk_c::Write(std::streamoff aSectorIdx, uint32_t aSectorCnt, CAddr_t aMainframeAddr) {
	std::streamoff Sector = aSectorIdx % mNumSectors;
	aSectorIdx = aSectorIdx / mNumSectors;
	std::streamoff Head = aSectorIdx % mNumHeads;
	aSectorIdx = aSectorIdx / mNumHeads;
	std::streamoff Cylinder = aSectorIdx;
	CRAY_ASSERT(Cylinder < mNumCylinders);
	Write(Cylinder, Head, Sector, aSectorCnt, aMainframeAddr);
}

void SimIopDisk_c::GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const {
	aStatus.put(aLongFormat ? "Cylinder" : "C", mCurrentCylinder);
	aStatus.put(aLongFormat ? "Head" : "H", mCurrentHead);
	aStatus.put(aLongFormat ? "Sector" : "S", mCurrentSector);
	aStatus.put(aLongFormat ? "Reads" : "R", mReads);
	aStatus.put(aLongFormat ? "Writes" : "W", mWrites);
}

std::string SimIopDisk_c::GetName() const {
	std::stringstream Name;
	Name << "DSK_" << mDiskId;
	return Name.str();
}

void SimIopDisk_c::RegisterCommands(CommandHooks_t &aHooks) {}
void SimIopDisk_c::MasterClear() {}


#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif
