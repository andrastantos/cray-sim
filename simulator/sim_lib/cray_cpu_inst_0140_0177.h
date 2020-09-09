// Cray-1S CPU simulator class

// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
//
// To be true to the documentation, octal numbers are extensively used in this file.
// Watch out for the leading 0-s!
//
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!

static uint64_t IntMult(const uint64_t aScalar, const uint64_t aVector) {
	uint64_t S0, S1, S2, S3, S4, S5, S6;
	uint32_t A6, A0;
	S1 = aScalar;
	S2 = aVector;
	S5 = (1ull << 48) - 1;
	S3 = S1 & S5;
	S4 = S2 & S5;
	S5 = 9;
	S4 = S4 << 15;
	S3 = S3 << 7;
	do {
		S0 = S4 << 1;
		S4 = S4 << 1;
		if (int64_t(S0) < 0) {
			S5 = S5 + S3;
		}
		S3 = S3 >> 1;
	} while (S0 != 0);
	S5 = S5 >> 7;
	S3 = S1;
	S4 = S2;
	S3 = S3 >> 48;
	S4 = S4 >> 16;
	S6 = (1ull << 15) - 1;
	S3 = S3 & S6;
	S4 = S4 & S6;
	A6 = 15;
	S6 = 0;
	do {
		S0 = S4 >> 1;
		S0 = S4 << 63;
		S4 = S4 >> 1;
		if (int64_t(S0) < 0) {
			S6 = S6 + S3;
		}
		S3 = S3 << 1;
		A0 = A6 - 1;
		A6 = A6 - 1;
	} while (A0 != 0);
	S6 = S6 << 17;
	S4 = S5 + S6;
	S3 = (1ull << 32) - 1;
	S4 = S4 & S3;
	return S4;
}


template <bool aDoExecute> size_t SoftCpu_c::Decode0140(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0140); // Vi Sj&Vk

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefSj << "&" << RefVk;
		aExplanation << RefViTarget << " = " << RefSj << " and " << RefVk;
	}
	if (aDoExecute) {
		RefVk.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = RefSj & mState.V[k][Idx];
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0141(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0141); // Vi Vj&Vk

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "&" << RefVk;
		aExplanation << RefViTarget << " = " << RefVj << " and " << RefVk;
	}
	if (aDoExecute) {
		RefVj.LogState("input");
		RefVk.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = mState.V[j][Idx] & mState.V[k][Idx];
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0142(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0142); // Vi Sj!Vk (or)

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefSj << "!" << RefVk;
		aExplanation << RefViTarget << " = " << RefSj << " or " << RefVk;
	}
	if (aDoExecute) {
		RefVk.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = RefSj | mState.V[k][Idx];
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0143(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0143); // Vi Vj!Vk (or)

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "!" << RefVk;
		aExplanation << RefViTarget << " = " << RefVj << " or " << RefVk;
	}
	if (aDoExecute) {
		RefVj.LogState("input");
		RefVk.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = mState.V[j][Idx] | mState.V[k][Idx];
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0144(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0144); // Vi Sj\Vk (xor)

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefSj << "\\" << RefVk;
		aExplanation << RefViTarget << " = " << RefSj << " xor " << RefVk;
	}
	if (aDoExecute) {
		RefVk.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = RefSj ^ mState.V[k][Idx];
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0145(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0145); // Vi Vj\Vk (xor)

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "\\" << RefVk;
		aExplanation << RefViTarget << " = " << RefVj << " xor " << RefVk;
	}
	if (aDoExecute) {
		RefVj.LogState("input");
		RefVk.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = mState.V[j][Idx] ^ mState.V[k][Idx];
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0146(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0146); // Vi Sj!Vk&VM (vector merge)

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefSj << "!" << RefVk << "&" << "VM";
		aExplanation << "select " << RefSj << " if VM bit=1 " << RefVk << " otherwise";
	}
	if (aDoExecute) {
		CInt_t Mask = 1ULL << 63;
		RefVk.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			if ((RefVM & Mask) != 0) {
				mState.V[i][Idx] = RefSj;
			}
			else {
				mState.V[i][Idx] = mState.V[k][Idx];
			}
			Mask >>= 1;
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0147(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0147); // Vi Vj!Vk&VM (vector merge)

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "!" << RefVk << "&" << "VM";
		aExplanation << "select " << RefVj << " if VM bit=1 " << RefVk << " otherwise";
	}
	if (aDoExecute) {
		RefVj.LogState("input");
		RefVk.LogState("input");
		CInt_t Mask = 1ULL << 63;
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			if ((RefVM & Mask) != 0) {
				mState.V[i][Idx] = mState.V[j][Idx];
			}
			else {
				mState.V[i][Idx] = mState.V[k][Idx];
			}
			Mask >>= 1;
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}

