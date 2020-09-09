#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"
#include <iostream>

/*
offset 0x00: 0x00000078
offset 0x04 : 0x0080
offset 0x06 : UNICOS padded to 16 bytes <--directory name
offset 0x16 : UNICOS padded to 16 bytes <--file name
offset 0x26 : 06 / 14 / 88 <--Creation date
offset 0x2e : 08 : 48 : 29 <--Creation time
offset 0x36 : 0xdfa50400 <--changes a lot for every file
offset 0x3a : 0x6400 <--Constant(maybe flags of sort ? )
offset 0x3c : 0x00000000
offset 0x40 : 0x5302 0x0100 0x0100
offset 0x42 : 0x0001 0x0100 0x0100
offset 0x44 : 0x163b 0x693d 0x6a3d
offset 0x46 : 0x5302 0x0100 0x0100
offset 0x48 : 0x00000000
offset 0x4c : 0x00000000
offset 0x50 : 0x00000000
offset 0x54 : 0x00000000
offset 0x58 : 0x00000000
offset 0x5c : 0x00000000
offset 0x60 : 0x00000000
offset 0x64 : 0x00000000
offset 0x68 : 0x00000000
offset 0x6c : 0x00000000
offset 0x70 : 0x00000000
offset 0x74 : 0x00000000
offset 0x78 : 0x00000000
offset 0x7c : 0x00000078
*/

struct FileHader_s {
	char DirName[17];
	char FileName[17];
};

FileHader_s ReadFileHader(FILE *aFile) {
	uint32_t UIntBuffer;
	char Buffer[16];
	FileHader_s RetVal;
	fread(&UIntBuffer, 1, 4, aFile); // Magic
	if (UIntBuffer != 0x78) throw Generic_x("Invalid header magic in file header");
	fread(Buffer, 1, 2, aFile);
	fread(&RetVal.DirName, 1, 16, aFile);
	RetVal.DirName[16] = 0;
	fread(&RetVal.FileName, 1, 16, aFile);
	RetVal.FileName[16] = 0;
	fread(&Buffer, 1, 8, aFile); // Date
	fread(&Buffer, 1, 8, aFile); // Time
	fread(&Buffer, 1, 4, aFile);
	fread(&Buffer, 1, 2, aFile);
	fread(&Buffer, 1, 4, aFile);
	fread(&Buffer, 1, 2, aFile);
	fread(&Buffer, 1, 2, aFile);
	fread(&Buffer, 1, 2, aFile);
	fread(&Buffer, 1, 2, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile);
	fread(&UIntBuffer, 1, 4, aFile); // Magic
	if (UIntBuffer != 0x78) throw Generic_x("Invalid trailer magic in file header");
	return RetVal;
}

struct BlockHeader_s {
	uint32_t PrevSize;
	uint32_t CurSize;
};

BlockHeader_s ReadBlockHeader(FILE *aFile, bool aSkipPrev = false) {
	BlockHeader_s RetVal;
	if (!aSkipPrev) {
		fread(&RetVal.PrevSize, 1, 4, aFile);
	}
	else {
		RetVal.PrevSize = 0;
	}
	fread(&RetVal.CurSize, 1, 4, aFile);
	return RetVal;
}

int PrintUsage(const char *aExecName, const char *ErrorStr = nullptr) {
	std::cout << "Usage: " << aExecName << " <image file> -s <output file name>" << std::endl;
	return 1;
}

int main(int argc, const char* argv[])
{
	CommandLine_c CommandLine(argc, argv);
	bool SingleFileMode = false;
	std::string OutputFileName;
	std::string InFileName;
	try {
		while (CommandLine.HasMoreParams()) {
			std::string CurParam = CommandLine.GetNextParam();
			if (CurParam.length() == 0) continue;
			if (CurParam == "-s") {
				OutputFileName = CommandLine.GetNextParam();
				SingleFileMode = true;
			}
			else {
				if (InFileName.empty()) {
					InFileName = CurParam;
				}
				else {
					throw Generic_x("Unkown command line parameter");
				}
			}
		}
		if (InFileName.empty()) throw Generic_x("Input file name must be specified");
	}
	catch (std::exception &Ex) {
		return PrintUsage(argv[0], Ex.what());
	}

	errno = 0;

	FILE *Input = fopen(InFileName.c_str(),"rb");
	FILE *Output = nullptr;
	if (Input == nullptr) {
		std::cout << "Can't open input file: " << InFileName << std::endl;
		return 1;
	}

	try {
		while (!feof(Input)) {
			if (!SingleFileMode) {
				FileHader_s FileHeader;
				FileHeader = ReadFileHader(Input);
				OutputFileName = FileHeader.FileName;
				std::cout << "Found file " << OutputFileName << " in directory " << FileHeader.DirName << std::endl;
			}
			BlockHeader_s BlockHeader = ReadBlockHeader(Input, SingleFileMode);
			if (BlockHeader.PrevSize != 0) throw Generic_x("Invalid first block header: prev should be 0");
			Output = fopen(OutputFileName.c_str(), "wb");
			if (Output == nullptr) throw Generic_x("Can't create output file");
			while (BlockHeader.CurSize != 0) {
				char Buffer[4096];
				if (BlockHeader.CurSize > sizeof(Buffer)) throw Generic_x("Current block size is too large");
				fread(Buffer, 1, BlockHeader.CurSize, Input);
				fwrite(Buffer, 1, BlockHeader.CurSize, Output);
				BlockHeader_s NewHeader = ReadBlockHeader(Input);
				if (NewHeader.PrevSize != BlockHeader.CurSize) throw Generic_x("Block header libnk error");
				BlockHeader = NewHeader;
			}
			fclose(Output);
			Output = nullptr;
			if (SingleFileMode) break;
		}
	}
	catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		if (Input != nullptr) fclose(Input);
		if (Output != nullptr) fclose(Output);
		return 1;
	}
	if (Input != nullptr) fclose(Input);
	if (Output != nullptr) fclose(Output);
	return 0;
}

