// Cray-1S CPU simulator class

// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
//
// To be true to the documentation, octal numbers are extensively used in this file.
// Watch out for the leading 0-s!
//
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!

template <bool aDoExecute> size_t SoftCpu_c::Decode0000(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0000);
	if (!aDoExecute) {
		aDisassembly << "ERR" << DecPrinter(int(ijk), 0);
	}
	if (aDoExecute) {
		// The actual interrupt handling will handle in the next Tick call, where P will have already advanced. Which is the expected behavior per HR-0097 5-8
		// TODO: who clears these bits? This gets written out in the EP before monitor mode takes effect, but it must be cleared in memory before its read back in. Or is it that when the CPU reads the EP back, it ignores this (and maybe other) mode-bits?
		mState.Flags.SetErrorExit();
		aBreakBurst = true;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0001(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0001);
	switch (i) {
	case 0: // CA, Aj Ak and PASS
		if (!aDoExecute) {
			if (!IsXmp() && j == 0) {
				aDisassembly << "PASS ";
				aExplanation << "";
				return 1;
			}
			aDisassembly << "CA," << RefAj << " " << RefAk;
		}
		if (aDoExecute) {
			if (!IsXmp() && j == 0) return 1;
			CRAY_ASSERT(mState.Mode.IsMonitorMode());
			if (mState.Mode.IsMonitorMode()) {
				size_t ChIdx = size_t(CAddr_t(RefAj)) - 8;
				mLogger << setloglevel(LogLevel_IoActivity) << SideEffectIndent << "CA" << " (ChIdx = " << DecPrinter(ChIdx + 8) << ", Value = " << HexPrinter(RefAk) << " )" << std::endl;
				if (ChIdx >= mMainframe.GetChannelCnt()) throw InstExecError_x() << "Channel index " << DecPrinter(ChIdx + 8) << " out of bounds";
				auto Channel = mMainframe.GetChannel(ChIdx);
				if (Channel == nullptr) mLogger << setloglevel(LogLevel_IoActivity) << SideEffectIndent << "NULL channel" << std::endl;
				if (Channel != nullptr) Channel->SetAddress(CAddr_t(RefAk));
				aBreakBurst = true;
			}
		}
		return 1;
	case 1: // CL, Aj Ak and PASS
		if (!aDoExecute) {
			if (!IsXmp() && j == 0) {
				aDisassembly << "PASS ";
				aExplanation << "";
				return 1;
			}
			aDisassembly << "CL," << RefAj << " " << RefAk;
		}
		if (aDoExecute) {
			if (!IsXmp() && j == 0) return 1;
			CRAY_ASSERT(mState.Mode.IsMonitorMode());
			if (mState.Mode.IsMonitorMode()) {
				size_t ChIdx = size_t(CAddr_t(RefAj)) - 8;
				mLogger << setloglevel(LogLevel_IoActivity) << SideEffectIndent << "CL" << " (ChIdx = " << DecPrinter(ChIdx + 8) << ", Value = " << HexPrinter(RefAk) << " )" << std::endl;
				if (ChIdx >= mMainframe.GetChannelCnt()) throw InstExecError_x() << "Channel index " << DecPrinter(ChIdx + 8) << " out of bounds";
				auto Channel = mMainframe.GetChannel(ChIdx);
				if (Channel == nullptr) mLogger << setloglevel(LogLevel_IoActivity) << SideEffectIndent << "NULL channel" << std::endl;
				if (Channel != nullptr) Channel->SetLimit(CAddr_t(RefAk));
				aBreakBurst = true;
			}
		}
		return 1;
	case 2: // multiple
		switch (k) {
		case 0: // CI,Aj and PASS
			if (!aDoExecute) {
				if (!IsXmp() && j == 0) {
					aDisassembly << "PASS ";
					aExplanation << "";
					return 1;
				}
				aDisassembly << "CI," << RefAj;
				aExplanation << "Clear interrupt and error flags for Channel " << RefAj << ". Clear device master-clear.";
			}
			if (aDoExecute) {
				if (!IsXmp() && j == 0) return 1;
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					size_t ChIdx = size_t(CAddr_t(RefAj)) - 8;
					mLogger << setloglevel(LogLevel_IoActivity) << SideEffectIndent << "CI" << " (ChIdx = " << DecPrinter(ChIdx + 8) << ")" << std::endl;
					if (ChIdx >= mMainframe.GetChannelCnt()) {
						mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << " Channel index out of bounds - ignoring" << std::endl;
					}
					else {
						auto Channel = mMainframe.GetChannel(ChIdx);
						if (Channel != nullptr) {
							Channel->ClearInterrupt();
							Channel->ClearError();
						}
					}
					aBreakBurst = true;
				}
			}
			return 1;
		case 1: // MC,Aj
			if (!aDoExecute) {
				if (!IsXmp() && j == 0) {
					aDisassembly << "PASS ";
					aExplanation << "";
					return 1;
				}
				aDisassembly << "MC," << RefAj;
				aExplanation << "Clear interrupt and error flags for Channel " << RefAj << ". Set device master-clear (output CH); clear device ready-held (input CH).";
			}
			if (aDoExecute) {
				if (!IsXmp() && j == 0) return 1;
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					size_t ChIdx = size_t(CAddr_t(RefAj)) - 8;
					mLogger << setloglevel(LogLevel_IoActivity) << SideEffectIndent << "MC" << " (ChIdx = " << DecPrinter(ChIdx + 8) << ")" << std::endl;
					if (ChIdx >= mMainframe.GetChannelCnt()) {
						mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << " Channel index out of bounds - ignoring" << std::endl;
					}
					else {
						auto Channel = mMainframe.GetChannel(ChIdx);
						if (Channel != nullptr) {
							Channel->ClearInterrupt();
							Channel->ClearError();
							Channel->MainframeSideMasterClear();
						}
					}
					mState.Flags.ClearIoInterrupt();
					aBreakBurst = true;
				}
			}
			return 1;
		default: CRAY_UNKNOWN
		}
		return 1;
	case 3: // XA Aj
		if (!aDoExecute) {
			if (!IsXmp() && j == 0) {
				aDisassembly << "PASS ";
				aExplanation << "";
				return 1;
			}
			aDisassembly << "XA " << RefAj;
			aExplanation << "Load the Exchange Address register";
		}
		if (aDoExecute) {
			if (!IsXmp() && j == 0) return 1;
			CRAY_ASSERT(mState.Mode.IsMonitorMode());
			if (mState.Mode.IsMonitorMode()) {
				LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
				if (LogLine.good()) LogLine << "\tXA <== " << HexPrinter(uint8_t(size_t(CAddr_t(RefAj)) >> 4), 2) << std::endl;
				mState.ExchangePacketAddress = uint8_t(size_t(CAddr_t(RefAj)) >> 4);
			}
		}
		return 1;
	case 4: // RT, PCI, CCO, ECI, DCI
		switch (k) {
		case 0: // RT Sj
			if (!aDoExecute) {
				aDisassembly << "RT " << RefSj;
				aExplanation << "Set the real-time-clock";
			}
			if (aDoExecute) {
				//						CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					mMainframe.SetRealTimeClock(RefSj);
				}
				else {
					std::stringstream Event;
					Event << "CPU" << mCpuId << ": RT set attempt in user mode";
					GetMainframe().GetEventDispatcher().Fire(Event.str());
				}
			}
			return 1;
		case 1: // IP,j 1 [X-MP]
			if (!aDoExecute) {
				if (!IsXmp()) {
					aDisassembly << "SIPI " << RefAj;
					aExplanation << "Set interprocessor interrupt request of CPU " << RefAj;
				}
				else {
					aDisassembly << "IP," << DecPrinter(int(j), 0) << " 1";
					aExplanation << "Set interprocessor interrupt request of CPU" << j;
				}
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					CRAY_ASSERT(IsXmp() || !IsXMode()); // We don't know how this instruction works in X mode on a Y-MP machine...
					size_t CpuIdx = IsXmp() ? j : RefAj;
					if (CpuIdx >= mMainframe.GetCpuCnt()) throw InstExecError_x() << "CPU index " << OctPrinter(CpuIdx) << " out of bounds";
					if (CpuIdx != mCpuId) {
						CRAY_ASSERT(mState.Mode.IsMonitorMode());
						mMainframe.GetCpu(CpuIdx).SetInterProcessorInterrupt();
						aBreakBurst = true;
					}
				}
			}
			return 1;
		case 2: // IP 0 [X-MP]
			if (!aDoExecute) {
				if (IsXmp()) {
					aDisassembly << "IP 0";
				}
				else {
					aDisassembly << "CIPI";
				}
				aExplanation << "Clear received interprocessor interrupt";
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					mState.Flags.ClearInterProcessorInterrupt();
					aBreakBurst = true;
				}
			}
			return 1;
		case 3: // CLN j [X-MP]
			if (!aDoExecute) {
				if (!IsXmp()) {
					aDisassembly << "CLN " << RefAj;
					aExplanation << "Set cluster number to " << DecPrinter(RefAj, 0);
				}
				else {
					aDisassembly << "CLN " << j;
					aExplanation << "Set cluster number to " << DecPrinter(int(j), 0);
				}
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					// Cluster numbers are 1-based. 0 is reserved to no-cluster selected
					CRAY_ASSERT(IsXmp() || !IsXMode()); // We don't know how this instruction works in X mode on a Y-MP machine...
					size_t ClusterIdx = IsXmp() ? j : RefAj;
					if (ClusterIdx > mMainframe.GetClusterCnt()) throw InstExecError_x() << "Cluster index " << OctPrinter(ClusterIdx) << " out of bounds";
					LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
					if (LogLine.good()) LogLine << "\tCLUSTER <== " << DecPrinter(int(ClusterIdx), 0) << std::endl;
					SetCluster(uint8_t(ClusterIdx));
				}
			}
			return 1;
		case 4: // PCI Sj
			if (!aDoExecute) {
				aDisassembly << "PCU " << RefSj;
				aExplanation << "Set Interrupt Interval register to " << RefSj;
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					LogLine_c LogLine = mLogger << setloglevel(LogLevel_SideEffects);
					if (LogLine.good()) LogLine << "\tPCI <== " << HexPrinter(RefSj, 0) << std::endl;
					mState.PeriodicInterruptLimit = uint32_t(CInt_t(RefSj));
					mState.PeriodicInterruptCountDown = mState.PeriodicInterruptLimit;
					aBreakBurst = true;
				}
			}
			return 1;
		case 5: // CCI
			if (!aDoExecute) {
				aDisassembly << "CCI";
				aExplanation << "Clear programmable clock interrupt request";
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					mState.Flags.ClearProgrammableClockInterrupt();
					aBreakBurst = true;
				}
			}
			return 1;
		case 6: // ECI
			if (!aDoExecute) {
				aDisassembly << "ECI";
				aExplanation << "Enable programmable clock interrupt request";
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					mState.PeriodicInterruptEnabled = true;
					aBreakBurst = true;
				}
			}
			return 1;
		case 7: // DCI
			if (!aDoExecute) {
				aDisassembly << "DCI";
				aExplanation << "Disable programmable clock interrupt request";
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					mState.PeriodicInterruptEnabled = false;
					aBreakBurst = true;
				}
			}
			return 1;
		default: CRAY_UNKNOWN
		}
		return 1;
	case 5: // multiple
		switch (k) {
		case 0: // SPM
			if (!aDoExecute) {
				aDisassembly << "SPM " << DecPrinter(int(j) & 3, 0);
				aExplanation << "Select performance monitor group (no CAL support)";
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					//CRAY_UNIMPLEMENTED;
				}
			}
			return 1;
		case 1: // multiple
			switch (j) {
			case 0:  // MRM
				if (!IsXmp()) CRAY_UNKNOWN
					if (!aDoExecute) {
						aDisassembly << "MRM";
						aExplanation << "Select maintenance read mode (no CAL support)";
					}
				if (aDoExecute) {
					CRAY_ASSERT(mState.Mode.IsMonitorMode());
					if (mState.Mode.IsMonitorMode()) {
						CRAY_UNIMPLEMENTED;
					}
				}
				return 1;
			case 1:  // LDC
				if (!aDoExecute) {
					aDisassembly << "LDC S1";
					aExplanation << "Load diagnostic checkbyte with S1 (no CAL support)";
				}
				if (aDoExecute) {
					CRAY_ASSERT(mState.Mode.IsMonitorMode());
					if (mState.Mode.IsMonitorMode()) {
						CRAY_UNIMPLEMENTED;
					}
				}
				return 1;
			case 2:  // SMW 1
				if (!aDoExecute) {
					aDisassembly << "SMW 1";
					aExplanation << "Set maintenance write mode 1 (no CAL support)";
				}
				if (aDoExecute) {
					CRAY_ASSERT(mState.Mode.IsMonitorMode());
					if (mState.Mode.IsMonitorMode()) {
						CRAY_UNIMPLEMENTED;
					}
				}
				return 1;
			case 3:  // SMW 2
				if (!aDoExecute) {
					aDisassembly << "SMW 2";
					aExplanation << "Set maintenance write mode 2 (no CAL support)";
				}
				if (aDoExecute) {
					CRAY_ASSERT(mState.Mode.IsMonitorMode());
					if (mState.Mode.IsMonitorMode()) {
						CRAY_UNIMPLEMENTED;
					}
				}
				return 1;
			default: CRAY_UNKNOWN
			}
			return 1;
		default: CRAY_UNKNOWN
		}
		return 1;
	case 6:
		if (IsXmp()) CRAY_UNKNOWN
			switch (k) {
			case 1:
				if (!aDoExecute) {
					aDisassembly << "IVC " << RefAj;
					aExplanation << "Invalidate cache of CPU " << RefAj;
				}
				if (aDoExecute) {
				}
				return 1;
			default: CRAY_UNKNOWN
			}
		return 1;
	case 7:
		switch (j) {
		case 0:
			if (aDoExecute) {
				if (k != 4) {
					std::stringstream Event;
					Event << "CPU" << mCpuId << ": Undocumented instruction code " << OctPrinter(FirstParcel(aParcels), 6);
					GetMainframe().GetEventDispatcher().Fire(Event.str());
				}
			}
			switch (k) {
			case 2:
				if (!aDoExecute) {
					aDisassembly << "*** UNK *** - 0001702 overridded to PASS";
					aExplanation << "Undocumented instruction that seems to be part of the startup process. Skipping it for now.";
				}
				if (aDoExecute) {
					mLogger << setloglevel(LogLevel_Event) << "*** UNK *** - 0001702 overridded to PASS ====== something in startup ======" << std::endl;
				}
				return 1;
			case 3:
				if (!aDoExecute) {
					aDisassembly << "*** UNK *** - 0001703 overridded to PASS";
					aExplanation << "Undocumented instruction that seems to be part of the HALT routine. Skipping it for now.";
				}
				if (aDoExecute) {
					// In COS_115 this instruction happens apparently outside of the HALT handler...
					std::cout << "*** UNK *** - 0001703 overridded to PASS ===== SYSTEM WENT INTO HALT ====== " << std::endl;
					mLogger << setloglevel(LogLevel_Event) << "*** UNK *** - 0001703 overridded to PASS ===== SYSTEM WENT INTO HALT ====== " << std::endl;
					//										CRAY_UNKNOWN
				}
				return 1;
			case 4:
				if (!aDoExecute) {
					aDisassembly << "*** UNK *** - 0001704 overridded to PASS";
					aExplanation << "Undocumented instruction that seems to be part of the IDLE task. Skipping it for now.";
				}
				if (aDoExecute) {
					mLogger << setloglevel(LogLevel_Event) << "*** UNK *** - 0001704 overridded to PASS ====== IDLE ======" << std::endl;
				}
				return 1;
			case 6:
				if (!aDoExecute) {
					aDisassembly << "*** UNK *** - 0001706 overridded to PASS";
					aExplanation << "Undocumented instruction that seems to be part of the startup process. Skipping it for now.";
				}
				if (aDoExecute) {
					mLogger << setloglevel(LogLevel_Event) << "*** UNK *** - 0001706 overridded to PASS ====== something in startup ======" << std::endl;
				}
				return 1;
			default: CRAY_UNKNOWN
			}
			return 1;
		default: CRAY_UNKNOWN
		}
		return 1;
	default: CRAY_UNKNOWN
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0002(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0002);
	switch (i) {
	case 0: // VL Ak
		if (!aDoExecute) {
			aDisassembly << "VL " << RefAk;
			aExplanation << "Set vector Vector Length register";
		}
		if (aDoExecute) {
			uint8_t NewLength = uint8_t(size_t(CAddr_t(RefAk)) & 0x3f);
			if (NewLength == 0) { // If the low-order 6 bits of Ak is 0, set VL to 64 (the maximum)
				NewLength = 64;
			}
			RefVL = NewLength;
		}
		return 1;
	case 1: // EFI
		if (!aDoExecute) {
			aDisassembly << "EFI";
			aExplanation << "Enable floating point exceptions";
		}
		if (aDoExecute) {
			CRAY_ASSERT(jk == 000);
			mState.Mode.SetFloatingPointErrorMode(true);
			CalcInterruptMask();
			mState.Flags.ClearFloatingPointError();
		}
		return 1;
	case 2:
		switch (jk) {
		case 000: // DFI
			if (!aDoExecute) {
				aDisassembly << "DFI";
				aExplanation << "Disable floating point exceptions";
			}
			if (aDoExecute) {
				mState.Mode.SetFloatingPointErrorMode(false);
				CalcInterruptMask();
				mState.Flags.ClearFloatingPointError();
			}
			return 1;
		case 010: // CBL
			if (IsSV1()) {
				if (!aDoExecute) {
					aDisassembly << "CBL";
					aExplanation << "Clear the Bit-matrix loaded bit";
				}
				if (aDoExecute) {
					mState.Mode.SetSv1BitMatrixLoaded(false);
					CalcInterruptMask();
				}
			}
			else {
				CRAY_UNKNOWN
			}
			return 1;
		default: CRAY_UNKNOWN
		}
	case 3: // ERI
		if (!aDoExecute) {
			aDisassembly << "ERI";
			aExplanation << "Enable range exceptions";
		}
		if (aDoExecute) {
			CRAY_ASSERT(jk == 000);
			mState.Mode.SetOperandRangeErrorMode(true);
			CalcInterruptMask();
		}
		return 1;
	case 4: // DRI
		if (!aDoExecute) {
			aDisassembly << "DRI";
			aExplanation << "Disable range exceptions";
		}
		if (aDoExecute) {
			CRAY_ASSERT(jk == 000);
			mState.Mode.SetOperandRangeErrorMode(false);
			CalcInterruptMask();
		}
		return 1;
	case 5: // DBM
		if (!aDoExecute) {
			aDisassembly << "DBM";
			aExplanation << "Disable bidirectional memory transfers";
		}
		if (aDoExecute) {
			CRAY_ASSERT(jk == 000);
			mState.Mode.SetBidirectionalMemoryMode(true);
			CalcInterruptMask();
		}
		return 1;
	case 6: // EBM
		if (!aDoExecute) {
			aDisassembly << "EBM";
			aExplanation << "Enable bidirectional memory transfers";
		}
		if (aDoExecute) {
			CRAY_ASSERT(jk == 000);
			mState.Mode.SetBidirectionalMemoryMode(false);
			CalcInterruptMask();
		}
		return 1;
	case 7: // CMR
		switch (jk) {
		case 000: // CMR
			if (!aDoExecute) {
				aDisassembly << "CMR";
				aExplanation << "Complete memory references";
			}
			// This is a no-op in the simulator: we don't have funky memory accesses, so no barrier primitives are needed
			if (aDoExecute) {
				//::TODO we might not need to break the burst here...
				aBreakBurst = true;
			}
			return 1;
			// NOTE: these are cache-control and memory-ordering instructions and as such are not simulated
		case 003: // ETSI
			if (IsJ90OrSV1()) {
				if (!aDoExecute) {
					aDisassembly << "ETSI";
					aExplanation << "Enable Test and Set Validate";
				}
				if (aDoExecute) {
				}
			}
			else CRAY_UNKNOWN;
			return 1;
		case 004: // CPA
			if (IsJ90OrSV1()) {
				if (!aDoExecute) {
					aDisassembly << "CPA";
					aExplanation << "Hold if Port A or B is busy";
				}
				if (aDoExecute) {
				}
			}
			else CRAY_UNKNOWN;
			return 1;
		case 005: // CPB
			if (IsJ90OrSV1()) {
				if (!aDoExecute) {
					aDisassembly << "CPB";
					aExplanation << "Hold if Port A or B is busy";
				}
				if (aDoExecute) {
				}
			}
			else CRAY_UNKNOWN;
			return 1;
		case 006: // CPW
			if (IsJ90OrSV1()) {
				if (!aDoExecute) {
					aDisassembly << "CPW";
					aExplanation << "Hold instruction issue if block store or scalar load/store busy";
				}
				if (aDoExecute) {
				}
			}
			else CRAY_UNKNOWN;
			return 1;
		case 007: // DTSI
			if (IsJ90OrSV1()) {
				if (!aDoExecute) {
					aDisassembly << "DTSI";
					aExplanation << "Enable Test and Set Validate";
				}
				if (aDoExecute) {
					aBreakBurst = true;
				}
			}
			else CRAY_UNKNOWN;
			return 1;
		default: CRAY_UNKNOWN
		}
	default: CRAY_UNKNOWN
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0003(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0003);
	switch (i) {
	case 0: // VM Sj
		if (!aDoExecute) {
			aDisassembly << "VM" << RefSj;
			aExplanation << "Set Vector Mask register to " << RefSj;
		}
		if (aDoExecute) {
			RefVM = RefSj;
		}
		return 1;
	case 4: // SMjk 1,TS
		if (!aDoExecute) {
			aDisassembly << "SM" << DecPrinter(int(jk), 0) << " 1,TS";
			aExplanation << "Test and set semaphore SM" << DecPrinter(int(jk), 0);
			return 1;
		}
		else {
			if (mState.Cluster == 0) return 1;
			//				std::unique_lock<std::mutex> Lock(mMainframe.GetCluster(mState.Cluster - 1).ClusterMutex);
			Lock_c Lock(mMainframe.GetCluster(mState.Cluster - 1).ClusterMutex);

			if (jk > 31) throw InstExecError_x() << "Semaphore index " << DecPrinter(jk) << " out of bounds";
			InterCpuCluster_c &Cluster = mMainframe.GetCluster(mState.Cluster - 1);
			aBreakBurst = true;
			if (!Cluster.SM[jk].exchange(true)) {
				//				if ((RefSM & Mask) == 0) {
				if (mState.TestAndSetBlocked) --mMainframe.GetCluster(mState.Cluster - 1).TestAndSetBlockedCnt;
				mState.TestAndSetBlocked = false;
				return 1;
			}
			else {
				if (!mState.TestAndSetBlocked) ++mMainframe.GetCluster(mState.Cluster - 1).TestAndSetBlockedCnt;
				mState.TestAndSetBlocked = true;

				if (Cluster.ClusterCpuCnt == Cluster.TestAndSetBlockedCnt) {
					mLogger << setloglevel(LogLevel_Interrupt) << "DEADLOCK interrupt fired on processor " << DecPrinter(mCpuId) << std::endl;
					std::cout << "DEADLOCK interrupt fired on processor " << DecPrinter(mCpuId) << std::endl;
					// We interrupt all CPUs in this cluster: we're in a test-and-set loop
					//						mMainframe.HandleDeadLock(mState.Cluster);
				}
				return 0;
			}
		}
		CRAY_ASSERT(false);
	case 6: // SMjk 0
		if (!aDoExecute) {
			aDisassembly << "SM" << DecPrinter(int(jk), 0) << " 0";
			aExplanation << "Clear semaphore SM" << jk;
		}
		if (aDoExecute) {
			if (mState.Cluster > 0) {
				if (jk > 31) throw InstExecError_x() << "Semaphore index " << DecPrinter(jk) << " out of bounds";
				InterCpuCluster_c &Cluster = mMainframe.GetCluster(mState.Cluster - 1);
				Cluster.SM[jk] = false;
				aBreakBurst = true;
			}
		}
		return 1;
	case 7: // SMjk 1
		if (!aDoExecute) {
			aDisassembly << "SM" << DecPrinter(int(jk), 0) << " 1";
			aExplanation << "Set semaphore SM" << jk;
		}
		if (aDoExecute) {
			if (mState.Cluster > 0) {
				if (jk > 31) throw InstExecError_x() << "Semaphore index " << DecPrinter(jk) << " out of bounds";
				InterCpuCluster_c &Cluster = mMainframe.GetCluster(mState.Cluster - 1);
				Cluster.SM[jk] = true;
				aBreakBurst = true;
			}
		}
		return 1;
	default: CRAY_UNKNOWN
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0004(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0004);

	if (!aDoExecute) {
		aDisassembly << "EX " << DecPrinter(int(ijk), 0);
	}
	if (aDoExecute) {
		// The actual interrupt handling will handle in the next Tick call, where P will have already advanced. Which is the expected behavior per HR-0097 5-8
		// TODO: who clears these bits? This gets written out in the EP before monitor mode takes effect, but it must be cleared in memory before its read back in. Or is it that when the CPU reads the EP back, it ignores this (and maybe other) mode-bits?
		mState.Flags.SetNormalExit();
		aBreakBurst = true;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0005(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0005);
	// J Bjk

	if (i != 0) CRAY_UNKNOWN
		if (!aDoExecute) {
			aDisassembly << "J " << RefBjk;
		}
	if (aDoExecute) {
		mState.ProgramAddress = CProgramAddr_t(RefBjk);
		// Indirect jumps are used to implement 'computed goto'-s as well as call-stack returns. So make sure we actually are returning to the previous context before popping the stack
		bool IsReturn = false;
		if (mCallStack.size() > 0) {
			CpuState_s &LastState = mCallStack.back();
			if (LastState.ProgramAddress == CProgramAddr_t(RefBjk)) {
				mCallStack.pop_back();
				IsReturn = true;
			}
		}
		UpdateJumpStats(true);
		if (IsReturn || (jk == 0)) {
			mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ RET ==========================" << std::endl;
			//					mLogger.UnIndent();
		}
		else {
			mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ JUMP =========================" << std::endl;
		}
		return 0;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0006(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0006);
	// J exp

	CProgramAddr_t Target = CProgramAddr_t(ijkm & ((1 << 24) - 1)); // manual says to ignore MSB even on YMP class
	if (!aDoExecute) {
		aDisassembly << "J " << InstAddr(Target);
	}
	if (aDoExecute) {
		mState.ProgramAddress = CProgramAddr_t(Target);
		UpdateJumpStats(true);
		mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ JUMP =========================" << std::endl;
		return 0;
	}
	return 2;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0007(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0007);
	// R exp

	CProgramAddr_t Target = CProgramAddr_t(ijkm & ((1 << 24) - 1)); // manual says to ignore MSB even on YMP class
	if (!aDoExecute) {
		aDisassembly << "R " << InstAddr(Target);
	}
	if (aDoExecute) {
		UpdateJumpStats(true);
		mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ CALL =========================" << std::endl;
		mState.ProgramAddress = mState.ProgramAddress + CProgramAddr_t(2);
		RefB(0) = mState.ProgramAddress;
		mCallStack.push_back(mState);
		mState.ProgramAddress = CAddr_t(Target);
		return 0;
	}
	return 2;
}

template <bool aDoExecute> size_t SoftCpu_c::Decode0010(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0010);
	// JAZ exp

	if (!aDoExecute) {
		if ((i & 4) == 0) {
			aDisassembly << "JAZ " << InstAddr(CAddr_t(ijkm));
			aExplanation << "Jump if A0 == 0";
		}
		else {
			aDisassembly << RefAhTarget << " " << HexPrinter(CXmpAddr_t(ijkm));
			if (!IsXMode()) aExplanation << "******* X-mode ONLY ********";
		}
	}
	if (aDoExecute) {
		if ((i & 4) == 0) {
			if (RefA0 == CAddr_t(0)) {
				mState.ProgramAddress = CAddr_t(ijkm);
				UpdateJumpStats(true);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ JUMP =========================" << std::endl;
				return 0;
			}
			else {
				UpdateJumpStats(false);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "----------------------------- NO JUMP -------------------------" << std::endl;
			}
		}
		else {
			if (IsXMode()) {
				RefAhTarget = CAddr_t(ijkm & ((1 << 24) - 1));
			}
		}
	}
	return 2;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0011(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0011);
	// JAN exp

	if (!aDoExecute) {
		if ((i & 4) == 0) {
			aDisassembly << "JAN " << InstAddr(CAddr_t(ijkm));
			aExplanation << "Jump if A0 != 0";
		}
		else {
			aDisassembly << RefAhTarget << " " << HexPrinter(CXmpAddr_t(ijkm));
			if (!IsXMode()) aExplanation << "******* X-mode ONLY ********";
		}
	}
	if (aDoExecute) {
		if ((i & 4) == 0) {
			if (RefA0 != CAddr_t(0)) {
				mState.ProgramAddress = CAddr_t(ijkm);
				UpdateJumpStats(true);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ JUMP =========================" << std::endl;
				return 0;
			}
			else {
				UpdateJumpStats(false);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "----------------------------- NO JUMP -------------------------" << std::endl;
			}
		}
		else {
			if (IsXMode()) {
				RefAhTarget = CAddr_t(ijkm & ((1 << 24) - 1));
			}
		}
	}
	return 2;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0012(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0012);
	// JAP exp

	if (!aDoExecute) {
		if ((i & 4) == 0) {
			aDisassembly << "JAP " << InstAddr(CAddr_t(ijkm));
			aExplanation << "Jump if A0 >= 0";
		}
		else {
			aDisassembly << RefAhTarget << " " << HexPrinter(CXmpAddr_t(ijkm));
			if (!IsXMode()) aExplanation << "******* X-mode ONLY ********";
		}
	}
	if (aDoExecute) {
		if ((i & 4) == 0) {
			if (SignExtend(CAddr_t(RefA0)) >= 0) {
				mState.ProgramAddress = CAddr_t(ijkm);
				UpdateJumpStats(true);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ JUMP =========================" << std::endl;
				return 0;
			}
			else {
				UpdateJumpStats(false);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "----------------------------- NO JUMP -------------------------" << std::endl;
			}
		}
		else {
			if (IsXMode()) {
				RefAhTarget = CAddr_t(ijkm & ((1 << 24) - 1));
			}
		}
	}
	return 2;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0013(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0013);
	// JAM exp

	if (!aDoExecute) {
		if ((i & 4) == 0) {
			aDisassembly << "JAM " << InstAddr(CAddr_t(ijkm));
			aExplanation << "Jump if A0 < 0";
		}
		else {
			aDisassembly << RefAhTarget << " " << HexPrinter(CXmpAddr_t(ijkm));
			if (!IsXMode()) aExplanation << "******* X-mode ONLY ********";
		}
	}
	if (aDoExecute) {
		if ((i & 4) == 0) {
			if (SignExtend(CAddr_t(RefA0)) < 0) {
				mState.ProgramAddress = CAddr_t(ijkm);
				UpdateJumpStats(true);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ JUMP =========================" << std::endl;
				return 0;
			}
			else {
				UpdateJumpStats(false);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "----------------------------- NO JUMP -------------------------" << std::endl;
			}
		}
		else {
			if (IsXMode()) {
				RefAhTarget = CAddr_t(ijkm & ((1 << 24) - 1));
			}
		}
	}
	return 2;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0014(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0014);
	// JSZ exp

	if (!aDoExecute) {
		if ((i & 4) == 0) {
			aDisassembly << "JSZ " << InstAddr(CAddr_t(ijkm));
			aExplanation << "Jump if S0 == 0";
		}
		else {
			aDisassembly << RefAhTarget << " " << HexPrinter(CXmpAddr_t(ijkm));
			if (!IsXMode()) aExplanation << "******* X-mode ONLY ********";
		}
	}
	if (aDoExecute) {
		if ((i & 4) == 0) {
			if (CInt_t(RefS0) == 0ULL) {
				mState.ProgramAddress = CAddr_t(ijkm);
				UpdateJumpStats(true);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ JUMP =========================" << std::endl;
				return 0;
			}
			else {
				UpdateJumpStats(false);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "----------------------------- NO JUMP -------------------------" << std::endl;
			}
		}
		else {
			if (IsXMode()) {
				RefAhTarget = CAddr_t(ijkm & ((1 << 24) - 1));
			}
		}
	}
	return 2;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0015(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0015);
	// JSN exp

	if (!aDoExecute) {
		if ((i & 4) == 0) {
			aDisassembly << "JSN " << InstAddr(CAddr_t(ijkm));
			aExplanation << "Jump if S0 != 0";
		}
		else {
			aDisassembly << RefAhTarget << " " << HexPrinter(CXmpAddr_t(ijkm));
			if (!IsXMode()) aExplanation << "******* X-mode ONLY ********";
		}
	}
	if (aDoExecute) {
		if ((i & 4) == 0) {
			if (CInt_t(RefS0) != 0ULL) {
				mState.ProgramAddress = CAddr_t(ijkm);
				UpdateJumpStats(true);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ JUMP =========================" << std::endl;
				return 0;
			}
			else {
				UpdateJumpStats(false);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "----------------------------- NO JUMP -------------------------" << std::endl;
			}
		}
		else {
			if (IsXMode()) {
				RefAhTarget = CAddr_t(ijkm & ((1 << 24) - 1));
			}
		}
	}
	return 2;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0016(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0016);
	// JSP exp

	if (!aDoExecute) {
		if ((i & 4) == 0) {
			aDisassembly << "JSP " << InstAddr(CAddr_t(ijkm));
			aExplanation << "Jump if S0 >= 0";
		}
		else {
			aDisassembly << RefAhTarget << " " << HexPrinter(CXmpAddr_t(ijkm));
			if (!IsXMode()) aExplanation << "******* X-mode ONLY ********";
		}
	}
	if (aDoExecute) {
		if ((i & 4) == 0) {
			if (int64_t(CInt_t(RefS0)) >= 0) {
				mState.ProgramAddress = CAddr_t(ijkm);
				UpdateJumpStats(true);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ JUMP =========================" << std::endl;
				return 0;
			}
			else {
				UpdateJumpStats(false);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "----------------------------- NO JUMP -------------------------" << std::endl;
			}
		}
		else {
			if (IsXMode()) {
				RefAhTarget = CAddr_t(ijkm & ((1 << 24) - 1));
			}
		}
	}
	return 2;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0017(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0017);
	// JSM exp

	if (!aDoExecute) {
		if ((i & 4) == 0) {
			aDisassembly << "JSM " << InstAddr(CAddr_t(ijkm));
			aExplanation << "Jump if S0 < 0";
		}
		else {
			aDisassembly << RefAhTarget << " " << HexPrinter(CXmpAddr_t(ijkm));
			if (!IsXMode()) aExplanation << "******* X-mode ONLY ********";
		}
	}
	if (aDoExecute) {
		if ((i & 4) == 0) {
			if (int64_t(CInt_t(RefS0)) < 0) {
				mState.ProgramAddress = CAddr_t(ijkm);
				UpdateJumpStats(true);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "================================ JUMP =========================" << std::endl;
				return 0;
			}
			else {
				UpdateJumpStats(false);
				mLogger << setloglevel(LogLevel_InstructionTrace) << "----------------------------- NO JUMP -------------------------" << std::endl;
			}
		}
		else {
			if (IsXMode()) {
				RefAhTarget = CAddr_t(ijkm & ((1 << 24) - 1));
			}
		}
	}
	return 2;
}

template <bool aDoExecute> size_t SoftCpu_c::Decode0020(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0020);
	// Ai exp
	if (IsXMode()) {
		CAddr_t Value = CAddr_t(uint32_t(jkm));
		if (!aDoExecute) {
			aDisassembly << RefAiTarget << " " << Addr(Value);
		}
		if (aDoExecute) {
			RefAiTarget = Value;
		}
		return 2;
	}
	else {
		CAddr_t Value = CAddr_t(mn);
		if (!aDoExecute) {
			aDisassembly << RefAiTarget << " " << Addr(Value);
		}
		if (aDoExecute) {
			CRAY_ASSERT(jk == 000);
			RefAiTarget = Value;
		}
		return 3;
	}
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0021(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0021);
	// Ai -exp
	if (IsXMode()) {
		CAddr_t Value = ~CAddr_t(uint32_t(jkm));
		if (!aDoExecute) {
			aDisassembly << RefAiTarget << " " << Addr(Value);
		}
		if (aDoExecute) {
			RefAiTarget = Value;
		}
		return 2;
	}
	else {
		CAddr_t Value = ~CAddr_t(mn);
		if (!aDoExecute) {
			aDisassembly << RefAiTarget << " " << Addr(Value);
		}
		if (aDoExecute) {
			CRAY_ASSERT(jk == 000);
			RefAiTarget = Value;
		}
		return 3;
	}
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0022(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0022);
	// Ai exp

	if (!aDoExecute) {
		aDisassembly << RefAiTarget << " " << Addr(CAddr_t(jk));
	}
	if (aDoExecute) {
		RefAi = CAddr_t(jk);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0023(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0023);
	// multiple
	switch (k) {

	case 0: // Ai Sj
		if (!aDoExecute) {
			aDisassembly << RefAiTarget << " " << RefSj;
		}
		if (aDoExecute) {
			RefAi = CAddr_t(CInt_t(RefSj));
		}
		return 1;
	case 1: // Ai VL
		if (!aDoExecute) {
			aDisassembly << RefAiTarget << " VL";
			aExplanation << "Read vector length register";
		}
		if (aDoExecute) {
			RefAi = CAddr_t(RefVL);
		}
		return 1;
	default:
		aDisassembly << "*** UNK *** - 0023ij" << DecPrinter(k, 1) << " overridded to PASS";
		aExplanation << "Undocumented instruction. Skipping it for now.";
		if (aDoExecute) {
			std::cout << "*** UNK *** - 0023ij" << DecPrinter(k, 1) << " overridded to PASS" << std::endl;
		}
		return 1;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0024(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0024);
	// Ai Bjk

	if (!aDoExecute) {
		aDisassembly << RefAiTarget << " " << RefBjk;
	}
	if (aDoExecute) {
		RefAi = CAddr_t(RefBjk);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0025(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0025);
	// Bjk Ai

	if (!aDoExecute) {
		aDisassembly << RefBjk << " " << RefAi;
	}
	if (aDoExecute) {
		RefBjk = CAddr_t(RefAi);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0026(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0026);
	switch (k) {
	case 0: // Ai PSj
		if (!aDoExecute) {
			aDisassembly << RefAiTarget << " P" << RefSj;
			aExplanation << RefAiTarget << " = population count of " << RefSj;
		}
		if (aDoExecute) {
			RefAi = PopulationCnt(RefSj);
		}
		return 1;
	case 1: // Ai QSj
		if (!aDoExecute) {
			aDisassembly << RefAiTarget << " Q" << RefSj;
			aExplanation << RefAiTarget << " = population count parity of " << RefSj;
		}
		if (aDoExecute) {
			RefAi = PopulationCntParity(RefSj);
		}
		return 1;
	case 7: // Ai SBj
		if (!aDoExecute) {
			aDisassembly << RefAiTarget << " " << RefSBj;
			aExplanation << "Load shared address register " << RefSBj << " to " << RefAiTarget;
		}
		if (aDoExecute) {
			if (mState.Cluster == 0) {
				RefAi = CAddr_t(0);
			}
			else {
				RefAi = RefSBj;
			}
		}
		return 1;
	default: CRAY_UNKNOWN
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0027(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0027);
	switch (k) {
	case 0: // Ai ZSj
		if (!aDoExecute) {
			aDisassembly << RefAiTarget << " Z" << RefSj;
			aExplanation << RefAiTarget << " = leading zero count of " << RefSj;
		}
		if (aDoExecute) {
			RefAi = LeadingZeroCnt(RefSj);
		}
		return 1;
	case 7: // SBj Ai
		if (!aDoExecute) {
			aDisassembly << RefSBj << " " << RefAi;
			aExplanation << "Load shared address register " << RefSBj << " with " << RefAi;
		}
		if (aDoExecute) {
			if (mState.Cluster != 0) {
				RefSBj = CAddr_t(RefAi);
			}
		}
		return 1;
	default: CRAY_UNKNOWN
	}
	return 1;
}

template <bool aDoExecute> size_t SoftCpu_c::Decode0030(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0030);
	// Ai Aj+Ak

	if (!aDoExecute) {
		if (j == 0) {
			if (k == 0) {
				aDisassembly << RefAiTarget << " 1";
			}
			else {
				aDisassembly << RefAiTarget << " " << RefAk;
			}
		}
		else {
			if (k == 0) {
				aDisassembly << RefAiTarget << " " << RefAj << "+1";
			}
			else {
				aDisassembly << RefAiTarget << " " << RefAj << "+" << RefAk;
			}
		}
	}
	if (aDoExecute) {
		RefAi = CAddr_t(RefAj) + CAddr_t(RefAk);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0031(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0031);
	// Ai Aj-Ak

	if (!aDoExecute) {
		if (j == 0) {
			if (k == 0) {
				aDisassembly << RefAiTarget << " -1";
			}
			else {
				aDisassembly << RefAiTarget << " -" << RefAk;
			}
		}
		else {
			if (k == 0) {
				aDisassembly << RefAiTarget << " " << RefAj << "-1";
			}
			else {
				aDisassembly << RefAiTarget << " " << RefAj << "-" << RefAk;
			}
		}
	}
	if (aDoExecute) {
		RefAi = CAddr_t(RefAj) - CAddr_t(RefAk);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0032(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0032);
	// Ai Aj*Ak

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefAiTarget << " 0";
		}
		else {
			if (k == 0) {
				aDisassembly << RefAiTarget << " " << RefAj;
			}
			else {
				aDisassembly << RefAiTarget << " " << RefAj << "*" << RefAk;
			}
		}
	}
	if (aDoExecute) {
		RefAi = CAddr_t((int(CAddr_t(RefAj)) * int(CAddr_t(RefAk)))); //TODO: do we do a signed multiply here?
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0033(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0033);
	if (j == 0) { // Ai CI

		if (!aDoExecute) {
			aDisassembly << RefAiTarget << " CI";
			aExplanation << RefAiTarget << " = channel number with the highest priority active interrupt request";
		}
		if (aDoExecute) {
			size_t ChannelIdx = 0;
			for (auto &Channel : mMainframe.GetChannels()) {
				if (Channel != nullptr && Channel->GetInterruptPending()) {
					ChannelIdx = Channel->GetChannelId();
					break;
				}
			}
			RefAi = CAddr_t(ChannelIdx);
			/*					size_t ChIdx;
			for(ChIdx=0;ChIdx<mMainframe.GetChannelCnt();++ChIdx) {
			if (mMainframe.GetChannel(ChIdx).GetInterruptPending()) {
			RefAi = CAddr_t(ChIdx+8);
			break;
			}
			}
			if (ChIdx == mMainframe.GetChannelCnt()) {
			RefAi = CAddr_t(0);
			//throw InstExecError_x(); //TODO: What to return if no interrupt was requested?
			}*/
			aBreakBurst = true;
		}
		return 1;
	}
	else {

		switch (k) {
		case 0: // Ai CA,Aj
			if (!aDoExecute) {
				aDisassembly << RefAiTarget << " CA," << RefAj;
				aExplanation << RefAiTarget << " = channel address register for channel " << RefAj;
			}
			if (aDoExecute) {
				size_t ChIdx = size_t(RefAj - CAddr_t(8));
				mLogger << setloglevel(LogLevel_IoActivity) << SideEffectIndent << "CA" << " (ChIdx = " << DecPrinter(ChIdx + 8) << ")" << std::endl;
				if (ChIdx >= mMainframe.GetChannelCnt()) throw InstExecError_x() << "Channel index " << DecPrinter(ChIdx + 8) << " out of bounds";
				auto Channel = mMainframe.GetChannel(ChIdx);
				RefAi = (Channel != nullptr) ? Channel->GetAddress() : CAddr_t(0);
				aBreakBurst = true;
			}
			return 1;
		case 1: // Ai CE,Aj
			if (!aDoExecute) {
				aDisassembly << RefAiTarget << " CE," << RefAj;
				aExplanation << RefAiTarget << " = channel error flag for channel " << RefAj;
			}
			if (aDoExecute) {
				size_t ChIdx = size_t(RefAj - CAddr_t(8));
				mLogger << setloglevel(LogLevel_IoActivity) << SideEffectIndent << "CE" << " (ChIdx = " << DecPrinter(ChIdx + 8) << ")" << std::endl;
				if (ChIdx >= mMainframe.GetChannelCnt()) throw InstExecError_x() << "Channel index " << DecPrinter(ChIdx + 8) << " out of bounds";
				auto Channel = mMainframe.GetChannel(ChIdx);
				RefAi = CAddr_t(Channel != nullptr ? (Channel->GetError() ? 1 : 0) : 0);
				aBreakBurst = true;
			}
			return 1;
		default: CRAY_UNKNOWN
		}
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0034(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0034);
	// Bjk,Ai, A0

	if (!aDoExecute) {
		aDisassembly << RefBjk << "," << RefAi << " [,A0]";
		aExplanation << "Transfer " << RefAi << " words from memory at address A0 to B registers starting from " << RefBjk;
	}
	if (aDoExecute) {
		size_t TransferSize = size_t(RefAi & CAddr_t(127));
		if (TransferSize > 64) {
			mLogger << setloglevel(LogLevel_Warning) << "Wrap-around in block transfer" << std::endl;
		}
		if (TransferSize != 0) {
			size_t BIdx = jk;
			for (size_t Idx = 0; Idx < TransferSize; ++Idx) {
				RefB(BIdx) = CAddr_t(ReadDataMem(RefA0 + CAddr_t(Idx)));
				++BIdx;
				if (BIdx >= sizeof(mState.B) / sizeof(mState.B[0])) BIdx = 0;
			}
		}
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0035(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0035);
	// ,A0 Bjk,Ai

	if (!aDoExecute) {
		aDisassembly << "[,A0] " << RefBjk << "," << RefAi;
		aExplanation << "Transfer " << RefAi << " words from B registers starting from " << RefBjk << " to memory at address A0";
	}
	if (aDoExecute) {
		size_t TransferSize = size_t(RefAi & CAddr_t(127));
		if (TransferSize > 64) {
			mLogger << setloglevel(LogLevel_Warning) << "Wrap-around in block transfer" << std::endl;
		}
		if (TransferSize != 0) {
			size_t BIdx = jk;
			for (size_t Idx = 0; Idx < TransferSize; ++Idx) {
				WriteDataMem(RefA0 + CAddr_t(Idx), CAddr_t(RefB(BIdx)));
				++BIdx;
				if (BIdx >= sizeof(mState.B) / sizeof(mState.B[0])) BIdx = 0;
			}
		}
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0036(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0036);
	// Tjk,Ai, A0

	if (!aDoExecute) {
		aDisassembly << RefTjk << "," << RefAi << " [,A0]";
		aExplanation << "Transfer " << RefAi << " words from memory at address A0 to T registers starting from " << RefTjk;
	}
	if (aDoExecute) {
		size_t TransferSize = size_t(RefAi & CAddr_t(127));
		if (TransferSize > 64) {
			mLogger << setloglevel(LogLevel_Warning) << "Wrap-around in block transfer" << std::endl;
		}
		if (TransferSize != 0) {
			size_t TIdx = jk;
			for (size_t Idx = 0; Idx < TransferSize; ++Idx) {
				RefT(TIdx) = ReadDataMem(RefA0 + CAddr_t(Idx));
				++TIdx;
				if (TIdx >= sizeof(mState.T) / sizeof(mState.T[0])) TIdx = 0;
			}
		}
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0037(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0037);
	// ,A0 Tjk,Ai

	if (!aDoExecute) {
		aDisassembly << "[,A0] " << RefTjk << "," << RefAi;
		aExplanation << "Transfer " << RefAi << " words from T registers starting from " << RefTjk << " to memory at address A0";
	}
	if (aDoExecute) {
		size_t TransferSize = size_t(RefAi & CAddr_t(127));
		if (TransferSize > 64) {
			mLogger << setloglevel(LogLevel_Warning) << "Wrap-around in block transfer" << std::endl;
		}
		if (TransferSize != 0) {
			size_t TIdx = jk;
			for (size_t Idx = 0; Idx < TransferSize; ++Idx) {
				WriteDataMem(RefA0 + CAddr_t(Idx), RefT(TIdx));
				++TIdx;
				if (TIdx >= sizeof(mState.T) / sizeof(mState.T[0])) TIdx = 0;
			}
		}
	}
	return 1;
}

