#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "utils.h"
#include <sstream>
#include <boost/filesystem.hpp>

// Normal sequences:
// BCW - if we're not in a dataset, this starts a dataset
//     - if we're not in a file, this starts a file
//     - if we're not in a record, this starts a record
//     - otherwise, this is just a continuation of the current record
// EOR - if we're in a record, it ends the current record
//     - otherwise it denotes a NULL record
// EOF - if we're in a file, it ends the current file
//     - otherwise it denotes a NULL file
// EOD - if we're in a dataset, it ends the dataset
//     - otherwise it denotes a NULL dataset
//
// Ending a file without ending a record is invalid, except for a NULL file
// Ending a dataset without ending a file is invalid
//
// If an EOR is followed by a BCW, that's fine, this
// simply means that the data for the next record is after
// the BCW. This technique can be used to skip over
// certain sections of a dataset: since the forward index
// of an EOR or EOF points to a BCW, the region between the 
// the two control words is assumed to not contain data
//
// There is some RLL compression for 'blank fields' whatever that means
// See 2240011R part 1 2-6 - this seems like it have been removed later
//
// There are some new bigs that are not in the original description
// See SR-0011P 2-15
//
// So, to add a file to the end:
// 1. Remove the EOD
// 2. Add the file, starting with a BCW
// 3. Terminate with an EOD
//
// To add a file to the beginning:
// 1. Create the file as normal, starting with a BCW
// 2. Append the old dataset to it
//
// To add a file at an arbitrary position
// A. If the EOF of the previous file points to an EOR (that is the first record of the next file
//   1. Patch up EOF to point to immediately after itself
//   2. Add file as normal, starting with a BCW
//   3. Patch up our EOF to point to the first EOR of the old file
//   4. Add the rest of the Dataset
// B. If the EOF of the previous file points to a BCW
//   1. Add the file as normal, starting with a BCW
//   2. Make sure our EOF points immediately after itself
//   2. Add the rest of the dataset
//
// To remove the first file
// 

enum CtrlWordType_e {
	CtrlWordType_BCW = 000,
	CtrlWordType_EOR = 010,
	CtrlWordType_EOF = 016,
	CtrlWordType_EOD = 017,
	CtrlWordType_UNK = 020
};

// Common fields
CtrlWordType_e CtrlWord_GetType(uint64_t aCtrlWord) {
	uint8_t CtrlType = ((uint8_t *)(&aCtrlWord))[7] >> 4;
	switch (CtrlType) {
		case CtrlWordType_BCW: return (CtrlWordType_e)CtrlType;
		case CtrlWordType_EOR: return (CtrlWordType_e)CtrlType;
		case CtrlWordType_EOF: return (CtrlWordType_e)CtrlType;
		case CtrlWordType_EOD: return (CtrlWordType_e)CtrlType;
		default: return CtrlWordType_UNK;
	}
}

bool CtrlWord_GetBadData(uint64_t aCtrlWord) {
	// get bit 52 out
	return ((aCtrlWord >> 52) & 1) == 0 ? false: true;
}

uint32_t CtrlWord_GetForwardWordIdx(uint64_t aCtrlWord) {
	uint32_t Lower = (uint32_t)aCtrlWord;
	return Lower & 0x1ff;
}

// BCW fields
uint32_t CtrlWord_GetBlockNumber(uint64_t aCtrlWord) {
	uint32_t Lower = (uint32_t)aCtrlWord;
	Lower >>= 9;
	return Lower;
}

// EOR,EOF,EOD fields
uint32_t CtrlWord_GetUnusedBitCnt(uint64_t aCtrlWord) {
	return ((aCtrlWord >> 54) & 63);
}

bool CtrlWord_GetTransparent(uint64_t aCtrlWord) {
	// get bit 53 out
	uint8_t Byte = ((uint8_t *)(&aCtrlWord))[6];
	return (Byte & 0x20) == 0 ? false: true;
}

bool CtrlWord_GetSkipRemainderSector(uint64_t aCtrlWord) {
	// get bit 51 out
	uint8_t Byte = ((uint8_t *)(&aCtrlWord))[6];
	return (Byte & 0x08) == 0 ? false: true;
}

uint32_t CtrlWord_GetPreviousFileIdx(uint64_t aCtrlWord) {
	return uint32_t(GetBitsReverse(aCtrlWord, 20, 39));
}

uint32_t CtrlWord_GetPreviousRecordIdx(uint64_t aCtrlWord) {
	return uint32_t(GetBitsReverse(aCtrlWord, 40, 54));
}

