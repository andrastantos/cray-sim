#ifndef __TAP_FILE_H__
#define __TAP_FILE_H__

#include "utils.h"
#include <fstream>
#include <time.h>
#include <boost/filesystem.hpp>

// Tape Dump format:
// - Each file on the tape is set up of a set of records
// - Each record contains a header and an optional footer, both of which are 4 bytes long.
// - The header and the footer contains the length of the record
// - The footer doesn't exist if length is set to 0, which marks the end of file

class TapFile_c : public std::fstream {
public:
	// Seek operations stop at the inter-record gap. Since there is an inter-record gap between the last record of a file and the EOF marker, this means that the EOF marker counts as an (empty) record
	// This also means that when seeking backwards to the previous file, after the last seek (that found the EOF marker) a seek-forward by 1 has to be issued to position the tape to the beginning of the file
	// TODO: A file might not terminate at feof. Make sure all is working under that condition
	enum class State_e {
		BeginningOfTape,
		EndOfTape,
		EndOfFile,
		EndOfRecord,
		Normal
	};

	TapFile_c() : std::fstream() { Reset(); mLastCmdDirection = Direction::None; }
	TapFile_c(const char *aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary) : std::fstream(aFileName, aMode), mFileName(aFileName), mMode(aMode) { Reset(); seekp(0, std::ios_base::beg); seekg(0, std::ios_base::beg); mLastCmdDirection = Direction::None; }
	TapFile_c(const std::string &aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary) : std::fstream(aFileName, aMode), mFileName(aFileName), mMode(aMode) { Reset(); seekp(0, std::ios_base::beg); seekg(0, std::ios_base::beg); mLastCmdDirection = Direction::None; }
	virtual ~TapFile_c() {}
	void open(const char* aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary) { std::fstream::open(aFileName, aMode); mFileName = aFileName; mMode = aMode; Reset(); seekp(0, std::ios_base::beg); seekg(0, std::ios_base::beg); mLastCmdDirection = Direction::None; }
	void open(const std::string& aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary) { std::fstream::open(aFileName, aMode); mFileName = aFileName; mMode = aMode; Reset(); seekp(0, std::ios_base::beg); seekg(0, std::ios_base::beg); mLastCmdDirection = Direction::None; }
	void close() { std::fstream::close(); Reset(); mLastCmdDirection = Direction::None; }

	State_e GetState();
	void SeekToNextRecord();
	void SeekToPrevRecord();
	std::vector<uint8_t> Read(uint32_t aMaxSize = UINT32_MAX);
/*	template <typename tElem> std::vector<tElem> Read(uint32_t aMaxSize = UINT32_MAX) {
		std::vector<char> CharRetVal = Read(aMaxSize);
		// Resize to integer returned elements
		CharRetVal.resize(((CharRetVal.size() + sizeof(tElem) - 1) / sizeof(tElem)) * sizeof(tElem));
		std::vector<tElem> RetVal(CharRetVal.size() / sizeof(tElem));
		for (size_t i = 0; i < RetVal.size(); ++RetVal) {
			RetVal[i] = *(&CharRetVal[i*sizeof(tElem)]);
		}
	}*/
	// Write methods truncate file at the beginning of the current record unless the record to be written is exactly the same size as the current one and the
	// read is positioned at the beginning of the record (or at the end of the previous one). Same goes for WriteEOF: it truncates unless it would write to an
	// already existing EOF (in which case it would be a no-op).
	void WriteRecord(const std::vector<uint8_t> &aData);
	void WriteEndOfFile();
	void SeekToBeginningOfTape();
//	uint32_t GetCurrentRecordSize();
	const std::string &GetFileName() const { return mFileName; }
	void ReOpen();
protected:
	void WriteRecordInternal(const std::vector<uint8_t> &aData);
	boost::uintmax_t GetFileSize() const;
	void Reset();
	void Truncate();

	enum class Direction {
		Forward,
		Backward,
		None
	} mLastCmdDirection;
	uint32_t mCurRecordLeft;
	std::streamoff mCurRecordStart;
	bool mCurRecordValid;
	bool mCurRecordEof;
	std::string mFileName;
	mutable boost::optional<boost::uintmax_t> mFileSize;
	ios_base::openmode mMode;
};

#endif // __TAP_FILE_H__
