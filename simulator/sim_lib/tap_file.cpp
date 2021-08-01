#include "tap_file.h"

// Tape behavior as far as I can tell:
// There are two physical markers on the tape: one for beginning of tape, one for end of tape.
// These markers are detected by photo-detectors and any operation that passes them aborts and sets the appropriate flags
// Files are broken into records. Each record is separated by *empty* tape. Seek and read operations stop at empty tape.
// A special end-of-file marker can be written to the tape. These are special 'records' in the sense that you need an
// extra read operation to pass them. Seek operations abort at EOF markers.
//
// The TAP file is orgnaized into blocks, where both the beginning and the end of the block is marked by a 32-bit length
// indicator. The EOF marker is a simple 32-bit 0 value.
//
// The consequence of this is that read and seek operations should normally stop in-between blocks.
//
// A special convention is to mark the end of tape by a double EOF marker. This is not handled by the HW (and thus TapFile_c)
// but recognized in SW.

// mCurRecordValid mCurRecordEof
// false           false          - we're sitting between two records
// false           true           - we're sitting before an EOF marker
// true            true           - we're sitting after an EOF marker <-- in this case mCurRecordStart and mCurRecordLeft are set up to be valid
// true            false          - we're sitting inside a data record

boost::uintmax_t TapFile_c::GetFileSize() const {
	if (!mFileSize.is_initialized()) mFileSize = boost::filesystem::file_size(mFileName);
	return mFileSize.get();
}

TapFile_c::State_e TapFile_c::GetState() {
//	if (mCurRecordValid && mCurRecordStart == GetFileSize()) return State_e::EndOfTape;
//	if (fail() && !boost::filesystem::exists(mFileName)) return State_e::EndOfTape;
	if (!boost::filesystem::exists(mFileName)) {
		if (mLastCmdDirection != Direction::Forward) return State_e::BeginningOfTape;
		return State_e::EndOfTape;
	}
	if (!is_open()) return State_e::BeginningOfTape;
	if (eof()) return State_e::EndOfTape;
	if (tellg() == std::streampos(0)) return State_e::BeginningOfTape;
	if (mCurRecordEof) return State_e::EndOfFile;
	if (!mCurRecordValid) return State_e::EndOfRecord;
	if (mCurRecordLeft == 0 && mLastCmdDirection == Direction::Backward) return State_e::EndOfRecord;
	return State_e::Normal;
}

// TODO: figure out what the expected behavior at BOT is. Right now it skips over the first record, but maybe it should just open the first record
void TapFile_c::SeekToNextRecord() {
	mLastCmdDirection = Direction::Forward;

	if (!boost::filesystem::exists(mFileName)) return;

	// If we have a valid record, skip to the next record
	// If we don't have a valid record (beginning of tape condition or EOF marker) skip to the next record

	// Valid record case: skip to the end of it
	if (mCurRecordValid && !mCurRecordEof) {
		seekg(mCurRecordLeft + 4, ios_base::cur);
		if (fail()) throw Generic_x("Can't read skip to end of record");
		mCurRecordValid = false;
		return;
	}
	// We've sitting between records (after an EOF marker or after a data record), so skip through the next record and set the EOF state according to the skipped record
	uint32_t RecordSize;
	if (eof()) return;
	read((char*)(&RecordSize), 4);
	if (eof()) return;
	if (fail()) throw Generic_x("Can't read beginning of record");
	if (RecordSize > 0) {
		seekg(RecordSize, ios_base::cur);
		if (fail()) throw Generic_x("Can't seek to end of record");
		uint32_t EndRecordSize;
		read((char*)(&EndRecordSize), 4);
		if (fail() && !eof()) throw Generic_x("Can't read end of record");
		if (EndRecordSize != RecordSize) throw Generic_x("Record size mismatch at end");
	}
	mCurRecordEof = RecordSize == 0;
	if (mCurRecordEof) {
		mCurRecordValid = true;
		mCurRecordLeft = 0;
		mCurRecordStart = tellg();
	}
	else {
		mCurRecordValid = false;
	}
}