template <bool aDoExecute> size_t SoftCpu_c::Decode0150(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0150); // Vi Vj < Ak

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "<" << RefAk;
	}
	if (aDoExecute) {
		RefVj.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = ShiftLeft(mState.V[j][Idx], RefAk);
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0151(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0151); // Vi Vj > Ak

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << ">" << RefAk;
	}
	if (aDoExecute) {
		RefVj.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = ShiftRight(mState.V[j][Idx], RefAk);
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0152(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0152); // Vi Vj,Vj < Ak

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "," << RefVj << "<" << RefAk;
	}
	if (aDoExecute) {
		RefVj.LogState("input");
		CRAY_ASSERT(mState.VL != 0);
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < size_t(mState.VL - 1); ++Idx) {
			mState.V[i][Idx] = DoubleShiftLeft(mState.V[j][Idx], mState.V[j][Idx + 1], RefAk);
		}
		mState.V[i][mState.VL - 1] = DoubleShiftLeft(mState.V[j][mState.VL - 1], CInt_t(0), RefAk);
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0153(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0153); // Vi Vj,Vj > Ak

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "," << RefVj << ">" << RefAk;
	}
	if (aDoExecute) {
		RefVj.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		mState.V[i][0] = DoubleShiftRight(CInt_t(0), mState.V[j][0], RefAk);
		for (size_t Idx = 1; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = DoubleShiftRight(mState.V[j][Idx - 1], mState.V[j][Idx], RefAk);
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0154(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0154); // Vi Sj+Vk

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefViTarget << " " << RefVk;
		}
		else {
			aDisassembly << RefViTarget << " " << RefSj << "+" << RefVk;
		}
	}
	if (aDoExecute) {
		RefVk.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(RefSj) + mState.V[k][Idx];
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0155(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0155); // Vi Vj+Vk

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "+" << RefVk;
	}
	if (aDoExecute) {
		RefVj.LogState("input");
		RefVk.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(mState.V[j][Idx]) + mState.V[k][Idx];
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0156(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0156); // Vi Sj-Vk

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefViTarget << " -" << RefVk;
		}
		else {
			aDisassembly << RefViTarget << " " << RefSj << "-" << RefVk;
		}
	}
	if (aDoExecute) {
		RefVk.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(RefSj) - mState.V[k][Idx];
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0157(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0157); // Vi Vj-Vk

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "-" << RefVk;
	}
	if (aDoExecute) {
		RefVj.LogState("input");
		RefVk.LogState("input");
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(mState.V[j][Idx]) - mState.V[k][Idx];
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}

