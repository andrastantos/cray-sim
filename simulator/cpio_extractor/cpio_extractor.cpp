#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"
#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

/*
From cpio.info
==============

   Old Binary Format
     The old binary cpio format stores numbers as 2-byte and 4-byte binary
     values.  Each entry begins with a header in the following format:

           struct header_old_cpio {
                   unsigned short   c_magic;
                   unsigned short   c_dev;
                   unsigned short   c_ino;
                   unsigned short   c_mode;
                   unsigned short   c_uid;
                   unsigned short   c_gid;
                   unsigned short   c_nlink;
                   unsigned short   c_rdev;
                   unsigned short   c_mtime[2];
                   unsigned short   c_namesize;
                   unsigned short   c_filesize[2];
           };

     The unsigned short fields here are 16-bit integer values; the unsigned
     int fields are 32-bit integer values.  The fields are as follows

     magic   The integer value octal 070707.  This value can be used to deter?
             mine whether this archive is written with little-endian or big-
             endian integers.

     dev, ino
             The device and inode numbers from the disk.  These are used by
             programs that read cpio archives to determine when two entries
             refer to the same file.  Programs that synthesize cpio archives
             should be careful to set these to distinct values for each entry.

     mode    The mode specifies both the regular permissions and the file
             type.  It consists of several bit fields as follows:
             0170000  This masks the file type bits.
             0140000  File type value for sockets.
             0120000  File type value for symbolic links.  For symbolic links,
                      the link body is stored as file data.
             0100000  File type value for regular files.
             0060000  File type value for block special devices.
             0040000  File type value for directories.
             0020000  File type value for character special devices.
             0010000  File type value for named pipes or FIFOs.
             0004000  SUID bit.
             0002000  SGID bit.
             0001000  Sticky bit.  On some systems, this modifies the behavior
                      of executables and/or directories.
             0000777  The lower 9 bits specify read/write/execute permissions
                      for world, group, and user following standard POSIX con?
                      ventions.

     uid, gid
             The numeric user id and group id of the owner.

     nlink   The number of links to this file.  Directories always have a
             value of at least two here.  Note that hardlinked files include
             file data with every copy in the archive.

     rdev    For block special and character special entries, this field con?
             tains the associated device number.  For all other entry types,
             it should be set to zero by writers and ignored by readers.

     mtime   Modification time of the file, indicated as the number of seconds
             since the start of the epoch, 00:00:00 UTC January 1, 1970.  The
             four-byte integer is stored with the most-significant 16 bits
             first followed by the least-significant 16 bits.  Each of the two
             16 bit values are stored in machine-native byte order.

     namesize
             The number of bytes in the pathname that follows the header.
             This count includes the trailing NUL byte.

     filesize
             The size of the file.  Note that this archive format is limited
             to four gigabyte file sizes.  See mtime above for a description
             of the storage of four-byte integers.

     The pathname immediately follows the fixed header.  If the namesize is
     odd, an additional NUL byte is added after the pathname.  The file data
     is then appended, padded with NUL bytes to an even length.

     Hardlinked files are not given special treatment; the full file contents
     are included with each copy of the file.

   Portable ASCII Format
     Version 2 of the Single UNIX Specification (“SUSv2”) standardized an
     ASCII variant that is portable across all platforms.  It is commonly
     known as the “old character” format or as the “odc” format.  It stores
     the same numeric fields as the old binary format, but represents them as
     6-character or 11-character octal values.

           struct cpio_odc_header {
                   char    c_magic[6];
                   char    c_dev[6];
                   char    c_ino[6];
                   char    c_mode[6];
                   char    c_uid[6];
                   char    c_gid[6];
                   char    c_nlink[6];
                   char    c_rdev[6];
                   char    c_mtime[11];
                   char    c_namesize[6];
                   char    c_filesize[11];
           };

     The fields are identical to those in the old binary format.  The name and
     file body follow the fixed header.  Unlike the old binary format, there
     is no additional padding after the pathname or file contents.  If the
     files being archived are themselves entirely ASCII, then the resulting
     archive will be entirely ASCII, except for the NUL byte that terminates
     the name field.



*/
/*
070707
000021
043062
100644
000000
000000
000001
000000
04244065231
000006
00000000000
*/

uint16_t GetUint16(std::istream &aStrm) {
	char Octal[7];
	Octal[6] = 0;
	aStrm.read(Octal, sizeof(Octal) - 1);
	return static_cast<uint16_t>(std::stoul(Octal, 0, 8));
}

uint32_t GetUint32(std::istream &aStrm) {
	char Octal[12];
	Octal[11] = 0;
	aStrm.read(Octal, sizeof(Octal) - 1);
	return std::stoul(Octal, 0, 8);
}

