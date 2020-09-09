#ifndef __POKED_TAP_FILE_H__
#define __POKED_TAP_FILE_H__

#include "tap_file.h"
#include "config_file.h"
#include <boost/container/flat_map.hpp>
#include "cray_logger.h"

class PokedTapFile_c : public TapFile_c {
public:
	PokedTapFile_c(CLogger_c &aLogger, const Configuration_c &aConfig);
	PokedTapFile_c(CLogger_c &aLogger, const Configuration_c &aConfig, const char *aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	PokedTapFile_c(CLogger_c &aLogger, const Configuration_c &aConfig, const std::string &aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	void open(const char* aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	void open(const std::string& aFileName, std::ios_base::openmode aMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	void close();

	void SeekToNextRecord();
	void SeekToPrevRecord();
	std::vector<uint8_t> Read(uint32_t aMaxSize = UINT32_MAX);
//	void WriteRecord(const std::vector<uint8_t> &aData);
	void WriteEndOfFile();
	void SeekToBeginningOfTape();
protected:
	// We store a list of pokes for each file name that we know about. For each TAP file, there can be multiple contained files and for each of those we can have a poke map
	typedef boost::container::flat_map<size_t, uint8_t> PokeMap_t;
	typedef std::vector<PokeMap_t> PokeVector_t;
	boost::container::flat_map<std::string, PokeVector_t> mPokes;
	std::vector<size_t> mFileSizes;

	size_t mCurFileIdx;
	size_t mCurFileOfs;
	PokeVector_t *mCurPokeMap;

	void ReadPokes(const Configuration_c &aConfig);
	void UpdateForNewFile();

	CLogger_c &mLogger;
};

#endif // __POKED_TAP_FILE_H__
