#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/range.hpp>

// Actually the PL format is described in SR-0013, appendix C.
// This program for now only supports format 2.

// Stale info:
// File header: 80 0C 80 00 40 01 80 00
//                                    ^---------- File number
// Line header: 80 24 80 00 80 01 80 00
//                                    ^---------- File number

// For now, let's assume a 'header' is of this format:
// 0x80 XX 0x80 XX XX XX 0x80 <file number>
// When file-number skips, a new file starts with the first record being the file header.
// The file string format is: ªDECK include/adrset,DW=*       
//                            ^---- 0xAA
//                                  ^-------------^------ file name


uint32_t ReadRecord(std::istream &aFile, std::vector<uint8_t> &aData, bool aSkipChkSum) {
	uint32_t RecordSize;
	if (aFile.eof()) {
		aData.resize(0);
		return 0;
	}
	aFile.read((char*)(&RecordSize), 4);
	if (aFile.eof()) {
		aData.resize(0);
		return 0;
	}
	if (RecordSize > 0x00100000) {
		throw Generic_x("Record size is suspicicously large. Aborting...");
	}
	uint32_t ReadSize = RecordSize;
	if (aSkipChkSum && ReadSize > 2) ReadSize -= 2;
	aData.resize(ReadSize);
	if (RecordSize > 0) {
		aFile.read((char*)(&(aData[0])), ReadSize);
		if (aSkipChkSum) {
			char Dummy[2];
			aFile.read(Dummy, 2);
		}
		uint32_t EndRecordSize;
		aFile.read((char*)(&EndRecordSize), 4);
		if (EndRecordSize != RecordSize) throw Generic_x("Record size mismatch at end");
	}
	return RecordSize;
}

int PrintUsage(const char *aExecName, const char *ErrorStr = nullptr) {
	std::cout << "Usage: " << aExecName << " <image file> <output base file name>" << std::endl;
	return 1;
}

const char CheckCharPl1 = 0141;
const char CheckCharPl2 = 0142;
const char CheckCharPl3 = 0143;
const char CheckCharPl4 = 0x64;

struct PlInfo_s {
	char CheckChar;
	char MasterChar;
	int IdCnt;
	uint32_t IdPos;
	char Date[8];
	char Level[8];
	int DataWidth;
	int LineWidth;
	char Signature[8];
};

struct IdEntry_s {
	std::string Name;
	enum class Type_e {
		Modification = 0,
		Deck = 1,
		CommonDeck = 2,
		CommonDeckNoProp = 3
	};
	Type_e Type;
	bool Yank;
	bool CorrectionHistoryGood;
	int Id;
	uint32_t Pos;
};

