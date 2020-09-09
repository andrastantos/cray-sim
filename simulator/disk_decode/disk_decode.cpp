#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <limits.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <list>
#include <map>
#include <algorithm>
#include <boost/filesystem.hpp>

#include "utils.h"

#ifndef SIZE_MAX
/* Limit of `size_t' type.  */
# if __WORDSIZE == 64
#  define SIZE_MAX		(18446744073709551615UL)
# else
#  define SIZE_MAX		(4294967295U)
# endif
#endif

const bool cPrintHistograms = false;

// Direct Python Conversion
// Original code by Yngve AAdlandsvik 
// Modified by Chris Fenton  -  9/7/11
// Modified by Andras Tantos - 12/3/12

enum Bit_e { Bit_Zero = 0, Bit_One = 1, Bit_Undef = 2 };

static const size_t cHeads = 5;
static const size_t cTracks = 823;
static const size_t cSectors = 32;
static const size_t cSectorSize = 512;

#ifdef _MSC_VER
#pragma optimize( "", off )
#endif
int PrintUsage(const char *aProgName) {
	std::cout << "Usage: " << aProgName << " <phase> [<param> <value>]" << std::endl;
	std::cout << "phase:" << std::endl;
	std::cout << "  1: initial recovery" << std::endl;
	std::cout << "  2: second phase recovery (incomplete)" << std::endl;
	std::cout << "possible paramters for phase 1:" << std::endl;
	std::cout << "  --StartFileIdx <number>: Index of the first scan file to read for each head" << std::endl;
	std::cout << "  --EndFileIdx <number>:   Index of the last scan file to read for each head - set to 0 to read all files" << std::endl;
	std::cout << "  --StartHead <number>:    Index of the first head directory to scan" << std::endl;
	std::cout << "  --EndHead <number>:      Index of the last head directory to scan - set to " << cHeads - 1 << " to scan all head directories" << std::endl;
	return 1;
}

static const size_t cSyncSize =           9;
static const size_t cCrcSize =            32;
static const size_t cHeaderReservedSize = 4;
static const size_t cHeaderTrackSize =    12;
static const size_t cHeaderHeadSize =     10;
static const size_t cHeaderSectorSize =   6;
static const size_t cHeaderCrcSize =      cCrcSize;
static const size_t cHeaderDataSize =     cHeaderReservedSize + cHeaderTrackSize + cHeaderHeadSize + cHeaderSectorSize;
static const size_t cHeaderTrackOfs =     cHeaderReservedSize;
static const size_t cHeaderHeadOfs =      cHeaderReservedSize + cHeaderTrackSize;
static const size_t cHeaderSectorOfs =    cHeaderReservedSize + cHeaderTrackSize + cHeaderHeadSize;
static const size_t cHeaderCrcOfs =       cHeaderReservedSize + cHeaderTrackSize + cHeaderHeadSize + cHeaderSectorSize;
static const size_t cHeaderSize =         cSyncSize + cHeaderDataSize + cHeaderCrcSize;
static const size_t cSectorDataSize =     4096;
static const size_t cSectorCrcSize =      cCrcSize;
static const size_t cSectorSizeInBits =   cSyncSize + cSectorDataSize + cSectorCrcSize;


class OldBitStream_c: public std::vector<uint8_t> {
public:
	OldBitStream_c() { Rewind(); }
	void Rewind() { mCurBytePos = 0; mCurBitPos = 7; mCurPutBitPos = 7; mCurByte = 0; }
	bool GetBit() {
		bool RetVal = ((*this)[mCurBytePos] & (1 << mCurBitPos)) != 0;
		if (mCurBitPos == 0) {
			++mCurBytePos;
			mCurBitPos = 7;
		} else {
			--mCurBitPos;
		}
		return RetVal;
	}
	void PutBit(uint8_t aBit) {
		if (aBit != 0) {
			mCurByte |= 1 << mCurPutBitPos;
		}
		if (mCurPutBitPos == 0) {
			push_back(mCurByte);
			mCurByte = 0;
			mCurPutBitPos = 7;
		} else {
			--mCurPutBitPos;
		}
	}
	size_t GetSizeInBits() const { return size() * 8 + (7 - mCurPutBitPos); }
protected:
	size_t mCurBytePos;
	uint8_t mCurBitPos;
	uint8_t mCurPutBitPos;
	uint8_t mCurByte;

};



class BitStream_c {
public:
	BitStream_c() { Clear(); }
	bool Read(const char *aFileName) {
		Clear();
		size_t FileSize = size_t(boost::filesystem::file_size(aFileName));
		if (FileSize == 0) return false;
		mData.resize(FileSize,0);
		std::ifstream File(aFileName, std::ios::in | std::ios::binary);
		if (!File.read((char *)(&mData[0]),mData.size())) return false;
		return true;
	}
	void Rewind() {
		mPos = 0;
		mSubPos = 7;
	}
	void Clear() {
		mData.clear();
		Rewind();
		mCurPushBitPos = 7;
	}
	const Bit_e operator[] (size_t aIdx) const {
		size_t ByteOfs = aIdx >> 3;
		size_t Bit = aIdx & 7;
		uint8_t Byte = mData[ByteOfs];
		return ((Byte >> (7-Bit)) & 1) == 0 ? Bit_Zero : Bit_One;
	}
	Bit_e PopBit() {
		if (mPos == mData.size()) return Bit_Undef;
		if (mPos == mData.size()-1 && mSubPos <= mCurPushBitPos) return Bit_Undef;
		Bit_e Bit = Bit_e((mData[mPos] >> mSubPos) & 1);
		if (mSubPos != 0) {
			--mSubPos;
		} else {
			mSubPos = 7;
			++mPos;
		}
		return Bit;
	}
	void PushBit(Bit_e aBit) {
		if (mCurPushBitPos == 7) {
			mData.push_back(0);
		}
		uint8_t &CurByte = mData.back();
		if (aBit == Bit_One) {
			CurByte |= 1 << mCurPushBitPos;
		}
		if (mCurPushBitPos == 0) {
			mCurPushBitPos = 7;
		} else {
			--mCurPushBitPos;
		}
	}
	size_t size() const { return mData.size() * 8; } // TODO: this doesn't really take into account the partial bytes written
protected:
	std::vector<uint8_t> mData;
	size_t mPos;
	uint8_t mSubPos;
	uint8_t mCurPushBitPos;
};

double CrossCorrelation(const BitStream_c &aStrmA, const BitStream_c &aStrmB, int aOffset, size_t aStartOffs, size_t aSize) {
	int Start = int(aStartOffs);
	int End = int(aStartOffs + aSize);
	// Make sure things within limits
	if (End > int(aStrmA.size())) End = int(aStrmA.size());
	if (End+aOffset > int(aStrmB.size())) End = int(aStrmB.size()-aOffset);
	if (Start+aOffset < 0) Start = -aOffset;
	if (End == Start) return 0.0;
	int CrossCorr = 0;
	for(int i=Start;i<End;++i) {
		CrossCorr += aStrmA[i] ^ aStrmB[i+aOffset];
	}
	return (double)(CrossCorr) / (double)(End-Start);
}
const uint8_t cNoiseLimit = 3;

double DetermineBitLength(const std::vector<uint32_t> &aPulseLengthHistogram) {
	size_t MaxVal = 10000;
	// Search for the first big maximum: that is going to be our bit-time
	size_t MaxLoc = 0;
	for(size_t i=0;i<aPulseLengthHistogram.size();++i) {
		if (aPulseLengthHistogram[i] > MaxVal) {
			MaxLoc = (int)i;
			MaxVal = aPulseLengthHistogram[i];
		} else {
			// It's not a maximum. Did we find one already?
			if (MaxLoc != 0) {
				// We did. It means that we've passed the first big maximum, so bail
				break;
			}
		}
	}
	// Calculate the bit-length from the weigthed average of the 3 histogram samples around the maximum
	double BitLengthSum = (double)(aPulseLengthHistogram[MaxLoc-1] + aPulseLengthHistogram[MaxLoc] + aPulseLengthHistogram[MaxLoc+1]);
	double BitLength = ((double)aPulseLengthHistogram[MaxLoc-1] * (double)(MaxLoc-1) + (double)aPulseLengthHistogram[MaxLoc] * (double)(MaxLoc) + (double)aPulseLengthHistogram[MaxLoc+1] * (double)(MaxLoc+1)) / BitLengthSum;
	return BitLength;
}

