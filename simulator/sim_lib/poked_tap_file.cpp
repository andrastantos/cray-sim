#include "poked_tap_file.h"

PokedTapFile_c::PokedTapFile_c(CLogger_c &aLogger, const Configuration_c &aConfig) : TapFile_c(), mCurFileIdx(0), mCurFileOfs(0), mCurPokeMap(nullptr), mLogger(aLogger) {
	ReadPokes(aConfig);
}

PokedTapFile_c::PokedTapFile_c(CLogger_c &aLogger, const Configuration_c &aConfig, const char *aFileName, std::ios_base::openmode aMode) : TapFile_c(aFileName, aMode), mCurFileIdx(0), mCurFileOfs(0), mCurPokeMap(nullptr), mLogger(aLogger) {
	ReadPokes(aConfig);
	UpdateForNewFile();
}

PokedTapFile_c::PokedTapFile_c(CLogger_c &aLogger, const Configuration_c &aConfig, const std::string &aFileName, std::ios_base::openmode aMode) : TapFile_c(aFileName, aMode), mCurFileIdx(0), mCurFileOfs(0), mCurPokeMap(nullptr), mLogger(aLogger) {
	ReadPokes(aConfig);
	UpdateForNewFile();
}

void PokedTapFile_c::open(const char* aFileName, std::ios_base::openmode aMode) {
	TapFile_c::open(aFileName, aMode);
	UpdateForNewFile();
}

void PokedTapFile_c::open(const std::string& aFileName, std::ios_base::openmode aMode) {
	TapFile_c::open(aFileName, aMode);
	UpdateForNewFile();
}

void PokedTapFile_c::close() {
	TapFile_c::close();
	mCurFileIdx = 0;
	mCurFileOfs = 0;
}

void PokedTapFile_c::SeekToNextRecord() {
	size_t Base = size_t(tellg());
	// If we're inside a valid record, we're going to skip to after the record. Adjust the beginning as if this was a full record of smaller size
	if (mCurRecordValid && !mCurRecordEof) Base -= 4;
	TapFile_c::SeekToNextRecord();
	size_t After = size_t(tellg());
	if (GetState() == State_e::EndOfFile || GetState() == State_e::EndOfTape){
		++mCurFileIdx;
		mCurFileOfs = 0;
	} else {
		CRAY_ASSERT(After >= Base + 8);
		mCurFileOfs += After - Base - 8;
	}
}

void PokedTapFile_c::SeekToPrevRecord() {
	size_t Base = size_t(tellg());
	// If we're inside a valid record, we're going to skip to before the record. Adjust the beginning as if this was a full record of smaller size
	if (mCurRecordValid) Base += 4;
	TapFile_c::SeekToPrevRecord();
	size_t After = size_t(tellg());
	if (GetState() == State_e::EndOfFile) {
		CRAY_ASSERT(mCurFileIdx > 0);
		--mCurFileIdx;
		mCurFileOfs = mFileSizes[mCurFileIdx];
	} else if (GetState() == State_e::BeginningOfTape) {
		mCurFileIdx = 0;
		mCurFileOfs = 0;
	} else {
		CRAY_ASSERT(Base >= After + 8);
		CRAY_ASSERT(Base - After - 8 >= mCurFileOfs);
		mCurFileOfs -= Base - After - 8;
	}
}

