#define _CRT_SECURE_NO_WARNINGS
// cray_ios_disasm.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "utils.h"
#include <map>
#include <boost/filesystem.hpp>

bool IsUpperCaseOrNum(char aC) {
	if (aC >= 'A' && aC <= 'Z') return true;
	if (aC >= '0' && aC <= '9') return true;
	return false;
}

struct ProcessedFileHeader_s {
	uint16_t FileSize;
	uint16_t Flags;
	uint16_t Flags1;
	uint16_t Flags2;
	char FileName[9];
};

const size_t cFileHeaderSize = 16;

ProcessedFileHeader_s DecodeHeader(uint8_t *aContent, size_t aMaxOffset, size_t aOffset) {
	ProcessedFileHeader_s RetVal;
	RetVal.FileSize = SwapBytes(*(uint16_t*)(aContent+aOffset)) * 2;
	aOffset += 2;
	RetVal.FileSize -= cFileHeaderSize;

	RetVal.Flags = SwapBytes(*(uint16_t*)(aContent+aOffset));
	aOffset += 2;

	memcpy(RetVal.FileName,aContent+aOffset,8);
	RetVal.FileName[8] = 0;
	aOffset += 8;

	RetVal.Flags1 = SwapBytes(*(uint16_t*)(aContent+aOffset));
	aOffset += 2;

	RetVal.Flags2 = SwapBytes(*(uint16_t*)(aContent+aOffset));
	aOffset += 2;

	return RetVal;
}

bool IsValidHeader(ProcessedFileHeader_s &aHeader, size_t aMaxOffset, size_t aOffset) {
	if (aHeader.FileSize > aMaxOffset - aOffset) return false;
//	if (aHeader.FileSize > 65536) return false;
//	if (aHeader.FileSize == -(int16_t)cFileHeaderSize) return false;
	if (aHeader.Flags != 0) return false;
	if (aHeader.Flags1 == 0) return false;
	if ((aHeader.Flags1 & 0xf000) != 0x0000 && (aHeader.Flags1 & 0xf000) != 0x8000) return false;

	if (!IsUpperCaseOrNum(aHeader.FileName[0])) return false;

	bool InStr = true;
	for(int i=1;i<8;++i) {
		if (InStr) {
			if (aHeader.FileName[i] == 0) {
				InStr = false;
			} else {
				if (!IsUpperCaseOrNum(aHeader.FileName[i])) return false;
			}
		} else {
			if (aHeader.FileName[i] != 0) return false;
		}
	}
	return true;
}

bool IsValidHeader(uint8_t *aContent, size_t aMaxOffset, size_t aOffset) {
	ProcessedFileHeader_s Header = DecodeHeader(aContent,aMaxOffset,aOffset);
	return IsValidHeader(Header, aMaxOffset, aOffset);
}

bool FindNextHeader(uint8_t *aContent, size_t aMaxOffset, size_t &aOffset) {
	while (aOffset < aMaxOffset-16) {
		if (IsValidHeader(aContent, aMaxOffset, aOffset)) return true;
		++aOffset;
	}
	return false;
}

typedef std::map<size_t, std::string> FileDirectory_t;

void ParseOvlNum(FileDirectory_t &aFileNames, std::vector<uint8_t> &aFile, size_t aFileCnt) {
	char Desc[255];
	for(size_t FileIdx=0;FileIdx<aFileCnt;++FileIdx) {
		uint16_t StartOffs = SwapBytes(*(uint16_t*)&aFile[FileIdx*8+4]) * 8 - 12;
		uint16_t Size = SwapBytes(*(uint16_t*)&aFile[FileIdx*8+6]);
		if (Size >= 255) break;
		memcpy(Desc,&aFile[StartOffs],Size);
		Desc[Size] = 0;
		std::cout << aFileNames[FileIdx] << ": " << Desc << std::endl;
	}
}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Usage: %s <image file>\n",argv[0]);
		return 1;
	}
	const char *InFileName = argv[1];
	FILE *Input = fopen(InFileName,"rb");
	if (Input == nullptr) {
		printf("Can't open input file: %s\n", InFileName);
		return 1;
	}
	uint64_t TotalFileSize = boost::filesystem::file_size(InFileName);
	const size_t MaxFileSize = 256*1024*1024;
	if (TotalFileSize > MaxFileSize) {
		printf("Can't use files larger than %uMB\n",unsigned(MaxFileSize/1024/1024));
		fclose(Input);
		return 1;
	}
	uint8_t *File = new uint8_t[(size_t)TotalFileSize+1];
	if (File == nullptr) {
		printf("Not enough memory to allocate buffer for image\n");
		fclose(Input);
		return 1;
	}

	size_t Read = fread(File,1,(size_t)TotalFileSize,Input);
	fclose(Input);
	if (Read != TotalFileSize) {
		printf("Can't read file.\n");
		return 1;
	}

	{
		size_t FileCnt = SwapBytes(*((uint16_t *)(File)));
		size_t Offset = 4;
		size_t FileIdx = 0;
		FileDirectory_t FileNames;
		std::vector<uint8_t> OvlNum;
		size_t MaxFileId = 0;
		while (FileIdx < FileCnt) {
			ProcessedFileHeader_s Header = DecodeHeader(File,(size_t)TotalFileSize,Offset);
			if (!IsValidHeader(Header,(size_t)TotalFileSize,Offset)) {
				printf("Lost track at offset 0x%08x\n",unsigned(Offset));
				break;
			}
			Offset += cFileHeaderSize;
			printf("0x%08x-0x%08x flags:0x%04x 0x%04x size:%8d %s\n",unsigned(Offset-cFileHeaderSize) , unsigned(Offset+Header.FileSize) ,Header.Flags1, Header.Flags2, Header.FileSize, Header.FileName);
			char OutFileName[255];
			size_t FileId = Header.Flags1 & 0x7fff;
			if (FileId > MaxFileId) MaxFileId = FileId;
			FileNames[FileId] = Header.FileName;
			if (strcmp(Header.FileName, "OVLNUM") == 0) {
				OvlNum.resize(Header.FileSize);
				memcpy(&OvlNum[0],File+Offset,Header.FileSize);
			}
			sprintf(OutFileName,"%04u_%s.%s",unsigned(FileIdx),Header.FileName,((Header.Flags1 & 0x8000) == 0) ? "cod":"dat");
			FILE *OutFile = fopen(OutFileName,"wb");
			if (OutFile != nullptr) {
				fwrite(File+Offset,1,Header.FileSize,OutFile);
				fclose(OutFile);
			}
			Offset += Header.FileSize;
			++FileIdx;
		}
		if (OvlNum.size() > 0) {
			std::cout << "Overlay file explanations" << std::endl;
			std::cout << "========================================" << std::endl;
			ParseOvlNum(FileNames,OvlNum,MaxFileId+1);
		}
	}
	delete[] File;
	return 0;
}