std::vector<uint8_t> MakeRuns(BitStream_c &aBitStream, double &aBitTime, std::vector<size_t> *aRunTimes = nullptr, bool aPrintHistograms = cPrintHistograms) {
	std::vector<uint32_t> PulseLengthHistogram0;
	std::vector<uint32_t> PulseLengthHistogram1;
	PulseLengthHistogram0.resize(256,0);
	PulseLengthHistogram1.resize(256,0);
	size_t MaxPulseLength = 0;

	std::vector<uint8_t> Runs;

	size_t PulseLength = 0;
	size_t NoisyPulseLength = 0;
	Bit_e CurrentBit = aBitStream.PopBit();
	Bit_e LastBit;
	// Create histogram and runs, but do some noise-filter (runs of 1-3 pulses are ignored)
	size_t BitTime = 0;
	while (CurrentBit != Bit_Undef) {
		LastBit = CurrentBit;
		CurrentBit = aBitStream.PopBit();
		++PulseLength;
		++NoisyPulseLength;
		if (LastBit != CurrentBit) {
			/*if (NoisyPulseLength > cNoiseLimit)*/ {
				// Make sure the runs start with 0. This is important since the pulse-widths of the 1's and 0's are not the same
				if (Runs.size() == 0) {
					if (LastBit != 0) {
						Runs.push_back(100);
						BitTime += 100;
						if (aRunTimes != nullptr) aRunTimes->push_back(BitTime);
					}
				}
				Runs.push_back(uint8_t(PulseLength));
				BitTime += PulseLength;
				if (aRunTimes != nullptr) aRunTimes->push_back(BitTime);
				if (PulseLength >= PulseLengthHistogram0.size()) PulseLength = PulseLengthHistogram0.size() - 1;
				if (LastBit == 0) {
					PulseLengthHistogram0[PulseLength]++;
				} else {
					PulseLengthHistogram1[PulseLength]++;
				}
				if (MaxPulseLength < PulseLength) MaxPulseLength = PulseLength;
				PulseLength = 0;
			}
			NoisyPulseLength = 0;
		}
	}
	double BitLength0 = DetermineBitLength(PulseLengthHistogram0);
	double BitLength1 = DetermineBitLength(PulseLengthHistogram1);

	aBitTime = (BitLength0 + BitLength1 ) / 2.0;

/***/
	if (aPrintHistograms) {
		for(size_t i=0;i<=MaxPulseLength;++i) {
			std::cout << DecPrinter(i,4) << " - " << DecPrinter(PulseLengthHistogram0[i],10)  << " - " << DecPrinter(PulseLengthHistogram1[i],10) << std::endl;
		}
	}
/***/
/*****

	// Calculate number of samples that are inside the range of valid bit-transitions
	size_t ValidSum = PulseLengthHistogram[(int)(BitLength * 1.0 - 1)] + PulseLengthHistogram[(int)(BitLength * 1.0)] + PulseLengthHistogram[(int)(BitLength * 1.0 + 1)] + //     bit-time pulses
	                  PulseLengthHistogram[(int)(BitLength * 1.5 - 1)] + PulseLengthHistogram[(int)(BitLength * 1.5)] + PulseLengthHistogram[(int)(BitLength * 1.5 + 1)] + // 1.5 bit-time pulses
	                  PulseLengthHistogram[(int)(BitLength * 2.0 - 1)] + PulseLengthHistogram[(int)(BitLength * 2.0)] + PulseLengthHistogram[(int)(BitLength * 2.0 + 1)];  // 2.0 bit-time pulses
	size_t SumAll = 0;
	for(size_t i=0;i<PulseLengthHistogram.size();++i) {
		SumAll += PulseLengthHistogram[i];
	}
	double GoodRatio = (double)(ValidSum) / (double)(SumAll);
	
	std::cout << "Valid ratio: " << setw(10) << GoodRatio * 100.0 << "%  " << "Determined bit-length: " << BitLength << std::endl;
********/

	return Runs;
}

struct PulseType_s {
	uint8_t HalfBitLen: 7; // Length in half-bit sizes. 0 for invalid
	bool Error:1;
};

struct PulseCategorizer_s {
	const PulseType_s *Pulses[2];
	size_t PulseTableSize;
};

static const PulseType_s cPulses0_Strict[] = {
	{ 0,      true }, // 0
	{ 0,      true }, // 1
	{ 0,      true }, // 2
	{ 0,      true }, // 3
	{ 0,      true }, // 4
	{ 0,      true }, // 5
	{ 2,      true }, // 6
	{ 2,     false }, // 7
	{ 2,     false }, // 8
	{ 2,      true }, // 9
	{ 0,      true }, // 10
	{ 3,     false }, // 11
	{ 3,     false }, // 12
	{ 3,     false }, // 13
	{ 0,      true }, // 14
	{ 4,     false }, // 15
	{ 4,     false }, // 16
	{ 4,     false }, // 17
	{ 4,      true }, // 18
	{ 0,      true }, // 19
	{ 0,      true }, // 20 or more
};
static const PulseType_s cPulses1_Strict[] = {
	{ 0,      true }, // 0
	{ 0,      true }, // 1
	{ 0,      true }, // 2
	{ 0,      true }, // 3
	{ 0,      true }, // 4
	{ 0,      true }, // 5
	{ 2,      true }, // 6
	{ 2,     false }, // 7
	{ 2,     false }, // 8
	{ 2,     false }, // 9
	{ 2,      true }, // 10
	{ 0,      true }, // 11
	{ 3,     false }, // 12
	{ 3,     false }, // 13
	{ 3,     false }, // 14
	{ 0,      true }, // 15
	{ 4,     false }, // 16
	{ 4,     false }, // 17
	{ 4,     false }, // 18
	{ 4,      true }, // 19
	{ 0,      true }, // 20 or more
};

static const PulseCategorizer_s cStrictCategorizer = { {cPulses0_Strict, cPulses1_Strict}, sizeof(cPulses0_Strict) / sizeof(cPulses0_Strict[0]) };

static const PulseType_s cPulses0_Linient[] = {
	{ 0,      true }, // 0
	{ 0,      true }, // 1
	{ 0,      true }, // 2
	{ 0,      true }, // 3
	{ 0,      true }, // 4
	{ 0,      true }, // 5
	{ 2,      true }, // 6
	{ 2,     false }, // 7
	{ 2,     false }, // 8
	{ 2,      true }, // 9
	{ 3,      true }, // 10
	{ 3,     false }, // 11
	{ 3,     false }, // 12
	{ 3,     false }, // 13
	{ 4,      true }, // 14
	{ 4,     false }, // 15
	{ 4,     false }, // 16
	{ 4,     false }, // 17
	{ 4,      true }, // 18
	{ 0,      true }, // 19
	{ 0,      true }, // 20 or more
};
static const PulseType_s cPulses1_Linient[] = {
	{ 0,      true }, // 0
	{ 0,      true }, // 1
	{ 0,      true }, // 2
	{ 0,      true }, // 3
	{ 0,      true }, // 4
	{ 0,      true }, // 5
	{ 2,      true }, // 6
	{ 2,     false }, // 7
	{ 2,     false }, // 8
	{ 2,     false }, // 9
	{ 2,      true }, // 10
	{ 3,      true }, // 11
	{ 3,     false }, // 12
	{ 3,     false }, // 13
	{ 3,     false }, // 14
	{ 4,      true }, // 15
	{ 4,     false }, // 16
	{ 4,     false }, // 17
	{ 4,     false }, // 18
	{ 4,      true }, // 19
	{ 0,      true }, // 20 or more
};

static const PulseCategorizer_s cLinientCategorizer = { {cPulses0_Linient, cPulses1_Linient}, sizeof(cPulses0_Strict) / sizeof(cPulses0_Strict[0]) };

static const PulseType_s cPulses0_LinientB[] = {
	{ 0,      true }, // 0
	{ 0,      true }, // 1
	{ 0,      true }, // 2
	{ 0,      true }, // 3
	{ 0,      true }, // 4
	{ 0,      true }, // 5
	{ 2,      true }, // 6
	{ 2,     false }, // 7
	{ 2,     false }, // 8
	{ 2,      true }, // 9
	{ 2,      true }, // 10
	{ 3,     false }, // 11
	{ 3,     false }, // 12
	{ 3,     false }, // 13
	{ 3,      true }, // 14
	{ 4,     false }, // 15
	{ 4,     false }, // 16
	{ 4,     false }, // 17
	{ 4,      true }, // 18
	{ 0,      true }, // 19
	{ 0,      true }, // 20 or more
};
static const PulseType_s cPulses1_LinientB[] = {
	{ 0,      true }, // 0
	{ 0,      true }, // 1
	{ 0,      true }, // 2
	{ 0,      true }, // 3
	{ 0,      true }, // 4
	{ 0,      true }, // 5
	{ 2,      true }, // 6
	{ 2,     false }, // 7
	{ 2,     false }, // 8
	{ 2,     false }, // 9
	{ 2,      true }, // 10
	{ 2,      true }, // 11
	{ 3,     false }, // 12
	{ 3,     false }, // 13
	{ 3,     false }, // 14
	{ 3,      true }, // 15
	{ 4,     false }, // 16
	{ 4,     false }, // 17
	{ 4,     false }, // 18
	{ 4,      true }, // 19
	{ 0,      true }, // 20 or more
};

