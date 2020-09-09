#define _CRT_SECURE_NO_WARNINGS

#include "utils.h"
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
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

class DsCtrlWord_c {
public:
	DsCtrlWord_c() : mData(0) {}
	explicit DsCtrlWord_c(uint64_t aData) : mData(SwapBytes(aData)) {}

	enum CtrlWordType_e {
		CtrlWordType_BCW = 000,
		CtrlWordType_EOR = 010,
		CtrlWordType_EOF = 016,
		CtrlWordType_EOD = 017,
		CtrlWordType_UNK = 020
	};

	void Clear() { mData = 0; }

	CtrlWordType_e GetType() const {
		uint8_t CtrlType = uint8_t(GetBitsReverse(mData, 0, 3));
		switch (CtrlType) {
			case CtrlWordType_BCW: return (CtrlWordType_e)CtrlType;
			case CtrlWordType_EOR: return (CtrlWordType_e)CtrlType;
			case CtrlWordType_EOF: return (CtrlWordType_e)CtrlType;
			case CtrlWordType_EOD: return (CtrlWordType_e)CtrlType;
			default: return CtrlWordType_UNK;
		}
	}
	void SetType(CtrlWordType_e aType) { mData = SetBitsReverse(mData, 0, 3, aType); }

	bool GetBadData() const { return GetBitsReverse(mData, 11, 11) != 0; }
	void SetBadData(bool aValue) { mData = SetBitsReverse(mData, 11, 11, aValue ? 1 : 0); }
	uint32_t GetForwardWordIdx() const { return uint32_t(GetBitsReverse(mData, 55, 63)); }
	void SetForwardWordIdx(uint32_t aValue) { mData = SetBitsReverse(mData, 55, 63, aValue); }

	// BCW fields
	uint32_t GetBlockNumber() const { return uint32_t(GetBitsReverse(mData, 31, 54)); }
	void SetBlockNumber(uint32_t aValue) { mData = SetBitsReverse(mData, 31, 54, aValue); }

	// EOR,EOF,EOD fields
	uint32_t GetUnusedBitCnt() const { return uint32_t(GetBitsReverse(mData, 4, 9)); }
	void SetUnusedBitCnt(uint32_t aValue) { mData = SetBitsReverse(mData, 4, 9, aValue); }
	bool GetTransparent() const { return GetBitsReverse(mData, 10, 10) != 0; }
	void SetTransparent(bool aValue) { mData = SetBitsReverse(mData, 10, 10, aValue ? 1 : 0); }
	bool GetSkipRemainderSector() const { return GetBitsReverse(mData, 12, 12) != 0; }
	void SetSkipRemainderSector(bool aValue) { mData = SetBitsReverse(mData, 12, 12, aValue ? 1 : 0); }
	uint32_t GetPreviousFileIdx() const { return uint32_t(GetBitsReverse(mData, 20, 39)); }
	void SetPreviousFileIdx(uint32_t aValue) { mData = SetBitsReverse(mData, 20, 39, aValue); }
	uint32_t GetPreviousRecordIdx() const { return uint32_t(GetBitsReverse(mData, 40, 54)); }
	void SetPreviousRecordIdx(uint32_t aValue) { mData = SetBitsReverse(mData, 40, 54, aValue); }

