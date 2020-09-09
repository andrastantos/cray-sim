#include "iop_cluster.h"
#include "cray_mainframe.h"

IopCluster_c::IopCluster_c(const Configuration_c &aConfig, CLogger_c &aLogger, size_t aIopClusterId, class Mainframe_c &aMainframe) :
	mIoMasterClear(false),
	mLogger(aConfig),
	mMainframe(aMainframe),
	mIopClusterId(aIopClusterId),
	mIopTickIdx(0)
{
	mLogger.SetParent(aLogger);
	try {
		// Create shared memories
		mBufferMemory.resize(aConfig.get("BufferMemorySize", 8 * 1024 * 1024) * sizeof(uint64_t), 0);

		// Set startup CPU
		mStartupIopIdx = aConfig.get<size_t>("StartupIopIdx", 0);

		// Load all buffer image files
		for (const auto &BufferImage : aConfig.get_child_safe("BufferImageFiles")) {
			size_t LoadAddress = FromString<size_t>(BufferImage.first);
			std::string ImageFileName = BufferImage.second.get_value<std::string>();
			LoadBufferImage(ImageFileName.c_str(), CAddr_t(LoadAddress));
		}

		// Create IOPs
		for (const auto &Iop : aConfig.get_child_safe("Iops")) {
			if (FromString<size_t>(Iop.first) != mIops.size()) throw InvalidParameter_x("IOP index must start at 0 and must be contiguous");
			mIops.push_back(std::make_unique<Iop_s>(
				Iop.second,
				uint8_t(mIops.size()),
				&mBufferMemory,
				*this
			));
		}

		// Connect IOPs together
		for (size_t i = 0; i<mIops.size(); ++i) {
			for (size_t j = i + 1; j<mIops.size(); ++j) {
				std::unique_ptr<IopIopChannel_c> Channel1 = std::make_unique<IopIopChannel_c>(aConfig.get_child_safe("IOP2IOP"), mIops[i]->Cpu, mIops[j]->Cpu, 6 + 2 * i, 7 + 2 * j - 2);
				std::unique_ptr<IopIopChannel_c> Channel2 = std::make_unique<IopIopChannel_c>(aConfig.get_child_safe("IOP2IOP"), mIops[j]->Cpu, mIops[i]->Cpu, 6 + 2 * j - 2, 7 + 2 * i);
				/*				mIops[i]->Cpu.Connect(6+2*j-2,Channel2->GetAI());
				mIops[i]->Cpu.Connect(7+2*j-2,Channel1->GetAO());
				mIops[j]->Cpu.Connect(6+2*i,Channel1->GetAI());
				mIops[j]->Cpu.Connect(7+2*i,Channel2->GetAO());*/
				mIops[i]->Cpu.Connect(&Channel2->GetChannel(0));
				mIops[i]->Cpu.Connect(&Channel1->GetChannel(1));
				mIops[j]->Cpu.Connect(&Channel1->GetChannel(0));
				mIops[j]->Cpu.Connect(&Channel2->GetChannel(1));
				mIopIopChannels.push_back(move(Channel1));
				mIopIopChannels.push_back(move(Channel2));
			}
		}

		// Execute buffer-memory pokes to change memory image
		for (const auto &BufferMemoryPoke : aConfig.get_child_safe("BufferMemoryPokes")) {
			IopInt_t Value = FromString<IopInt_t>(BufferMemoryPoke.second.data());
			size_t Addr = FromString<size_t>(BufferMemoryPoke.first);
			if (Addr * sizeof(IopInt_t) >= mBufferMemory.size()) throw InvalidParameter_x("Buffer poke address outside of memory");
			BufferMemAccessByType<IopInt_t>(CAddr_t(Addr)) = SwapBytes(Value);
		}

		mBufferMemoryDumpFileName = aConfig.get_optional<std::string>("BufferMemoryDumpFile");
	}
	catch (...) {
		mIops.clear();
		mIopIopChannels.clear();
		mBufferMemory.clear();
		throw;
	}
}