static const PulseCategorizer_s cLinientBCategorizer = { {cPulses0_LinientB, cPulses1_LinientB}, sizeof(cPulses0_Strict) / sizeof(cPulses0_Strict[0]) };

PulseType_s CategorizePulse(uint8_t aPulseWidth, uint8_t aSignalLevel, const PulseCategorizer_s &aPulseCategorizer) {
	if (aPulseWidth >= aPulseCategorizer.PulseTableSize) aPulseWidth = uint8_t(aPulseCategorizer.PulseTableSize - 1);
	return aPulseCategorizer.Pulses[aSignalLevel & 1][aPulseWidth];
}

size_t SyncScan(const std::vector<uint8_t> &aRuns, size_t aMinLength, size_t aStart, const PulseCategorizer_s &aPulseCategorizer) {
	size_t Idx = aStart;
	size_t Count = 0;
	while (true) {
		if (Idx >= aRuns.size()) return 0;
		uint8_t CurBit = Idx % 2; // The sequence is guaranteed to start with 0
		PulseType_s PulseType = CategorizePulse(aRuns[Idx], CurBit, aPulseCategorizer);
		if (PulseType.HalfBitLen == 2) {
			++Count;
		} else {
			if (Count >= aMinLength) {
				// TODO: only valid termination is a double-bit run. Remove that from decode and change 9 to 8 in bits beginning at the header and data
				if (PulseType.HalfBitLen == 3) return Idx;
			}
			Count = 0;
		}
		++Idx;
	}
}

struct DecodeResults_s {
	std::vector<Bit_e> Data;
	size_t ErrorRate;
};

DecodeResults_s Decode(const std::vector<uint8_t> &aRuns, size_t aStartIdx, size_t aEndIdx, double aBitTime, const PulseCategorizer_s &aPulseCategorizer) {
	DecodeResults_s RetVal;

	RetVal.ErrorRate = 0;
	if (aEndIdx > aRuns.size()) aEndIdx = aRuns.size();
	//size_t ErrorRate = 0;
	Bit_e LastBit = Bit_Zero;
	Bit_e CurrentBit = Bit_Zero;
	//BitStream_c DecodedStream;
	for(size_t Idx=aStartIdx;Idx<aEndIdx;++Idx) {
		LastBit = CurrentBit;
		uint8_t CurBit = Idx % 2; // The sequence is guaranteed to start with 0
		uint8_t PulseWidth = aRuns[Idx];
		PulseType_s PulseType = CategorizePulse(PulseWidth, CurBit, aPulseCategorizer);
		if (PulseType.HalfBitLen == 0) return RetVal;
		if (PulseType.Error) ++RetVal.ErrorRate;
		// We always start decoding with 0-s, so assume transitions on the bit-boundaries.
		switch (PulseType.HalfBitLen) {
			case 2: // We don't change bit
				CurrentBit = LastBit;
				RetVal.Data.push_back(CurrentBit);
				//DecodedStream.PushBit(CurrentBit);
			break;
			case 3: // Bit changes, plus pulse-location shifts by 0.5 bit-locations. This means that every other time we have to push two values in
				if (LastBit == Bit_Zero) {
					CurrentBit = Bit_One;
					RetVal.Data.push_back(LastBit);
					RetVal.Data.push_back(CurrentBit);
					//DecodedStream.PushBit(CurrentBit);
				} else {
					CurrentBit = Bit_Zero;
					RetVal.Data.push_back(CurrentBit);
					//DecodedStream.PushBit(LastBit);
					//DecodedStream.PushBit(CurrentBit);
				}
			break;
			case 4: // This is the '0' in a 1-0-1 sequence.
				if (LastBit != Bit_One) return RetVal;
				RetVal.Data.push_back(Bit_Zero);
				RetVal.Data.push_back(LastBit);
				//DecodedStream.PushBit(LastBit);
				//DecodedStream.PushBit(Bit_Zero);
				CurrentBit = LastBit;
			break;
			default: return RetVal;
		}
	} 
	return RetVal;
}

std::vector<Bit_e> IntBin(uint64_t aNumber, size_t aWidth) {
	std::vector<Bit_e> RetVal;
	while (aWidth > 0) {
		if ((aNumber >> (aWidth - 1)) && 1 == 1) {
			RetVal.push_back(Bit_One);
		} else {
			RetVal.push_back(Bit_Zero);
		}
	}
	return RetVal;
}

uint64_t BinInt(const std::vector<Bit_e> &aString, size_t aStartIdx, size_t aEndIdx) {
	uint64_t RetVal = 0;
	if (aEndIdx > aString.size()) aEndIdx = aString.size();
	for (size_t Idx=aStartIdx;Idx < aEndIdx; ++Idx) {
		Bit_e C = aString[Idx];
		RetVal <<= 1;
		if (C == Bit_One) RetVal += 1;
	}
	return RetVal;
}

bool Validate(std::vector<Bit_e> &aData, size_t aStartIdx, size_t aEndIdx) {
	uint32_t LocalCrc = 0;
	const uint32_t Polynom = (1 << 23) | (1 << 21) | (1 << 11) | (1 << 2) | (1 << 0);
	uint32_t CodeWord = Polynom;

	if (aEndIdx > aData.size()) aEndIdx = aData.size();
	// Go through the payload portion of the data and re-calculate the CRC
	for(size_t Idx=aEndIdx-cCrcSize-1;Idx>=aStartIdx;--Idx) {
		Bit_e C = aData[Idx];
		if (C == Bit_One) LocalCrc ^= CodeWord;
		if ((CodeWord >> 31) == 1) {
			CodeWord <<= 1;
			CodeWord = CodeWord ^ Polynom;
		} else {
			CodeWord <<= 1;
		}
	}
	uint32_t StreamCrc = uint32_t(BinInt(aData,aEndIdx-32,aEndIdx));
	return (StreamCrc == LocalCrc);
}

std::vector<uint8_t> ToBytes(const std::vector<Bit_e> &aBits, size_t aStartIdx, size_t aEndIdx) {
	std::vector<uint8_t> RetVal;
	if (aEndIdx > aBits.size()) aEndIdx = aBits.size();
	for(size_t ByteIdx=aStartIdx;ByteIdx<aEndIdx;ByteIdx+=8) {
		uint8_t Byte = 0;
		for(size_t BitIdx=0;BitIdx<8;++BitIdx) {
			Byte <<= 1;
			if (aBits[ByteIdx+BitIdx] == Bit_One) {
				Byte |= 1;
			}
		}
		RetVal.push_back(Byte);
	}
	return RetVal;
}

bool AreSectorsEqual(const std::vector<uint8_t> &aSec1, const std::vector<uint8_t> &aSec2) {
	if (aSec1.size() != aSec2.size()) return false;
	return memcmp(&aSec1[0],&aSec2[0],aSec1.size()) == 0;
}

#ifdef _MSC_VER
#pragma optimize( "", on )
#endif
class Sector_c {
public:
	Sector_c() {}

	void SetData(const std::vector<uint8_t> &aData, size_t aErrorRate) {
		if (!IsValid()) {
			mData.push_back(SectorInfo_c(aData,aErrorRate));
		} else {
			for (size_t Idx=0;Idx < mData.size(); ++Idx) {
				if (mData[Idx] == aData) {
					mData[Idx].ErrorRate = std::min(mData[Idx].ErrorRate,aErrorRate);
					++mData[Idx].RecoveryCount;
					return;
				}
			}
			// Add an alternative
			mData.push_back(SectorInfo_c(aData,aErrorRate));
		}
	}
	const std::vector<uint8_t> &GetBestData() const {
		CRAY_ASSERT(IsValid());
		size_t BestIdx = 0;
		for (size_t Idx=0;Idx < mData.size(); ++Idx) {
			if (mData[Idx].BetterThan(mData[BestIdx])) BestIdx = Idx;
		}
		return mData[BestIdx].Data;
	}
	size_t GetAlternateCnt() const { CRAY_ASSERT(IsValid()); return mData.size()-1; }
	const std::vector<uint8_t> &GetAlternateData(size_t aIdx) const {
		CRAY_ASSERT(IsValid());
		size_t BestIdx = 0;
		for (size_t Idx=0;Idx < mData.size(); ++Idx) {
			if (mData[Idx].BetterThan(mData[BestIdx])) BestIdx = Idx;
		}
		if (aIdx == BestIdx) ++aIdx;
		CRAY_ASSERT(aIdx < mData.size());
		return mData[aIdx].Data;
	}
	bool IsValid() const { return mData.size() != 0; }
	bool IsUnique() const { return mData.size() == 1; }
	char GetLogCode() const { 
		if (!IsValid()) return '.';
		if (!IsUnique()) return 'M';
		CRAY_ASSERT(mData[0].RecoveryCount > 0);
		if (mData[0].RecoveryCount > 9) return '+';
		return char('0' + mData[0].RecoveryCount);
	}
protected:
	class SectorInfo_c {
	public:
		std::vector<uint8_t> Data;
		size_t ErrorRate;
		size_t RecoveryCount;

