#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/crc.hpp>

/*
Tape Dump format:
- Each file on the tape is set up of a set of records
- Each record contains a header and an optional footer, both of which are 4 bytes long.
- The header and the footer contains the length of the record
- The footer doesn't exist if length is set to 0, which marks the end of file
*/

// This CRC is documented in http://bitsavers.trailing-edge.com/pdf/ibm/24xx/A22-6862-4_2400_Series_Magnetic_Tape_Units_OEM.pdf
// page 8
class MyCrc_c {
public:
	explicit MyCrc_c(uint8_t aInitValue = 0, bool aOddParity = false) : mAccumulator(aInitValue), mOddParity(aOddParity), mAccumulatorOddParity(true) {}
	void process_byte(uint8_t aByte) {
		// Calculate parity first...
		uint8_t Parity = 0;
		uint16_t Data = aByte;
		for (int i = 0; i < 7; ++i) {
			Parity ^= Data & 1;
			Data >>= 1;
		}
		CRAY_ASSERT(Parity <= 1);
		if (mOddParity) {
			Parity = 1 - Parity;
		}
		Data = (Parity << 8) | aByte;
		mAccumulator ^= Data;
		// Rotate by 1
		uint8_t Msb = (mAccumulator & 256) >> 8;
		if (Msb == 1) {
			mAccumulator ^= (1 << 2 | 1 << 3 | 1 << 4 | 1 << 5);
		}
		mAccumulator = ((mAccumulator & 255) << 1) | Msb;
	}
	void process_bytes(void *aData, size_t aDataSize) {
		uint8_t *Data = (uint8_t*)aData;
		for (size_t i = 0; i < aDataSize; ++i) {
			process_byte(Data[i]);
		}
	}
	uint16_t checksum() const { 
		uint16_t RetVal = mAccumulator ^ ((~(1 << 2 | 1 << 4) & 511));
		if (!mAccumulatorOddParity) {
			RetVal ^= 1;
		}
		return RetVal;
	}
protected:
	bool mOddParity;
	bool mAccumulatorOddParity;
	uint16_t mAccumulator;
};

bool IsPowerOf2(uint32_t aVal) {
	if (aVal == 0) return false;
	return (aVal & (aVal - 1)) == 0;
}

uint32_t ReadRecord(std::istream &aFile, std::vector<uint8_t> &aData, bool aCorrectRecordSize, size_t aExpectedRecordSize, bool &aHasChecksum, int aRecordIdx) {
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
	// Test to see if record size is 2^n+2 format. If it is, it's likely the record has a checksum and CRC at the end
	bool LikelyHasChecksum = IsPowerOf2(RecordSize - 2);

	bool LikelyCorruptedRecord = false;
	if (aRecordIdx > 0) {
		if (LikelyHasChecksum || aHasChecksum) {
			LikelyCorruptedRecord = RecordSize - 2 != aExpectedRecordSize;
		}
		else {
			LikelyCorruptedRecord = RecordSize != aExpectedRecordSize;
		}
	}
	if (RecordSize > 0x00100000) {
		throw Generic_x("Record size is suspicicously large. Aborting...");
	}
	uint32_t ReadSize = RecordSize;
	aData.resize(ReadSize);

	if (RecordSize > 0) {
		aFile.read((char*)(&(aData[0])), ReadSize);

		uint8_t LRC = 0;
		for (uint32_t i = 0; i < ReadSize; ++i) {
			LRC ^= aData[i];
		}
		if (LikelyHasChecksum || aHasChecksum) {
			aData.resize(aData.size() - 2); // Remove checksum from record
		}
		if (LRC != 0) {
			if (LikelyHasChecksum || aHasChecksum) {
				std::cout << "LRC error in record " << DecPrinter(aRecordIdx+1) << std::endl;
			}
		}
		else {
			if (aRecordIdx == 0) aHasChecksum = true;
		}
		uint32_t EndRecordSize;
		aFile.read((char*)(&EndRecordSize), 4);
		if (EndRecordSize != RecordSize) throw Generic_x("Record size mismatch at end");
		if (LikelyCorruptedRecord) {
			// Peek into the stream to see if this is the last record
			uint32_t NextRecordSize;
			aFile.read((char*)(&NextRecordSize), 4);
			if (NextRecordSize != 0 && aCorrectRecordSize) {
				std::cout << "Incorrect record size " << DecPrinter(aData.size()) << " detected in record " << DecPrinter(aRecordIdx+1) << ". Correcting to " << DecPrinter(aExpectedRecordSize) << std::endl;
				aData.resize(aExpectedRecordSize);
			}
			aFile.seekg(-4, std::ios_base::cur);
		}
	}
	return RecordSize;
}

int PrintUsage(const char *aExecName, const char *ErrorStr = nullptr) {
	std::cout << "Usage: " << aExecName << " <image file> -c <output base file name>" << std::endl;
	std::cout << "     -c: assume all records in the file are the same length. Correct if not." << std::endl;
	return 1;
}

int main(int argc, const char* argv[])
{
	CommandLine_c CommandLine(argc, argv);
	std::string OutputBaseFileName;
	std::string InFileName;
	bool CorrectRecordSize = false;
	try {
		while (CommandLine.HasMoreParams()) {
			std::string CurParam = CommandLine.GetNextParam();
			if (CurParam.length() == 0) continue;
			if (CurParam == "-c") {
				CorrectRecordSize = true;
			} else if (InFileName.empty()) {
				InFileName = CurParam;
			}
			else if (OutputBaseFileName.length() == 0) {
				OutputBaseFileName = CurParam;
			} else {
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

	size_t EOTCount = 0;
	try {
		size_t FileIdx = 1;
		while (!InStrm.eof()) {
			std::vector<uint8_t> Record;
			int RecordIdx = 0;
			std::ofstream OutStrm;
			size_t ExpectedRecordSize = 0;
			bool HasChecksum = false;
			while (ReadRecord(InStrm, Record, CorrectRecordSize, ExpectedRecordSize, HasChecksum, RecordIdx) > 0) { // Read till EOF marker
				if (ExpectedRecordSize == 0) ExpectedRecordSize = Record.size();
				if (RecordIdx == 0) {
					if (EOTCount > 0) {
						std::cout << "Warning: non-0 file after EOT" << std::endl;
					}
					std::stringstream OutFileName;
					OutFileName << OutputBaseFileName << "." << std::setw(3) << std::setfill('0') << FileIdx;
					OutStrm.open(OutFileName.str(), std::ios_base::out | std::ios_base::binary);
					if (!OutStrm.good()) throw Generic_x("Can't create file");
				}
				OutStrm.write((char*)(&(Record[0])), Record.size());
				RecordIdx++;
			}
			OutStrm.close();
			if ((RecordIdx == 0) && !InStrm.eof()) { ++EOTCount; }
			if (RecordIdx > 0) ++FileIdx; // Only increment file name if it was in fact created
		}
	}
	catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
	if (EOTCount == 0) {
		std::cout << "Warning: no EOT marker found" << std::endl;
	}
	if (EOTCount != 1) {
		std::cout << "Warning: " << EOTCount << " of EOT markers found" << std::endl;
	}
	return 0;
}

