#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include "utils.h"
#include "cray_iop.h"
#include <boost/filesystem.hpp>
#include "cray_mainframe.h"

int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " <image file> <iop type>" << std::endl;
		return 1;
	}
	bool DoSwapBytes = true;
	uint64_t FileSize = boost::filesystem::file_size(argv[1])/sizeof(IopInt_t);
	const size_t MaxFileSize = 65536*2;
	if (FileSize > MaxFileSize) {
		std::cout << "Can't use files larger than " << MaxFileSize/1024 << "kB" << std::endl;
		return 1;
	}
	std::vector<uint8_t> File((size_t)FileSize*2);
	std::ifstream Input(argv[1], std::ios::in | std::ios::binary);
	if (Input.bad()) {
		std::cout << "Can't open input file: " << argv[1] << std::endl;
		return 1;
	}
	Input.read((char*)&File[0],File.size());
	if (Input.bad()) {
		std::cout << "Can't read file." << std::endl;
		return 1;
	}
	Input.close();
	if (DoSwapBytes) {
		for(size_t i=0;i<FileSize;++i) {
			*(uint16_t*)(&File[i*2]) = SwapBytes(*(uint16_t*)(&File[i*2]));
		}
	}

	size_t Offset = 0;
	IopTypes_e IopType;
	if (strcmp(argv[2],"MIOP") == 0) {
		IopType = IopType_MIOP;
	} else if (strcmp(argv[2],"BIOP") == 0) {
		IopType = IopType_BIOP;
	} else if (strcmp(argv[2],"DIOP") == 0) {
		IopType = IopType_DIOP;
	} else if (strcmp(argv[2],"XIOP") == 0) {
		IopType = IopType_XIOP;
	} else {
		std::cout << "invalid IOP type. Valid values are: MIOP, BIOP, DIOP or XIOP" << std::endl;
		return 1;
	}
	IopCpu_c Iop(IopType, 0,File, nullptr);
	while (Offset < FileSize) {
		std::stringstream Disassembly;
		std::stringstream Explanation;
		std::stringstream InstFields;
		IopInt_t Increment;
		Increment = Iop.Disassemble(IopInt_t(Offset),Disassembly,Explanation,InstFields);
		std::cout << HexPrinter(Offset,4) << " (" << HexPrinter(Offset*2,6) << ") " << InstFields.str() << " | " << std::left << std::setw(30) << Disassembly.str() << std::setw(0);
		if (Explanation.str().length() > 0) std::cout << " | " << Explanation.str();
		std::cout << std::endl;
		Offset += Increment;
	}
	return 0;
}

