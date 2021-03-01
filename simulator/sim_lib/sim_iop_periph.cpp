// Cray-XMP I/O Processor simulator class
#include "sim_iop.h"
#include "sim_iop_periph.h"

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

// SimIopPeripheral_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SimIopPeripheral_c::SimIopPeripheral_c(const Configuration_c &aConfig, SimIopCluster_c &aParent):
	mLogger(aConfig, "PERI"),
	mParent(aParent)
{
	mLogger.SetParent(aParent.GetLogger());
	mPrinterFileName = aConfig.get<std::string>("PrinterFileName");
	mDiskDir = aConfig.get<std::string>("DiskDir");
	mTapeDir = aConfig.get<std::string>("TapeDir");
	mLastTapeFile = 0;
	mRewindOnClose = true;
}

void SimIopPeripheral_c::GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const {}

std::string SimIopPeripheral_c::GetName() const {
	std::stringstream Name;
	Name << "EXP";
	return Name.str();
}

void SimIopPeripheral_c::RegisterCommands(CommandHooks_t &aHooks) {}
void SimIopPeripheral_c::Reset() {
}

void SimIopPeripheral_c::Write(
	uint32_t *aAddr,
	uint32_t *aDataLen,
	uint32_t *aTransferLen,
	size_t aNumBuffers,
	std::ostream &aFile
) {
	for (size_t Idx = 0; Idx < aNumBuffers; ++Idx) {
		if (aDataLen[Idx] == 0) continue;

		mLogger << setloglevel(LogLevel_IoTrace) << "Writing to expander device from address: " << HexPrinter(aAddr[Idx]) << " len: " << HexPrinter(aDataLen[Idx]) << std::endl;
		LogLine_c TraceLine = mLogger << setloglevel(LogLevel_IoTrace);
		const size_t cLineSize = 4;
		size_t AccessSize = (aDataLen[Idx] + sizeof(CInt_t) * cLineSize - 1) / (sizeof(CInt_t) * cLineSize);
		size_t DataLeft = aDataLen[Idx];
		for (size_t i = 0; i < AccessSize; ++i) {
			CAddr_t BaseAddr = CAddr_t(aAddr[Idx] + i * cLineSize);
			CInt_t Data[cLineSize];
			size_t LineSize = std::min(DataLeft, cLineSize * sizeof(CInt_t));
			for (size_t j = 0; j < LineSize; j += sizeof(CInt_t)) {
				Data[j / sizeof(CInt_t)] = mParent.GetMainframe().MemRead<CInt_t>(CAddr_t(BaseAddr + j / sizeof(CInt_t)));
			}

			aFile.write((char*)(&Data), LineSize);
			CRAY_ASSERT(!aFile.bad());

			if (TraceLine.good()) {
				TraceLine << HexPrinter(aAddr[Idx] + i * cLineSize) << " : ";
				for (size_t j = 0; j < LineSize; j += sizeof(CInt_t)) {
					TraceLine << HexPrinter(SwapBytes(Data[j / sizeof(CInt_t)])) << " ";
				}
				TraceLine << " - ";
				for (size_t j = 0; j < LineSize; j += sizeof(CInt_t)) {
					TraceLine << AsciiDumpPrinter(SwapBytes(Data[j / sizeof(CInt_t)])) << " ";
				}
				TraceLine << std::endl;
			}
			DataLeft -= LineSize;
		}
		TraceLine << std::endl;
		CRAY_ASSERT(DataLeft == 0);
		aTransferLen[Idx] = aDataLen[Idx];
	}
}