	std::string Print() const {
		std::stringstream Str;
		switch (GetType()) {
			case CtrlWordType_BCW: Str << "BCW: "; break;
			case CtrlWordType_EOR: Str << "EOR: "; break;
			case CtrlWordType_EOF: Str << "EOF: "; break;
			case CtrlWordType_EOD: Str << "EOD: "; break;
			default: Str << "UNK: "; break;
		}
		Str << "Forward word IDX: " << DecPrinter(GetForwardWordIdx(), 5) << " ";
		switch (GetType()) {
			case CtrlWordType_BCW:
				Str << "Block Number: " << GetBlockNumber() << " ";
				break;
			case CtrlWordType_EOR:
			case CtrlWordType_EOF:
			case CtrlWordType_EOD:
				Str << "Unused bit count: " << DecPrinter(GetUnusedBitCnt(), 2) << " ";
				Str << "Prev file idx: " << HexPrinter(GetPreviousFileIdx(), 7) << " ";
				Str << "Prev record idx: " << HexPrinter(GetPreviousRecordIdx(), 6) << " ";
				Str << (GetTransparent() ? "TRANS " : "");
				Str << (GetSkipRemainderSector() ? "SKIP " : "");
				break;
			default: Str << "UNK: "; break;
		}
		Str << (GetBadData() ? "BAD" : "");
		return Str.str();
	}
	uint64_t GetData() const { return SwapBytes(mData); }
	void Read(std::fstream &aFile) {
		uint64_t Data;
		aFile.read((char*)(&Data), sizeof(Data));
		if (aFile.gcount() != sizeof(Data)) throw Generic_x() << "Can't read from file";
		mData = SwapBytes(Data);
	}
	void Write(std::fstream &aFile) const {
		uint64_t Data = GetData();
		aFile.write((char*)(&Data), sizeof(Data));
		if (aFile.fail()) throw Generic_x() << "Can't write to file";
	}
	void Write(std::vector<uint8_t> &aBuffer, size_t aOffset) const {
		uint64_t Data = GetData();
		CRAY_ASSERT(aOffset <= aBuffer.size() - sizeof(Data));
		uint8_t *DataBegin = (uint8_t*)(&Data);
		uint8_t *DataEnd = DataBegin + sizeof(Data);
		std::copy(DataBegin, DataEnd, aBuffer.begin() + aOffset);
	}
protected:
	uint64_t mData;
};


class DataSet_c: public std::fstream {
public:
	static const size_t mBlockSize = 4096;
	DataSet_c() : std::fstream(), mBlock(mBlockSize) { Init(); }
	DataSet_c(const char *aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary) : std::fstream(aFileName, aMode), mFileName(aFileName), mMode(aMode), mBlock(mBlockSize) { Init(); }
	DataSet_c(const std::string &aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary) : std::fstream(aFileName, aMode), mFileName(aFileName), mMode(aMode), mBlock(mBlockSize) { Init(); }
	~DataSet_c() override { close(); }
	void open(const char* aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary) { close(); std::fstream::open(aFileName, aMode); mFileName = aFileName; mMode = aMode; Init(); }
	void open(const std::string& aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary) { close(); std::fstream::open(aFileName, aMode); mFileName = aFileName; mMode = aMode; Init(); }
	void close() { Flush(); std::fstream::close(); }
	void WriteRecord(std::vector<uint8_t> aRecord, bool aDoSwap);
	std::vector<uint8_t> ReadRecord();
	bool IsEof();
	bool IsEod();
	void WriteEof();
protected:
	void Init();
	void Flush();
	void WriteBlock(bool aFullBlock = true);
	std::vector<uint8_t> mBlock;
	size_t mBlockOfs;
	size_t mBlockCnt;
	size_t mCurrentFileStart;
	size_t mCurrentRecordStart;
	DsCtrlWord_c mLastCtrlWord; // We'll hang on to the last control word and only push it out on close. The reason is that we have to patch up the forward pointer

	std::string mFileName;
	ios_base::openmode mMode;
};

void DataSet_c::Init() {
	std::fill(mBlock.begin(), mBlock.end(), 0);
	mBlockCnt = 0;
	mBlockOfs = 0;
	mCurrentFileStart = 0;
	mCurrentRecordStart = 0;
	mLastCtrlWord.Clear();
	mLastCtrlWord.SetType(DsCtrlWord_c::CtrlWordType_BCW);
	mLastCtrlWord.SetBlockNumber(uint32_t(mBlockCnt));
	mLastCtrlWord.SetForwardWordIdx(0);
}

void DataSet_c::WriteBlock(bool aFullBlock) {
	write((char*)(&mBlock[0]), aFullBlock ? mBlock.size() : mBlockOfs);
	if (fail()) throw Generic_x() << "Can't write to file " << mFileName;
	std::fill(mBlock.begin(), mBlock.end(), 0);
	++mBlockCnt;
	mBlockOfs = 0;
}