struct CpioHeader_s {
	uint16_t   c_magic;
	uint16_t   c_dev;
	uint16_t   c_ino;
	uint16_t   c_mode;
	uint16_t   c_uid;
	uint16_t   c_gid;
	uint16_t   c_nlink;
	uint16_t   c_rdev;
	uint32_t   c_mtime;
	uint16_t   c_namesize;
	uint32_t   c_filesize;
};

CpioHeader_s ReadFileHeader(std::istream &aStrm) {
	CpioHeader_s RetVal;
	RetVal.c_magic = GetUint16(aStrm);
	RetVal.c_dev = GetUint16(aStrm);
	RetVal.c_ino = GetUint16(aStrm);
	RetVal.c_mode = GetUint16(aStrm);
	RetVal.c_uid = GetUint16(aStrm);
	RetVal.c_gid = GetUint16(aStrm);
	RetVal.c_nlink = GetUint16(aStrm);
	RetVal.c_rdev = GetUint16(aStrm);
	RetVal.c_mtime = GetUint32(aStrm);
	RetVal.c_namesize = GetUint16(aStrm);
	RetVal.c_filesize = GetUint32(aStrm);
	if (RetVal.c_magic != 070707) throw Generic_x() << "Invalid header magic. Expected 070707, got: " << OctPrinter(RetVal.c_magic);
	return RetVal;
}

const size_t NameBlockAlignSize = 1;
const size_t FileBlockAlignSize = 1;

void CreateDirectoryTree(const char *aPath) {
	boost::filesystem::path Path(aPath);
	boost::filesystem::path Dir = Path.parent_path();
	if (Dir.empty()) return;
	if (!boost::filesystem::exists(Dir)) {
		boost::filesystem::create_directories(Dir);
	}
}

size_t PatchupFileSize(const CpioHeader_s &aHdr, size_t aFileIdx) {
	size_t RawSize = aHdr.c_filesize;
/*	switch (RawSize) {
	case 254952: RawSize += 496; break;
	case 220944: RawSize += 432; break;
	}*/
	return ((RawSize + FileBlockAlignSize - 1) / FileBlockAlignSize) * FileBlockAlignSize;
}

size_t SearchMagicAhead(std::istream &aStrm) {
	std::streamoff CurPos = aStrm.tellg();
	char Octal[7];
	Octal[6] = 0;
	aStrm.read(Octal, sizeof(Octal) - 1);
	while (!aStrm.eof()) {
		if (strncmp(Octal, "070707", 6) == 0) {
			std::streamoff FoundPos = aStrm.tellg() - std::streamoff(6);
			if (FoundPos != CurPos) {
				std::cout << "Skipped forward " << FoundPos - CurPos << " bytes" << std::endl;
			}
			aStrm.seekg(CurPos);
			return size_t(FoundPos - CurPos);
		}
		memcpy(Octal, Octal + 1, 5);
		aStrm.read(Octal + 5, 1);
	}
	aStrm.seekg(CurPos);
	return 0;
}

bool ReadFile(std::istream &aStrm, size_t aFileIdx) {
	CpioHeader_s Hdr = ReadFileHeader(aStrm);
	size_t FileNameSize = ((Hdr.c_namesize + NameBlockAlignSize - 1) / NameBlockAlignSize) * NameBlockAlignSize;
	size_t FileSize = PatchupFileSize(Hdr, aFileIdx);
	std::vector<char> FileName(FileNameSize + 1, 0);
	aStrm.read(&FileName[0], FileNameSize);
	if (strcmp(&FileName[0], "TRAILER!!!") == 0) return false;
	std::cout << (Hdr.c_filesize != 0 ? "Read file: " : "Skipping empty file: ") << &FileName[0] << std::endl;
	if (Hdr.c_filesize != 0) {
		std::vector<char> File(FileSize, 0);
		aStrm.read(&File[0], FileSize);
		CreateDirectoryTree(&FileName[0]);
		std::ofstream Output;
		Output.open(&FileName[0], std::ios_base::binary | std::ios_base::out);
		Output.write(&File[0], Hdr.c_filesize);
		// Find the next header and output the remains
		size_t RestSize = SearchMagicAhead(aStrm);
		if (RestSize != 0) {
			std::vector<char> RestFile(RestSize, 0);
			aStrm.read(&RestFile[0], RestSize);
			Output.write(&RestFile[0], RestSize);
		}
		Output.close();
	}
	return true;
}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " <image file>" << std::endl;
		return 1;
	}

	errno = 0;

	std::ifstream Input;
	Input.open(argv[1], std::ios_base::binary | std::ios_base::in);
	// Read file header
	uint32_t Hdr;
	Input.read(reinterpret_cast<char *>(&Hdr), 4);

	size_t FileIdx = 0;
	try {
		while (ReadFile(Input, FileIdx)) { ++FileIdx; };
	}
	catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		std::cout << "At file offset: " << HexPrinter(Input.tellg()) << std::endl;
		return 1;
	}
	return 0;
}