void SimIopPeripheral_c::Read(
	uint32_t *aAddr,
	uint32_t *aDataLen,
	uint32_t *aTransferLen,
	size_t aNumBuffers,
	std::istream &aFile
) {
	for (size_t Idx = 0; Idx < aNumBuffers; ++Idx) {
		if (aDataLen[Idx] == 0) continue;
		if (aFile.eof()) {
			aTransferLen[Idx] = 0;
			continue;
		}

		mLogger << setloglevel(LogLevel_IoTrace) << "Reading from expander device to address: " << HexPrinter(aAddr[Idx]) << " len: " << HexPrinter(aDataLen[Idx]) << std::endl;
		LogLine_c TraceLine = mLogger << setloglevel(LogLevel_IoTrace);
		const size_t cLineSize = 4;
		size_t AccessSize = (aDataLen[Idx] + sizeof(CInt_t) * cLineSize - 1) / (sizeof(CInt_t) * cLineSize);
		size_t DataLeft = aDataLen[Idx];
		for (size_t i = 0; i < AccessSize; ++i) {
			CAddr_t BaseAddr = CAddr_t(aAddr[Idx] + i * cLineSize);
			CInt_t Data[cLineSize];
			size_t LineSize = std::min(DataLeft, cLineSize * sizeof(CInt_t));

			aFile.read((char*)(&Data), LineSize);
			size_t BytesRead = size_t(aFile.gcount());
			if (BytesRead < LineSize) {
				CRAY_ASSERT(aFile.eof());
			} else {
				CRAY_ASSERT(!aFile.bad());
			}
			LineSize = BytesRead;

			for (size_t j = 0; j < LineSize; j += sizeof(CInt_t)) {
				mParent.GetMainframe().MemWrite(CAddr_t(BaseAddr + j / sizeof(CInt_t)), Data[j / sizeof(CInt_t)]);
			}

			if (TraceLine.good()) {
				TraceLine << HexPrinter(aAddr[Idx] + i * cLineSize) << " : ";
				for (size_t j = 0; j < LineSize; j += sizeof(CInt_t)) {
					TraceLine << HexPrinter(SwapBytes(Data[j / sizeof(CInt_t)])) << " ";
				}
				TraceLine << " - ";
				for (size_t j = 0; j < LineSize; j += sizeof(CInt_t)) {
					TraceLine << AsciiDumpPrinter(SwapBytes(Data[j / sizeof(CInt_t)])) << " ";
				}
				TraceLine << std::endl;
			}
			DataLeft -= LineSize;
			if (aFile.eof()) break;
		}
		TraceLine << std::endl;
		CRAY_ASSERT(DataLeft == 0 || aFile.eof());
		aTransferLen[Idx] = aDataLen[Idx] - uint32_t(DataLeft);
//		aDataLen[Idx] = aTransferLen[Idx];
	}
}

void SimIopPeripheral_c::SendPacket(std::vector<CInt_t> &aPacket, bool aIosEStyle) {
	if (!aIosEStyle) {
		mParent.SendPacket(aPacket);
		return;
	} else {
		IopEPacket_c Response = mParent.GenerateResponse();
		Response.SetSize(aPacket.size()+1);
		Response.Data(0) = 0x6600030000000000ULL; // Let's assume response code is the same as request, except lower-case 'f'. Could be wrong...
		for (size_t Idx = 0; Idx < aPacket.size(); ++Idx) Response.Data(Idx + 1) = aPacket[Idx];
		mParent.SendPacket(Response);
	}
}

void SimIopPeripheral_c::HandleIosDPacket(std::vector<CInt_t> &aPacket) {
	HandlePacket(aPacket, false);
}

void SimIopPeripheral_c::HandleIosEPacket(IopEPacket_c &aPacket) {
	// Extract the IosD-style packet
	std::vector<CInt_t> Packet(aPacket.GetLength() - 2);
	for (size_t Idx = 0; Idx < Packet.size(); ++Idx) Packet[Idx] = aPacket.Data(Idx + 1);
	HandlePacket(Packet, true);
}