void TapFile_c::SeekToPrevRecord() {
	mLastCmdDirection = Direction::Backward;

	if (!boost::filesystem::exists(mFileName)) return;

	if (eof()) clear();
	// If we have a valid record, skip to in-between the current and the previous record
	// If we are at the beginning of tape don't do anything
	// If we don't have a valid record, skip back a full record, stop in-between records.
	// If we're on an EOF record, skip to the end of the previous record

	// BOT case
	if ((tellg() == std::streamoff(0))) {
		Reset();
		mCurRecordEof = false;
		return;
	}
	// Valid record case
	if (mCurRecordValid) {
		// if we're in middle of a record, we should seek to the end of the previous one (if any)
		// We do it in two steps: we seek between the two records, then we get to the end of the previous one. <-- actually we shouldn't do this. Just stay between the two records
		// There's one exception though: if we've been after the EOF marker we should just jump in front of it
		seekg(mCurRecordStart - 4, ios_base::beg);
		if (fail()) throw Generic_x("Can't skip to end of record");
		Reset();
		if (mCurRecordEof) {
#ifndef NO_ASSERTS
			uint32_t RecordSize;
			read((char*)(&RecordSize), 4);
			if (fail()) throw Generic_x("Can't read EOF marker");
			CRAY_ASSERT(RecordSize == 0);
			seekg(-4, ios_base::cur);
#endif
			return;
		}
		return;
	}
	// We're either sitting before an EOF marker or between two data records...
	mCurRecordEof = false;
	// Since we don't have a valid record, open the previous record if any
	if (!(tellg() == std::streamoff(0))) {
		seekg(-4, ios_base::cur); // Seek back to the end-of-record marker of the previous record
		//std::streamoff CurPos = tellg();
		if (fail()) throw Generic_x("Can't seek to end of record");
		uint32_t EndRecordSize;
		read((char*)(&EndRecordSize), 4);
		if (fail()) throw Generic_x("Can't read end of record");
		if (EndRecordSize != 0) {
			// Previous record is not an EOF marker, so we have to skip the whole record
			seekg(-std::streamoff(EndRecordSize + 8), ios_base::cur); // Seek back to the beginning of the record
			if (fail()) throw Generic_x("Can't seek to beginning of record");
			// Make sure the record size at the beginning matches the end
			uint32_t RecordSize;
			read((char*)(&RecordSize), 4);
			if (fail()) throw Generic_x("Can't read beginning of record");
			if (EndRecordSize != RecordSize) throw Generic_x("Record size mismatch");
			// We seek back in-between the records
			Reset();
			seekg(-4, ios_base::cur);
		}
		else {
			// Set up stream state to the end of the record
			mCurRecordEof = true;
			mCurRecordValid = false;
			seekg(-4, ios_base::cur); // Seek back before the EOF marker
			if (fail()) throw Generic_x("Can't seek to beginning of EOF marker");
		}
	}
}

// - if more then whole record is read in one move, we should position to the point AFTER the EOR marker, and set current record to nothing
// - if exactly the whole record is read, we should position to right BEFORE the EOR marker and set the current record to valid, but 0 bytes left
// - Consequently we might need one EXTRA read before we can determine the EOR condition
std::vector<uint8_t> TapFile_c::Read(uint32_t aMaxSize) {
	mLastCmdDirection = Direction::Forward;
	if (!boost::filesystem::exists(mFileName)) {
		return std::vector<uint8_t>();
	}
	CRAY_ASSERT(aMaxSize > 0);
	// Reading when no current record is open or we're after an EOF marker: open the next record
	if (!mCurRecordValid || mCurRecordEof) {
		// We don't have a valid record, so let's see what's in the file...
		uint32_t RecordSize;
		if (eof()) return std::vector<uint8_t>();
		read((char*)(&RecordSize), 4);
		if (eof()) return std::vector<uint8_t>();
		if (fail()) throw Generic_x("Can't read beginning of record");
		// Verify record size
		if (RecordSize > 0) {
			std::streampos CurPos = tellg();
			seekg(RecordSize, ios_base::cur);
			if (fail()) throw Generic_x("Can't seek to end of record");
			uint32_t EndRecordSize;
			read((char*)(&EndRecordSize), 4);
			if (fail() && !eof()) throw Generic_x("Can't read end of record");
			if (EndRecordSize != RecordSize) throw Generic_x("Record size mismatch at end");
			seekg(CurPos, ios_base::beg);
			if (fail()) throw Generic_x("Can't seek back to beginning of record");
		}
		mCurRecordLeft = RecordSize;
		mCurRecordStart = tellg();
		mCurRecordValid = true;
		mCurRecordEof = RecordSize == 0;
		// We've just opened an EOF record: return 0 bytes, and leave the record open
		if (mCurRecordEof) {
			return std::vector<uint8_t>(); // This is an EOF condition
		}
	}
	CRAY_ASSERT(mCurRecordValid);
	CRAY_ASSERT(!mCurRecordEof);

	size_t ReadSize = std::min(aMaxSize, mCurRecordLeft);
	std::vector<uint8_t> RetVal(ReadSize);
	if (ReadSize != 0) read((char*)(&(RetVal[0])), ReadSize);
	if (fail()) throw Generic_x("Can't read from file");
	// Read past through the just opened record: close the record
	if (aMaxSize >= mCurRecordLeft) {
		// Read the end record marker
		uint32_t EndRecordSize;
		read((char*)(&EndRecordSize), 4);
		if (fail()) throw Generic_x("Can't read from end of record");
		// Set current record to invalid (so status is EOR)
		mCurRecordValid = false;
	}
	mCurRecordLeft -= uint32_t(ReadSize);
	return RetVal;
}

