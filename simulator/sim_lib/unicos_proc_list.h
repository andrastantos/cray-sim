#ifndef __UNICOS_PROC_LIST_H__
#define __UNICOS_PROC_LIST_H__

#include "utils.h"
#include "cray_types.h"
#include "exchange_packet.h"
#include "config_file.h"

class UnicosProcList_c {
public:
	UnicosProcList_c(const class Mainframe_c &aMainframe, const Configuration_c &aConfig);
	void TestMemAccess(CAddr_t aAddress, CInt_t aData);
	std::string Dump();
protected:
	CAddr_t mProcTableBase;
	size_t mProcTableLength;
	const class Mainframe_c &mMainframe;
	static const size_t cProcEntryLen = 0x170; // Proc entries are this long
};

#endif // __UNICOS_PROC_LIST_H__