void DataSet_c::WriteRecord(std::vector<uint8_t> aRecord, bool aDoSwap) {
	size_t RecordOfs = 0;
	mCurrentRecordStart = mBlockCnt;
	do {
		size_t RecordLeft = aRecord.size() - RecordOfs;
		size_t BlockLeft = mBlock.size() - mBlockOfs;
		size_t RoundedRecordLeft = ((RecordLeft + 7) / 8) * 8;

		if (RoundedRecordLeft <= BlockLeft-8) {
			// Rest of the record fits in the block:
			// - Patch up the last ctr word
			// - Put last ctr word into block
			// - Put the data into the block
			// - Prepare the EOR marker
			// - Check if the buffer is now full, if full
			//    - Write block to disk
			//    - Create new block
			//    - Put BCW in new record
			// - Prepare EOW in last ctrl word
			// - Break out of loop
			mLastCtrlWord.SetForwardWordIdx(uint32_t(RoundedRecordLeft / 8));
			mLastCtrlWord.Write(mBlock, mBlockOfs);
			mLastCtrlWord.Clear();
			mBlockOfs += 8;
			std::copy(aRecord.begin() + RecordOfs, aRecord.begin() + aRecord.size(), mBlock.begin() + mBlockOfs);
			mBlockOfs += RoundedRecordLeft;
			if (mBlockOfs == mBlock.size()) {
				WriteBlock();
				DsCtrlWord_c BCW;
				BCW.SetType(DsCtrlWord_c::CtrlWordType_BCW);
				BCW.SetBlockNumber(uint32_t(mBlockCnt));
				BCW.SetForwardWordIdx(0);
				BCW.Write(mBlock, mBlockOfs);
				mBlockOfs += 8;
			}
			mLastCtrlWord.SetType(DsCtrlWord_c::CtrlWordType_EOR);
			mLastCtrlWord.SetUnusedBitCnt(uint32_t((RoundedRecordLeft - RecordLeft) * 8));
			mLastCtrlWord.SetPreviousFileIdx(uint32_t(mBlockCnt - mCurrentFileStart));
			mLastCtrlWord.SetPreviousRecordIdx(uint32_t(mBlockCnt - mCurrentRecordStart));
			break;
		}
		else {
			// Rest of the record is still too large
			// - Patch up the last ctrl word
			// - Put last ctrl word into block
			// - Put the data into the block
			// - Write the block to disk
			// - Create new block
			// - Prepare BCW in last ctrl word
			// - Continue looping
			CRAY_ASSERT(BlockLeft % 8 == 0);
			mLastCtrlWord.SetForwardWordIdx(uint32_t(BlockLeft / 8 - 1));
			mLastCtrlWord.Write(mBlock, mBlockOfs);
			mLastCtrlWord.Clear();
			mBlockOfs += 8;
			BlockLeft -= 8;
			std::copy(aRecord.begin() + RecordOfs, aRecord.begin() + RecordOfs + BlockLeft, mBlock.begin() + mBlockOfs);
			WriteBlock();
			mLastCtrlWord.SetType(DsCtrlWord_c::CtrlWordType_BCW);
			mLastCtrlWord.SetBlockNumber(uint32_t(mBlockCnt));
			mLastCtrlWord.SetForwardWordIdx(0);
			RecordOfs += BlockLeft;
		}
	} while (true);
}