		SectorInfo_c(): RecoveryCount(0) {}
		SectorInfo_c(const std::vector<uint8_t> &aData, size_t aErrorRate, size_t aRecoveryCount = 1): ErrorRate(aErrorRate), RecoveryCount(aRecoveryCount) {
			Data.resize(aData.size());
			memcpy(&Data[0],&aData[0],aData.size());
		}
		bool operator == (const SectorInfo_c &aB) const {
			if (Data.size() != aB.Data.size()) return false;
			return memcmp(&Data[0],&aB.Data[0],Data.size()) == 0;
		}
		bool operator == (const std::vector<uint8_t> &aB) const {
			if (Data.size() != aB.size()) return false;
			return memcmp(&Data[0],&aB[0],Data.size()) == 0;
		}
		bool operator != (const SectorInfo_c &aB) const { return ! (*this == aB); }
		bool operator != (const std::vector<uint8_t> &aB) const { return ! (*this == aB); }
		bool BetterThan (const SectorInfo_c &aB) const {
			if (RecoveryCount > aB.RecoveryCount) return true;
			if (RecoveryCount == aB.RecoveryCount && ErrorRate < aB.ErrorRate) return true;
			return false;
		}
	};

	std::vector<SectorInfo_c> mData;
};

class DiskImage_c {
public:
	DiskImage_c(): mHeads(0), mTracks(0), mSectors(0) {
		InitMissingSector();
	}
	DiskImage_c(size_t aHeads, size_t aTracks, size_t aSectors) {
		InitMissingSector();
		SetSize(aHeads, aTracks, aSectors);
	}

	void SetSize(size_t aHeads, size_t aTracks, size_t aSectors) {
		mSectorData.resize(aHeads * aTracks * aSectors);
		mTrackScans.resize(aHeads * aTracks);
		mHeads = aHeads;
		mTracks = aTracks;
		mSectors = aSectors;
	}

	Sector_c &GetSector(size_t aHead, size_t aTrack, size_t aSector) {
		if (aHead >= mHeads) throw std::range_error("head is out of range");
		if (aTrack > mTracks || aTrack == 0) throw std::range_error("track is out of range");
		if (aSector >= mSectors) throw std::range_error("sector is out of range");
		size_t Idx = (aSector) + (aTrack-1) * mSectors + (aHead) * mTracks * mSectors;
		return mSectorData[Idx];
	}

	bool IsValidIdx(size_t aHead, size_t aTrack, size_t aSector) const {
		if (aHead >= mHeads) return false;
		if (aTrack > mTracks || aTrack == 0) return false;
		if (aSector >= mSectors) return false;
		return true;
	}

	void PrintSectorStats(size_t aHead, size_t aTrack, std::ostream &aStrm, bool aTerminateLine = true, bool aPrintHeader = true) {
		if (aPrintHeader) aStrm << "Head:" << DecPrinter(aHead,1) << " Track:" << DecPrinter(aTrack,3) << " - ";
		size_t GoodCnt = 0;
		for(size_t Sector=0;Sector<mSectors;++Sector) {
			Sector_c &CurSector = GetSector(aHead,aTrack,Sector);
			if (CurSector.IsValid()) ++GoodCnt;
			aStrm << CurSector.GetLogCode();
		}
		if (aPrintHeader) aStrm << " = " << DecPrinter(GoodCnt,0);
		if (aTerminateLine) aStrm << std::endl;
	}

	bool WriteToDisk(const char *aFileName, const char *aStatFileName, bool aPrintStats = true) {
		std::ofstream OutFile(aFileName, std::ios::out | std::ios::binary);
		std::ofstream StatFile(aStatFileName, std::ios::out | std::ios::binary);
		if (OutFile.bad()) return false;
		for(size_t Track=1;Track<=mTracks;++Track) {
			for(size_t Head=0;Head<mHeads;++Head) {
				if (aPrintStats) {
					PrintSectorStats(Head, Track, std::cout);
					PrintSectorStats(Head, Track, StatFile);
				}
				for(size_t Sector=0;Sector<mSectors;++Sector) {
					Sector_c &CurSector = GetSector(Head,Track,Sector);
					if (CurSector.IsValid()) {
						const std::vector<uint8_t> &Data = CurSector.GetBestData();
						OutFile.write((char *)&Data[0],Data.size());
						if (OutFile.bad()) return false;
					} else {
						OutFile.write((char *)&mMissingSector[0],mMissingSector.size());
						if (OutFile.bad()) return false;
					}
				}
			}
		}
		OutFile.close();
		StatFile.close();
		return true;
	}
	bool LoadFromDisk(const char *aFileName, size_t &aMissingSectorCnt) {
		aMissingSectorCnt = 0;
		std::ifstream InFile(aFileName, std::ios::in | std::ios::binary);
		if (InFile.bad()) return false;
		for(size_t Track=1;Track<=mTracks;++Track) {
			for(size_t Head=0;Head<mHeads;++Head) {
				for(size_t Sector=0;Sector<mSectors;++Sector) {
					Sector_c &CurSector = GetSector(Head,Track,Sector);
					std::vector<uint8_t> Data;
					Data.resize(cSectorSize);
					InFile.read((char *)&Data[0],cSectorSize);
					if(!AreSectorsEqual(Data,mMissingSector)) {
						CurSector.SetData(Data,SIZE_MAX);
					} else {
						++aMissingSectorCnt;
					}
					if (InFile.bad()) return false;
				}
			}
		}
		InFile.close();
		return true;
	}
	bool LoadScanInfo(const char *aFileName) {
		std::ifstream InFile(aFileName, std::ios::in);
		if (InFile.bad()) return false;
		while (!InFile.eof()) {
			char LineBuf[32768*4];
			InFile.getline(LineBuf,sizeof(LineBuf)/sizeof(LineBuf[0]));
			if (InFile.bad()) return false;
			char *Token = strtok(LineBuf,",");
			size_t TokenCnt = 0;
			char *ScanFileName = nullptr;
			int Head = -1;
			int Track = -1;
			while (Token != nullptr && TokenCnt < 3) {
				switch (TokenCnt) {
					case 0: // File name
						ScanFileName = Token;
					break;
					case 1: // Head
						Head = atoi(Token);
					break;
					case 2: // Sector
						Track = atoi(Token);
					break;
				}
				Token = strtok(nullptr,",");
				++TokenCnt;
			}
			if (TokenCnt == 3 && IsValidIdx(Head,Track,0)) {
				// Full line retrieved
				mTrackScans[Head * mTracks + Track].push_back(std::string(ScanFileName));
			}
		}
		InFile.close();
		return true;
	}

	bool WriteAlternatesToDisk(const char *aBaseFileName) {
		for(size_t Track=1;Track<=mTracks;++Track) {
			for(size_t Head=0;Head<mHeads;++Head) {
				for(size_t Sector=0;Sector<mSectors;++Sector) {
					Sector_c &CurSector = GetSector(Head,Track,Sector);
					if (CurSector.IsValid() && !CurSector.IsUnique()) {
						size_t AlternateCnt = CurSector.GetAlternateCnt();
						for (size_t Idx=0;Idx<AlternateCnt;++Idx) {
							const std::vector<uint8_t> &Data = CurSector.GetAlternateData(Idx);
							std::stringstream FileName;
							FileName << aBaseFileName << "_" << HexPrinter((Sector + Head * mSectors + Track * mSectors * mHeads) * cSectorSize,8) << "_S" << DecPrinter(Sector,0) << "H" << DecPrinter(Head,0) << "T" << DecPrinter(Track,0) << "_" << DecPrinter(Idx,0) << ".alt";
							std::ofstream OutFile(FileName.str(), std::ios::out | std::ios::binary);
							if (OutFile.bad()) return false;
							OutFile.write((char *)&(Data[0]),Data.size());
							if (OutFile.bad()) return false;
							OutFile.close();
						}
					}
				}
			}
		}
		return true;
	}
	const std::vector<std::string> GetScans(size_t aHead, size_t aTrack) const {
		if (!IsValidIdx(aHead,aTrack,0)) throw std::invalid_argument("Invalid argument to GetScans");
		return mTrackScans[aHead * mTracks + aTrack];
	}

protected:
	void InitMissingSector() {
		mMissingSector.resize(cSectorSize);
		for(size_t i=0;i<cSectorSize/16;++i) {
			const char *Data = "<<<<MISSING>>>>>";
			memcpy(&mMissingSector[i*16],Data,16);
		}
	}
	size_t mHeads;
	size_t mTracks;
	size_t mSectors;
	std::vector<uint8_t> mMissingSector;
	std::vector<Sector_c> mSectorData;
	std::vector<std::vector<std::string>> mTrackScans;
};
#ifdef _MSC_VER
#pragma optimize( "", off )
#endif

DiskImage_c Image;