// mCurRecordValid mCurRecordEof
// false           false          - we're sitting between two records
// false           true           - we're sitting before an EOF marker
// true            true           - we're sitting after an EOF marker <-- in this case mCurRecordStart and mCurRecordLeft are set up to be valid
// true            false          - we're sitting inside a data record

void TapFile_c::WriteRecordInternal(const std::vector<uint8_t> &aData) {
	mFileSize.reset();
	if (!boost::filesystem::exists(mFileName)) {
		close();
		ios_base::openmode Mode = mMode;
		open(mFileName, ios_base::out | ios_base::binary);
		write(nullptr, 0);
		close();
		open(mFileName, Mode);
	}
	uint32_t RecordSize = uint32_t(aData.size());
	if (!eof()) {
		if (mCurRecordValid && !mCurRecordEof) {
			// Writing when in the middle of a record: truncate at the beginning of the record
			seekg(mCurRecordStart - 4, ios_base::beg);
			if (fail()) throw Generic_x("Can't seek to beginning of record");
			Truncate();
		}
		else {
			// Writing when no current record is open or we're after an EOF marker: truncate unless record length is the same
			uint32_t DiskRecordSize;
			read((char*)&DiskRecordSize, 4);
			if (!eof()) {
				if (fail()) throw Generic_x("Can't read beginning of record");
				if (DiskRecordSize != 0) {
					seekg(DiskRecordSize, ios_base::cur);
					if (fail()) throw Generic_x("Can't seek to end of record");
					uint32_t EndDiskRecordSize;
					read((char*)&EndDiskRecordSize, 4);
					if (fail()) throw Generic_x("Can't read end of record");
					if (DiskRecordSize != EndDiskRecordSize) throw Generic_x("Record size mismatch at end");
					seekg(-std::streamoff(DiskRecordSize + 4), ios_base::cur);
					if (fail()) throw Generic_x("Can't seek to beginning of record");
				}
				seekg(-4, ios_base::cur);
				if (fail()) throw Generic_x("Can't seek to beginning of record");
				// We truncate at the begining of the current record, unless record sizes match
				if (DiskRecordSize != RecordSize) {
					Truncate();
				}
			}
		}
	}
	if (eof()) clear();
	seekp(tellg(), ios_base::beg);
	if (eof()) clear();
	write((char*)(&RecordSize), 4);
	mCurRecordStart = tellp();
	mCurRecordLeft = 0;
	if (fail()) throw Generic_x("Can't write beginning of record to file");
	if (RecordSize != 0) {
		write((char*)(&(aData[0])), RecordSize);
		if (fail()) throw Generic_x("Can't write record data to file");
		write((char*)(&RecordSize), 4);
		if (fail()) throw Generic_x("Can't write end of record to file");
	}
	std::streamoff Pos = tellp();
	// Re-create the proper internal state: the read is positioned at the end of the currenty written record with or without EOF depending on what we just did
	seekg(Pos, ios_base::beg);
	seekp(Pos, ios_base::beg);
	if (fail()) throw Generic_x("Can't reposition read after write");
	mCurRecordEof = RecordSize == 0;
	mCurRecordValid = mCurRecordEof;
}

void TapFile_c::WriteRecord(const std::vector<uint8_t> &aData) {
	mLastCmdDirection = Direction::Forward;
	CRAY_ASSERT(aData.size() != 0);
	WriteRecordInternal(aData);
}

void TapFile_c::WriteEndOfFile() {
	mLastCmdDirection = Direction::Forward;
	WriteRecordInternal(std::vector<uint8_t>());
}

void TapFile_c::SeekToBeginningOfTape() {
	mLastCmdDirection = Direction::Backward;
	if (eof()) clear();
	Reset();
	mCurRecordEof = false;
	seekg(0, ios_base::beg);
}

void TapFile_c::Reset() {
	mCurRecordLeft = 0;
	mCurRecordStart = 0;
	mCurRecordValid = false;
}

void TapFile_c::Truncate() {
	Direction OldDir = mLastCmdDirection;
	std::streamoff CurPos = tellg();
	close();
	mFileSize.reset();
	if (CurPos != 0) {
		boost::filesystem::resize_file(mFileName, CurPos);
		open(mFileName, mMode);
	}
	else {
		ios_base::openmode Mode = mMode;
		open(mFileName, ios_base::out | ios_base::binary);
		write(nullptr, 0);
		close();
		open(mFileName, Mode);
	}
	seekg(CurPos, ios_base::beg);
	mLastCmdDirection = OldDir;
}

void TapFile_c::ReOpen() {
	if (is_open()) return;
	open(mFileName, mMode);
}

/*********
uint32_t TapFile_c::GetCurrentRecordSize() {
	if (!mCurRecordValid) return 0;
	if (mCurRecordEof) return 0;
	return uint32_t(tellg() - mCurRecordStart) + mCurRecordLeft;
}
***********/