PlInfo_s ReadInfo(std::ifstream &aStrm) {
	std::streampos CurPos = aStrm.tellg();

	aStrm.seekg(-6 * 8, std::ios_base::end); // The Info table in the stream is 5 QWORD long

	PlInfo_s RetVal;

	uint64_t Word0;
	uint64_t Word3;
	uint64_t Word4;
	aStrm.read((char*)(&Word0), 8);
	if (aStrm.gcount() != 8) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());
	aStrm.read(RetVal.Date, 8);
	if (aStrm.gcount() != 8) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());
	aStrm.read(RetVal.Level, 8);
	if (aStrm.gcount() != 8) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());
	aStrm.read((char*)(&Word3), 8);
	if (aStrm.gcount() != 8) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());
	aStrm.read((char*)(&Word4), 8);
	if (aStrm.gcount() != 8) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());
	aStrm.read(RetVal.Signature, 8);
	if (aStrm.gcount() != 8) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());

	aStrm.seekg(CurPos);

	Word0 = SwapBytes(Word0);
	Word3 = SwapBytes(Word3);
	Word4 = SwapBytes(Word4);

	RetVal.CheckChar = char(GetBits(Word0, 63 - 0, 63 - 7));
	switch (RetVal.CheckChar) {
	case CheckCharPl2:
	case CheckCharPl3:
		RetVal.MasterChar = char(GetBits(Word0, 63 - 8, 63 - 15));
		if (GetBits(Word0, 63 - 16, 63 - 16) != 0) RetVal.MasterChar = 0125;
		RetVal.IdCnt = int(GetBits(Word0, 63 - 18, 63 - 31));
		RetVal.IdPos = uint32_t(GetBits(Word0, 63 - 32, 63 - 63));
		RetVal.DataWidth = int(GetBits(Word3, 63 - 44, 63 - 53));
		if (RetVal.DataWidth == 0) RetVal.DataWidth = 72;
		RetVal.LineWidth = int(GetBits(Word3, 63 - 54, 63 - 63));
		if (RetVal.LineWidth == 0) RetVal.LineWidth = 80;
		break;
	case CheckCharPl4:
		RetVal.MasterChar = char(GetBits(Word0, 63 - 8, 63 - 15));
		if (GetBits(Word0, 63 - 16, 63 - 16) != 0) RetVal.MasterChar = 0125;
		RetVal.IdCnt = int(GetBits(Word0, 63 - 18, 63 - 31));
		RetVal.IdPos = uint32_t(GetBits(Word0, 63 - 32, 63 - 63));
		RetVal.DataWidth = int(GetBits(Word3, 63 - 44, 63 - 53));
		if (RetVal.DataWidth == 0) RetVal.DataWidth = 72;
		RetVal.LineWidth = int(GetBits(Word3, 63 - 54, 63 - 63));
		if (RetVal.LineWidth == 0) RetVal.LineWidth = 80;
		// Level is changed here: it's just a pointer to the last set added. So for now, we'll just clear it
		memset(RetVal.Level, 0, sizeof(RetVal.Level));
		break;
	default:
		throw Generic_x("PL check character mismatch");
	}
	return RetVal;
}

std::vector<IdEntry_s> ReadIdTable(std::ifstream &aStrm, const PlInfo_s& aPlInfo) {
	std::vector<IdEntry_s> RetVal(aPlInfo.IdCnt);

	std::streampos CurPos = aStrm.tellg();
	switch (aPlInfo.CheckChar) {
	case CheckCharPl2:
	case CheckCharPl3:
		aStrm.seekg(aPlInfo.IdPos * 8);
		for (auto &IdEntry : RetVal) {
			uint64_t Word1;
			char Name[8];
			aStrm.read(Name, 8);
			if (aStrm.gcount() != 8) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());
			aStrm.read((char*)(&Word1), 8);
			if (aStrm.gcount() != 8) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());

			Word1 = SwapBytes(Word1);
			uint8_t TypeBits = uint8_t(GetBits(Word1, 63 - 0, 63 - 7));
			if (TypeBits > 3) throw Generic_x() << "Invalid ID type: " << DecPrinter(TypeBits);
			IdEntry.Type = IdEntry_s::Type_e(TypeBits);
			IdEntry.Yank = GetBits(Word1, 63 - 16, 63 - 16) != 0;
			IdEntry.CorrectionHistoryGood = GetBits(Word1, 63 - 17, 63 - 17) != 0;
			IdEntry.Id = int(GetBits(Word1, 63 - 18, 63 - 31));
			IdEntry.Pos = uint32_t(GetBits(Word1, 63 - 32, 63 - 63));

			// Extract the file name into a proper string
			IdEntry.Name.resize(boost::size(Name));
			size_t RealSize = 0;
			for (; RealSize < boost::size(Name); ++RealSize) {
				if (Name[RealSize] == 0) break;
				IdEntry.Name[RealSize] = Name[RealSize];
			}
			IdEntry.Name.resize(RealSize);
		}
		break;
	case CheckCharPl4:
		aStrm.seekg(aPlInfo.IdPos * 8);
		for (auto &IdEntry : RetVal) {
			uint64_t Words[3];
			aStrm.read((char*)(&Words), 3*8);
			if (aStrm.gcount() != 3*8) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());

			for(auto &Word: Words) Word = SwapBytes(Word);

			uint8_t TypeBits = uint8_t(GetBits(Words[1], 63 - 0, 63 - 7));
			if (TypeBits > 3) throw Generic_x() << "Invalid ID type: " << DecPrinter(TypeBits);
			IdEntry.Type = IdEntry_s::Type_e(TypeBits);
			IdEntry.Yank = GetBits(Words[1], 63 - 16, 63 - 16) != 0;
			IdEntry.CorrectionHistoryGood = GetBits(Words[1], 63 - 17, 63 - 17) != 0;
			IdEntry.Id = int(GetBits(Words[1], 63 - 18, 63 - 31));
			IdEntry.Pos = uint32_t(GetBits(Words[1], 63 - 32, 63 - 63));

			// Get the file name
			std::streampos CurPos = aStrm.tellg();
			int NameOfs = int(GetBits(Words[0], 0, 13));
			aStrm.seekg((aPlInfo.IdPos + aPlInfo.IdCnt*3 + NameOfs) * 8);
			char C;
			do {
				aStrm.read(&C, 1);
				if (aStrm.gcount() != 1) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());
				if (C == 0) break;
				IdEntry.Name.push_back(C);
			} while (true);
			aStrm.seekg(CurPos);
		}
		break;
	default:
		throw Generic_x("PL check character mismatch");
	}
	aStrm.seekg(CurPos);
	return RetVal;
}