struct SectorStats_s {
	SectorStats_s(size_t aStartHeader,size_t aStartData): StartHeader(aStartHeader), StartData(aStartData) {}
	SectorStats_s(): StartHeader(0), StartData(0) {}

	size_t StartHeader;
	size_t StartData;
};

struct TrackStats_s {
	std::vector<std::vector<SectorStats_s>> Sectors;
	size_t Track;
	size_t SectorCnt;
};

TrackStats_s Extract(const std::vector<uint8_t> &aRuns, std::vector<size_t> aRunTimes, double aBitTime, DiskImage_c &aImage, size_t aExpectedHead, const PulseCategorizer_s &aPulseCategorizer, std::ostream *aRecoveryLog = nullptr) {
	TrackStats_s TrackStats;

	size_t CurrentRunIdx = 0;
	size_t Track = 0;
//	size_t Head = 0;
	size_t SectorCnt = 0;

	TrackStats.Sectors.resize(cSectors);

	// Test code to save categorized data
	//{
	//	fstream PulseFile("pulses.txt",ios::out);
	//	size_t Idx = 0;
	//	size_t Count = 0;
	//	while (true) {
	//		if (Idx >= aRuns.size()) break;
	//		uint8_t CurBit = Idx % 2; // The sequence is guaranteed to start with 0
	//		PulseType_s PulseType = CategorizePulse(aRuns[Idx], CurBit, aPulseCategorizer);
	//		if (PulseType.HalfBitLen == 0) {
	//			PulseFile << '.';
	//		} else {
	//			PulseFile << char(PulseType.HalfBitLen + '0');
	//		}
	//		++Idx;
	//	}
	//}

	int LastSector = -1;
	while(true) {
		///////////////////////////////
		// HEADER
		///////////////////////////////

		size_t HeaderStart = SyncScan(aRuns,50,CurrentRunIdx,aPulseCategorizer);
		if (HeaderStart == 0) break;
		CurrentRunIdx = HeaderStart;

		DecodeResults_s Header = Decode(aRuns,HeaderStart,HeaderStart+cHeaderSize,aBitTime,aPulseCategorizer);
		if (Header.Data.size() < cHeaderSize || BinInt(Header.Data,0,cSyncSize) != 0xf0) continue;
//		std::cout << "first: " << DecPrinter(Runs[HeaderStart]) << std::endl;

		bool IsHeaderValid = Validate(Header.Data,cSyncSize,cSyncSize+cHeaderSize);
		if (!IsHeaderValid) continue;
		size_t CurrentTrack  = size_t(BinInt(Header.Data,cSyncSize+cHeaderTrackOfs,  cSyncSize+cHeaderTrackOfs+ cHeaderTrackSize));
		size_t CurrentHead   = size_t(BinInt(Header.Data,cSyncSize+cHeaderHeadOfs,   cSyncSize+cHeaderHeadOfs+  cHeaderHeadSize));
		size_t CurrentSector = size_t(BinInt(Header.Data,cSyncSize+cHeaderSectorOfs, cSyncSize+cHeaderSectorOfs+cHeaderSectorSize));

		//std::cout << "HTS: " << CurrentHead << ":" << CurrentTrack << ":" << CurrentSector << " ";

		if (CurrentHead != aExpectedHead) continue;
		if (Track != 0 && Track != CurrentTrack) continue;
		if (!aImage.IsValidIdx(CurrentHead, CurrentTrack, CurrentSector)) continue;
		if (LastSector != -1) {
			// Make sure sector numbers are consequtive, but only if we don't experience gaps
			if (CurrentSector != 0 && (LastSector + 1 != int(CurrentSector))) {
				LastSector = -1;
				continue; 
			}
		}
		Track = CurrentTrack;
//		Head = CurrentHead;

		///////////////////////////////
		// DATA
		///////////////////////////////

		size_t DataStart = SyncScan(aRuns,100,HeaderStart,aPulseCategorizer);
		if (DataStart == 0) break;
		DecodeResults_s Data = Decode(aRuns,DataStart,DataStart+cSectorSizeInBits,aBitTime,aPulseCategorizer);
		//std::cout << "data of " << Data.Data.size() << " bits found ";
		if (Data.Data.size() < cSectorSizeInBits || BinInt(Data.Data,0,cSyncSize) != 0xf0) {
			//std::cout << std::endl;
			continue;
		}
//		std::cout << "first: " << DecPrinter(Runs[HeaderStart]) << std::endl;
		bool IsDataValid = Validate(Data.Data,cSyncSize,cSectorSizeInBits);
		if (!IsDataValid) {
			if (LastSector != -1) { std::cout << "\t\t\tsector: " << CurrentSector << " killed only for CRC" << std::endl; }
			//std::cout << std::endl;
			continue;
		}
		//std::cout << "valid " << std::endl;
		std::vector<uint8_t> SectorBytes = ToBytes(Data.Data,cSyncSize,cSyncSize+cSectorDataSize);
		if (aRecoveryLog != nullptr) *aRecoveryLog << DecPrinter(CurrentSector) << ',';
		// Save the data into the image
		++SectorCnt;
		Sector_c &ImgSector = aImage.GetSector(CurrentHead, CurrentTrack, CurrentSector);
		ImgSector.SetData(SectorBytes, Data.ErrorRate);

		// Update last sector to track continuity
		std::cout << "\t\t\tfound sector: " << CurrentSector << " with previous sector: " << LastSector << std::endl;
		LastSector = int(CurrentSector);

		TrackStats.Sectors[CurrentSector].push_back(SectorStats_s(HeaderStart,DataStart));
		//std::cout << "found sector HTS: " << setw(1) << CurrentHead << ":" << setw(3) << CurrentTrack << ":" << setw(2) << CurrentSector << " at offset: " << hex << setw(8) << HeaderStart << " data: " << DataStart << dec << setw(0) << std::endl;

		// Step over the data portion
		CurrentRunIdx = DataStart;
	}
	TrackStats.SectorCnt = SectorCnt;
	TrackStats.Track = Track;
	return TrackStats;
}

size_t IdxToTime(const std::vector<uint8_t> aRuns, size_t aIdx) {
	aIdx = std::min(aIdx,aRuns.size()-1);
	size_t Time = 0;
	for(size_t Idx=0;Idx<=aIdx;++aIdx) {
		Time += aRuns[Idx];
	}
	return Time;
}

inline double sqr(double aVal) { return aVal * aVal; }

BitStream_c FoldStream(BitStream_c &aBitStream,const std::vector<size_t> &aFoldLocations, size_t aFoldSize,uint8_t aExcludeIdx) {
/*	BitStream_c RetVal;
	size_t RawFoldCnt = aFoldLocations.size();
	if (RawFoldCnt < 3) return RetVal;
	CRAY_ASSERT(RawFoldCnt == 4); // The logic below seems to be broken for anything but four...
	size_t FoldCnt = RawFoldCnt;
	if ((FoldCnt & 1) == 0) --FoldCnt; // Make sure FoldCnt is odd
	size_t Cutoff = FoldCnt >> 1;
	if (FoldCnt > 3) FoldCnt = 3;
	//size_t Offset = (RawFoldCnt > 3) ? aBitStream.size() / 10 : 0; // Try not to use the first 10% of the samples (head position might not have stabilized after the step) if possible
	size_t Offset = 0;
	if (RawFoldCnt <= 3) aExcludeIdx = 4; // Make sure we only do exclusions if we in fact have 4 copies at least
	for(size_t Idx=0;Idx<aFoldLocations[1];++Idx) {
		size_t Cnt = 0;
		for(size_t Fold=0;Fold<RawFoldCnt;++Fold) {
			if (Fold != aExcludeIdx) {
				Cnt += aBitStream[Offset + Idx + aFoldLocations[Fold]];
			}
		}
		RetVal.PushBit(Cnt > Cutoff ? Bit_One : Bit_Zero);
	}
	// Duplicate the stream to make sure we have at least one good reading of all sectors even of the one that wraps around at the fold
	for(size_t Idx=0;Idx<aFoldLocations[1];++Idx) {
		RetVal.PushBit(RetVal.PopBit());
	}
	return RetVal;*/
	int LastBit = 0; // We always start from 0
	BitStream_c RetVal;
	size_t Offset = 0;
	size_t FoldCnt = aFoldLocations.size();
	for(size_t Idx=0;Idx<aFoldSize;++Idx) {
		size_t Cnt = 0;
		for(size_t Fold=0;Fold<FoldCnt;++Fold) {
			Cnt += aBitStream[Offset + Idx + aFoldLocations[Fold]];
		}
		if (LastBit == 0) {
			// Last bit is 0 --> write 0-s first, 1-s next
			size_t Fold;
			for(Fold=0;Fold<FoldCnt-Cnt;++Fold) {
				RetVal.PushBit(Bit_Zero);
				LastBit = 0;
			}
			for(;Fold<FoldCnt;++Fold) {
				RetVal.PushBit(Bit_One);
				LastBit = 1;
			}
		} else {
			size_t Fold;
			for(Fold=0;Fold<Cnt;++Fold) {
				RetVal.PushBit(Bit_One);
				LastBit = 1;
			}
			for(;Fold<FoldCnt;++Fold) {
				RetVal.PushBit(Bit_Zero);
				LastBit = 0;
			}
		}
	}
	// Duplicate the stream to make sure we have at least one good reading of all sectors even of the one that wraps around at the fold
	for(size_t Idx=0;Idx<aFoldLocations[1];++Idx) {
		RetVal.PushBit(RetVal.PopBit());
	}
	return RetVal;
}