void DataSet_c::WriteEof() {
	CRAY_ASSERT((mBlock.size() - mBlockOfs) % 8 == 0);
	CRAY_ASSERT((mBlock.size() - mBlockOfs) >= 8);
	// - Patch up the last ctrl word
	// - Put last ctrl word into block
	// - Check if the buffer is now full, if full
	//    - Write block to disk
	//    - Create new block
	//    - Put BCW in new record
	// - Prepare EOF in last ctrl word
	mLastCtrlWord.SetForwardWordIdx(0);
	mLastCtrlWord.Write(mBlock, mBlockOfs);
	mLastCtrlWord.Clear();
	mBlockOfs += 8;
	if (mBlockOfs == mBlock.size()) {
		WriteBlock();
		DsCtrlWord_c BCW;
		BCW.SetType(DsCtrlWord_c::CtrlWordType_BCW);
		BCW.SetBlockNumber(uint32_t(mBlockCnt));
		BCW.SetForwardWordIdx(0);
		BCW.Write(mBlock, mBlockOfs);
		mBlockOfs += 8;
	}
	mLastCtrlWord.SetType(DsCtrlWord_c::CtrlWordType_EOF);
	mLastCtrlWord.SetUnusedBitCnt(0);
	mLastCtrlWord.SetPreviousFileIdx(uint32_t(mBlockCnt - mCurrentFileStart));
	mLastCtrlWord.SetPreviousRecordIdx(0);
	mCurrentFileStart = mBlockCnt;
}

void DataSet_c::Flush() {
	if ((mMode & std::ios_base::out) == 0) return;
	size_t BlockLeft = mBlock.size() - mBlockOfs;
	CRAY_ASSERT(BlockLeft % 8 == 0);
	CRAY_ASSERT(BlockLeft >= 8);
	// - Patch up the last ctrl word
	// - Put last ctrl word into block
	// - Check if the buffer is now full, if full
	//    - Write block to disk
	//    - Create new block
	//    - Put BCW in new record
	// - Prepare EOF in last ctrl word
	mLastCtrlWord.SetForwardWordIdx(0);
	mLastCtrlWord.Write(mBlock, mBlockOfs);
	mLastCtrlWord.Clear();
	mBlockOfs += 8;
	if (mBlockOfs == mBlock.size()) {
		WriteBlock();
		DsCtrlWord_c BCW;
		BCW.SetType(DsCtrlWord_c::CtrlWordType_BCW);
		BCW.SetBlockNumber(uint32_t(mBlockCnt));
		BCW.SetForwardWordIdx(0);
		BCW.Write(mBlock, mBlockOfs);
		mBlockOfs += 8;
	}
	mLastCtrlWord.SetType(DsCtrlWord_c::CtrlWordType_EOD);
	mLastCtrlWord.SetUnusedBitCnt(0);
	mLastCtrlWord.SetPreviousFileIdx(0);
	mLastCtrlWord.SetPreviousRecordIdx(0);
	mLastCtrlWord.Write(mBlock, mBlockOfs);
	mLastCtrlWord.Clear();
	mBlockOfs += 8;
	WriteBlock(false);
}

bool DataSet_c::IsEof() {
	DsCtrlWord_c CtrlWord;
	CtrlWord.Read(*this);
	bool RetVal = CtrlWord.GetType() == DsCtrlWord_c::CtrlWordType_EOF;
	if (CtrlWord.GetType() == DsCtrlWord_c::CtrlWordType_EOR) {
		CtrlWord.Read(*this);
		if (CtrlWord.GetType() == DsCtrlWord_c::CtrlWordType_EOF) {
			RetVal = true;
		} else {
			seekg(-8, std::ios::cur);
		}
	}
	// Position back to before the EOF record
	seekg(-8, std::ios::cur);
	if (fail()) throw Generic_x() << "Can't read dataset " << mFileName;
	return RetVal;
}

bool DataSet_c::IsEod() {
	DsCtrlWord_c CtrlWord;
	CtrlWord.Read(*this);
	bool RetVal = CtrlWord.GetType() == DsCtrlWord_c::CtrlWordType_EOD;
	if (CtrlWord.GetType() == DsCtrlWord_c::CtrlWordType_EOF) {
		CtrlWord.Read(*this);
		if (CtrlWord.GetType() == DsCtrlWord_c::CtrlWordType_EOD) {
			RetVal = true;
		}
		else {
			seekg(-8, std::ios::cur);
		}
	}
	// Position back to before the RCW record
	seekg(-8, std::ios::cur);
	if (fail()) throw Generic_x() << "Can't read dataset " << mFileName;
	return RetVal;
}

