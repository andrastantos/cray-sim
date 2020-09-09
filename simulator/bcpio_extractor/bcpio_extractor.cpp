#define _CRT_SECURE_NO_WARNINGS
// cray_ios_disasm.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include <boost/filesystem.hpp>

/*
UNICOS .gzc format:
- It's gzip compressed
Each file has the following header:

00000000000071C7 <-- seems constant
0000000000002243 <-- seems constant within file, but different between files (2243 for non-unicos media, 2279 for unicos root and usr)
0000000000000465 <-- variable
00000000000081A0 <-- mode and type
0000000000000002 <-- seems constant
0000000000000002 <-- seems constant
0000000000000001 <-- seems constant
0000000000000000 <-- seems constant
0000000000000000 <-- seems constant
0000000000000000 <-- seems constant
0000000000000000 <-- seems constant
0000000000000000 <-- seems constant
0000000000000000 <-- seems constant
0000000000000000 <-- seems constant
0000000000000000 <-- seems constant
0000000000000000 <-- seems constant
0000000033C5DF41 <-- creation (?) time
000000000000000A <-- length of file name (file name is 0-terminated and follows immediately after the header)
0000000000000C8B <-- file payload length (payload starts immediately after file name)

[0x00000000]	0x00000000000071c7	unsigned __int64
[0x00000001]	0x0000000000002243	unsigned __int64
[0x00000002]	0x0000000000007ec0	unsigned __int64
[0x00000003]	0x00000000000045e8	unsigned __int64
[0x00000004]	0x0000000000000002	unsigned __int64
[0x00000005]	0x0000000000000002	unsigned __int64
[0x00000006]	0x00000000000000d3	unsigned __int64
[0x00000007]	0x0000000000000000	unsigned __int64
[0x00000008]	0x0000000000000000	unsigned __int64
[0x00000009]	0x0000000000000000	unsigned __int64
[0x0000000a]	0x0000000000000000	unsigned __int64
[0x0000000b]	0x0000000000000000	unsigned __int64
[0x0000000c]	0x0000000000000000	unsigned __int64
[0x0000000d]	0x0000000000000000	unsigned __int64
[0x0000000e]	0x0000000000000000	unsigned __int64
[0x0000000f]	0x0000000000000000	unsigned __int64
[0x00000010]	0x00000000355e00c0	unsigned __int64

This is a binary cpio header!!!
Field Width 	Field Name     Meaning
========================================================
6 	            magic          magic number "070707"
6 	            dev            device where file resides
6 	            ino            I-number of file
6 	            mode           file mode
6 	            uid            owner user ID
6 	            gid            owner group ID
6 	            nlink          number of links to file
6 	            rdev           device major/minor for special file
11 	            mtime          modify time of file
6 	            namesize       length of file name
11 	            filesize       length of file to follow

So, chances are, the unicos CPIO can open this file

It is. And what this tool should be doing is to convert these binary cpio files into text cpio ones. Doing that on stdio would allow for pipeing through gzip and re-create the archives in a
compatible, but easy-to-use format. To turn stdio into binary (on windows), do this:

#include <fstream>
#include <iostream>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
#endif


int main() {
#ifdef _WIN32
  setmode(fileno(stdout),O_BINARY);
  setmode(fileno(stdin),O_BINARY);
#endif
	std::cout << "hi" << std::endl;
}
*/
struct FileHeader_s {
	uint64_t Words[17];
	uint64_t FileNameLen;
	uint64_t FileLen;
};

int PrintUsage(const char *aExecName, const char *ErrorStr) {
	if (ErrorStr != nullptr) std::cout << "Error: " << ErrorStr << std::endl;
	std::cout << "Usage: " << aExecName << " <Uncompressed gzc file name>" << std::endl;
	return 1;
}

int main(int argc, const char **argv) {
	CommandLine_c CommandLine(argc, argv);
	try {
		std::string InFileName;
		while (CommandLine.HasMoreParams()) {
			std::string CurParam = CommandLine.GetNextParam();
			if (InFileName.empty()) {
				InFileName = CurParam;
			} else {
				throw Generic_x("Invalid command line");
			}
		}
		if (InFileName.empty()) throw Generic_x("File name was not specified");
		std::cout << "Extracting file..." << std::endl;
		std::ifstream InFile(InFileName, std::ios::in | std::ios::binary);
		while (!InFile.eof()) {
			FileHeader_s FileHeader;
			InFile.read((char*)(&FileHeader), sizeof(FileHeader_s));
			if (!InFile.good()) throw Generic_x() << "Can't read from file";
			if (InFile.gcount() != sizeof(FileHeader_s)) throw Generic_x() << "Can't read file header at offset: " << HexPrinter(InFile.tellg());
			for (size_t i = 0; i < sizeof(FileHeader_s::Words) / sizeof(FileHeader_s::Words[0]); ++i) {
				FileHeader.Words[i] = SwapBytes(FileHeader.Words[i]);
			}
			FileHeader.FileNameLen = SwapBytes(FileHeader.FileNameLen);
			FileHeader.FileLen = SwapBytes(FileHeader.FileLen);
			std::string FileName;
			FileName.resize(FileHeader.FileNameLen);
			InFile.read((char*)(&FileName[0]), FileHeader.FileNameLen);
			if (!InFile.good()) throw Generic_x() << "Can't read from file";
			if (uint64_t(InFile.gcount()) != FileHeader.FileNameLen) throw Generic_x() << "Can't read file name at offset: " << HexPrinter(InFile.tellg());
			std::cout << FileName;
			if (FileHeader.FileLen > 0) {
				std::cout << " size: " << DecPrinter(FileHeader.FileLen);

				boost::filesystem::path Path(FileName);
				boost::filesystem::path ParentPath = Path.parent_path();
				if (!ParentPath.empty()) boost::filesystem::create_directories(ParentPath);

				std::ofstream OutFile(FileName, std::ios::out | std::ios::binary);
				bool SkipWrite = false;
				if (!OutFile.good()) {
					std::cerr << "Can't create file: " << FileName << std::endl;
					SkipWrite = true;
				}

				char C;
				for (uint64_t i = 0; i < FileHeader.FileLen; ++i) {
					InFile.read(&C, 1);
					if (!InFile.good()) throw Generic_x() << "Can't read from file";
					if (InFile.gcount() != 1) throw Generic_x() << "Can't read file data at offset: " << HexPrinter(InFile.tellg());
					if (!SkipWrite) {
						OutFile.write(&C, 1);
						if (!OutFile.good()) throw Generic_x() << "Can't create file: " << FileName;
					}
				}
				OutFile.close();
			} else {
				if (FileName == "TRAILER!!!") {
					break;
				}
			}
			std::cout << std::endl;
		}
	}
	catch (std::exception &Ex) {
		return PrintUsage(argv[0], Ex.what());
	}
	return 0;
}