void ExtractWithFold(BitStream_c &aBitStream,const std::vector<size_t> &aFoldLocations, size_t aFoldSize, const char *aFileName, DiskImage_c &aImage, size_t aExpectedHead, const PulseCategorizer_s &aPulseCategorizer, uint8_t aExcludeIdx, std::ostream *aRecoveryLog = nullptr) {
	if (aFoldLocations.size() < 2) return;
	BitStream_c FoldedStream = FoldStream(aBitStream,aFoldLocations,aFoldSize,aExcludeIdx);
	FoldedStream.Rewind();

	double BitTime;
	std::vector<size_t> RunTimes;
	std::vector<uint8_t> Runs = MakeRuns(FoldedStream,BitTime,&RunTimes,true);

	TrackStats_s TrackStats;
	TrackStats = Extract(Runs, RunTimes, BitTime, aImage, aExpectedHead, aPulseCategorizer, aRecoveryLog);

	std::cout << "* ";
	if (aImage.IsValidIdx(aExpectedHead, TrackStats.Track, 0)) {
		aImage.PrintSectorStats(aExpectedHead, TrackStats.Track, std::cout, false);
	} else {
		std::cout << "-";
	}
	std::cout << " file: " << aFileName << " extacted sectors: " << TrackStats.SectorCnt;
	std::cout << " *" << DecPrinter(aExcludeIdx,0) << std::endl;
}

size_t Extract(const char *aFileName, DiskImage_c &aImage, size_t aExpectedHead, const PulseCategorizer_s &aPulseCategorizer, std::ostream *aRecoveryLog = nullptr) {
	BitStream_c BitStream;

	if (!BitStream.Read(aFileName)) return 0;

	double BitTime;
	std::vector<size_t> RunTimes;
	std::vector<uint8_t> Runs = MakeRuns(BitStream,BitTime,&RunTimes,true);

	TrackStats_s TrackStats;
	TrackStats = Extract(Runs, RunTimes, BitTime, aImage, aExpectedHead, aPulseCategorizer, aRecoveryLog);

	std::cout << "- ";
	if (aImage.IsValidIdx(aExpectedHead, TrackStats.Track, 0)) {
		aImage.PrintSectorStats(aExpectedHead, TrackStats.Track, std::cout, false);
	} else {
		std::cout << "-";
	}
	// Print statistics
	std::cout << " file: " << aFileName << " extacted sectors: " << TrackStats.SectorCnt << std::endl;

	std::vector<size_t> StrideEstimates;
	for(size_t Sector=0;Sector<TrackStats.Sectors.size();++Sector) {
		std::vector<SectorStats_s> &Stats = TrackStats.Sectors[Sector];
		if (Stats.size() > 1) {
			size_t CurHeadTime = RunTimes[Stats[0].StartHeader];
			size_t CurDataTime = RunTimes[Stats[0].StartData];
			size_t PrevHeadTime;
			size_t PrevDataTime;
			for(size_t Idx=1;Idx<Stats.size();++Idx) {
				PrevHeadTime = CurHeadTime;
				PrevDataTime = CurDataTime;
				CurHeadTime = RunTimes[Stats[Idx].StartHeader];
				CurDataTime = RunTimes[Stats[Idx].StartData];
				size_t HeadStride = CurHeadTime - PrevHeadTime;
				size_t DataStride = CurDataTime - PrevDataTime;
				// TODO: can we figure out the cut-off lines dinamically?
				if (HeadStride > 2000000) HeadStride /= 2;
				if (HeadStride > 3000000) HeadStride /= 3;
				if (DataStride > 2000000) DataStride /= 2;
				if (DataStride > 3000000) DataStride /= 3;
				if (HeadStride > 1300000 && HeadStride < 1320000) StrideEstimates.push_back(HeadStride);
				if (DataStride > 1300000 && DataStride < 1320000) StrideEstimates.push_back(DataStride);
			}
		}
	}
	do {
		if (StrideEstimates.size() == 0) break; // We don't have enough data to do anything interesting
		// Calculate average
		sort(StrideEstimates.begin(),StrideEstimates.end());
		size_t Sum = 0;
		size_t Cnt = 0;
		for(size_t i=0;i<StrideEstimates.size();++i) {
			Sum += StrideEstimates[i];
			++Cnt;
		}
		double Avg = (double)Sum/(double)Cnt;
		// Calculate deviation
		double Dev = 0.0;
		for(size_t i=0;i<StrideEstimates.size();++i) {
			Dev += sqr(Avg - StrideEstimates[i]);
		}
		Dev = sqrt(Dev)/Cnt;
		// TODO: how to figure out the limit manually?
		if (Dev > 100.0) break;
/***
		vector<size_t> FoldLocations;

		size_t Fold = int(Avg);
		FoldLocations.push_back(0);
		while (Fold < BitStream.size()) {
			double MinCrossCorr = 1.0;
			int MinOffset = INT_MIN;
			for(int Offset=-50;Offset<=50;++Offset) {
				double CrossCorr = CrossCorrelation(BitStream, BitStream, Fold + Offset, int(Avg)*1/3, int(Avg)*2/3);
				CRAY_ASSERT(CrossCorr <= 1.0);
				//std::cout << "\tCross correlation for offset: " << setw(3) << Offset << setw(0) << " is: " << CrossCorr << ((MinCrossCorr > CrossCorr) ? " <--" : "") << std::endl;
				if (MinCrossCorr > CrossCorr) {
					//std::cout << "\t\tNew minimum found" << std::endl;
					MinCrossCorr = CrossCorr;
					MinOffset = Offset;
				}
			}
			std::cout << "\t\tCross-correlation: " << MinCrossCorr << std::endl;
			CRAY_ASSERT(MinOffset != INT_MIN);
			if (Fold + int(Avg) < BitStream.size()) {
				if (MinCrossCorr < 0.1) {
					FoldLocations.push_back(Fold + MinOffset);
				}
			}
			Fold += int(Avg)+MinOffset;
		}

//		for(uint8_t ExcludeIdx = 0; ExcludeIdx < FoldLocations.size(); ++ExcludeIdx) {
			ExtractWithFold(BitStream, FoldLocations, int(Avg), aFileName, aImage, aExpectedHead, aPulseCategorizer, 0, aRecoveryLog);
//		}
***/
	} while(false);
	return TrackStats.Track;
}

const char *ImageFileName = "disk.img";
const char *Phase2ImageFileName = "disk_p2.img";
const char *AlternateBaseName = "disk";
const char *StatFileName = "disk.stat";
const char *Phase2StatFileName = "disk_p2.stat";
const char *InfoFileName = "scan.info";