struct ComDeckLine_s {
	std::string Line;
	bool Active;
	int SequenceNum;
	struct ModificationDescriptor {
		bool Activated;
		int IdIdx;
	};
	std::vector<ModificationDescriptor> ModificationDescriptors;
};

std::vector<ComDeckLine_s> ReadComDeck(std::ifstream &aStrm, const IdEntry_s &aIdEntry) {
	std::vector<ComDeckLine_s> RetVal;

	std::streampos CurPos = aStrm.tellg();
	aStrm.seekg(aIdEntry.Pos * 8);

	while (true) {
		ComDeckLine_s Line;
		// Read PL line header
		uint64_t Word;
		aStrm.read((char*)(&Word), 8);
		if (aStrm.gcount() != 8) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());
		Word = SwapBytes(Word);
		if (Word == 0ULL) break;

		Line.Active = GetBits(Word, 63 - 0, 63 - 0) != 0;
		Line.SequenceNum = int(GetBits(Word, 63 - 17, 63 - 33));
//		if (Line.SequenceNum != aIdEntry.Id) throw Generic_x() << "Line sequence number " << DecPrinter(Line.SequenceNum) << " doesn't match entry ID: " << DecPrinter(aIdEntry.Id);
		int HDC = int(GetBits(Word, 63 - 34, 63 - 47));
		Line.ModificationDescriptors.resize(HDC);
		int LineLen = int(GetBits(Word, 63 - 1, 63 - 16));
		int HDCIdx = 3;
		for (auto &ModificationDescriptor : Line.ModificationDescriptors) {
			if (HDCIdx == 4) {
				aStrm.read((char*)(&Word), 8);
				if (aStrm.gcount() != 8) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());
				Word = SwapBytes(Word);
				HDCIdx = 0;
			}
			uint32_t BinaryDescriptor = uint32_t(GetBits(Word, 63 - HDCIdx * 16, 63 - (HDCIdx * 16 + 15)));
			ModificationDescriptor.Activated = GetBits(BinaryDescriptor, 15 - 0, 15 - 0) != 0;
			ModificationDescriptor.IdIdx = uint16_t(GetBits(BinaryDescriptor, 15 - 2, 15 - 15));
			++HDCIdx;
		}
		// Read PL line image
		int LineReadSize = ((LineLen + 7) / 8) * 8;
		Line.Line.resize(LineReadSize);
		aStrm.read(&(Line.Line[0]), LineReadSize);
		if (aStrm.gcount() != LineReadSize) throw Generic_x() << "Can't read from file at offset: " << HexPrinter(aStrm.tellg());
		Line.Line.resize(LineLen);
		RetVal.emplace_back(Line);
	}

	aStrm.seekg(CurPos);
	return RetVal;
}