template <bool aDoExecute> size_t SoftCpu_c::Decode0160(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0160); // Vi Sj*FVk

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefViTarget << " 0";
		}
		else {
			aDisassembly << RefViTarget << " " << RefSj << "*F" << RefVk;
			aExplanation << "Floating-point vector multiplication";
		}
	}
	if (aDoExecute) {
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(CFloat_t(RefSj) * CFloat_t(mState.V[k][Idx]));
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0161(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0161); // Vi Vj*FVk

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "*F" << RefVk;
		aExplanation << "Floating-point vector multiplication";
	}
	if (aDoExecute) {
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(CFloat_t(mState.V[j][Idx]) * CFloat_t(mState.V[k][Idx]));
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0162(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0162); // Vi Sj*HVk

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefViTarget << " 0";
		}
		else {
			aDisassembly << RefViTarget << " " << RefSj << "*H" << RefVk;
			aExplanation << "Half-precision floating-point vector multiplication";
		}
	}
	if (aDoExecute) {
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(CHalfFloat_t(RefSj) * CHalfFloat_t(mState.V[k][Idx]));
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0163(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0163); // Vi Vj*HVk

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "*H" << RefVk;
		aExplanation << "Half-precision floating-point vector multiplication";
	}
	if (aDoExecute) {
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(CHalfFloat_t(mState.V[j][Idx]) * CHalfFloat_t(mState.V[k][Idx]));
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0164(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0164); // Vi Sj*RVk

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefViTarget << " 0";
		}
		else {
			aDisassembly << RefViTarget << " " << RefSj << "*R" << RefVk;
			aExplanation << "Rounded floating-point vector multiplication";
		}
	}
	if (aDoExecute) {
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(RoundedMult(CFloat_t(RefSj), CFloat_t(mState.V[k][Idx])));
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0165(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0165); // Vi Vj*RVk

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "*R" << RefVk;
		aExplanation << "Rounded floating-point vector multiplication";
	}
	if (aDoExecute) {
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(RoundedMult(CFloat_t(mState.V[j][Idx]), CFloat_t(mState.V[k][Idx])));
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0166(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	//This instruction is totally different between the YMP and XMP lines
	EXTRACT_FIELDS(aParcels);

	if (IsXMode()) {
		CRAY_ASSERT(gh == 0166); // Vi Sj*IVk

		if (!aDoExecute) {
			aDisassembly << RefViTarget << " " << RefSj << "*I" << RefVk;
			aExplanation << "Vector reciprocial iteration: " << RefViTarget << " = 2-" << RefSj << "*" << RefVk;
		}
		if (aDoExecute) {
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				mState.V[i][Idx] = CInt_t(ReciprocIterate(CFloat_t(RefSj), CFloat_t(mState.V[k][Idx])));
			}
			RefVi.LogState();
			mState.VectorNotUsed = false;
		}
		return 1;
	}
	else {
		CRAY_ASSERT(gh == 0166); // Vi Sj*Vk

		if (!aDoExecute) {
			aDisassembly << RefViTarget << " " << RefSj << "*" << RefVk;
			aExplanation << "32-bit integer multiplication. Sj is 31-bit left-shited, Vk is 16-bit left-shifted. Result is lower 32-bit only";
		}
		if (aDoExecute) {
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				//mState.V[i][Idx] = ((CInt_t(RefSj) >> 31) * (CInt_t(mState.V[k][Idx]) >> 16)) & 0xffffffffull;
				mState.V[i][Idx] = IntMult(RefSj, mState.V[k][Idx]);
			}
			RefVi.LogState();
			mState.VectorNotUsed = false;
		}
		return 1;
	}
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0167(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0167); // Vi Vj*IVk

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "*I" << RefVk;
		aExplanation << "Vector reciprocial iteration: " << RefViTarget << " = 2-" << RefVj << "*" << RefVk;
	}
	if (aDoExecute) {
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(ReciprocIterate(CFloat_t(mState.V[j][Idx]), CFloat_t(mState.V[k][Idx])));
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}

