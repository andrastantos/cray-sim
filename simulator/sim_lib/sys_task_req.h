#ifndef __SYS_TASK_REQ_H__
#define __SYS_TASK_REQ_H__

#include "utils.h"
#include "cray_types.h"
#include "exchange_packet.h"
#include "config_file.h"

enum class OsTypes_e {
	COS,
	UNICOS,
	None
};

void SetupSystaskParser(const Configuration_c &aConfig);
void ParseExchangePacket(const ExchangePacket_c &aCurrentEP, const ExchangePacket_c &aNewEP, std::ostream &aLogger, const class Cpu_c &aCpu, OsTypes_e aOsType);
void DumpSyscallStats(std::ostream &aLogger, const class Mainframe_c &aMainframe, OsTypes_e aOsType);

#endif // __SYS_TASK_REQ_H__