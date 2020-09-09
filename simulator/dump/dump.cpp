#define _CRT_SECURE_NO_WARNINGS
// cray_ios_disasm.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include <boost/filesystem.hpp>

// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
//
// To be true to the documentation, octal numbers are extensively used in this file.
// Watch out for the leading 0-s!
//
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!

char PrintByte(uint8_t aByte) {
	if (aByte < ' ') return '.';
	if (aByte > 127) return '.';
	return (char)aByte;
}

int main(int argc, char* argv[])
{
	if (argc != 4 && argc != 5) {
		printf("Usage: %s <image file> <start addr> <end addr> [base]\n",argv[0]);
		return 1;
	}
	const char *InFileName = argv[1];

	FILE *Input = fopen(InFileName,"rb");
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
	int32_t Base = StartAddr;
	if (argc == 5) {
		Base = strtoul(argv[4],&EndPtr,0);
		if (errno != 0 || EndPtr[0] != 0) {
			printf("Invalid base address\n");
			return 1;
		}
	}

	if (Input == nullptr) {
		printf("Can't open input file: %s\n", InFileName);
		return 1;
	}

	uint64_t FileSize = boost::filesystem::file_size(InFileName);

	if (EndAddr == 0) EndAddr = (uint32_t)FileSize;
	if (EndAddr > (uint32_t)FileSize) EndAddr = (uint32_t)FileSize;
	const size_t MaxFileSize = 256*1024*1024;
	if (FileSize > MaxFileSize) {
		printf("Can't use files larger than %uMB\n",unsigned(MaxFileSize/1024/1024));
		fclose(Input);
		return 1;
	}
	uint8_t *File = new uint8_t[(size_t)FileSize+1];
	if (File == nullptr) {
		printf("Not enough memory to allocate buffer for image\n");
		fclose(Input);
		return 1;
	}
	size_t Read = fread(File,sizeof(uint8_t),(size_t)FileSize,Input);
	fclose(Input);
	if (Read != FileSize) {
		printf("Can't read file.\n");
		return 1;
	}

	uint32_t Offset = StartAddr;
	while (Offset < EndAddr) {
		printf("0x%06x    ",(Offset-StartAddr+Base)/8);
		for(int i=0;i<8;++i) {
			printf(" %02x",File[Offset+i]);
		}
		printf("   ");
		for(int i=0;i<8;++i) {
			printf("%c",PrintByte(File[Offset+i]));
		}
		printf("\n");
		Offset += 8;
	}
	delete[] File;
	return 0;
}