void IopCluster_c::LoadBufferImage(const char *aFileName, CAddr_t aLoadAddr) {
	CRAY_ASSERT(mBufferMemory.size() % sizeof(CInt_t) == 0);
	CAddr_t MemSize = CAddr_t(mBufferMemory.size() / sizeof(uint64_t));
	if (aLoadAddr > MemSize) throw Mainframe_c::ImageLoadFailure_x(aFileName, "load address is beyond buffer memory size");
	std::ifstream File(aFileName, std::ios_base::in | std::ios_base::binary);
	if (File.fail()) throw Mainframe_c::ImageLoadFailure_x(aFileName, "can't open file");
	uint64_t FileSize = boost::filesystem::file_size(aFileName); // we use 64-bit arithmetic here to avoid truncation of very large file sizes even though we can't load them
	if (FileSize % sizeof(uint64_t) != 0) throw Mainframe_c::ImageLoadFailure_x(aFileName, "file size is not modulo 8");
	if (FileSize / sizeof(uint64_t) + size_t(aLoadAddr) > size_t(MemSize)) throw Mainframe_c::ImageLoadFailure_x(aFileName, "file doesn't fit in buffer memory");
	File.read((char*)(&mBufferMemory[(size_t(aLoadAddr) * sizeof(uint64_t))]), FileSize);
	size_t BytesRead = size_t(File.gcount());
	if (BytesRead != FileSize || File.fail()) throw Mainframe_c::ImageLoadFailure_x(aFileName, "can't read file");
}

void IopCluster_c::DeadStart() {
	MasterClear();

	if (mStartupIopIdx < mIops.size()) {
		mIops[mStartupIopIdx]->Cpu.DeadStart();
	}
}

void IopCluster_c::MasterClear() {
	for (auto &Iop : GetIops()) Iop.MasterClear();
}

void IopCluster_c::GetPeripheralStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const {
	for (auto &Iop : mIops) {
		StatusReport_c Status;
		Iop->Cpu.GetPeripheralStatus(Status, aFilter, aElapsedTime, aLongFormat);
		if (!Status.empty()) {
			aStatus.put_child(Iop->Cpu.GetName(), Status);
		}
	}
}

std::string IopCluster_c::GetName() const {
	std::stringstream Name;
	Name << "ICL" << DecPrinter(mIopClusterId);
	return Name.str();
}

std::string IopCluster_c::GetLongName() const {
	std::stringstream Name;
	Name << "IOP Cluster " << DecPrinter(mIopClusterId);
	return Name.str();
}

void IopCluster_c::Dump(size_t aIdent) const {
	for (size_t Iop = 0; Iop<mIops.size(); ++Iop) {
		if (mIops[Iop] == nullptr) continue;
		mLogger << setloglevel(LogLevel_Dump) << "Iop" << DecPrinter(Iop, 0) << std::endl;
		mIops[Iop]->Cpu.Dump(aIdent + 1);
	}
}

void IopCluster_c::Tick() {
	if (mIopTickIdx < mIops.size()) {
		mIops[mIopTickIdx]->Cpu.Tick();
	}
	++mIopTickIdx;
	if (mIopTickIdx >= mIops.size()) mIopTickIdx = 0;
}

void IopCluster_c::GetCpuStatus(StatusReport_c &aStatus, boost::timer::nanosecond_type aElapsedTime, bool aLongFormat) const {
	size_t IopCnt = GetIopCnt();
	for (size_t IopIdx = 0; IopIdx < IopCnt; ++IopIdx) {
		StatusReport_c IopStatus;
		GetIop(IopIdx).GetStatus(IopStatus, aElapsedTime, aLongFormat);
		aStatus.put_child(GetIop(IopIdx).GetName(), IopStatus);
	}
}

