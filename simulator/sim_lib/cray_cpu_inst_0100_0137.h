// Cray-1S CPU simulator class

// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
//
// To be true to the documentation, octal numbers are extensively used in this file.
// Watch out for the leading 0-s!
//
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!

template <bool aDoExecute> size_t SoftCpu_c::Decode010x(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT((gh & 0170) == 0100);
	// Ai exp,Ah

	CAddr_t Offset;
	if (IsXMode()) Offset = SignExtend(SignExtendShort(CXmpAddr_t(jkm))); else
		Offset = CAddr_t(mn);
	if (!aDoExecute) {
		if (h == 0) {
			aDisassembly << RefAiTarget << " [" << Addr(Offset) << ",]";
		}
		else {
			aDisassembly << RefAiTarget << " [" << Addr(Offset) << "," << RefAh << "]";
		}
	}
	if (aDoExecute) {
		CRAY_ASSERT(IsXMode() || jk == 000);
		CAddr_t Addr = CAddr_t(RefAh) + Offset;
		if (IsXmp()) {
			if (!mState.XmpEnhancedAddressingMode) Addr = ApplyShortMask(CXmpAddr_t(Addr));
		}
		RefAi = CAddr_t(ReadDataMem(Addr));
	}
	return IsXMode() ? 2 : 3;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode011x(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT((gh & 0170) == 0110);
	// exp,Ah Ai

	CAddr_t Offset;
	if (IsXMode()) Offset = SignExtend(SignExtendShort(CXmpAddr_t(jkm))); else
		Offset = CAddr_t(mn);
	if (!aDoExecute) {
		if (h == 0) {
			aDisassembly << "[" << Addr(Offset) << ",] " << RefAi;
		}
		else {
			aDisassembly << "[" << Addr(Offset) << "," << RefAh << "] " << RefAi;
		}
	}
	if (aDoExecute) {
		CRAY_ASSERT(IsXMode() || jk == 000);
		CAddr_t Addr = CAddr_t(RefAh) + Offset;
		if (IsXmp()) {
			if (!mState.XmpEnhancedAddressingMode) Addr = ApplyShortMask(CXmpAddr_t(Addr));
		}
		WriteDataMem(Addr, CAddr_t(RefAi));
	}
	return IsXMode() ? 2 : 3;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode012x(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT((gh & 0170) == 0120);
	// Si exp,Ah

	CAddr_t Offset;
	if (IsXMode()) Offset = SignExtend(SignExtendShort(CXmpAddr_t(jkm))); else
		Offset = CAddr_t(mn);
	if (!aDoExecute) {
		if (h == 0) {
			aDisassembly << RefSiTarget << " [" << Addr(Offset) << ",]";
		}
		else {
			aDisassembly << RefSiTarget << " [" << Addr(Offset) << "," << RefAh << "]";
		}
	}
	if (aDoExecute) {
		CRAY_ASSERT(IsXMode() || jk == 000);
		CAddr_t Addr = CAddr_t(RefAh) + Offset;
		if (IsXmp()) {
			if (!mState.XmpEnhancedAddressingMode) Addr = ApplyShortMask(CXmpAddr_t(Addr));
		}
		RefSi = ReadDataMem(Addr);
	}
	return IsXMode() ? 2 : 3;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode013x(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT((gh & 0170) == 0130);
	// exp,Ah Si

	CAddr_t Offset;
	if (IsXMode()) Offset = SignExtend(SignExtendShort(CXmpAddr_t(jkm))); else
		Offset = CAddr_t(mn);
	if (!aDoExecute) {
		if (h == 0) {
			aDisassembly << "[" << Addr(Offset) << ",] " << RefSi;
		}
		else {
			aDisassembly << "[" << Addr(Offset) << "," << RefAh << "] " << RefSi;
		}
	}
	if (aDoExecute) {
		CRAY_ASSERT(IsXMode() || jk == 000);
		CAddr_t Addr = CAddr_t(RefAh) + Offset;
		if (IsXmp()) {
			if (!mState.XmpEnhancedAddressingMode) Addr = ApplyShortMask(CXmpAddr_t(Addr));
		}
		WriteDataMem(Addr, RefSi);
	}
	return IsXMode() ? 2 : 3;
}