int ScannerPhase1(CommandLine_c aCmdLine)
{
	DiskImage_c Image;
	Image.SetSize(cHeads,cTracks,cSectors);

//	size_t LastTrack = 0;
//	size_t LastHead = 0;
	size_t CurrentTrack = 0;

	remove(ImageFileName);
	remove(StatFileName);
	remove(InfoFileName);

	size_t StartFileIdx = 0;
	size_t EndFileIdx = 0;
	size_t StartHead = 0;
	size_t EndHead = cHeads-1;

	try {
		while(aCmdLine.HasMoreParams()) {
			std::string Param = aCmdLine.GetNextParam();
			if (Param == "--StartFileIdx") {
				StartFileIdx = atol(aCmdLine.GetNextParam().c_str());
			} else if (Param == "--EndFileIdx") {
				EndFileIdx = atol(aCmdLine.GetNextParam().c_str());
			} else if (Param == "--StartHead") {
				StartHead = atol(aCmdLine.GetNextParam().c_str());
			} else if (Param == "--EndHead") {
				EndHead = atol(aCmdLine.GetNextParam().c_str());
			} else {
				throw Generic_x("Invalid command line parameter");
			}
		}
		if (EndFileIdx < StartFileIdx) {
			throw Generic_x("EndFileIdx must not be less than StartFileIdx");
		}
		if (EndHead < StartHead) {
			throw Generic_x("EndHead must not be less than StartHead");
		}
	}
	catch(std::exception &e) {
		std::cout << "Error processing command line arguments: " << e.what() << std::endl;
		return PrintUsage(aCmdLine.GetProgramName().c_str());
	}
/***
	const size_t StartFileIdx = 4215; // track 242
	const size_t EndFileIdx = 4229;
	const size_t StartHead = 3;
	const size_t EndHead = 3;
***/

	std::ofstream InfoFile;
	try {
		for(size_t Head=StartHead;Head<=EndHead;++Head) {
			size_t FileIdx = StartFileIdx;
			size_t MissCnt = 0;
			do {
				std::stringstream FileName;
				FileName << "h" << Head << "\\" << "s" << FileIdx << "_h" << Head << ".txt";
				if (!boost::filesystem::exists(FileName.str())) {
					++MissCnt;
					if (MissCnt > 100) break;
					continue;
				}
				MissCnt = 0;
//				if (CurrentTrack != 0) LastTrack = CurrentTrack;
				std::stringstream ExtractionLog;
				CurrentTrack = Extract(FileName.str().c_str(),Image,Head,cLinientCategorizer,&ExtractionLog);

				try {
					// Update scan file info
					InfoFile.open(InfoFileName, std::ios::out | std::ios::app);
					InfoFile << FileName.str() << "," << Head;
					if (CurrentTrack != 0) {
						InfoFile << "," << CurrentTrack << "," << ExtractionLog.str();
						//Image.PrintSectorStats(Head, CurrentTrack, InfoFile, false, false);
					}
					InfoFile << std::endl;
					InfoFile.close();
				} catch (...) {
					try {
						InfoFile.close();
					}
					catch(...) {}
				}

//				if (LastTrack != CurrentTrack && CurrentTrack != 0 && LastTrack != 0) {
//					LastHead = Head;
//				}

				++FileIdx;
				if (FileIdx > EndFileIdx && EndFileIdx != 0) break;
			} while (true);
		}
	}
	catch (...) {
		std::cout << std::endl << "Exception caught. Writing out whaterver we've gotten so far" << std::endl;
	}

	// By now we should have the whole image on disk, unless we skipped whole tracks. But just to be safe, overwrite the whole thing again.
	Image.WriteToDisk(ImageFileName,StatFileName);
	Image.WriteAlternatesToDisk(AlternateBaseName);

	return 0;
}

struct ScanOffsetInfo_s {
	ScanOffsetInfo_s(size_t aOffset, size_t aIdx): Offset(aOffset), Idx(aIdx) {}
	size_t Offset;
	size_t Idx;
};

void FixTrack(size_t aHead, size_t aTrack, DiskImage_c &aImage) {
	std::vector<BitStream_c> BitStreams;
	std::vector<std::vector<size_t>> RunTimes;
	std::vector<std::vector<uint8_t>> Runs;
	std::vector<TrackStats_s> TrackStats;

	const std::vector<std::string> &Scans = aImage.GetScans(aHead, aTrack);

	size_t ScanCnt = Scans.size();
	for(size_t Idx = 0; Idx < ScanCnt; ++Idx) {
		std::cout << "\tLoading scan data " << DecPrinter(Idx+1,0) << " out of " << DecPrinter(ScanCnt,0) << " from file: " << Scans[Idx] << std::endl;
		std::stringstream ExtractionLog;
		Extract(Scans[Idx].c_str(),aImage,aHead,cLinientCategorizer,&ExtractionLog);
		std::cout << ExtractionLog.str() << std::endl;
	}
/**************
	size_t ScanCnt = Scans.size();

	// Read-in all the scans and parse them into bit-streams
	BitStreams.resize(ScanCnt);
	Runs.resize(ScanCnt);
	RunTimes.resize(ScanCnt);
	TrackStats.resize(ScanCnt);
	for(size_t Idx = 0; Idx < ScanCnt; ++Idx) {
		std::cout << "\tLoading scan data " << DecPrinter(Idx+1,0) << " out of " << DecPrinter(ScanCnt,0) << std::endl;
		if (!BitStreams[Idx].Read(Scans[Idx].c_str())) return;
		double BitTime;
		Runs[Idx] = MakeRuns(BitStreams[Idx],BitTime,&RunTimes[Idx]);
		TrackStats[Idx] = Extract(Runs[Idx], RunTimes[Idx], BitTime, aImage, aHead, cLinientCategorizer);
		if (TrackStats[Idx].Track != aTrack) {
			// If track doesn't match, clear the content so it doesn't interfere with future work
			TrackStats[Idx].SectorCnt = 0;
			TrackStats[Idx].Sectors.resize(0);
		}
	}
	// Select the one with the most number of recovered sectors as the 'driver'
	size_t DriverIdx = 0;
	for(size_t Idx=0;Idx<ScanCnt;++Idx) {
		if (TrackStats[Idx].SectorCnt > TrackStats[DriverIdx].SectorCnt) {
			DriverIdx = Idx;
		}
	}

	// Calculate stride for folded scans
	vector<size_t> StrideEstimates;
	for(size_t Sector=0;Sector<TrackStats[DriverIdx].Sectors.size();++Sector) {
		vector<SectorStats_s> &Stats = TrackStats[DriverIdx].Sectors[Sector];
		if (Stats.size() > 1) {
			size_t CurHeadTime = RunTimes[DriverIdx][Stats[0].StartHeader];
			size_t CurDataTime = RunTimes[DriverIdx][Stats[0].StartData];
			size_t PrevHeadTime;
			size_t PrevDataTime;
			for(size_t Idx=1;Idx<Stats.size();++Idx) {
				PrevHeadTime = CurHeadTime;
				PrevDataTime = CurDataTime;
				CurHeadTime = RunTimes[DriverIdx][Stats[Idx].StartHeader];
				CurDataTime = RunTimes[DriverIdx][Stats[Idx].StartData];
				size_t HeadStride = CurHeadTime - PrevHeadTime;
				size_t DataStride = CurDataTime - PrevDataTime;
				// TODO: can we figure out the cut-off lines dinamically?
				if (HeadStride > 2000000) HeadStride /= 2;
				if (HeadStride > 3000000) HeadStride /= 3;
				if (DataStride > 2000000) DataStride /= 2;
				if (DataStride > 3000000) DataStride /= 3;
				if (HeadStride > 1300000 && HeadStride < 1320000) StrideEstimates.push_back(HeadStride);
				if (DataStride > 1300000 && DataStride < 1320000) StrideEstimates.push_back(DataStride);
			}
		}
	}
	if (StrideEstimates.size() == 0) return; // We don't have enough data to do anything interesting
	sort(StrideEstimates.begin(),StrideEstimates.end());
	size_t Sum = 0;
	size_t Cnt = 0;
	for(size_t i=0;i<StrideEstimates.size();++i) {
		Sum += StrideEstimates[i];
		++Cnt;
	}
	size_t Stride;
	{
		double Avg = (double)Sum/(double)Cnt;
		// Calculate deviation
		double Dev = 0.0;
		for(size_t i=0;i<StrideEstimates.size();++i) {
			Dev += sqr(Avg - StrideEstimates[i]);
		}
		Dev = sqrt(Dev)/Cnt;
		// TODO: how to figure out the limit manually?
		if (Dev > 100.0) return;
		Stride = size_t(int(Avg + 0.5));
	}

	// Align the scans over one another
	vector<size_t> ScanOffsets;
	ScanOffsets.resize(ScanCnt);
	for(size_t Idx = 0; Idx < ScanCnt; ++Idx) {
		std::cout << "Aligning sector: " << Idx << std::endl;
		if (Idx == DriverIdx) continue;
		ScanOffsets[Idx] = SIZE_MAX;
		vector<size_t> Anchors;
		for(size_t Sector=0;Sector<TrackStats[Idx].Sectors.size();++Sector) {
			vector<SectorStats_s> &DriverStats = TrackStats[DriverIdx].Sectors[Sector];
			vector<SectorStats_s> &Stats = TrackStats[Idx].Sectors[Sector];
			if (Stats.size() > 1 && DriverStats.size() > 1) {
				// Both scans contain valid entries for the same sector. This creates an anchor
				// For now we'll only use the first occurance, but we could use all of them.
				int DriverHeadTime = RunTimes[DriverIdx][DriverStats[0].StartHeader];
				int DriverDataTime = RunTimes[DriverIdx][DriverStats[0].StartData];
				int HeadTime = RunTimes[Idx][Stats[0].StartHeader];
				int DataTime = RunTimes[Idx][Stats[0].StartData];
				int HeadAnchor = HeadTime - DriverHeadTime;
				while(HeadAnchor < 0) HeadAnchor += Stride;
				while(HeadAnchor > int(Stride)) HeadAnchor -= Stride;
				int DataAnchor = DataTime - DriverDataTime;
				while(DataAnchor < 0) DataAnchor += Stride;
				while(DataAnchor > int(Stride)) DataAnchor -= Stride;
				// Fine-tune the anchors, using cross-correlation
				double CrossCorr = 0.0;
				double MaxCrossCorr = 0.0;
				int MaxOffset = -100;
				for(int AnchorOffset=-5;AnchorOffset<=5;++AnchorOffset) {
					CrossCorr = CrossCorrelation(BitStreams[DriverIdx], BitStreams[Idx], HeadAnchor + AnchorOffset, BitStreams[Idx].size()*1/3, BitStreams[Idx].size()*2/3);
					std::cout << "\tCross correlation for offset: " << AnchorOffset << " is: " << CrossCorr << std::endl;
					if (MaxCrossCorr < CrossCorr) {
						std::cout << "\t\tNew maximum found" << std::endl;
						MaxCrossCorr = CrossCorr;
						MaxOffset = AnchorOffset;
					}
				}
				if (MaxOffset == -100) {
					std::cout << "\tCross correlation failed!!!" << std::endl;
				} else {
					HeadAnchor += MaxOffset;
					DataAnchor += MaxOffset;
				}
				Anchors.push_back(HeadAnchor);
				Anchors.push_back(DataAnchor);
			}
		}
		// If we got no anchors, remove this scan from the pool of possible scans
		if (Anchors.size() == 0) {
			TrackStats[Idx].SectorCnt = 0;
			TrackStats[Idx].Sectors.resize(0);
			continue;
		}
		// Let's calculate the average offset
		{
			double Sum = 0.0;
			for (size_t AnchorIdx = 0; AnchorIdx < Anchors.size(); ++AnchorIdx) {
				Sum += Anchors[AnchorIdx];
			}
			double Avg = (double)Sum/(double)Anchors.size();
			// Calculate deviation
			double Dev = 0.0;
			for (size_t AnchorIdx = 0; AnchorIdx < Anchors.size(); ++AnchorIdx) {
				Dev += sqr(Avg - Anchors[AnchorIdx]);
			}
			Dev = sqrt(Dev)/(double)Anchors.size();
			// If our deviation is too high, explcude this scan
			// TODO: how to figure out the limit manually?
			if (Dev > 10.0) {
				TrackStats[Idx].SectorCnt = 0;
				TrackStats[Idx].Sectors.resize(0);
				continue;
			}
			ScanOffsets[Idx] = size_t(int(Avg + 0.5));
		}
	}

	// OK, we have the possible other scans, combine all of them (including all the folds) into a single bit-stream using majority voting.
	// BTW: The ScanOffsets[Driver] is set to 0, so we don't even care who the driver was any more

	// First, compress ScanOffsets into a dense array
	vector<ScanOffsetInfo_s> ScanOffsetInfo;
	for(size_t ScanIdx = 0; ScanIdx < ScanCnt; ++ScanIdx) {
		if (ScanOffsets[ScanIdx] != SIZE_MAX) {
			ScanOffsetInfo.push_back(ScanOffsetInfo_s(ScanOffsets[ScanIdx],ScanIdx));
		}
	}

	BitStream_c FoldedStream;
	size_t FoldCnt = BitStreams[DriverIdx].size() / Stride - 1;
	CRAY_ASSERT(FoldCnt > 0);
	//size_t Offset = 0;
	size_t SampleCnt = ScanOffsetInfo.size() * FoldCnt;
	bool SkipLast = (SampleCnt & 1) == 0; // If we have an even number of samples, skip the last one to make majority voting possible
	if (SkipLast) --SampleCnt;
	size_t Cutoff = SampleCnt / 2;
	std::cout << "\tGenerating folded stream from " << DecPrinter(SampleCnt,0) << " samples per bit..." << std::endl;
	for(size_t Idx=0;Idx<Stride;++Idx) {
		size_t Cnt = 0;
		size_t LastVal;
		for(size_t OffsetIdx = 0; OffsetIdx < ScanOffsetInfo.size(); ++OffsetIdx) {
			size_t ScanIdx = ScanOffsetInfo[OffsetIdx].Idx;
			size_t Offset = ScanOffsetInfo[OffsetIdx].Offset;
			for(size_t Fold=0;Fold<FoldCnt;++Fold) {
				LastVal = BitStreams[ScanIdx][Offset + Idx + Stride * Fold];
				Cnt += LastVal;
			}
		}
		if (SkipLast) Cnt -= LastVal;
		FoldedStream.PushBit(Cnt > Cutoff ? Bit_One : Bit_Zero);
	}
	// Duplicate the stream to make sure we have at least one good reading of all sectors even of the one that wraps around at the fold
	std::cout << "\tDuplicating generated stream..." << std::endl;
	for(size_t Idx=0;Idx<Stride;++Idx) {
		FoldedStream.PushBit(FoldedStream.PopBit());
	}
	std::cout << "\tDetecting sectors..." << std::endl;

	FoldedStream.Rewind();

	double BitTime;
	vector<size_t> FoldedRunTimes;
	vector<uint8_t> FoldedRuns = MakeRuns(FoldedStream,BitTime,&FoldedRunTimes,true);

	TrackStats_s FoldedTrackStats;
	FoldedTrackStats = Extract(FoldedRuns, FoldedRunTimes, BitTime, aImage, aHead, cLinientCategorizer);

	std::cout << "\t* ";
	if (aImage.IsValidIdx(aHead, FoldedTrackStats.Track, 0)) {
		aImage.PrintSectorStats(aHead, FoldedTrackStats.Track, std::cout, false);
	} else {
		std::cout << "-";
	}
	std::cout << " extacted sectors: " << FoldedTrackStats.SectorCnt << std::endl;
*********/
}

