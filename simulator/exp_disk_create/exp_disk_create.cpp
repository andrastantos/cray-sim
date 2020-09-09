#include <fstream>
#include <vector>
#include <stdint.h>
#include <string>
#include "../sim_lib/utils.h"
#include <boost/filesystem.hpp>

const size_t Heads = 5;
const size_t Sectors = 35;
const size_t Tracks = 823;
const size_t SectorSize = 512;
const size_t LogicalSectorSize = SectorSize * 8;
const size_t LogicalSectors = Sectors * SectorSize / LogicalSectorSize;

const size_t SectorCount = Heads*Sectors*Tracks;

/***********************************************************************
=========================================================================
Disk label structure: stored at physical sector (Sector, Head, Cylinder: 24,0,0)
=========================================================================
offset    0: initialized flag (set to 0xffff)
offset    1: ASCII volume name (8 chars)
offset    5: ASCII last init date (8 chars)
offset    9: ASCII last init time (8 chars)
offset   13: ASCII last update date (8 chars)
offset   17: ASCII last update time (8 chars)
offset   21: free list begin index
offset   22: free list end end
offset   23: directory begin index
offset   24: directory end index
------------- PARTITIONS --------------
offset   25: partition first block
offset   26: number of blocks
.... lots of reserved
offset 1017: disk flow count
offset 1018: disk flow table (one bit per block, flow count # of words)


=========================================================================
Each free list sector contains the following:
Stored at physical sector (Sector, Head, Cylinder: 0,1,0);
=========================================================================
offset    0: next free list index
offset    1: prev free list index
offest  2-6: reserved
offset    7: number of entries in this map

rest is a table of the following structure:

offset    0: first sector in sector block
offset    1: number of sector in sector block

Each block is a contiguous set of sectors that are all empty


=========================================================================
A file descriptor has the following layout
Stored one after another starting at physical sector (Sector, Head, Cylinder: 8, 1, 0)
=========================================================================
offset     0, bit 15:   entry status (1: in use)
offset     0, bit 14:   modification flag (1: modified)
offset     0, bit 13:   flawed (1: there are bad blocks in file)
offset     1:           Directory name (16 chars)
offset     8:           File name (16 chars)
offset    17:           Creation date (8 chars)
offset    21:           Creation time (8 chars)
offset    25:           Size (high 16 bits)
offset    26:           Size (low 16 bits)
offset    27:           Number of blocks (I think it's only used if the file gets too fragmented and directory blocks are needed)
-- two reserved words --
offset    30:           Logical number of blocks
offset    31:           Block list size

rest is a table of the following structure:

offset    0: first sector in sector block
offset    1: number of sector in sector block

Each block is a contiguous set of sectors all belonging to the file

Total table size is 60 words


=========================================================================
Directory block
=========================================================================
offset    0: next free list index
offset    1: prev free list index
offest  2-6: reserved
offset    7: number of entries in this map

rest is a table of the following structure:

offset    0: first sector in sector block
offset    1: number of sector in sector block

Each block is a contiguous set of sectors all belonging to the same file

************************************************************************/



struct FileDesc_s {
	std::string FileName;
	std::string DirectoryName;
	std::string CreationDate;
	std::string CreationTime;
	std::string SourceFileName;
	uint32_t StartSector;
	bool IsFixedPlacement;
	bool SwapBytes;
};