// IOSD packet structure is in fpacket.h, IOSE packet structure is in epackfp.h
// They seem to be more or less identical, except for the filename location and size
void SimIopPeripheral_c::HandlePacket(std::vector<CInt_t> &aPacket, bool aIosEStyle) {
	using namespace IosFPacket_n;

	FunctionCodes_e Function = FunctionCodes_e(GetBits(aPacket[0], RequestFunctionRange));
	StatusCodes_e Status = StatusCodes_e(GetBits(aPacket[0], StatusRange));
	uint8_t IopNumber = uint8_t(GetBits(aPacket[0], IopNumberRange));
	ChannelNumbers_e ChannelNumber = ChannelNumbers_e(GetBits(aPacket[0], ChannelNumberRange));

	switch (Function) {
		case FunctionCodes_e::OPN: {
			switch (ChannelNumber) {
				case ChannelNumbers_e::Printer:
					mPrinterFile.open(mPrinterFileName, std::ios::out | std::ios::app);
					ReversePacket(aPacket);
					SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
					SetBits(aPacket[0], StatusNumberRange, uint64_t(0));
					SendPacket(aPacket, aIosEStyle);
				break;
				case ChannelNumbers_e::Disk:
					ReversePacket(aPacket);
					SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
					SetBits(aPacket[0], StatusNumberRange, uint64_t(0));
					SendPacket(aPacket, aIosEStyle);
				break;
				case ChannelNumbers_e::Tape: {
					bool DoWrite = GetBits(aPacket[5], ReadWriteRange) != 0;
					mRewindOnClose = GetBits(aPacket[5], RewindRange) != 0;
					uint16_t FileNumber = uint16_t(GetBits(aPacket[5], FileNumberRange));
					mLastTapeFile = FileNumber;

					char UserName[9];
					UserName[8] = 0;
					memcpy(UserName, &aPacket[1], 8);
					*(uint64_t *)(UserName) = SwapBytes(*(uint64_t *)(UserName));

					boost::filesystem::path FullPath(mTapeDir);
					if (!boost::filesystem::exists(FullPath)) boost::filesystem::create_directories(FullPath);
					std::stringstream FileNumberStr;
					FileNumberStr << DecPrinter(FileNumber) << ".dat";
					FullPath /= FileNumberStr.str();
					if (!boost::filesystem::exists(FullPath) && !DoWrite) {
						ReversePacket(aPacket);
						SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::ILDRV)); // :::TODO how to return errors?
						SetBits(aPacket[0], StatusNumberRange, uint64_t(0));
						SendPacket(aPacket, aIosEStyle);
						break;
					}
					mFile.open(FullPath.generic_string(), std::ios::binary | (DoWrite ? std::ios::out : std::ios::in));
					ReversePacket(aPacket);
					SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
					SetBits(aPacket[0], StatusNumberRange, uint64_t(0));
					SendPacket(aPacket, aIosEStyle);
					break;
				}
				case ChannelNumbers_e::File:
					ReversePacket(aPacket);
					SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
					SetBits(aPacket[0], StatusNumberRange, uint64_t(0));
					SendPacket(aPacket, aIosEStyle);
				break;
				default:
					CRAY_ASSERT(false);
			}
		} break;
		case FunctionCodes_e::CLS: {
			switch (ChannelNumber) {
				case ChannelNumbers_e::Printer:
					mPrinterFile.close();
				break;
				case ChannelNumbers_e::Disk:
					mDiskFile.close();
				break;
				case ChannelNumbers_e::Tape:
					if (mRewindOnClose) {
						mLastTapeFile = 0;
					} else {
						++mLastTapeFile;
					}
					mTapeFile.close();
				break;
				case ChannelNumbers_e::File:
					mFile.close();
				break;
				default:
					CRAY_ASSERT(false);
			}
			ReversePacket(aPacket);
			SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
			SetBits(aPacket[0], StatusNumberRange, uint64_t(0));
			SendPacket(aPacket, aIosEStyle);
		} break;
		case FunctionCodes_e::RD:
		case FunctionCodes_e::RDHLD:
		case FunctionCodes_e::RDRD: {
			int BufferCnt = (Function == FunctionCodes_e::RDRD) ? 2 : 1;
			uint32_t Addr[2];
			uint32_t DataLen[2];
			uint32_t TransferLen[2];
			Addr[0] = uint32_t(GetBits(aPacket[2], AddrReq1Range));
			Addr[1] = uint32_t(GetBits(aPacket[2], AddrReq2Range));
			DataLen[0] = uint32_t(GetBits(aPacket[3], DataLenReq1Range));
			DataLen[1] = uint32_t(GetBits(aPacket[3], DataLenReq2Range));
			TransferLen[0] = uint32_t(GetBits(aPacket[4], TransferLenReq1Range));
			TransferLen[1] = uint32_t(GetBits(aPacket[4], TransferLenReq2Range));
			switch (ChannelNumber) {
			case ChannelNumbers_e::Printer:
				CRAY_ASSERT(false);
				break;
			case ChannelNumbers_e::Disk:
				break;
			case ChannelNumbers_e::Tape:
				Read(Addr, DataLen, TransferLen, BufferCnt, mTapeFile);
				break;
			case ChannelNumbers_e::File:
				Read(Addr, DataLen, TransferLen, BufferCnt, mFile);
				break;
			default:
				CRAY_ASSERT(false);
			}
			ReversePacket(aPacket);
			SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
			SetBits(aPacket[0], StatusNumberRange, uint64_t(0));
			SetBits(aPacket[4], TransferLenReq1Range, uint64_t(TransferLen[0]));
			SetBits(aPacket[4], TransferLenReq2Range, uint64_t(TransferLen[1]));
			SendPacket(aPacket, aIosEStyle);
		} break;
		case FunctionCodes_e::WR:
		case FunctionCodes_e::WRHLD:
		case FunctionCodes_e::WRWR: {
			int BufferCnt = (Function == FunctionCodes_e::RDRD) ? 2 : 1;
			uint32_t Addr[2];
			uint32_t DataLen[2];
			uint32_t TransferLen[2];
			Addr[0] = uint32_t(GetBits(aPacket[2], AddrReq1Range));
			Addr[1] = uint32_t(GetBits(aPacket[2], AddrReq2Range));
			DataLen[0] = uint32_t(GetBits(aPacket[3], DataLenReq1Range));
			DataLen[1] = uint32_t(GetBits(aPacket[3], DataLenReq2Range));
			TransferLen[0] = uint32_t(GetBits(aPacket[4], TransferLenReq1Range));
			TransferLen[1] = uint32_t(GetBits(aPacket[4], TransferLenReq2Range));
			switch (ChannelNumber) {
				case ChannelNumbers_e::Printer:
					Write(Addr, DataLen, TransferLen, BufferCnt, mPrinterFile);
				break;
				case ChannelNumbers_e::Disk:
				break;
				case ChannelNumbers_e::Tape:
					Write(Addr, DataLen, TransferLen, BufferCnt, mTapeFile);
				break;
				case ChannelNumbers_e::File:
					Write(Addr, DataLen, TransferLen, BufferCnt, mFile);
				break;
				default:
					CRAY_ASSERT(false);
			}
			ReversePacket(aPacket);
			SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
			SetBits(aPacket[0], StatusNumberRange, uint64_t(0));
			SetBits(aPacket[4], TransferLenReq1Range, uint64_t(TransferLen[0]));
			SetBits(aPacket[4], TransferLenReq2Range, uint64_t(TransferLen[1]));
			SendPacket(aPacket, aIosEStyle);
		} break;
		case FunctionCodes_e::DRF32:
			switch(ChannelNumber) {
				case ChannelNumbers_e::File: {
					boost::filesystem::path FullPath(mDiskDir);
					bool DoWrite;
					bool DoOverwrite;

					if (!aIosEStyle) {
						char DirName[17];
						char FileName[17];
						DirName[16] = 0;
						FileName[16] = 0;
						memcpy(DirName, &aPacket[1], 16);
						memcpy(FileName, &aPacket[3], 16);
						*(uint64_t *)(DirName + 0) = SwapBytes(*(uint64_t *)(DirName + 0));
						*(uint64_t *)(DirName + 8) = SwapBytes(*(uint64_t *)(DirName + 8));
						*(uint64_t *)(FileName + 0) = SwapBytes(*(uint64_t *)(FileName + 0));
						*(uint64_t *)(FileName + 8) = SwapBytes(*(uint64_t *)(FileName + 8));
						DoWrite = GetBits(aPacket[5], ReadWriteRange) != 0;
						DoOverwrite = GetBits(aPacket[5], OverwriteRange) != 0;

						FullPath /= DirName;
						FullPath /= FileName;
					} else {
						const size_t MaxFileNameLen = 60 * 8;
						char FileName[MaxFileNameLen + 1];
						FileName[MaxFileNameLen] = 0;
						memcpy(FileName, &aPacket[6], MaxFileNameLen);
						for (size_t Idx = 0; Idx < MaxFileNameLen / 8; ++Idx) *(uint64_t *)(FileName + Idx * 8) = SwapBytes(*(uint64_t *)(FileName + Idx * 8));

						DoWrite = GetBits(aPacket[5], ReadWriteRange) != 0;
						DoOverwrite = GetBits(aPacket[5], OverwriteRange) != 0;

						char *PathPart = strtok(FileName, "/");
						while (PathPart != nullptr) {
							FullPath /= PathPart;
							PathPart = strtok(nullptr, "/");
						}
					}
					if (FullPath.has_parent_path()) {
						boost::filesystem::path Parent = FullPath.parent_path();
						if (!boost::filesystem::exists(Parent)) boost::filesystem::create_directories(Parent);
					}

					if (boost::filesystem::exists(FullPath) && !DoOverwrite && DoWrite) {
						ReversePacket(aPacket);
						SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::ILDRV)); // :::TODO how to return errors?
						SetBits(aPacket[0], StatusNumberRange, uint64_t(0));
						SendPacket(aPacket, aIosEStyle);
						break;
					}
					if (!boost::filesystem::exists(FullPath) && !DoWrite) {
						ReversePacket(aPacket);
						SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::ILDRV)); // :::TODO how to return errors?
						SetBits(aPacket[0], StatusNumberRange, uint64_t(0));
						SendPacket(aPacket, aIosEStyle);
						break;
					}
					std::cout << "OPENING extender file '" << FullPath.generic_string() << "' for " << (DoWrite ? "writing" : "reading") << std::endl;
					mFile.open(FullPath.generic_string(), std::ios::binary | (DoWrite ? std::ios::out : std::ios::in));
					ReversePacket(aPacket);
					SetBits(aPacket[0], StatusRange, uint64_t(StatusCodes_e::OK));
					SetBits(aPacket[0], StatusNumberRange, uint64_t(0));
					SendPacket(aPacket, aIosEStyle);
					break;
				}
				default:
					CRAY_ASSERT(false);
			}
		break;
		default:
			CRAY_ASSERT(false);
		break;
	}
}


namespace IosFPacket_n {
	const char *LPNAME = "UPRINT";
	const char *EXNAME = "UDISK";
	const char *ETNAME = "UTAPE";
	const char *EFNAME = "UFILE";
};


#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif
