#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"
#include <boost/filesystem.hpp>

int main(int argc, char* argv[])
{
	if (argc != 5) {
		printf("Usage: %s <image file> <start addr> <end addr> <output file>\n",argv[0]);
		return 1;
	}

	uint32_t StartAddr;
	uint32_t EndAddr;
	char *EndPtr;
	errno = 0;

	StartAddr = strtoul(argv[2],&EndPtr,0);
	if (errno != 0 || EndPtr[0] != 0) {
		printf("Invalid start address\n");
		return 1;
	}
	EndAddr = strtoul(argv[3],&EndPtr,0);
	if (errno != 0 || EndPtr[0] != 0) {
		printf("Invalid end address\n");
		return 1;
	}

	const char *InFileName = argv[1];
	const char *OutFileName = argv[4];

	FILE *Input = fopen(InFileName,"rb");
	if (Input == nullptr) {
		printf("Can't open input file: %s\n", InFileName);
		return 1;
	}
	FILE *Output = fopen(OutFileName,"wb");
	if (Output == nullptr) {
		fclose(Input);
		printf("Can't create output file: %s\n", OutFileName);
	}

	uint64_t FileSize = boost::filesystem::file_size(InFileName);

	if (EndAddr == 0) EndAddr = (uint32_t)FileSize;
	if (EndAddr > (uint32_t)FileSize) EndAddr = (uint32_t)FileSize;
	const size_t MaxFileSize = 1024*1024*1024;
	if (FileSize > MaxFileSize) {
		printf("Can't use files larger than %uMB\n",unsigned(MaxFileSize/1024/1024));
		fclose(Input);
		fclose(Output);
		return 1;
	}
	uint8_t *File = new uint8_t[(size_t)FileSize+1];
	if (File == nullptr) {
		printf("Not enough memory to allocate buffer for image\n");
		fclose(Input);
		fclose(Output);
		return 1;
	}

	size_t Read = fread(File,sizeof(uint8_t),(size_t)FileSize,Input);
	fclose(Input);
	if (Read != FileSize) {
		printf("Can't read file.\n");
		fclose(Output);
		return 1;
	}
	printf("Start addr: 0x%08x\n", StartAddr);
	printf("End addr: 0x%08x\n", EndAddr);
	size_t Written = fwrite(File + StartAddr,sizeof(uint8_t),EndAddr-StartAddr,Output);
	if (Written != EndAddr-StartAddr) {
		printf("Can't write output file.\n");
		return 1;
	}
	fclose(Output);
	delete[] File;
	return 0;
}

