// Cray-1S CPU simulator class

// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
//
// To be true to the documentation, octal numbers are extensively used in this file.
// Watch out for the leading 0-s!
//
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!
// !!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!

template <bool aDoExecute> size_t SoftCpu_c::Decode0040(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0040);
	// Si exp
	if (IsXMode()) {
		CInt_t Value = CInt_t(jkm);
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " " << HexPrinter(Value);
		}
		if (aDoExecute) {
			RefSiTarget = Value;
		}
		return 2;
	}
	else {
		CInt_t Value = CInt_t(mn);
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " " << HexPrinter(Value);
		}
		if (aDoExecute) {
			CRAY_ASSERT(jk == 000);
			RefSiTarget = Value;
		}
		return 3;
	}
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0041(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0041);
	// Si -exp
	if (IsXMode()) {
		CInt_t Value = ~CInt_t(jkm);
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " " << HexPrinter(Value);
		}
		if (aDoExecute) {
			RefSiTarget = Value;
		}
		return 2;
	}
	else {
		CInt_t Value = ~CInt_t(mn);
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " " << HexPrinter(Value);
		}
		if (aDoExecute) {
			CRAY_ASSERT(jk == 000);
			RefSiTarget = Value;
		}
		return 3;
	}
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0042(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0042);
	// Si <exp

	if (!aDoExecute) {
		switch (jk) {
		case 077:
			aDisassembly << RefSiTarget << " 1";
			break;
		case 000:
			aDisassembly << RefSiTarget << " -1";
			break;
		default:
			aDisassembly << RefSiTarget << " <" << 64 - jk;
			aExplanation << "form " << 64 - jk << " ones from right";
			break;
		}
	}
	if (aDoExecute) {
		RefSi = (0xffffffffffffffffULL) >> (jk);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0043(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0043);
	// Si >exp

	if (!aDoExecute) {
		switch (jk) {
		case 000:
			aDisassembly << RefSiTarget << " 0";
			break;
		default:
			aDisassembly << RefSiTarget << " >" << jk;
			aExplanation << "form " << jk << " ones from left";
			break;
		}
	}
	if (aDoExecute) {
		if (jk == 0) {
			RefSi = 0;
		}
		else {
			RefSi = (0xffffffffffffffffULL) << (64 - jk);
		}
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0044(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0044);
	// Si Sj&Sk

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSj << "&" << RefSk;
	}
	if (aDoExecute) {
		RefSi = RefSj & RefSk;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0045(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0045);
	// Si #Sk&Sj

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " #" << RefSk << "&" << RefSj;
		aExplanation << RefSiTarget << " not(" << RefSk << ") and (" << RefSj << ")";
	}
	if (aDoExecute) {
		RefSi = RefSj & ~RefSk;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0046(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0046);
	// Si Sj\Sk (xor)

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSj << "\\" << RefSk;
		aExplanation << RefSiTarget << " " << RefSj << " xor " << RefSk;
	}
	if (aDoExecute) {
		RefSi = RefSj ^ RefSk;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0047(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0047);
	// Si #Sj\Sk (xnor)

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " #" << RefSj << "\\" << RefSk;
		aExplanation << RefSiTarget << " not(" << RefSj << ") xor " << RefSk;
	}
	if (aDoExecute) {
		RefSi = ~RefSj ^ RefSk;
	}
	return 1;
}

