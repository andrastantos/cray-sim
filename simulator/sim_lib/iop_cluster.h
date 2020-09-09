#ifndef __IOP_CLUSTER_H__
#define __IOP_CLUSTER_H__

#include "cray_iop.h"
#include "iop_cluster_if.h"
#include "iop_iop2iop.h"
#include "iop_iop2cray.h"

class IopCluster_c: public IopClusterBase_i {
public:
	IopCluster_c(const Configuration_c &aConfig, CLogger_c &aLogger, size_t aIopClusterId, class Mainframe_c &aMainframe);
	size_t GetIopCnt() const { return mIops.size(); }
	IopCpu_c &GetIop(size_t aIdx) {
		CRAY_ASSERT(aIdx < mIops.size());
		return mIops[aIdx]->Cpu;
	}
	const IopCpu_c &GetIop(size_t aIdx) const {
		CRAY_ASSERT(aIdx < mIops.size());
		return mIops[aIdx]->Cpu;
	}

	typedef IteratorWrapper_t<IopCluster_c, IopCpu_c, &IopCluster_c::GetIopCnt, &IopCluster_c::GetIop> IopIteratorWrapper_c;
	const IopIteratorWrapper_c GetIops() { return IopIteratorWrapper_c(*this); }
	typedef ConstIteratorWrapper_t<IopCluster_c, IopCpu_c, &IopCluster_c::GetIopCnt, &IopCluster_c::GetIop> ConstIopIteratorWrapper_c;
	const ConstIopIteratorWrapper_c GetIops() const { return ConstIopIteratorWrapper_c(*this); }

	template <typename aType> aType &BufferMemAccess(const CAddr_t aAddr) { return *(aType *)(&mBufferMemory[aAddr * 8]); } // Default addresses are by QWORDs
	template <typename aType> aType &BufferMemAccessByType(const CAddr_t aAddr) { return *(aType *)(&mBufferMemory[aAddr * sizeof(aType)]); }
	class Mainframe_c &GetMainframe() { return mMainframe; }
	const class Mainframe_c &GetMainframe() const { return mMainframe; }
	CLogger_c &GetLogger() { return mLogger; }

	virtual void DeadStart() override;
	virtual void MasterClear() override;

	virtual void GetPeripheralStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const override;
	virtual void GetCpuStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const override;
	virtual std::string GetName() const override;
	virtual std::string GetLongName() const override;
	virtual void Dump(size_t aIdent = 0) const override;
	virtual void RegisterCommands(CommandHooks_t &aHooks)  override {}
	virtual void Tick() override;
protected:
	void LoadBufferImage(const char *aFileName, CAddr_t aLoadAddr);

	struct Iop_s {
		Iop_s(
			const Configuration_c &aConfig,
			uint8_t aCpuId,
			std::vector<uint8_t> *aBufferMemory,
			IopCluster_c &aMainFrame
		) :
			Cpu(aConfig, aCpuId, IoMemory, aBufferMemory, aMainFrame),
			IoMemory(65536 * sizeof(IopInt_t))
		{
			MemoryDumpFileName = aConfig.get_optional<std::string>("MemoryDumpFile");
		}

		IopCpu_c Cpu;
		std::vector<uint8_t> IoMemory;
		boost::optional<std::string> MemoryDumpFileName;
	};

	std::vector<std::unique_ptr<Iop_s>> mIops;
	std::vector<std::unique_ptr<IopIopChannel_c>> mIopIopChannels;

	bool mIoMasterClear;
	std::vector<uint8_t> mBufferMemory;
	boost::optional<std::string> mBufferMemoryDumpFileName;

	mutable CLogger_c mLogger;
	class Mainframe_c &mMainframe;

	size_t mStartupIopIdx;
	size_t mIopClusterId;

	size_t mIopTickIdx;
};
#endif // __IOP_CLUSTER_H__