template <bool aDoExecute> size_t SoftCpu_c::Decode0170(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0170); // Vi Sj+FVk

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefViTarget << " F" << RefVk;
		}
		else {
			aDisassembly << RefViTarget << " " << RefSj << "+F" << RefVk;
		}
		aExplanation << "Vector floating-point add";
	}
	if (aDoExecute) {
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(CFloat_t(RefSj) + CFloat_t(mState.V[k][Idx]));
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0171(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0171); // Vi Vj+FVk

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "+F" << RefVk;
		aExplanation << "Vector floating-point add";
	}
	if (aDoExecute) {
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(CFloat_t(mState.V[j][Idx]) + CFloat_t(mState.V[k][Idx]));
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0172(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0172); // Vi Sj-FVk

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefViTarget << " F" << RefVk;
		}
		else {
			aDisassembly << RefViTarget << " " << RefSj << "-F" << RefVk;
		}
		aExplanation << "Vector floating-point subtract";
	}
	if (aDoExecute) {
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(CFloat_t(RefSj) - CFloat_t(mState.V[k][Idx]));
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0173(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0173); // Vi Vj-FVk

	if (!aDoExecute) {
		aDisassembly << RefViTarget << " " << RefVj << "-F" << RefVk;
		aExplanation << "Vector floating-point subtract";
	}
	if (aDoExecute) {
		CRAY_ASSERT(RefVL <= cVecSize);
		for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
			mState.V[i][Idx] = CInt_t(CFloat_t(mState.V[j][Idx]) - CFloat_t(mState.V[k][Idx]));
		}
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0174(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0174); // multiple
	switch (k) {
	case 0: // Vi /HVj
		if (!aDoExecute) {
			aDisassembly << RefViTarget << " /H" << RefVj;
			aExplanation << "Vector reciprocal";
		}
		if (aDoExecute) {
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				mState.V[i][Idx] = CInt_t(ReciprocApprox(CFloat_t(mState.V[j][Idx])));
			}
			RefVi.LogState();
			mState.VectorNotUsed = false;
		}
		return 1;
	case 1: // Vi PVj
		if (!aDoExecute) {
			aDisassembly << RefViTarget << " P" << RefVj;
			aExplanation << "Vector population count";
		}
		if (aDoExecute) {
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				mState.V[i][Idx] = PopulationCnt(mState.V[j][Idx]);
			}
			RefVi.LogState();
			mState.VectorNotUsed = false;
		}
		return 1;
	case 2: // Vi QVj
		if (!aDoExecute) {
			aDisassembly << RefViTarget << " Q" << RefVj;
			aExplanation << "Vector population count parity";
		}
		if (aDoExecute) {
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				mState.V[i][Idx] = PopulationCntParity(mState.V[j][Idx]);
			}
			RefVi.LogState();
			mState.VectorNotUsed = false;
		}
		return 1;
	case 3: // Vi ZVj
		if (IsJ90OrSV1()) {
			if (!aDoExecute) {
				aDisassembly << RefViTarget << " Z" << RefVj;
				aExplanation << "Leading zero count";
			}
			if (aDoExecute) {
				CRAY_ASSERT(RefVL <= cVecSize);
				for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
					mState.V[i][Idx] = LeadingZeroCnt(mState.V[j][Idx]);
				}
				RefVi.LogState();
				mState.VectorNotUsed = false;
			}
		}
		else CRAY_UNKNOWN
			return 1;
	case 4: // BMM Vj
		if (IsSV1()) {
			if (!aDoExecute) {
				aDisassembly << "BMM " << RefVj;
				aExplanation << "Transfer " << RefVj << " to BMM";
			}
			if (aDoExecute) {
				CRAY_ASSERT(RefVL <= cVecSize);
				for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
					mState.BitMatrix[Idx] = RefVj[Idx];
				}
				RefBM.LogState();
				mState.VectorNotUsed = false;
				mState.Mode.SetSv1BitMatrixLoaded(true);
			}
		}
		else CRAY_UNKNOWN
			return 1;
	case 6: // Vi Vj*BMM^T
		if (IsSV1()) {
			if (!aDoExecute) {
				aDisassembly << RefViTarget << " " << RefVj << " * BMM^T";
				aExplanation << "Bit-matrix product";
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsSv1BitMatrixLoaded());
				CRAY_ASSERT(RefVL <= cVecSize);
				for (int j = 0; j < mState.VL; ++j) {
					CInt_t Result = 0;
					for (int i = 0; i < mState.VL; ++i) {
						Result ^= RefSj & RefBM[i];
					}
					mState.V[i][j] = Result;
				}
				RefVi.LogState();
				mState.VectorNotUsed = false;
			}
		}
		else CRAY_UNKNOWN
			return 1;
	default: CRAY_UNKNOWN
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0175(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0175); // multiple
	switch (k) {
	case 0: // VM Vj,Z
		if (!aDoExecute) {
			aDisassembly << RefVM << " " << RefVj << ",Z";
			aExplanation << "Set vector mask to 1 if " << RefVj << " element == 0";
		}
		if (aDoExecute) {
			CInt_t Mask = 1ULL << 63;
			CInt_t NewVM = 0;
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				NewVM |= (mState.V[j][Idx] == 0) ? Mask : 0;
				Mask >>= 1;
			}
			RefVM = NewVM;
			mState.VectorNotUsed = false;
		}
		return 1;
	case 1: // VM Vj,N
		if (!aDoExecute) {
			aDisassembly << RefVM << " " << RefVj << ",N";
			aExplanation << "Set vector mask to 1 if " << RefVj << " element != 0";
		}
		if (aDoExecute) {
			CInt_t Mask = 1ULL << 63;
			CInt_t NewVM = 0;
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				NewVM |= (mState.V[j][Idx] != 0) ? Mask : 0;
				Mask >>= 1;
			}
			RefVM = NewVM;
			mState.VectorNotUsed = false;
		}
		return 1;
	case 2: // VM Vj,P
		if (!aDoExecute) {
			aDisassembly << RefVM << " " << RefVj << ",P";
			aExplanation << "Set vector mask to 1 if " << RefVj << " element >= 0";
		}
		if (aDoExecute) {
			CInt_t Mask = 1ULL << 63;
			CInt_t NewVM = 0;
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				NewVM |= (int64_t(mState.V[j][Idx]) >= 0) ? Mask : 0;
				Mask >>= 1;
			}
			RefVM = NewVM;
			mState.VectorNotUsed = false;
		}
		return 1;
	case 3: // VM Vj,M
		if (!aDoExecute) {
			aDisassembly << RefVM << " " << RefVj << ",M";
			aExplanation << "Set vector mask to 1 if " << RefVj << " element < 0";
		}
		if (aDoExecute) {
			CInt_t Mask = 1ULL << 63;
			CInt_t NewVM = 0;
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				NewVM |= (int64_t(mState.V[j][Idx]) < 0) ? Mask : 0;
				Mask >>= 1;
			}
			RefVM = NewVM;
			mState.VectorNotUsed = false;
		}
		return 1;
	case 4: // Vi,VM Vj,Z
		if (!aDoExecute) {
			aDisassembly << RefViTarget << "," << RefVM << " " << RefVj << ",Z";
			aExplanation << "Set vector mask to 1 and transfer (compressed) element if " << RefVj << " element == 0";
		}
		if (aDoExecute) {
			CInt_t Mask = 1ULL << 63;
			CInt_t NewVM = 0;
			size_t DstIdx = 0;
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				if (mState.V[j][Idx] == 0) {
					NewVM |= Mask;
					mState.V[i][DstIdx] = Idx;
					++DstIdx;
				}
				Mask >>= 1;
			}
			RefVM = NewVM;
			RefVi.LogState();
			mState.VectorNotUsed = false;
		}
		return 1;
	case 5: // Vi,VM Vj,N
		if (!aDoExecute) {
			aDisassembly << RefViTarget << "," << RefVM << " " << RefVj << ",N";
			aExplanation << "Set vector mask to 1 and transfer (compressed) element if " << RefVj << " element != 0";
		}
		if (aDoExecute) {
			CInt_t Mask = 1ULL << 63;
			CInt_t NewVM = 0;
			size_t DstIdx = 0;
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				if (mState.V[j][Idx] != 0) {
					NewVM |= Mask;
					mState.V[i][DstIdx] = Idx;
					++DstIdx;
				}
				Mask >>= 1;
			}
			RefVM = NewVM;
			RefVi.LogState();
			mState.VectorNotUsed = false;
		}
		return 1;
	case 6: // Vi,VM Vj,P
		if (!aDoExecute) {
			aDisassembly << RefViTarget << "," << RefVM << " " << RefVj << ",P";
			aExplanation << "Set vector mask to 1 and transfer (compressed) element if " << RefVj << " element >= 0";
		}
		if (aDoExecute) {
			CInt_t Mask = 1ULL << 63;
			CInt_t NewVM = 0;
			size_t DstIdx = 0;
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				if (int64_t(mState.V[j][Idx]) >= 0) {
					NewVM |= Mask;
					mState.V[i][DstIdx] = Idx;
					++DstIdx;
				}
				Mask >>= 1;
			}
			RefVM = NewVM;
			RefVi.LogState();
			mState.VectorNotUsed = false;
		}
		return 1;
	case 7: // Vi,VM Vj,M
		if (!aDoExecute) {
			aDisassembly << RefViTarget << "," << RefVM << " " << RefVj << ",M";
			aExplanation << "Set vector mask to 1 and transfer (compressed) element if " << RefVj << " element < 0";
		}
		if (aDoExecute) {
			CInt_t Mask = 1ULL << 63;
			CInt_t NewVM = 0;
			size_t DstIdx = 0;
			CRAY_ASSERT(RefVL <= cVecSize);
			for (size_t Idx = 0; Idx < mState.VL; ++Idx) {
				if (int64_t(mState.V[j][Idx]) < 0) {
					NewVM |= Mask;
					mState.V[i][DstIdx] = Idx;
					++DstIdx;
				}
				Mask >>= 1;
			}
			RefVM = NewVM;
			RefVi.LogState();
			mState.VectorNotUsed = false;
		}
		return 1;
	default: CRAY_UNKNOWN
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0176(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0176);
	switch (j) {
	case 0:
		// Vi ,A0,Ak
		if (!aDoExecute) {
			aDisassembly << RefViTarget << " [,A0," << RefAk << "]";
		}
		if (aDoExecute) {
			const int32_t Increment = SignExtend(CAddr_t(RefAk));
			CRAY_ASSERT(RefVL <= cVecSize);
			for (int32_t Idx = 0; Idx < RefVL; ++Idx) {
				CAddr_t Addr = CAddr_t(RefA0) + (Increment * Idx);
				mState.V[i][Idx] = ReadDataMem(Addr);
			}
			RefVi.LogState();
			mState.VectorNotUsed = false;
		}
		return 1;
	case 1:
		// Vi ,A0,Vk
		if (!aDoExecute) {
			aDisassembly << RefViTarget << " [,A0," << RefVk << "]";
		}
		if (aDoExecute) {
			CRAY_ASSERT(RefVL <= cVecSize);
			for (int32_t Idx = 0; Idx < RefVL; ++Idx) {
				int32_t Offset = int32_t(mState.V[k][Idx]);
				CAddr_t Addr = CAddr_t(RefA0) + Offset;
				mState.V[i][Idx] = ReadDataMem(Addr);
			}
			RefVi.LogState();
			mState.VectorNotUsed = false;
		}
		return 1;
	default:
		CRAY_UNIMPLEMENTED;
	}
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0177(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0177);
	switch (i) {
	case 0:
		// ,A0,Ak Vj
		if (!aDoExecute) {
			aDisassembly << "[,A0," << RefAk << "] " << RefVj;
		}
		if (aDoExecute) {
			const int32_t Increment = SignExtend(CAddr_t(RefAk));
			CRAY_ASSERT(RefVL <= cVecSize);
			for (int32_t Idx = 0; Idx < RefVL; ++Idx) {
				CAddr_t Addr = CAddr_t(RefA0) + (Increment * Idx);
				WriteDataMem(Addr, mState.V[j][Idx]);
			}
		}
		return 1;
	case 1:
		// ,A0,Ak Vj
		if (!aDoExecute) {
			aDisassembly << "[,A0," << RefVk << "] " << RefVj;
		}
		if (aDoExecute) {
			CRAY_ASSERT(RefVL <= cVecSize);
			for (int32_t Idx = 0; Idx < RefVL; ++Idx) {
				int32_t Offset = int32_t(mState.V[k][Idx]);
				CAddr_t Addr = CAddr_t(RefA0) + Offset;
				WriteDataMem(Addr, mState.V[j][Idx]);
			}
		}
		return 1;
	default:
		CRAY_UNIMPLEMENTED;
	}
}