template <bool aDoExecute> size_t SoftCpu_c::Decode0050(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0050);
	// Si Sj!Si&Sk (scalar merge)

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSj << "!" << RefSi << "&" << RefSk;
		aExplanation << RefSiTarget << " bit-wise " << RefSk << "==1 ? " << RefSj << ":" << RefSi;
	}
	if (aDoExecute) {
		RefSi = (RefSj & RefSk) | (RefSi & ~RefSk);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0051(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0051);
	// Si Sj!Sk (or)

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSj << "!" << RefSk;
		aExplanation << RefSiTarget << " " << RefSj << " or " << RefSk;
	}
	if (aDoExecute) {
		RefSi = RefSj | RefSk;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0052(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0052);
	// S0 Si < exp

	if (!aDoExecute) {
		aDisassembly << "S0 " << RefSi << " < " << jk;
	}
	if (aDoExecute) {
		RefS0 = ShiftLeft(RefSi, CAddr_t(jk));
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0053(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0053);
	// S0 Si > exp

	if (!aDoExecute) {
		aDisassembly << "S0 " << RefSi << " > " << 64 - jk;
	}
	if (aDoExecute) {
		RefS0 = ShiftRight(RefSi, CAddr_t(64 - jk)); // It should be unsigned already, but just to be extra sure in case someone changes CInt_t to signed
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0054(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0054);
	// Si Si < exp

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSi << " < " << jk;
	}
	if (aDoExecute) {
		RefSi = ShiftLeft(RefSi, CAddr_t(jk));
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0055(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0055);
	// Si Si > exp

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSi << " > " << 64 - jk;
	}
	if (aDoExecute) {
		RefSi = ShiftRight(RefSi, CAddr_t(64 - jk)); // It should be unsigned already, but just to be extra sure in case someone changes CInt_t to signed
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0056(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0056);
	// Si Si,Sj < Ak

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSi << "," << RefSj << " < " << RefAk;
	}
	if (aDoExecute) {
		RefSi = DoubleShiftLeft(RefSi, RefSj, RefAk);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0057(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0057);
	// Si Sj,Si > Ak

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSj << "," << RefSi << " > " << RefAk;
	}
	if (aDoExecute) {
		RefSi = DoubleShiftRight(RefSj, RefSi, RefAk);
	}
	return 1;
}

template <bool aDoExecute> size_t SoftCpu_c::Decode0060(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0060);
	// Si Sj+Sk

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefSiTarget << " " << RefSk;
		}
		else {
			aDisassembly << RefSiTarget << " " << RefSj << "+" << RefSk;
		}
	}
	if (aDoExecute) {
		RefSi = CInt_t(RefSj) + CInt_t(RefSk);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0061(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0061); // Si Sj-Sk

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefSiTarget << " -" << RefSk;
		}
		else {
			aDisassembly << RefSiTarget << " " << RefSj << "-" << RefSk;
		}
	}
	if (aDoExecute) {
		RefSi = CInt_t(RefSj) - CInt_t(RefSk);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0062(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0062); // Si Sj+FSk

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefSiTarget << " F" << RefSk;
			aExplanation << "Normalize " << RefSk;
		}
		else {
			aDisassembly << RefSiTarget << " " << RefSj << "+F" << RefSk;
			aExplanation << "floating point addition";
		}
	}
	if (aDoExecute) {
		CFloat_t J = CFloat_t(RefSj);
		CFloat_t K = CFloat_t(RefSk);
		CFloat_t I = J + K;
		//		double dI = J.ToDouble() + K.ToDouble();
		//		if (!TestResult(dI, I, mLogger)) {
		//			mLogger << setloglevel(LogLevel_Always) << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			mLogger << setloglevel(LogLevel_Always) << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " + " << DoublePrinter(K) << std::endl;
		//			std::cout << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			std::cout << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " + " << DoublePrinter(K) << std::endl;
		//			//CRAY_ASSERT(false);
		//		}
		mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " + " << DoublePrinter(K) << std::endl;
		RefSi = CInt_t(I);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0063(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0063); // Si Sj-FSk

	if (!aDoExecute) {
		if (j == 0) {
			aDisassembly << RefSiTarget << " -F" << RefSk;
			aExplanation << "floating point subtraction";
		}
		else {
			aDisassembly << RefSiTarget << " " << RefSj << "-F" << RefSk;
			aExplanation << "floating point subtraction";
		}
	}
	if (aDoExecute) {
		CFloat_t J = CFloat_t(RefSj);
		CFloat_t K = CFloat_t(RefSk);
		CFloat_t I = J - K;
		//		double dI = J.ToDouble() - K.ToDouble();
		//		if (!TestResult(dI, I, mLogger)) {
		//			mLogger << setloglevel(LogLevel_Always) << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			mLogger << setloglevel(LogLevel_Always) << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " - " << DoublePrinter(K) << std::endl;
		//			std::cout << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			std::cout << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " - " << DoublePrinter(K) << std::endl;
		//			//CRAY_ASSERT(false);
		//		}
		mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " - " << DoublePrinter(K) << std::endl;
		RefSi = CInt_t(I);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0064(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0064); // Si Sj*FSk

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSj << "*F" << RefSk;
		aExplanation << "floating point multiplication";
	}
	if (aDoExecute) {
		CFloat_t J = CFloat_t(RefSj);
		CFloat_t K = CFloat_t(RefSk);
		CFloat_t I = J * K;
		//		{
		//			bool IntegerMult = (J.RawExp() == 0 && K.RawExp() == 0);
		//			if (IntegerMult) {
		//				uint64_t FractA = J.UFract();
		//				uint64_t FractB = K.UFract();
		//				if ((FractA & 0xffffffull) != 0 || (FractB & 0xffffffull) != 0) {
		//					DumpHistory();
		//				}
		//			}
		//		}
		//		double dI = J.ToDouble() * K.ToDouble();
		//		if ((J.Exp() != -CFloat_t::ExpOfs || K.Exp() != -CFloat_t::ExpOfs) && !TestResult(dI, I, mLogger)) {
		//			mLogger << setloglevel(LogLevel_Always) << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			mLogger << setloglevel(LogLevel_Always) << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " H* " << DoublePrinter(K) << std::endl;
		//			std::cout << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			std::cout << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " H* " << DoublePrinter(K) << std::endl;
		//			//CRAY_ASSERT(false);
		//		}
		mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " F* " << DoublePrinter(K) << std::endl;
		RefSi = CInt_t(I);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0065(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0065); // Si Sj*HSk

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSj << "*H" << RefSk;
		aExplanation << "Half-precision floating point multiplication";
	}
	if (aDoExecute) {
		CHalfFloat_t J = CHalfFloat_t(RefSj);
		CHalfFloat_t K = CHalfFloat_t(RefSk);
		CHalfFloat_t I = HalfPrecisionMult(J, K);
		//		double dI = J.ToDouble() * K.ToDouble();
		//		if ((J.Exp() != -CFloat_t::ExpOfs || K.Exp() != -CFloat_t::ExpOfs) && !TestResult(dI, I, mLogger)) {
		//			mLogger << setloglevel(LogLevel_Always) << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			mLogger << setloglevel(LogLevel_Always) << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " H* " << DoublePrinter(K) << std::endl;
		//			std::cout << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			std::cout << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " H* " << DoublePrinter(K) << std::endl;
		//			//CRAY_ASSERT(false);
		//		}
		mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " H* " << DoublePrinter(K) << std::endl;
		RefSi = CInt_t(I);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0066(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0066); // Si Sj*RSk

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSj << "*R" << RefSk;
		aExplanation << "Rounded floating point multiplication";
	}
	if (aDoExecute) {
		CFloat_t J = CFloat_t(RefSj);
		CFloat_t K = CFloat_t(RefSk);
		CFloat_t I = RoundedMult(J, K);
		//		double dI = J.ToDouble() * K.ToDouble();
		//		if ((J.Exp() != -CFloat_t::ExpOfs || K.Exp() != -CFloat_t::ExpOfs) && !TestResult(dI, I, mLogger)) {
		//			mLogger << setloglevel(LogLevel_Always) << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			mLogger << setloglevel(LogLevel_Always) << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " R* " << DoublePrinter(K) << std::endl;
		//			std::cout << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			std::cout << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " R* " << DoublePrinter(K) << std::endl;
		//			//CRAY_ASSERT(false);
		//		}
		mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " R* " << DoublePrinter(K) << std::endl;
		RefSi = CInt_t(I);
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0067(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0067); // Si Sj*ISk

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefSj << "*I" << RefSk;
		aExplanation << "Reciprocial iteration: " << RefSiTarget << " = 2-" << RefSj << "*" << RefSk;
	}
	if (aDoExecute) {
		CFloat_t J = CFloat_t(RefSj);
		CFloat_t K = CFloat_t(RefSk);
		CFloat_t I = ReciprocIterate(J, K);
		//		double dI = 2.0 - J.ToDouble() * K.ToDouble();
		//		if (!TestResult(dI, I, mLogger)) {
		//			mLogger << setloglevel(LogLevel_Always) << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			mLogger << setloglevel(LogLevel_Always) << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " (RI) " << DoublePrinter(K) << std::endl;
		//			std::cout << "Binary input values: " << HexPrinter(J.Value) << " " << HexPrinter(K.Value) << std::endl;
		//			std::cout << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " (RI) " << DoublePrinter(K) << std::endl;
		//			//CRAY_ASSERT(false);
		//		}
		mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << "Floating point values: " << DoublePrinter(I) << " = " << DoublePrinter(J) << " (RI) " << DoublePrinter(K) << std::endl;
		RefSi = CInt_t(I);
	}
	return 1;
}