std::vector<uint8_t> PokedTapFile_c::Read(uint32_t aMaxSize) {
	size_t Base = size_t(tellg());
	// If we're outside a record, read will get 4 extra bytes
	if (!mCurRecordValid || mCurRecordEof) {
		Base += 4;
	}
	auto RetVal = TapFile_c::Read(aMaxSize);
	size_t After = size_t(tellg());
	// If we are not in a valid record, read just got extra 4 bytes
	if (!mCurRecordValid) {
		After -= 4;
	}
	if (mCurPokeMap != nullptr && mCurPokeMap->size() > mCurFileIdx) {
		const auto &Pokes = (*mCurPokeMap)[mCurFileIdx];
		if (Pokes.size() != 0) {
			for (size_t i = 0; i < RetVal.size(); ++i) {
				size_t Offs = mCurFileOfs + i;
				const auto Poke = Pokes.find(Offs);
				if (Poke != Pokes.end()) {
					mLogger << setloglevel(LogLevel_IoTrace) << "Overwriting value " << HexPrinter(RetVal[i]) << " with " << HexPrinter(Poke->second) << " at offset " << HexPrinter(Offs) << std::endl;
					//std::cout << "Overwriting value " << HexPrinter(RetVal[i]) << " with " << HexPrinter(Poke->second) << " at offset " << HexPrinter(Offs) << std::endl;
					RetVal[i] = Poke->second;
				}
			}
		}
	}
	if (GetState() == State_e::EndOfFile || GetState() == State_e::EndOfTape) {
		++mCurFileIdx;
		mCurFileOfs = 0;
	}
	else {
		CRAY_ASSERT(After >= Base);
		mCurFileOfs += After - Base;
	}
	return RetVal;
}

//void PokedTapFile_c::WriteRecord(const std::vector<uint8_t> &aData) {
//	TapFile_c::WriteRecord(aData);
//}

void PokedTapFile_c::WriteEndOfFile() {
	TapFile_c::WriteEndOfFile();
	++mCurFileIdx;
}

void PokedTapFile_c::SeekToBeginningOfTape() {
	TapFile_c::SeekToBeginningOfTape();
	mCurFileIdx = 0;
}

void PokedTapFile_c::ReadPokes(const Configuration_c &aConfig) {
	for (const auto &TapFile : aConfig.get_child_safe("Pokes")) {
		std::string FileName = TapFile.first;
		PokeVector_t PokeVector;
		for (const auto &File : TapFile.second) {
			size_t FileIdx = FromString<size_t>(File.first); // Internally files are 0-based
			if (FileIdx >= PokeVector.size()) PokeVector.resize(FileIdx + 1);
			for (const auto &Poke: File.second) {
				uint8_t Size = Poke.second.get<uint8_t>("Size",1);
				bool BigEndien = Poke.second.get<bool>("BigEndien",true);
				size_t Addr = FromString<size_t>(Poke.first);
				Addr *= Size; // Make sure addr is measured in bytes
				switch (Size) {
				case 1:
				case 2:
				case 4:
				case 8: {
					uint64_t Value = Poke.second.get<uint64_t>("Value");
					if (BigEndien) {
						for (size_t ByteIdx = 0; ByteIdx < Size; ++ByteIdx) {
							PokeVector[FileIdx][Addr + ByteIdx] = ((uint8_t*)(&Value))[Size - ByteIdx - 1];
						}
					}
					else {
						for (size_t ByteIdx = 0; ByteIdx < Size; ++ByteIdx) {
							PokeVector[FileIdx][Addr + ByteIdx] = ((uint8_t*)(&Value))[ByteIdx];
						}
					}
				} break;
				default: throw InvalidParameter_x("Poke size must be 1,2,4 or 8");
				}
			}
		}
		mPokes[FileName] = PokeVector;
	}
}

void PokedTapFile_c::UpdateForNewFile() {
	mCurFileIdx = 0;
	mCurFileOfs = 0;

	mCurPokeMap = nullptr;
	auto It = mPokes.find(GetFileName());
	if (It != mPokes.end()) mCurPokeMap = &(It->second);
	// Scan the file and record file size for each file encountered
	mFileSizes.clear();
	if (!good()) return;

	SeekToBeginningOfTape();
	while (GetState() != TapFile_c::State_e::EndOfTape) {
		size_t FileSize = 0;
		size_t RecCnt = 1;
		do {
			std::vector<uint8_t> Buffer;
			Buffer = TapFile_c::Read();
			if (GetState() == TapFile_c::State_e::EndOfTape) break;
			FileSize += Buffer.size();
			if (GetState() == TapFile_c::State_e::EndOfRecord) ++RecCnt;
		} while (GetState() != TapFile_c::State_e::EndOfFile && GetState() != TapFile_c::State_e::EndOfTape);
		if (GetState() != TapFile_c::State_e::EndOfTape) {
			mFileSizes.push_back(FileSize);
		}
	}
	SeekToBeginningOfTape();
}
