#pragma once

#include "cray_types.h"
#include <stdint.h>
#include "cray_mainframe.h"
#include "config_file.h"

// Cray simulator virtual base and common functionality for all CPU classes

class Cpu_c {
public:
	explicit Cpu_c(const Configuration_c &aConfig, Mainframe_c &aMainframe, size_t aCpuId) :
		mMainframe(aMainframe),
		mCpuId(aCpuId),
		mLogger(aConfig, "CPU", aCpuId)
	{}

	explicit Cpu_c(Mainframe_c &aMainframe) :
		mMainframe(aMainframe),
		mCpuId(0),
		mLogger(Configuration_c::GetEmptyConfig(), "CPU", 0) {
	}

	virtual ~Cpu_c() {}

	virtual void MasterClear() = 0; // Enters the CPU into it's reset state, stops executing
	virtual void DeadStart() = 0;   // Removes the CPU from it's reset state, starts executing

	virtual void Tick() = 0; // Called periodically to execute (a batch of) instructions

	// These are used for interrupt routing
	virtual bool IsTsBlocked() const = 0;
	virtual bool IsSelectedForExternalInterrupts() const = 0;
	virtual void SetIoInterrupt() = 0;

	// Used to trigger interrupts from one processor in another
	virtual void SetInterProcessorInterrupt() = 0;

	// Used during dead-lock checks
	virtual uint8_t GetCluster() const = 0;
	virtual void SetDeadLockInterrupt() = 0;

	// Used for global breakpoint checks
	virtual CProgramAddr_t GetAbsoluteProgramCounter() const = 0;

	// Used for status reporting
	virtual void GetStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const = 0;
	virtual std::string GetName() const = 0;

	// Used for dump-points
	virtual void Dump(size_t aIdent = 0) const = 0;

	// Used for event/breakpoint logging to inject these trace messages into the triggering CPUs logger conext.
	CLogger_c &GetLogger() { return mLogger; }

	// Used by Events to get trigger string replacement correct (replace {cpu} with the proper ID)
	size_t GetCpuId() const { return mCpuId; }

	// Used by dump events to get to the main memory to dump and by sys_task_req for similar reasons
	Mainframe_c &GetMainframe() { return mMainframe; }
	const Mainframe_c &GetMainframe() const { return mMainframe; }

	// Used by sys_task_req for logging purposes only
	virtual uint8_t GetExchangePacketAddress() const = 0;

	// Used by mainframe debug functions
	virtual void DumpHistory() = 0;
	virtual void ClearHistory() = 0;
#ifdef COLLECT_PERF
	virtual uint64_t GetJumpsTaken() const = 0;
	virtual uint64_t GetJumpsNotTaken() const = 0;
#endif

	virtual void Exchange() = 0;

	// TODO: we'll need a direct interface to clusters: when we're in a mixed environment, all inter-processor communication needs to be abstracted out. For now, however it's OK.
protected:
	virtual void SetProgrammableClockInterrupt() = 0;

	size_t mCpuId;
	mutable CLogger_c mLogger;
	Mainframe_c &mMainframe;
};

