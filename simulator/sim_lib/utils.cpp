#include "utils.h"

std::string Dump(std::vector<uint8_t> &aVector) {
	std::stringstream RetVal;
	const size_t cLineSize = 16;
	size_t End = aVector.size();
	for (size_t Addr = 0; Addr < End; Addr += cLineSize) {
		uint8_t Data[cLineSize];
		for (size_t j = 0; j < cLineSize; ++j) {
			if (Addr + j >= End) break;
			Data[j] = aVector[Addr + j];
		}
		RetVal << HexPrinter(Addr, 4) << " : ";
		for (size_t j = 0; j < cLineSize; ++j) {
			if (Addr + j >= End) {
				RetVal << "   ";
			}
			else {
				RetVal << std::noshowbase << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << int(Data[j]) << " ";
			}
		}
		RetVal << " - ";
		for (size_t j = 0; j < cLineSize; ++j) {
			if (Addr + j >= End) break;
			RetVal << AsciiDumpPrinter(Data[j]) << "";
		}
		RetVal << std::endl;
	}
	RetVal << std::endl;
	return RetVal.str();
}


//#if !defined(__CYGWIN__) && defined(_WIN32)
#if defined(__CYGWIN__) || defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

void SetThreadAffinity(std::thread &aThread, size_t aCpuId) {
	CRAY_ASSERT(aCpuId < std::thread::hardware_concurrency());
	CRAY_ASSERT(aCpuId < (sizeof(DWORD_PTR) *8));
	DWORD_PTR CpuMask = DWORD_PTR(1) << aCpuId;
	DWORD_PTR RetVal = SetThreadAffinityMask(aThread.native_handle(), CpuMask);
	if (RetVal == 0) throw Generic_x() << "Error calling SetThreadAffinityMask: " << HexPrinter(GetLastError());
}

void SetThreadAffinity(size_t aCpuId) {
	CRAY_ASSERT(aCpuId < std::thread::hardware_concurrency());
	CRAY_ASSERT(aCpuId < (sizeof(DWORD_PTR) *8));
	DWORD_PTR CpuMask = DWORD_PTR(1) << aCpuId;
	DWORD_PTR RetVal = SetThreadAffinityMask(GetCurrentThread(), CpuMask);
	if (RetVal == 0) throw Generic_x() << "Error calling SetThreadAffinityMask: " << HexPrinter(GetLastError());
}

#else

#include <pthread.h>

void SetThreadAffinity(std::thread &aThread, size_t aCpuId) {
	CRAY_ASSERT(aCpuId < std::thread::hardware_concurrency());
	cpu_set_t CpuSet;
	CPU_ZERO(&CpuSet);
	CPU_SET(aCpuId, &CpuSet);
	int RetVal = pthread_setaffinity_np(aThread.native_handle(), sizeof(CpuSet), &CpuSet);
	if (RetVal != 0) throw Generic_x() << "Error calling pthread_setaffinity_np: " << DecPrinter(RetVal);
}

void SetThreadAffinity(size_t aCpuId) {
	CRAY_ASSERT(aCpuId < std::thread::hardware_concurrency());
	cpu_set_t CpuSet;
	CPU_ZERO(&CpuSet);
	CPU_SET(aCpuId, &CpuSet);
	int RetVal = pthread_setaffinity_np(pthread_self(), sizeof(CpuSet), &CpuSet);
	if (RetVal != 0) throw Generic_x() << "Error calling pthread_setaffinity_np: " << DecPrinter(RetVal);
}

#endif // _WIN32