std::vector<uint8_t> DataSet_c::ReadRecord() {
	std::vector<uint8_t> Record;
	DsCtrlWord_c CtrlWord;
	CtrlWord.Read(*this);
	do {
		switch (CtrlWord.GetType()) {
			case DsCtrlWord_c::CtrlWordType_BCW: break;
			case DsCtrlWord_c::CtrlWordType_EOR: break;
			case DsCtrlWord_c::CtrlWordType_EOF: break;
			case DsCtrlWord_c::CtrlWordType_EOD: throw Generic_x("End of dataset encountered");
			default: throw Generic_x("Unknwon RCW encountered");
		}
		size_t OldRecordSize = Record.size();
		size_t BlockSize = size_t(CtrlWord.GetForwardWordIdx() * 8);
		Record.resize(OldRecordSize + BlockSize);
		if (BlockSize != 0) {
			read((char*)(&Record[OldRecordSize]), BlockSize);
			if (size_t(gcount()) != BlockSize) throw Generic_x() << "Can't read dataset " << mFileName;
		}
		CtrlWord.Read(*this);
	} while (CtrlWord.GetType() != DsCtrlWord_c::CtrlWordType_EOR);
	CRAY_ASSERT(Record.size() % 8 == 0);
	if (Record.size() >= 8) {
		Record.resize(Record.size() - CtrlWord.GetUnusedBitCnt() / 8);
	}
	// Position back to before the EOR record
	seekg(-8, std::ios::cur);
	if (fail()) throw Generic_x() << "Can't read dataset " << mFileName;
	return Record;
}

int PrintUsage(const char *aExecName, const char *aErrorStr = nullptr) {
	if (aErrorStr != nullptr) {
		std::cout << "Error interpreting the command line: " << aErrorStr << std::endl;
		std::cout << std::endl;
	}
	std::cout << "Usage: " << aExecName << " <command> <switchs>" << std::endl;
	std::cout << "There are two commands: create and extract." << std::endl;
	std::cout << "You can abbreviate create with 'c' and extract with 'e'." << std::endl;
	std::cout << std::endl;
	std::cout << "CREATE switches" << std::endl;
	std::cout << "   [-a[a] <input file>] ... -o <output dataset file name>" << std::endl;
	std::cout << "     -a[a] <input file> [record size]" << std::endl;
	std::cout << "        Add the specified binary file to the dataset" << std::endl;
	std::cout << "        If 'a' is added, the file is treated as an ASCII file" << std::endl;
	std::cout << "        If record size is specified, the input file is considered to consist of multiple " << std::endl;
	std::cout << "        fixed sized records.Record size is specified in bytes" << std::endl;
	std::cout << "        Record size can only be specified for binary files" << std::endl;
	std::cout << std::endl;
	std::cout << "EXTRACT switches" << std::endl;
	std::cout << "   -i <input dataset file name> [-xall[a] <file name patter>] [-x[a] <position> <output file name>] ..." << std::endl;
	std::cout << "     -x[a] <position> <output file name>" << std::endl;
	std::cout << "        Position specifies the n-th file in the dataset" << std::endl;
	std::cout << "        The specified file is extracted as binary or if 'a' is added, in ASCII" << std::endl;
	std::cout << "     -xall[a] <file name patter>" << std::endl;
	std::cout << "        Extract all files in the dataset" << std::endl;
	std::cout << "        In the file name pattern %d will be replaced with the file index" << std::endl;
	std::cout << "        The specified file is extracted as binary or if 'a' is added, in ASCII" << std::endl;
	return 1;
}