void WriteComDeck(const std::vector<ComDeckLine_s> &aComDeck, const std::vector<IdEntry_s> aIdEntries, int aIdIdx) {
	const IdEntry_s &IdEntry = aIdEntries[aIdIdx];

	// Make sure the directory exists
	boost::filesystem::path Path(IdEntry.Name);
	boost::filesystem::path ParentPath = Path.parent_path();
	if (!ParentPath.empty()) boost::filesystem::create_directories(ParentPath);

	std::ofstream OutStrm(IdEntry.Name, std::ios_base::out);
	if (!OutStrm.good()) throw Generic_x() << "Can't create file: " << IdEntry.Name;

	for (auto &ComDeckLine : aComDeck) {
/*		for (auto &ModificationDescriptor: ComDeckLine.ModificationDescriptors) {
			if (ModificationDescriptor.IdIdx != aIdIdx) {
				if (ModificationDescriptor.IdIdx < aIdEntries.size()) {
					if (aIdEntries[ModificationDescriptor.IdIdx].Yank) OutStrm << "^^";
					if (aIdEntries[ModificationDescriptor.IdIdx].Type != IdEntry_s::Type_e::Modification) {
						OutStrm << "!!";
					}
					if (ModificationDescriptor.Activated) OutStrm << "<<"; else OutStrm << ">>";
					OutStrm << aIdEntries[ModificationDescriptor.IdIdx].Name;
				}
				else {
					if (ModificationDescriptor.Activated) OutStrm << "<<"; else OutStrm << ">>";
					OutStrm << "**UNKN**";
				}
			}
		}
		if (ComDeckLine.Active) {
			OutStrm << " ";
		}
		else {
			OutStrm << "-";
		}
		OutStrm << DecPrinter(ComDeckLine.SequenceNum, 4) << ": " << ComDeckLine.Line << std::endl;*/
		if (ComDeckLine.Active) {
			OutStrm << ComDeckLine.Line << std::endl;
		}
	}
}

int main(int argc, const char* argv[])
{
	CommandLine_c CommandLine(argc, argv);
	std::string OutputBaseFileName;
	std::string InFileName;
	try {
		while (CommandLine.HasMoreParams()) {
			std::string CurParam = CommandLine.GetNextParam();
			if (CurParam.length() == 0) continue;
			else if (InFileName.empty()) {
				InFileName = CurParam;
			}
			else if (OutputBaseFileName.length() == 0) {
				OutputBaseFileName = CurParam;
			}
			else {
				throw Generic_x("Unkown command line parameter");
			}
		}
		if (InFileName.empty()) throw Generic_x("Input file name must be specified");
	}
	catch (std::exception &Ex) {
		return PrintUsage(argv[0], Ex.what());
	}
	if (OutputBaseFileName.length() == 0) {
		//Figure out base-name from input
		OutputBaseFileName = boost::filesystem::basename(InFileName);
	}

	errno = 0;

	std::ifstream InStrm;
	try {
		InStrm.open(InFileName, std::ios_base::in | std::ios_base::binary);
	}
	catch (std::exception &e) {
		std::cout << "Can't open input file: " << InFileName << " - " << e.what() << std::endl;
		return 1;
	}

	try {
		PlInfo_s PlInfo = ReadInfo(InStrm);
		std::vector<IdEntry_s> IdEntries = ReadIdTable(InStrm, PlInfo);
		for (size_t IdIdx = 0; IdIdx < IdEntries.size(); ++IdIdx) {
			auto &IdEntry = IdEntries[IdIdx];
			std::vector<ComDeckLine_s> ComDeck = ReadComDeck(InStrm, IdEntry);
			if (IdEntry.Type != IdEntry_s::Type_e::Modification) {
				if (IdEntry.Yank) {
					std::cout << "Skipping yanked file: " << IdEntry.Name << std::endl;
				}
				else {
					std::cout << "Writing file: " << IdEntry.Name << std::endl;
					WriteComDeck(ComDeck, IdEntries, int(IdIdx));
				}
			}
		}
	}
	catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
	return 0;
}