struct DirEntry_s {
	DirEntry_s() {
		Invalidate();
	}
	DirEntry_s(const FileDesc_s &aFileDesc, size_t aFileSize, size_t aFirstSector) {
		Invalidate();
		Flags = 0x8000;
		memcpy(DirectoryName,aFileDesc.DirectoryName.c_str(),aFileDesc.DirectoryName.length());
		memcpy(FileName,aFileDesc.FileName.c_str(),aFileDesc.FileName.length());
		memcpy(CreationDate,aFileDesc.CreationDate.c_str(),aFileDesc.CreationDate.length());
		memcpy(CreationTime,aFileDesc.CreationTime.c_str(),aFileDesc.CreationTime.length());
		FileSize_High = uint16_t((aFileSize) >> 16);
		FileSize_Low = (aFileSize) & 0xffff;
		NumSectors = uint16_t((aFileSize + SectorSize -1 ) / SectorSize);
		NumSectorChains = 1;
		SectorChains[0].FirstSector = uint16_t(aFirstSector);
		SectorChains[0].SectorCnt = NumSectors;

		for(size_t i=0;i<sizeof(DirectoryName)/sizeof(uint16_t);++i) {
			((uint16_t*)(DirectoryName))[i] = SwapBytes(((uint16_t*)(DirectoryName))[i]);
		}
		for(size_t i=0;i<sizeof(FileName)/sizeof(uint16_t);++i) {
			((uint16_t*)(FileName))[i] = SwapBytes(((uint16_t*)(FileName))[i]);
		}
		for(size_t i=0;i<sizeof(CreationDate)/sizeof(uint16_t);++i) {
			((uint16_t*)(CreationDate))[i] = SwapBytes(((uint16_t*)(CreationDate))[i]);
		}
		for(size_t i=0;i<sizeof(CreationTime)/sizeof(uint16_t);++i) {
			((uint16_t*)(CreationTime))[i] = SwapBytes(((uint16_t*)(CreationTime))[i]);
		}
	}
	void Invalidate() {
		Flags = 0x0000;
		for(size_t i=0;i<sizeof(DirectoryName)/sizeof(DirectoryName[0]);++i) {
			DirectoryName[i] = 0;
		}
		for(size_t i=0;i<sizeof(FileName)/sizeof(FileName[0]);++i) {
			FileName[i] = 0;
		}
		for(size_t i=0;i<sizeof(CreationDate)/sizeof(CreationDate[0]);++i) {
			CreationDate[i] = 0;
		}
		for(size_t i=0;i<sizeof(CreationTime)/sizeof(CreationTime[0]);++i) {
			CreationTime[i] = 0;
		}
		NumOfBlocks = 0;
		Entry28 = 0;
		Entry29 = 0;
		FileSize_High = 0;
		FileSize_Low = 0;
		NumSectors = 0;
		NumSectorChains = 0;
		for(size_t i=0;i<sizeof(SectorChains)/sizeof(SectorChains[0]);++i) {
			SectorChains[i].FirstSector = 0;
			SectorChains[i].SectorCnt = 0;
		}
	}

	uint16_t Flags;
	char DirectoryName[16];
	char FileName[16];
	char CreationDate[8];
	char CreationTime[8];
	uint16_t FileSize_High; // in QWORDs
	uint16_t FileSize_Low; // in QWORDs
	uint16_t NumOfBlocks;
	uint16_t Entry28;
	uint16_t Entry29;
	uint16_t NumSectors;
	uint16_t NumSectorChains;
	struct SectorChain_s {
		uint16_t FirstSector;
		uint16_t SectorCnt;
	} SectorChains[14];
};

struct DiskImage_s {
	std::vector<uint16_t> Image;
	size_t mCurrentSector;
	size_t mFileCnt;

	DiskImage_s(): Image(SectorCount * SectorSize / sizeof(uint16_t)), mCurrentSector(6), mFileCnt(0) {
	}

	template<typename tRetType> tRetType *GetCurrentSector() { 
		// Even though we have 35 physical sectors, the expander disk format only uses 32 per cylinder. So we'll have to convert to SHC format using that...
		return GetLogicalSectorAddr<tRetType>(mCurrentSector);
	}
	template<typename tRetType> tRetType *GetLogicalSectorAddr(size_t aLogicalSector) {
		// We have 'LogicalSectors' number of logical sectors per cylinder.
		size_t PhysicalSector = (aLogicalSector % LogicalSectors) * (LogicalSectorSize / SectorSize);
		size_t Track = aLogicalSector / LogicalSectors;
		size_t Head = Track % Heads;
		size_t Cylinder = Track / Heads;
		return GetPhysicalSectorAddr<tRetType>(PhysicalSector, Head, Cylinder);
	}
	template<typename tRetType> tRetType *GetPhysicalSectorAddr(size_t aSector, size_t aHead, size_t aCylinder) { return (tRetType *)(&Image[(aCylinder * Sectors * Heads + aHead * Sectors + aSector) * SectorSize / sizeof(uint16_t)]); }

	void CopyString(const std::string &aStr, uint16_t *aMem, size_t aMaxSizeInWords) {
		for (size_t i = 0; i < aMaxSizeInWords; ++i) {
			aMem[i] = 0;
		}
		memcpy(aMem, aStr.c_str(), aStr.length());
	}