int Create(const char * aExecName, CommandLine_c &aCommandLine) {
	struct InputFile_s {
		InputFile_s(bool aIsBinary, const std::string &aFileName) : IsBinary(aIsBinary), FileName(aFileName), RecordSize(MaxValue(RecordSize)) {}
		InputFile_s(bool aIsBinary, const std::string &aFileName, size_t aRecordSize) : IsBinary(aIsBinary), FileName(aFileName), RecordSize(aRecordSize) {}
		bool IsBinary;
		std::string FileName;
		size_t RecordSize;
	};

	std::string OutputFileName;
	std::vector<InputFile_s> InputFiles;
	try {
		while (aCommandLine.HasMoreParams()) {
			std::string CurParam = aCommandLine.GetNextParam();
			if (CurParam.length() == 0) continue;
			if (CurParam == "-a") {
				std::string FileName = aCommandLine.GetNextParam();
				size_t RecordSize = MaxValue(RecordSize);
				try {
					RecordSize = std::stol(aCommandLine.GetNextParam());
				}
				catch (std::exception &) {
					aCommandLine.UnGetParam();
				}
				InputFiles.push_back(InputFile_s(true, FileName, RecordSize));
			}
			else if (CurParam == "-aa") {
				InputFiles.push_back(InputFile_s(false, aCommandLine.GetNextParam()));
			}
			else if (CurParam == "-o") {
				if (!OutputFileName.empty()) throw Generic_x("Output file name is already specified");
				OutputFileName = aCommandLine.GetNextParam();
			}
			else {
				throw Generic_x("Unkown command line parameter");
			}
		}
		if (OutputFileName.empty()) throw Generic_x("Output file name must be specified");
		if (InputFiles.size() == 0) throw Generic_x("No input files are specified");
		for (auto &File : InputFiles) {
			if (!boost::filesystem::exists(File.FileName)) throw Generic_x() << "Input file " << File.FileName << " doesn't exist";
		}
	}
	catch (std::exception &Ex) {
		return PrintUsage(aExecName, Ex.what());
	}

	// Walk the input file list and build the dataset
	try {
		DataSet_c OutFile(OutputFileName, std::ios::out | std::ios::binary);
		size_t FileIdx = 0;
		for (auto &File : InputFiles) {
			std::ifstream InFile(File.FileName, std::ios_base::in | File.IsBinary ? std::ios::binary : std::ios_base::openmode(0));
			if (File.IsBinary) {
				size_t RecordSize = std::min(uintmax_t(File.RecordSize), boost::filesystem::file_size(File.FileName));
				std::vector<uint8_t> Record(RecordSize);
				// We read the file one record at a time, put it in the dataset, while being careful about the partial record in the end
				while (!InFile.eof()) {
					InFile.read((char*)(&Record[0]), RecordSize);
					size_t ReadCount = size_t(InFile.gcount());
					if (ReadCount != RecordSize && !InFile.eof()) throw Generic_x() << "Can't read from input file " << File.FileName;
					if (InFile.eof()) break;
					Record.resize(ReadCount);
					OutFile.WriteRecord(Record, false); // TODO: add swapping feature
				}
				OutFile.WriteEof();
			}
			else {
				while (!InFile.eof()) {
					std::string Line;
					std::getline(InFile, Line);
					if (InFile.eof()) break;
					auto LastChar = Line.end() - 1;
					while (*LastChar == 0x0d || *LastChar == 0x0a) --LastChar;
					std::vector<uint8_t> LineBuf(Line.begin(), LastChar + 1);
					OutFile.WriteRecord(LineBuf, false);
				}
				OutFile.WriteEof();
			}
			++FileIdx;
		}
	}
	catch (std::exception &Ex) {
		std::cout << "Can't create output file with error: " << Ex.what() << std::endl;
		return 1;
	}
	return 0;
}