template <bool aDoExecute> size_t SoftCpu_c::Decode0070(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0070); // Si /HSj
	switch (k) {
	case 0:
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " /H" << RefSj;
			aExplanation << "Reciproc";
		}
		if (aDoExecute) {
			CFloat_t J = CFloat_t(RefSj);
			CFloat_t I = ReciprocApprox(J);
			//				double dI = 1.0 / J.ToDouble();
			//				if (!TestResult(dI, I, mLogger)) {
			//					mLogger << setloglevel(LogLevel_Always) << "Binary input values: " << HexPrinter(J.Value) << std::endl;
			//					mLogger << setloglevel(LogLevel_Always) << "Floating point values: " << DoublePrinter(I) << " = 1/" << DoublePrinter(J) << std::endl;
			//					std::cout << "Binary input values: " << HexPrinter(J.Value) << std::endl;
			//					std::cout << "Floating point values: " << DoublePrinter(I) << " = 1/" << DoublePrinter(J) << std::endl;
			//					//CRAY_ASSERT(false);
			//				}
			mLogger << setloglevel(LogLevel_SideEffects) << SideEffectIndent << "Floating point values: " << I.ToDouble() << " = 1/" << J.ToDouble() << std::endl;
			RefSi = CInt_t(I);
		}
		return 1;
	case 6: // Si Sj * BT
		if (IsSV1()) {
			if (!aDoExecute) {
				aDisassembly << RefSiTarget << " " << RefSj << " * BT";
				aExplanation << "Transmit the bit-matrix product of (Sj ) and transpose of (BMM) to Si. *********** UNIMPLEMENTED FOR NOW ***********";
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsSv1BitMatrixLoaded());
				CInt_t Result = 0;
				for (int i = 0; i < mState.VL; ++i) {
					Result ^= RefSj & RefBM[i];
				}
				RefSiTarget = Result;
			}
		}
		else {
			CRAY_UNKNOWN
		}
		return 1;
	default:
		CRAY_UNKNOWN
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0071(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0071);
	switch (j) {

	case 0: // Si Ak
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " " << RefAk;
			aExplanation << "Assign with no sign-extension";
		}
		if (aDoExecute) {
			RefSi = CInt_t(CAddr_t(RefAk));
		}
		return 1;
	case 1: // Si +Ak
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " +" << RefAk;
			aExplanation << "Assign with sign-extension";
		}
		if (aDoExecute) {
			if (IsXMode()) {
				RefSi = uint64_t(SignExtendLongLong(CXmpAddr_t(RefAk)));
			}
			else {
				RefSi = uint64_t(SignExtendLongLong(CAddr_t(RefAk)));
			}
		}
		return 1;
	case 2: // Si +FAk
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " +F" << RefAk;
			aExplanation << "Assign as unnormalized float";
		}
		if (aDoExecute) {
			int32_t Ak = SignExtend(RefAk);
			CInt_t Result = 040060ULL << 48;
			if (Ak < 0) Result |= CFloat_t::FractSignMask;
			Result |= abs(Ak);
			RefSiTarget = Result;
		}
		return 1;
	case 3: // Si 0.6
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " 0.6";
		}
		if (aDoExecute) {
			RefSi = 0400606000000000000000ULL;
		}
		return 1;
	case 4: // Si 0.4
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " 0.4";
		}
		if (aDoExecute) {
			RefSi = 0400004000000000000000ULL;
		}
		return 1;
	case 5: // Si 1.0
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " 1.";
		}
		if (aDoExecute) {
			RefSi = 0400014000000000000000ULL;
		}
		return 1;
	case 6: // Si 2.0
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " 2.";
		}
		if (aDoExecute) {
			RefSi = 0400024000000000000000ULL;
		}
		return 1;
	case 7: // Si 4.0
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " 4.";
		}
		if (aDoExecute) {
			RefSi = 0400034000000000000000ULL;
		}
		return 1;
	default: CRAY_UNKNOWN
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0072(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0072);
	switch (k) {
	case 0: // Si RT
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " RT";
			aExplanation << "Read the real-time-clock";
		}
		if (aDoExecute) {
			RefSi = mMainframe.GetRealTimeClock();
			aBreakBurst = true;
		}
		return 1;
	case 2: // Si SM
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " SM";
			aExplanation << "Read the semaphores";
		}
		if (aDoExecute) {
			if (mState.Cluster > 0) {
				uint64_t RetVal = 0;
				auto &Cluster = mMainframe.GetCluster(mState.Cluster - 1);
				for (int i = 0; i < 32; ++i) {
					if (Cluster.SM[i].load()) RetVal |= (1ULL << 31);
					RetVal <<= 1;
				}
				RefSi = RetVal;
				aBreakBurst = true;
			}
		}
		return 1;
	case 3: // Si STj
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " " << RefSTj;
		}
		if (aDoExecute) {
			if (mState.Cluster > 0) {
				RefSi = RefSTj;
				aBreakBurst = true;
			}
		}
		return 1;
	default: CRAY_UNKNOWN
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0073(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0073);
	switch (k) {
	case 0: // Si VM
		if (!aDoExecute) {
			aDisassembly << RefSiTarget << " VM";
			aExplanation << "Read the vector-mask register";
		}
		if (aDoExecute) {
			RefSi = RefVM;
		}
		return 1;
	case 1: // multiple
		switch (j) {
		case 0: // Si SR0
			if (!aDoExecute) {
				aDisassembly << RefSiTarget << " " << RefSRj;
				aExplanation << "See HR-0097 5-60 for bit-definitions of SR0 registers";
			}
			if (aDoExecute) {
				if (j != 0) {
					// We don't know what SRj registers are...
					CRAY_UNIMPLEMENTED;
				}
				else {
					RefSi = GetSR(0);
					aBreakBurst = true;
				}
			}
			return 1;
		case 1: // Read perf counter to Si
			if (!aDoExecute) {
				aDisassembly << RefSiTarget << " PFPTR++";
				aExplanation << "Read (high or low order bits of) performance counter into " << RefSiTarget << " than increment performance counter pointer";
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					//CRAY_UNIMPLEMENTED;
					RefSi = CInt_t(0);
				}
			}
			return 1;
		case 2: // Increment perf counter
			if (!aDoExecute) {
				if (IsXmp()) {
					aDisassembly << "PFC++";
					aExplanation << "Increment the current performance counter (low or high-part) pointed by PFPTR";
				}
				else {
					aDisassembly << "PFC++ - low";
					aExplanation << "Increment the current performance counter (low-part) pointed by PFPTR";

				}
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					//CRAY_UNIMPLEMENTED;
				}
			}
			return 1;
		case 3: // Clear all maintanace modes
			if (!aDoExecute) {
				aDisassembly << "CLM " << RefSiTarget << " -1";
				aExplanation << "Clear all maintenance modes. Loads " << RefSiTarget << " with -1 as a side-effect";
			}
			if (aDoExecute) {
				CRAY_ASSERT(mState.Mode.IsMonitorMode());
				if (mState.Mode.IsMonitorMode()) {
					//CRAY_UNIMPLEMENTED;
				}
			}
			return 1;
		case 4: // SIM-TO-HOST COMMUNICATION CHANNEL.
			if (!aDoExecute) {
				aDisassembly << "HOST_WR " << RefSi << " -1";
				aExplanation << "Sends " << RefSi << " to host";
			}
			if (aDoExecute) {
				RefSiTarget = mMainframe.SimToHost(RefSi);
			}
		case 5: // Gets defined in C90
			CRAY_UNKNOWN;
		case 6: // Increment perf counter (higher)
			if (!IsXmp()) {
				if (!aDoExecute) {
					aDisassembly << "PFC++ - high";
					aExplanation << "Increment the current performance counter (high-part) pointed by PFPTR";
				}
				if (aDoExecute) {
					CRAY_ASSERT(mState.Mode.IsMonitorMode());
					if (mState.Mode.IsMonitorMode()) {
						//CRAY_UNIMPLEMENTED;
					}
				}
			}
			else CRAY_UNKNOWN
				return 1;
		case 7: // HOST-TO-SIM COMMUNICATION CHANNEL.
			if (!aDoExecute) {
				aDisassembly << "HOST_WR " << RefSiTarget << " -1";
				aExplanation << "Sends " << RefSiTarget << " to host";
			}
			if (aDoExecute) {
				RefSiTarget = mMainframe.HostToSim();
			}
		default: CRAY_UNKNOWN
		}
		return 1;
	case 2: // SM Si
		if (!aDoExecute) {
			aDisassembly << "SM " << RefSi;
		}
		if (aDoExecute) {
			if (mState.Cluster > 0) {
				uint64_t NewVal = RefSi;
				auto &Cluster = mMainframe.GetCluster(mState.Cluster - 1);
				for (int i = 0; i < 32; ++i) {
					Cluster.SM[i].store(GetBit(NewVal, 63 - i) != 0);
				}
				aBreakBurst = true;
			}
		}
		return 1;
	case 3: // STj Si
		if (!aDoExecute) {
			aDisassembly << RefSTjTarget << " " << RefSi;
		}
		if (aDoExecute) {
			if (mState.Cluster > 0) {
				RefSTj = RefSi;
				aBreakBurst = true;
			}
		}
		return 1;
	case 5: // Multiple
		if (i == 0 && j == 1) { // SPECIAL SIM OPERATION TO TERMINATE SIMULATOR
			throw TerminateInstError_x();
		}
		else {
			CRAY_UNKNOWN
		}
	default: CRAY_UNKNOWN
	}
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0074(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0074);
	// Si Tjk

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefTjk;
	}
	if (aDoExecute) {
		RefSi = RefTjk;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0075(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0075);
	// Tjk Si

	if (!aDoExecute) {
		aDisassembly << RefTjkTarget << " " << RefSi;
	}
	if (aDoExecute) {
		RefTjk = RefSi;
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0076(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0076);
	// Si Vj,Ak

	if (!aDoExecute) {
		aDisassembly << RefSiTarget << " " << RefVj << "," << RefAk;
	}
	if (aDoExecute) {
		RefSi = mState.V[j][CAddr_t(RefAk) % cVecSize];
	}
	return 1;
}
template <bool aDoExecute> size_t SoftCpu_c::Decode0077(uint64_t aParcels, size_t aMaxParcelCnt, std::ostream &aDisassembly, std::ostream &aExplanation, bool &aBreakBurst) {
	EXTRACT_FIELDS(aParcels);
	CRAY_ASSERT(gh == 0077);
	// Vi,Ak Sj

	if (!aDoExecute) {
		aDisassembly << RefViTarget << "," << RefAk << " " << RefSj;
	}
	if (aDoExecute) {
		mState.V[i][CAddr_t(RefAk) % cVecSize] = RefSj;
		RefVi.LogState();
		mState.VectorNotUsed = false;
	}
	return 1;
}