	void InitVolume(const std::string &aVolumeLabel, const std::string &aInitDate, const std::string &aInitTime) {
		uint16_t *Sector = GetPhysicalSectorAddr<uint16_t>(24,0,0);
		Sector[0] = 0xffff;
		CopyString(aVolumeLabel, Sector + 1, 8);
		CopyString(aInitDate, Sector + 5, 8);
		CopyString(aInitTime, Sector + 9, 8);
		CopyString(aInitDate, Sector + 13, 8);
		CopyString(aInitTime, Sector + 17, 8);
	}
	void ReadFile(FileDesc_s &aFileDesc) {
		uint64_t FileSize = boost::filesystem::file_size(aFileDesc.SourceFileName);
		if (!aFileDesc.IsFixedPlacement) {
			if (FileSize % 8 != 0) {
				std::cout << "File size is not divisible by 8: padding with 0-s at the end" << std::endl;
			}
			if (FileSize > Image.size() - mCurrentSector * LogicalSectorSize) throw Generic_x("Not enough room in disk image for file");
			FileSize = (FileSize + 7) / 8;
			if (aFileDesc.FileName.length() > 16) throw Generic_x("File name is too long");
			if (aFileDesc.DirectoryName.length() > 16) throw Generic_x("Directory name is too long");
			if (aFileDesc.CreationDate.length() > 8) throw Generic_x("Creation date is too long");
			if (aFileDesc.CreationTime.length() > 8) throw Generic_x("Creation time is too long");
			if (mFileCnt >= 34) throw Generic_x("Too many files in the image");
		}
		// Read the file and put it in the image from the current sector
		std::ifstream InStrm(aFileDesc.SourceFileName, std::ios::in | std::ios::binary);
		size_t CurrentSector = aFileDesc.IsFixedPlacement ? aFileDesc.StartSector : mCurrentSector;
		size_t StartSector = CurrentSector;
		while (InStrm.good()) {
			InStrm.read(GetLogicalSectorAddr<char>(CurrentSector),LogicalSectorSize);
			if (aFileDesc.SwapBytes) {
				uint64_t *Sector = GetLogicalSectorAddr<uint64_t>(CurrentSector);
				for(size_t i=0;i<LogicalSectorSize/sizeof(uint64_t);++i) {
					Sector[i] = SwapBytesInWords(Sector[i]);
				}
			}
			++CurrentSector;
		}
		if (!aFileDesc.IsFixedPlacement) {
			mCurrentSector = CurrentSector;
			// Create the directory entry
			DirEntry_s DirEntry(aFileDesc, (size_t)FileSize, StartSector);
			memcpy(GetPhysicalSectorAddr<char>(8, 1, 0) + 8 * sizeof(uint16_t) + sizeof(DirEntry_s)*mFileCnt, &DirEntry, sizeof(DirEntry));
			++mFileCnt;
		}
	}
	void Finalize() {
		// Create the free sector list
		uint16_t *Sector = GetPhysicalSectorAddr<uint16_t>(0,1,0);
		Sector[7] = 1;
		Sector[8] = uint16_t(mCurrentSector);
		Sector[9] = uint16_t(SectorCount-mCurrentSector);
		for(size_t i=0;i<Image.size();++i) {
			Image[i] = SwapBytes(Image[i]);
		}
	}
	void Write(std::ofstream &aFile) const {
		aFile.write((char*)(&Image[0]), Image.size() * sizeof(uint16_t));
	}
};

int PrintUsage(const char *aExecName, const char *ErrorStr) {
	if (ErrorStr != nullptr) std::cout << "Error: " << ErrorStr << std::endl;
	std::cout << "Usage: " << aExecName << " -o <Image file name> [-l <volume label>] [-D] [-d <directory name] [-s] [-n] [-f <file name>] [-c <creation date> <creation time>] <source file name>" << std::endl;
	std::cout << std::endl;
	std::cout << "\t" << "Specify a a maximum of 34 source files to be stored in the disk image." << std::endl;
	std::cout << "\t" << "Options:" << std::endl;
	std::cout << "\t" << "-o: specifies the output image name" << std::endl;
	std::cout << "\t" << "-l: specifies the volume label of the image" << std::endl;
	std::cout << "\t" << "-d: specifies the image directory name for the next files" << std::endl;
	std::cout << "\t" << "    overwrite with the next -d or -D" << std::endl;
	std::cout << "\t" << "-D: sets the image directory to be the root for the next files" << std::endl;
	std::cout << "\t" << "    overwrite with the next -d or -D" << std::endl;
	std::cout << "\t" << "-f: specifies the image file name for the next file" << std::endl;
	std::cout << "\t" << "    defaults to be the same as the source file, if not specified" << std::endl;
	std::cout << "\t" << "-fixed: specifies the file to be placed at a fixed sector" << std::endl;
	std::cout << "\t" << "-c: specifies the image creation date and time the next file" << std::endl;
	std::cout << "\t" << "Byte-ordering is a 'sticky' setting that deafults to on. It can be changed by the following options:" << std::endl;
	std::cout << "\t" << "-s: swap byte-order on 64-bit boundaries" << std::endl;
	std::cout << "\t" << "-n: no-swap byte-order on 64-bit boundaries" << std::endl;
	std::cout << "\t" << "    format: MM/DD/YY HH:MM:SS" << std::endl;
	return 1;
}