std::string PrintCtrlWord(uint64_t aCtrlWord) {
	std::stringstream Str;
	switch (CtrlWord_GetType(aCtrlWord)) {
		case CtrlWordType_BCW: Str << "BCW: "; break;
		case CtrlWordType_EOR: Str << "EOR: "; break;
		case CtrlWordType_EOF: Str << "EOF: "; break;
		case CtrlWordType_EOD: Str << "EOD: "; break;
		default: Str << "UNK: "; break;
	}
	Str << "Forward word IDX: " << DecPrinter(CtrlWord_GetForwardWordIdx(aCtrlWord), 5) << " ";
	switch (CtrlWord_GetType(aCtrlWord)) {
		case CtrlWordType_BCW:
			Str << "Block Number: " << CtrlWord_GetBlockNumber(aCtrlWord) << " ";
		break;
		case CtrlWordType_EOR:
		case CtrlWordType_EOF:
		case CtrlWordType_EOD:
			Str << "Unused bit count: " << DecPrinter(CtrlWord_GetUnusedBitCnt(aCtrlWord),2) << " ";
			Str << "Prev file idx: " << HexPrinter(CtrlWord_GetPreviousFileIdx(aCtrlWord),7) << " ";
			Str << "Prev record idx: " << HexPrinter(CtrlWord_GetPreviousRecordIdx(aCtrlWord),6) << " ";
			Str << (CtrlWord_GetTransparent(aCtrlWord) ? "TRANS ":"");
			Str << (CtrlWord_GetSkipRemainderSector(aCtrlWord) ? "SKIP ":"");
		break;
		default: Str << "UNK: "; break;
	}
	Str << (CtrlWord_GetBadData(aCtrlWord) ? "BAD":"");
	return Str.str();
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
		size_t Offset = 0;
		bool InDataSet = false;
		int DataSetCnt = 0;
		int FileCnt = 0;
		int RecordCnt = 0;
		char FileName[255];
		int LastSeqCnt = -1;
		FILE *OutFile = nullptr;
		while (Offset < TotalFileSize) {
			uint64_t CtrlWord = SwapBytes(*(uint64_t*)(File+Offset));
			int SeqCount;

			CtrlWordType_e Type = CtrlWord_GetType(CtrlWord);
			std::cout << PrintCtrlWord(CtrlWord) << std::endl;
			if (Type == CtrlWordType_UNK) {
				printf("Lost track at 0x%08x\n",unsigned(Offset));
				break;
			}
			switch (Type) {
				case CtrlWordType_BCW:
					SeqCount = (int)CtrlWord_GetBlockNumber(CtrlWord);
					if (SeqCount == 0 && InDataSet) {
						printf("BCW Start sequence inconsitency at 0x%08x\n",unsigned(Offset));
					}
					if (SeqCount != LastSeqCnt + 1) {
						printf("BCW Sequence inconsitency at 0x%08x\n",unsigned(Offset));
					}
					if (!InDataSet) {
						InDataSet = true;
						// New dataset: let's close the old file, and start a new one
						FileCnt = 0;
						RecordCnt = 0;
					}
					LastSeqCnt = SeqCount;
				break;
				case CtrlWordType_EOR:
				break;
				case CtrlWordType_EOF:
				break;
				case CtrlWordType_EOD:
				break;
				default:
					printf("Internal error at offset 0x%08x\n",unsigned(Offset));
					exit(1);
				break;
			}
			uint32_t DataSize = CtrlWord_GetForwardWordIdx(CtrlWord) * 8;
			uint32_t RecordSize = DataSize;
			// See what the next control word is and if there's anything to save
			uint32_t UnusedBitCnt = 0;
			CtrlWordType_e NextType = CtrlWordType_UNK;
			if (Type != CtrlWordType_EOD) {
				uint64_t NextCtrlWord = SwapBytes(*(uint64_t*)(File+Offset+8+RecordSize));
				NextType = CtrlWord_GetType(NextCtrlWord);
				if (NextType == CtrlWordType_EOR) UnusedBitCnt = CtrlWord_GetUnusedBitCnt(NextCtrlWord);
			}
			DataSize -= UnusedBitCnt / 8;
			if (DataSize != 0) {
				if (OutFile == nullptr) {
					sprintf(FileName,"DS%03d_F%03d.dat",DataSetCnt,FileCnt);
					OutFile = fopen(FileName,"wb");
					if (OutFile == nullptr) {
						printf("Can't create file: %s\n",FileName);
						return 1;
					}
				}
				size_t Written = fwrite(File+Offset+8,1,DataSize,OutFile);
				if (Written != DataSize) {
					printf("Can't write to file\n");
					return 1;
				}
			}

			switch (NextType) {
				case CtrlWordType_BCW:
				break;
				case CtrlWordType_EOR:
					++RecordCnt;
					//if (OutFile != nullptr) fclose(OutFile); OutFile = nullptr;
					if (OutFile == nullptr) {
						printf("EOR encountered with no file at offset 0x%08x\n",unsigned(Offset));
						exit(1);
					}
					fprintf(OutFile,"\n"); // TODO: this should only happen for text files
				break;
				case CtrlWordType_EOF:
					++FileCnt;
					RecordCnt = 0;
					if (OutFile != nullptr) fclose(OutFile); OutFile = nullptr;
				break;
				case CtrlWordType_EOD:
					++DataSetCnt;
					InDataSet = false;
					if (OutFile != nullptr) fclose(OutFile); OutFile = nullptr;
					LastSeqCnt = -1;
				break;
				case CtrlWordType_UNK:
					if (Type != CtrlWordType_EOD) {
						printf("Internal error at offset 0x%08x\n",unsigned(Offset));
						exit(1);
					}
				break;
				default:
					printf("Internal error at offset 0x%08x\n",unsigned(Offset));
					exit(1);
				break;
			}

			Offset += 8 + RecordSize;
			if (Type == CtrlWordType_EOD) {
				// Jump over to the next data record boundary
				Offset = (Offset & 0xfffff000) + 0x1000;
			}
		}
	}

	delete[] File;
	return 0;
}

