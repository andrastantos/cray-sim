#ifndef __IOP_CLUSTER_IF_H__
#define __IOP_CLUSTER_IF_H__

#include "cray_iop.h"
#include "iop_iop2iop.h"
#include "iop_iop2cray.h"

class IopClusterBase_i {
public:
	virtual ~IopClusterBase_i() {}

	virtual void DeadStart() = 0;
	virtual void MasterClear() = 0;

	virtual void GetPeripheralStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const = 0;
	virtual void GetCpuStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const = 0;
	virtual std::string GetName() const = 0;
	virtual std::string GetLongName() const = 0;
	virtual void Dump(size_t aIdent = 0) const = 0;
	virtual void RegisterCommands(CommandHooks_t &aHooks) = 0;
	virtual void Tick() = 0;
};
#endif // __IOP_CLUSTER_IF_H__