int main(int argc, const char **argv) {
	CRAY_ASSERT(sizeof(DirEntry_s) == 60*2);
	CommandLine_c CommandLine(argc,argv);
	try {
		std::vector<FileDesc_s> Files;
		FileDesc_s CurrentFileDesc;
		CurrentFileDesc.SwapBytes = true;
		std::string ImgFileName;
		std::string VolumeLabel;
		while(CommandLine.HasMoreParams()) {
			std::string CurParam = CommandLine.GetNextParam();
			if (CurParam == "-o") {
				if (!ImgFileName.empty()) throw Generic_x("Image file name is already specified");
				ImgFileName = CommandLine.GetNextParam();
			} else if (CurParam == "-l") {
				if (!VolumeLabel.empty()) throw Generic_x("Volume label is already specified");
				VolumeLabel = CommandLine.GetNextParam();
			} else if (CurParam == "-d") {
				CurrentFileDesc.DirectoryName = CommandLine.GetNextParam();
			} else if (CurParam == "-D") {
				CurrentFileDesc.DirectoryName.clear();
			} else if (CurParam == "-f") {
				CurrentFileDesc.FileName = CommandLine.GetNextParam();
			} else if (CurParam == "-fixed") {
				CurrentFileDesc.IsFixedPlacement = true;
				CurrentFileDesc.StartSector = std::stoul(CommandLine.GetNextParam());
			} else if (CurParam == "-s") {
				CurrentFileDesc.SwapBytes = true;
			} else if (CurParam == "-n") {
				CurrentFileDesc.SwapBytes = false;
			} else if (CurParam == "-c") {
				CurrentFileDesc.CreationDate = CommandLine.GetNextParam();
				CurrentFileDesc.CreationTime = CommandLine.GetNextParam();
				if (CurrentFileDesc.CreationDate.length() != 8) throw Generic_x("Creation date length must be 8");
				if (CurrentFileDesc.CreationTime.length() != 8) throw Generic_x("Creation time length must be 8");
			} else {
				CurrentFileDesc.SourceFileName = CurParam;
				if (CurrentFileDesc.FileName.empty()) {
					CurrentFileDesc.FileName = CurrentFileDesc.SourceFileName;
				}
				if (CurrentFileDesc.CreationDate.empty()) {
					CurrentFileDesc.CreationDate = "01/01/89";
				}
				if (CurrentFileDesc.CreationTime.empty()) {
					CurrentFileDesc.CreationTime = "01:01:01";
				}
				if (VolumeLabel.empty()) {
					VolumeLabel = "EXP_DISK";
				}
				if (VolumeLabel.length() > 16) throw Generic_x() << "Volume label " << VolumeLabel << " is too long";
				if (CurrentFileDesc.FileName.length() > 16) throw Generic_x() << "File name " << CurrentFileDesc.FileName << " is too long";
				if (CurrentFileDesc.DirectoryName.length() > 16) throw Generic_x() << "Directory name " << CurrentFileDesc.DirectoryName <<  " is too long";
				if (!boost::filesystem::exists(CurrentFileDesc.SourceFileName)) throw Generic_x() << "Source file " << CurrentFileDesc.SourceFileName << " does't exist";
				std::cout << "Adding file: '" << CurrentFileDesc.SourceFileName << "'";
				if (CurrentFileDesc.IsFixedPlacement) {
					std::cout << " for direct placement at sector: " << CurrentFileDesc.StartSector;
				}
				else {
					std::cout << " as " << CurrentFileDesc.DirectoryName << "/" << CurrentFileDesc.FileName;
				}
				std::cout << std::endl;
				Files.push_back(CurrentFileDesc);
				CurrentFileDesc.FileName.clear();
				CurrentFileDesc.SourceFileName.clear();
				CurrentFileDesc.CreationDate.clear();
				CurrentFileDesc.CreationTime.clear();
				CurrentFileDesc.IsFixedPlacement = false;
			}
		}
		if (ImgFileName.empty()) throw Generic_x("Image file name was not specified");
		std::cout << "Generating image..." << std::endl;
		DiskImage_s DiskImage;
		DiskImage.InitVolume(VolumeLabel, "01/01/89", "01:01:01");
		for(size_t i=0;i<Files.size();++i) DiskImage.ReadFile(Files[i]);
		DiskImage.Finalize();
		std::ofstream Strm(ImgFileName.c_str(), std::ios::out | std::ios::binary);
		DiskImage.Write(Strm);
		Strm.close();
		std::cout << "Done" << std::endl;
	}
	catch(std::exception &Ex) {
		return PrintUsage(argv[0], Ex.what());
	}
	return 0;
}