int Extract(const char * aExecName, CommandLine_c &aCommandLine) {
	struct OutputFile_s {
		OutputFile_s(size_t aPosition, bool aIsBinary, const std::string &aFileName, bool aDoAll) : Position(aPosition), IsBinary(aIsBinary), FileName(aFileName), DoAll(aDoAll) {}
		size_t Position;
		bool IsBinary;
		std::string FileName;
		bool DoAll;
	};

	std::string InputFileName;
	std::vector<OutputFile_s> OutputFiles;
	try {
		while (aCommandLine.HasMoreParams()) {
			std::string CurParam = aCommandLine.GetNextParam();
			if (CurParam.length() == 0) continue;
			if (CurParam == "-x" || CurParam == "-xa") {
				size_t Position;
				try {
					Position = std::stol(aCommandLine.GetNextParam());
				}
				catch (std::exception &) {
					throw Generic_x("Position must be a natural number");
				}
				std::string FileName = aCommandLine.GetNextParam();
				OutputFiles.push_back(OutputFile_s(Position, CurParam == "-x", FileName, false));
			}
			else if (CurParam == "-xall" || CurParam == "-xalla") {
				std::string FileName = aCommandLine.GetNextParam();
				OutputFiles.push_back(OutputFile_s(-1, CurParam == "-xall", FileName, true));
			}
			else if (CurParam == "-i") {
				if (!InputFileName.empty()) throw Generic_x("Input file name is already specified");
				InputFileName = aCommandLine.GetNextParam();
			}
			else {
				throw Generic_x("Unkown command line parameter");
			}
		}
		if (InputFileName.empty()) throw Generic_x("Input file name must be specified");
		if (OutputFiles.size() == 0) throw Generic_x("No output files are specified");
		if (!boost::filesystem::exists(InputFileName)) throw Generic_x() << "Input file " << InputFileName << " doesn't exist";
	}
	catch (std::exception &Ex) {
		return PrintUsage(aExecName, Ex.what());
	}

	// Walk the output file list and extract each of them
	try {
		DataSet_c InFile(InputFileName, std::ios::in | std::ios::binary);
		size_t FileIdx = 0;
		for (auto &File : OutputFiles) {
			if (File.DoAll) {
				InFile.seekg(0);
				FileIdx = 0;
				do {
					std::stringstream FileIdxStr;
					FileIdxStr << DecPrinter(FileIdx);
					std::string FileName = Replace(File.FileName, "%d", FileIdxStr.str());
					std::ofstream OutFile(FileName, std::ios_base::out | (File.IsBinary ? std::ios::binary : std::ios_base::openmode(0)));
					do {
						std::vector<uint8_t> Record = InFile.ReadRecord();
						OutFile.write((char*)(&Record[0]), Record.size());
						if (!File.IsBinary) OutFile << std::endl;
						if (OutFile.fail()) throw Generic_x() << "Can't write to file " << File.FileName;
					} while (!InFile.IsEof());
					++FileIdx;
				} while (!InFile.IsEod());
				InFile.seekg(0);
				FileIdx = 0;
			}
			else {
				if (File.Position < FileIdx) {
					InFile.seekg(0);
					FileIdx = 0;
				}
				std::ofstream OutFile(File.FileName, std::ios_base::out | (File.IsBinary ? std::ios::binary : std::ios_base::openmode(0)));
				// Seek to the right file in the dataset
				while (FileIdx != File.Position) {
					do {
						InFile.ReadRecord();
					} while (!InFile.IsEof());
					++FileIdx;
				}
				do {
					std::vector<uint8_t> Record = InFile.ReadRecord();
					OutFile.write((char*)(&Record[0]), Record.size());
					if (!File.IsBinary) OutFile << std::endl;
					if (OutFile.fail()) throw Generic_x() << "Can't write to file " << File.FileName;
				} while (!InFile.IsEof());
				++FileIdx;
			}
		}
	}
	catch (std::exception &Ex) {
		std::cout << "Can't extract from dataset with error: " << Ex.what() << std::endl;
		return 1;
	}
	return 0;
}

int main(int argc, const char* argv[]) {
	CommandLine_c CommandLine(argc, argv);

	if (!CommandLine.HasMoreParams()) return PrintUsage(argv[0]);

	std::string Command;
	try {
		while (CommandLine.HasMoreParams()) {
			Command = CommandLine.GetNextParam();
			if (Command.length() != 0) break;
		}
		if (Command == "create" || Command == "c") return Create(argv[0], CommandLine);
		else if (Command == "extract" || Command == "e") return Extract(argv[0], CommandLine);
		else throw Generic_x() << "Invalid command line argument: " << Command;
	}
	catch (std::exception &Ex) {
		return PrintUsage(argv[0], Ex.what());
	}
	CRAY_ASSERT(false); // We should never get here, but are compilers smart enough to figure that out?
	return 0;
}