/*******
	for(uint8_t ExcludeIdx = 0; ExcludeIdx < 4; ++ExcludeIdx) {
		for(int Offset = -2; Offset <= 2; ++Offset) {
			ExtractWithFold(BitStream, size_t(int(Avg + 0.5) + Offset), aFileName, aImage, aExpectedHead, aPulseCategorizer, ExcludeIdx, aRecoveryLog);
		}
	}

	return TrackStats.Track;
*****/

int ScannerPhase2(CommandLine_c aCmdLine) {
	DiskImage_c Image;
	Image.SetSize(cHeads,cTracks,cSectors);
	size_t MissingSectorCnt;
	std::cout << "Loading image..." << std::flush;
	if (!Image.LoadFromDisk(ImageFileName,MissingSectorCnt)) return 1;
	std::cout << " done with " << DecPrinter(MissingSectorCnt,0) << " sectors missing" << std::endl;
	std::cout << "Loading scan info..." << std::flush;
	if (!Image.LoadScanInfo(InfoFileName)) return 1;
	std::cout << " done" << std::endl;

	size_t MissingTrackCnt = 0;
	size_t FixedTrackCnt = 0;
	for(size_t Track=1;Track<=cTracks;++Track) {
		for(size_t Head=0;Head<cHeads;++Head) {
			bool TrackValid = true;
			size_t MissingSectorsInTrack = 0;
			for(size_t Sector=0;Sector<cSectors;++Sector) {
				Sector_c &CurSector = Image.GetSector(Head,Track,Sector);
				if (!CurSector.IsValid()) {
					TrackValid = false;
					++MissingSectorsInTrack;
				}
			}
			if (!TrackValid) {
				std::cout << "Attempting to fix track " << DecPrinter(Track,0) << ", head " << DecPrinter(Head,0) << "..." << std::endl;
				++MissingTrackCnt;
				FixTrack(Head,Track,Image);
				size_t MissingSectorsInTrackAfterFix = 0;
				for(size_t Sector=0;Sector<cSectors;++Sector) {
					Sector_c &CurSector = Image.GetSector(Head,Track,Sector);
					if (!CurSector.IsValid()) {
						TrackValid = false;
						++MissingSectorsInTrackAfterFix;
					}
				}
				size_t FixedCnt = MissingSectorsInTrack-MissingSectorsInTrackAfterFix;
				std::cout << "Fixed " << DecPrinter(FixedCnt,0) << " sectors out of " << DecPrinter(MissingSectorsInTrack,0) << std::endl;
				if (MissingSectorsInTrackAfterFix == 0) ++FixedTrackCnt;
				if (FixedCnt > 0) {
					std::cout << "Writing back image..." << std::flush;
					Image.WriteToDisk(Phase2ImageFileName,Phase2StatFileName,false);
					std::cout << " done" << std::endl;
				}
				MissingSectorCnt -= FixedCnt;
			}
		}
	}
	std::cout << "A total of " << DecPrinter(MissingSectorCnt,0) << " sectors remained unfixed." << std::endl;
	std::cout << "Fixed " << DecPrinter(FixedTrackCnt,0) << " tracks out of " << DecPrinter(MissingSectorCnt) << " missing." << std::endl;
	return 0;
}

int main(int argc, const char* argv[]) {
	CommandLine_c CmdLine(argc,argv);
	if (argc < 2) return PrintUsage(argv[0]);
	std::string Phase = CmdLine.GetNextParam();
	if (Phase == "1") {
		return ScannerPhase1(CmdLine);
	} else if (Phase == "2") {
		return ScannerPhase2(CmdLine);
	} else {
		return PrintUsage(argv[0]);
	}
}
