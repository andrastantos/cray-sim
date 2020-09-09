#include "utils.h"
#include "cray_types.h"
#include "cray_logger.h"

// Cygwin didn't quite like abs(long long) so let's have a local equivalent...
template <typename tType> static tType Abs(const tType &aA) { return aA > tType(0) ? aA : -aA; }

static const size_t cFractShiftAmount = 0;

//static long long StickyShift(long long aFract, size_t aAmount) {
//	long long Mask = (1LL << aAmount) - 1;
//	if ((aFract & Mask) != 0) {
//		return (aFract >> aAmount) | 1;
//	} else {
//		return (aFract >> aAmount);
//	}
//}

static long long Shift(long long aFract, size_t aAmount) {
	return (aFract >> aAmount);
}

// Math operators and functions
CFloat_t operator +(const CFloat_t  &aA, const CFloat_t  &aB) {
	// Handle underflow
	CFloat_t A = aA.UnderflowOrValue();
	CFloat_t B = aB.UnderflowOrValue();

	long ExpA = A.Exp();
	long ExpB = B.Exp();
	long long FractA = A.UFract() << cFractShiftAmount;
	long long FractB = B.UFract() << cFractShiftAmount;

	long long ResultFract;
	long ResultExp;
	if (ExpA > ExpB) {
		FractB = Shift(FractB, std::min<long>(63,(ExpA-ExpB)));
		//if ((FractB & 1) != 0) FractB += 1;
		//FractB >>= 1;
		ResultExp = ExpA;
	} else if (ExpA < ExpB) {
		FractA = Shift(FractA, std::min<long>(63,(ExpB-ExpA)));
		//if ((FractA & 1) != 0) FractA += 1;
		//FractA >>= 1;
		ResultExp = ExpB;
	} else {
		ResultExp = ExpB;
	}
	bool ResultPositive = true;
	if (!A.IsPositive() && !B.IsPositive()) {
		ResultFract = FractA + FractB;
		ResultPositive = false;
	} else if (A.IsPositive() && B.IsPositive()) {
		ResultFract = FractA + FractB;
		ResultPositive = true;
	} else if (A.IsPositive() && !B.IsPositive()) {
		ResultFract = FractA - FractB;
		if (ResultFract < 0) {
			ResultPositive = false;
			ResultFract = -ResultFract;
		} else {
			ResultPositive = true;
		}
	} else {
		ResultFract = FractB - FractA;
		if (ResultFract < 0) {
			ResultPositive = false;
			ResultFract = -ResultFract;
		}
		else {
			ResultPositive = true;
		}
	}

	if ((Abs(ResultFract) & (1LL << (CFloat_t::FractSize + cFractShiftAmount))) != 0) {
		++ResultExp;
//		if ((ResultFract & 1) != 0) ResultFract += 1;
		ResultFract = Shift(ResultFract, 1);
	}
	CFloat_t RetVal(ResultExp, ResultPositive, Shift(ResultFract, cFractShiftAmount));
	RetVal.Normalize();
	return RetVal.UnderflowOrValue();
}

CFloat_t operator -(const CFloat_t  &aA) {
	CFloat_t RetVal(aA);
	RetVal.FlipSign();
	return RetVal;
}

CFloat_t operator -(const CFloat_t  &aA, const CFloat_t  &aB) {
	return aA + (-aB);
}

static CFloat_t Mult(const CFloat_t  &aA, const CFloat_t  &aB, bool aDoRounding, bool aHalfPrecision = false, bool aTwoMinus = false) {
	// Each fractional is between 0 and 2.
	// The result is thus between 0 and 4.
	// Each fractional is 48 bits, so the result is 49 bits, with the potential of one normalization shift in the end if the MSB is '1'.
	// Cut each fractional into two 24-bit portions and implement the multiply that way - this makes carry handling in the addition step easier

	// Handle underflow
	CFloat_t A = aA.RawExp() == 0 ? aA : aA.UnderflowOrValue();
	CFloat_t B = aB.RawExp() == 0 ? aB : aB.UnderflowOrValue();

	if (A.Value == 0) return CFloat_t(CInt_t(0));
	if (B.Value == 0) return CFloat_t(CInt_t(0));

	long Exp = A.Exp() + B.Exp();
	bool ResultPositive = A.IsPositive() == B.IsPositive();

	bool IntegerMult = (A.RawExp() == 0 && B.RawExp() == 0);
	if  (IntegerMult) {
		// Integer multiply domain (1)
		Exp = -CFloat_t::ExpOfs; // Zero-out exponent
	}

	// Multiplier is only 56-bit wide
	uint64_t FractA = A.UFract() << 8;
	uint64_t FractB = B.UFract() << 8;
	uint64_t CorrectionFactor = 18;
	uint64_t ResultFract = 0;
	if (aDoRounding) {
		CorrectionFactor |= 0x80 | 0x40;
	}
	if (aHalfPrecision) {
		CorrectionFactor |= (1 << 24) | (1 << 25);
	}
	if (aTwoMinus) {
		CorrectionFactor = ~CorrectionFactor;
	}
	//std::cout << "Correction factor: " << HexPrinter(CorrectionFactor) << std::endl;
	for (size_t BitIdx = 0; BitIdx < 48; ++BitIdx) {
		if (((FractB >> (47 + 8 - BitIdx)) & 1) != 0) {
			ResultFract = ResultFract + FractA;
		}
		FractA >>= 1;
	}
	//std::cout << "Before adjustment: " << HexPrinter(ResultFract) << std::endl;
	ResultFract += CorrectionFactor;
	//std::cout << "After adjustment: " << HexPrinter(ResultFract) << std::endl;
	if (aTwoMinus) {
		ResultFract = ~ResultFract;
	}
	//std::cout << "After inversion: " << HexPrinter(ResultFract) << std::endl;
	// Re-normalizing if necessary
	if (!IntegerMult) {
		if ((ResultFract & (1ull << 56)) != 0) {
			++Exp;
			// This is most likely wrong: the original implementation almost certainly didn't do this rounding step.
			//ResultFract += ResultFract & 1;
			ResultFract >>= 1;
		}
		if (Exp + CFloat_t::ExpOfs < CFloat_t::ExpMin) {
			Exp = -CFloat_t::ExpOfs;
			ResultFract = 0;
			ResultPositive = true;
		}
		if (aHalfPrecision) {
			ResultFract &= 0xFFFFFFFFF8000000ULL; // Mask out the bottom 19 bits (plus 8 for the shift later)
		}
	} else {
		ResultFract >>= 1;
	}
	CFloat_t RetVal(Exp, ResultPositive, ResultFract >> 8);
	return RetVal;
}

CFloat_t operator *(const CFloat_t  &aA, const CFloat_t  &aB) {
	return Mult(aA,aB,false);
}

static uint64_t ReciprocLookup_s[] = {
	0x0000000007FBF804ull, 0x0000000007EBE824ull, 0x0000000007DBD864ull, 0x0000000007CBC8C4ull, 0x0000000007BBB944ull, 0x0000000007ABA9E4ull, 0x0000000007A3A240ull, 0x0000000007939310ull,
	0x0000000007838400ull, 0x0000000007737510ull, 0x00000000076B6DA4ull, 0x00000000075B5EE4ull, 0x00000000074B5044ull, 0x0000000007434900ull, 0x0000000007333A90ull, 0x0000000007232C40ull,
	0x00000000071B2524ull, 0x00000000070B1704ull, 0x0000000007031000ull, 0x0000000006F30210ull, 0x0000000006EAFB24ull, 0x0000000006DAED64ull, 0x0000000006D2E690ull, 0x0000000006C2D900ull,
	0x0000000006BAD244ull, 0x0000000006AAC4E4ull, 0x0000000006A2BE40ull, 0x00000000069AB7A4ull, 0x00000000068AAA84ull, 0x000000000682A400ull, 0x00000000067A9D84ull, 0x00000000066A90A4ull,

	0x0000000006628A40ull, 0x00000000065A83E4ull, 0x0000000006527D90ull, 0x0000000006427100ull, 0x00000000063A6AC4ull, 0x0000000006326490ull, 0x00000000062A5E64ull, 0x0000000006225840ull,
	0x0000000006124C10ull, 0x00000000060A4604ull, 0x0000000006024000ull, 0x0000000005FA3A04ull, 0x0000000005F23410ull, 0x0000000005EA2E24ull, 0x0000000005E22840ull, 0x0000000005DA2264ull,
	0x0000000005D21C90ull, 0x0000000005CA16C4ull, 0x0000000005C21100ull, 0x0000000005BA0B44ull, 0x0000000005B20590ull, 0x0000000005A9FFE4ull, 0x0000000005A1FA40ull, 0x000000000599F4A4ull,
	0x000000000591EF10ull, 0x000000000589E984ull, 0x000000000581E400ull, 0x000000000579DE84ull, 0x000000000571D910ull, 0x000000000569D3A4ull, 0x000000000561CE40ull, 0x000000000559C8E4ull,

	0x000000000551C390ull, 0x000000000549BE44ull, 0x000000000541B900ull, 0x000000000541B900ull, 0x000000000539B3C4ull, 0x000000000531AE90ull, 0x000000000529A964ull, 0x000000000521A440ull,
	0x0000000005199F24ull, 0x0000000005199F24ull, 0x0000000005119A10ull, 0x0000000005099504ull, 0x0000000005019000ull, 0x0000000004F98B04ull, 0x0000000004F98B04ull, 0x0000000004F18610ull,
	0x0000000004E98124ull, 0x0000000004E17C40ull, 0x0000000004E17C40ull, 0x0000000004D97764ull, 0x0000000004D17290ull, 0x0000000004C96DC4ull, 0x0000000004C96DC4ull, 0x0000000004C16900ull,
	0x0000000004B96444ull, 0x0000000004B96444ull, 0x0000000004B15F90ull, 0x0000000004A95AE4ull, 0x0000000004A95AE4ull, 0x0000000004A15640ull, 0x00000000049951A4ull, 0x00000000049951A4ull,

	0x0000000004914D10ull, 0x0000000004894884ull, 0x0000000004894884ull, 0x0000000004814400ull, 0x0000000004793F84ull, 0x0000000004793F84ull, 0x0000000004713B10ull, 0x0000000004713B10ull,
	0x00000000046936A4ull, 0x0000000004613240ull, 0x0000000004613240ull, 0x0000000004592DE4ull, 0x0000000004592DE4ull, 0x0000000004512990ull, 0x0000000004492544ull, 0x0000000004492544ull,
	0x0000000004412100ull, 0x0000000004412100ull, 0x0000000004391CC4ull, 0x0000000004391CC4ull, 0x0000000004311890ull, 0x0000000004291464ull, 0x0000000004291464ull, 0x0000000004211040ull,
	0x0000000004211040ull, 0x0000000004190C24ull, 0x0000000004190C24ull, 0x0000000004110810ull, 0x0000000004110810ull, 0x0000000004090404ull, 0x0000000004090404ull, 0x0000000004010000ull,
};


static uint64_t Mult(uint64_t &S3, uint64_t &S5, uint32_t &A6) {
	uint64_t S0, S2;
	uint32_t A0;
	// Output: S2 = S5 * S3
	S2 = 0;
	do {
		S0 = S3 << 1;
		S3 = S3 << 1;
		if (int64_t(S0) < 0) {
			S2 = S2 + S5;
		}
		S5 = S5 >> 1;
		A0 = A6 - 1;
		A6 = A6 - 1;
	} while (A0 != 0);
	return S2;
}

static uint64_t Mult(uint64_t &S2, uint64_t &S3, uint64_t &S5, uint32_t &A6) {
	uint64_t S0;
	uint32_t A0;
	// Output: S2 = S5 * S3
	do {
		S0 = S3 << 1;
		S3 = S3 << 1;
		if (int64_t(S0) < 0) {
			S2 = S2 + S5;
		}
		S5 = S5 >> 1;
		A0 = A6 - 1;
		A6 = A6 - 1;
	} while (A0 != 0);
	return S2;
}

static uint64_t MaskRight(uint32_t aValue) {
	CRAY_ASSERT(aValue <= 63);
	return (1ull << aValue) - 1;
}

static uint64_t MaskLeft(uint32_t aValue) {
	CRAY_ASSERT(aValue <= 63);
	return (0xffffffffffffffffULL) << (64 - aValue);
}




CInt_t ReciprocApproxFinalMul(const CInt_t aA, const CInt_t aB, bool aDoFinalRounding) {
	uint64_t A = aA & ((1ull << 37) - 1);
	uint64_t B = (aB & ((1ull << 37) - 1)) << 3;
	uint64_t P = 0;
	uint64_t SubP = 0;

	SubP = 0;
	SubP += (GetBit(A,  0) != 0 ? (B >> 35) : 0ull) & ~3ull;
	SubP += (GetBit(A,  1) != 0 ? (B >> 34) : 0ull) & ~3ull;
////SubP += (GetBit(A,  2) != 0 ? (B >> 33) : 0ull) & ~3ull;
	SubP += (GetBit(A,  3) != 0 ? (B >> 32) : 0ull) & ~3ull;
	SubP += (GetBit(A,  4) != 0 ? (B >> 31) : 0ull) & ~3ull;
	SubP += (GetBit(A,  5) != 0 ? (B >> 30) : 0ull) & ~3ull;
	SubP += (GetBit(A,  6) != 0 ? (B >> 29) : 0ull) & ~3ull;
	SubP += (GetBit(A,  7) != 0 ? (B >> 28) : 0ull) & ~3ull;
	SubP &= ~7ull;
	P += SubP;
	SubP = 0;
	SubP += (GetBit(A,  8) != 0 ? (B >> 27) : 0ull) & ~1ull;
	SubP += (GetBit(A,  9) != 0 ? (B >> 26) : 0ull) & ~0ull;
	SubP += (GetBit(A, 10) != 0 ? (B >> 25) : 0ull) & ~0ull;
	SubP += (GetBit(A, 11) != 0 ? (B >> 24) : 0ull) & ~0ull;
	SubP &= ~3ull;
	P += SubP;
	SubP = 0;
	SubP += (GetBit(A, 12) != 0 ? (B >> 23) : 0ull) & ~3ull;
	SubP += (GetBit(A, 13) != 0 ? (B >> 22) : 0ull) & ~3ull;
	SubP += (GetBit(A, 14) != 0 ? (B >> 21) : 0ull) & ~7ull;
	SubP += (GetBit(A, 15) != 0 ? (B >> 20) : 0ull) & ~15ull;
	P += SubP;
	SubP = 0;
	SubP += (GetBit(A, 16) != 0 ? (B >> 19) : 0ull) & ~3ull;
	SubP += (GetBit(A, 17) != 0 ? (B >> 18) : 0ull) & ~7ull;
	SubP += (GetBit(A, 18) != 0 ? (B >> 17) : 0ull) & ~3ull;
	SubP += (GetBit(A, 19) != 0 ? (B >> 16) : 0ull) & ~3ull;
	SubP += (GetBit(A, 20) != 0 ? (B >> 15) : 0ull) & ~3ull;
	SubP += (GetBit(A, 21) != 0 ? (B >> 14) : 0ull) & ~3ull;
	SubP += (GetBit(A, 22) != 0 ? (B >> 13) : 0ull) & ~3ull;
	SubP += (GetBit(A, 23) != 0 ? (B >> 12) : 0ull) & ~3ull;
	SubP &= ~7ull;
	P += SubP;
	SubP = 0;
	SubP += (GetBit(A, 24) != 0 ? (B >> 11) : 0ull) & ~1ull;
	SubP += (GetBit(A, 25) != 0 ? (B >> 10) : 0ull) & ~0ull;
	SubP += (GetBit(A, 26) != 0 ? (B >>  9) : 0ull) & ~0ull;
	SubP += (GetBit(A, 27) != 0 ? (B >>  8) : 0ull) & ~0ull;
	SubP &= ~3ull;
	P += SubP;
	SubP = 0;
	SubP += (GetBit(A, 28) != 0 ? (B >>  7) : 0ull) & ~3ull;
	SubP += (GetBit(A, 29) != 0 ? (B >>  6) : 0ull) & ~3ull;
	SubP += (GetBit(A, 30) != 0 ? (B >>  5) : 0ull) & ~7ull;
	SubP += (GetBit(A, 31) != 0 ? (B >>  4) : 0ull) & ~15ull;
	P += SubP;
	SubP = 0;
	SubP += (GetBit(A, 32) != 0 ? (B >>  3) : 0ull) & ~3ull;
	SubP += (GetBit(A, 33) != 0 ? (B >>  2) : 0ull) & ~3ull;
	SubP += (GetBit(A, 34) != 0 ? (B >>  1) : 0ull) & ~3ull;
	SubP += (GetBit(A, 35) != 0 ? (B >>  0) : 0ull) & ~3ull;
	SubP &= ~7ull;
	P += SubP;

	P >>= 2;
	if (aDoFinalRounding) {
		uint64_t Bit1 = GetBit(P, 1) != 0 ? 1 : 0;
		P >>= 2;
		P += Bit1;
		P >>= 2;
	}
	return P;
}

uint64_t FractionalMult(uint64_t aA, uint64_t aB, size_t aBitCnt) {
	uint64_t RetVal = 0;
	for (size_t BitIdx = 0; BitIdx < aBitCnt; ++BitIdx) {
		RetVal += (GetBit(aA, BitIdx) != 0) ? (aB >> (aBitCnt - BitIdx - 1)) : 0ull;
	}
	return RetVal;
}

uint64_t ReciprocApprox(const uint64_t aInput) {
	// Start by looking up 2*A0 and A0^2. They're combined into a single lookup table so some bit-twiddling is needed to get to them...
	size_t LookupIdx = (aInput >> 40) & 127;
	CRAY_ASSERT(LookupIdx < sizeof(ReciprocLookup_s) / sizeof(ReciprocLookup_s[0]));
	uint64_t LookupVal = ReciprocLookup_s[LookupIdx];
	uint64_t twiceA0 = (LookupVal >> 18);     // Put top 9 bits to S6 (this is 2A0 according to the comment)
	uint64_t A0sq = (LookupVal >> 2) & 0xffff; // Bottom 16 bits: this is A0^2 according to the comment

	// Do the first iteration: compute 2*A0 - A0^2 * Mantissa, but with some tweaks:
	// 1. Do some funky rounding on 2*A0
	// 2. Have some offset in the addition
	// 3. Shift things to the 'right' place to avoid truncation
	// 4. Truncate the result to the correct number of bits and mask out the top ones (is that needed???)
	uint64_t Mantissa = (aInput & CFloat_t::FractMask);
	uint64_t A0sqB = FractionalMult(A0sq, Mantissa >> 24, 16);
	uint64_t RoundedA0 = (((twiceA0 << 8) + 1) << 8) + (twiceA0 >> 1);
	uint64_t A1 = (RoundedA0 - A0sqB - 256) >> 6;
	A1 = A1 & ((1 << 18) - 1);

	// Do the second iteration: compute A1^2 and extract the proper section of the mantissa (B2)
	uint64_t A1sq = A1 * A1;
	uint64_t B2 = (Mantissa | (1ULL << 47)) >> 11;
	// Compute A1^2 * Mantissa'
	uint64_t A1sqB2 = ReciprocApproxFinalMul(A1sq, B2, true);

	// We have all the pieces to finalize the computation:
	// 1. Compute 2*A1 - A1^2 * B2
	// 2. Mask it to the right size and make sure it's normalized
	// 3. Extract the exponent, check for over/underflow
	// 4. Compute final exponent
	// 5. Assemble floating point result from the pieces
	uint64_t FinalMantissa = ((A1 << 31) - (A1sqB2 << 14) - 1);
	FinalMantissa = (FinalMantissa | (1ULL << 47)) & 0xffffffff8000ULL;
	uint16_t Exp = uint16_t(CFloat_t(aInput).RawExp());
	uint16_t FinalExp;
	bool Sign = (aInput >> CFloat_t::FractSignBitPos) == 0;
	if (Exp < 0x6000 && Exp > 0x2001) {
		FinalExp = 2*CFloat_t::ExpOfs - Exp - 1;
	} else {
		FinalExp = 0x6000;
		FinalMantissa = FinalMantissa & (~(1ULL << 47)); // Clear the normalized bit
	}
	CFloat_t Result = CFloat_t(FinalExp - CFloat_t::ExpOfs, Sign, FinalMantissa);
	return Result.Value;
}












static uint64_t DebugB;
static uint64_t DebugA0sq;
static uint64_t DebugA0;
static uint64_t DebugA0sqB;
static uint64_t DebugA1sq;
static uint64_t DebugA1;
static uint64_t DebugB2;
static uint64_t DebugA1sqB2;
static uint64_t DebugRoundedA0;
static uint64_t DebugResult;
// Actual source code can be found in
//    ...\cray-xmp\unicos\unicos_4.0_xmp\UC-04.0-U#-PL30_UNICOS_4.0_usr_src_diag\diag\crayxmp\xmp\SFR
// starting from label 'SIM'
// That code has lots of comments in it, but all constants are in octal, which is a pain to translate
// The general algorithm is described here:
//    https://en.wikipedia.org/wiki/Division_algorithm#Newton%E2%80%93Raphson_division
// The iteration seems to be:
// X[i+1] = X[i] * (2 - D*X[i]), where D is the number to be inverted.
// This can be expressed as 2*X[i] - D * X[i]^2, which the algorithm seem to compute according to the notes in the sources
static uint64_t OldReciprocApprox(uint64_t aInput) {
	uint64_t S0, S1, S2, S3, S4, S5, S6, S7;
	uint32_t A6, A7;
	uint64_t T8;
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@ SW implementation of reciproc approximation
	//@@ Input:  S1 - the value to compute the reciprocal for
	//@@ Output : S4 - output S4 ~= 1.0 / S1
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	S1 = aInput;
	S0 = 0;
	S2 = MaskRight(7);   // Mask out bits 40...47 from the input, and use it to look up initial approximation; form 7 ones from right
	S2 = S2 << 40;
	S2 = S1 & S2;
	S2 = S2 >> 40;
	A7 = uint32_t(S2);
	CRAY_ASSERT(A7 < sizeof(ReciprocLookup_s) / sizeof(ReciprocLookup_s[0]));
	S7 = ReciprocLookup_s[A7];
	S6 = S7;                      // Put top 9 bits to S6 (this is 2A0 according to the comment)
	S6 = S6 >> 18;                // ....
	S7 = S7 << 46;                // Put the bottom 16 bits into S7
	S5 = S7;                      // ....
	S5 = S5 >> 48;                // S5 now contains bottom 16 bits: this is A0^2 according to the comment
	S7 = S1;                      // Extract the top bottom 48 bits of mantissa of the input exponent into S7
	S7 = S7 << 16;                // ....
	S7 = S7 >> 40;                // ....
	DebugB = S1;
	DebugA0sq = S5 << 040;
	DebugA0 = S6 << 047;

	// At this point we have :
	// S1 : input
	// S7 : input mantissa
	// S6 : 2A0 (from lookup)
	// S5 : A0^2 (from lookup)
	// Now the comment says: FORM A1=-(A0**2B1-2A0)=2A0-A0**2B1, which translates to:
	// A1 = 2A0 - A0^2 * B1, I think, where B1 is the input mantissa

	// Multiply S5 by S7, put the result in S4, that is A0^2 * B1
	S5 = S5 << 47;                // Align the bits 2...17 of the lookup to the top of S5
	S4 = 0;
	do {
		S0 = S5 << 1;
		S5 = S5 << 1;

		if (int64_t(S0) < 0) {
			S4 = S4 + S7;
		}
		S7 = S7 >> 1;
	} while (S0 != 0);
	DebugA0sqB = S4;
	// According to the comments in the source we do some roungin of 2A0 here (2A0 is 9 bits)
	S7 = S6;
	S6 = S6 << 8;
	S7 = S7 >> 1;
	S3 = 1;
	S6 = S3 + S6;
	S6 = S6 << 8;
	S6 = S6 + S7;                 // We did some rounding of 2A0 apparently and aligned to bit position -17 according to the comment (???)
	DebugRoundedA0 = S6;
	S6 = ~S6;                     // Original comment: COMP S6 W/O S**5 COMP CORR ???? This seems to be changing the sign, somehow
	S7 = S4 + S6;                 // Original comment: FORM (A0**2B1-2A0), so apparently we've formed A0^2 * B1 (in S4) - 2A0 (in S6)
	S5 = 0x0000000000000100ull;   // This is some rounding constant
	S5 = S5 + S7;                 // Do the rounding
	S7 = ~S5;                     // Complement the result for some reason, probably to get the sign-bits right
	S7 = S7 >> 6;                 // Truncate results to 18 bits, according to the comment (here we loose the bottom 'noise' bits)
	S5 = MaskRight(18);           // Do the actual masking
	S7 = S5 & S7;                 // ...
	S7 = S7 << 30;                // This most likely achieves nothing (used in the original code to store A1 into memory
	DebugA1 = S7;
	S7 = S7 >> 30;                // ...

								  // According to original comment, now we create A1^2 in S6; 2*A1 in S7 and B2 in S5
	S5 = S7;
	S6 = 0;
	S7 = S7 << 17;
	S5 = S5 << 45;
	S4 = S7;
	do {
		S0 = S5 << 1;
		S5 = S5 << 1;
		if (int64_t(S0) < 0) {
			S6 = S6 + S4;
		}
		S4 = S4 >> 1;
	} while (S0 != 0);
	// At this point S6 contains A1^2
	// Difference with source!!!!! These two instructions are missing:
	//*
	//*        S6 now contains A1^2.  Clear bits 2 and 1,
	//*        which represent bits 2^-34 and 2^-35.
	//*
	//S5        6
	//S6        #S5&S6
	S5 = 1;                       // We're taking S1 (the input, or B1), extract the mantissa, making sure it's normalized and put it in S5
	S5 = S5 << 47;                // ...
	S1 = S1 | S5;                 // ...
	S5 = MaskRight(48);           // ...
	S5 = S1 & S5;                 // ...
	S5 = S5 >> 11;                // We keep only the top 37 bits, which (according to the source) forms B2
	DebugB2 = S5;
	S7 = S7 << 2;                 // DIFFERENCE WITH THE SOURCE!!!! THIS IS S7 S7<1 there!!!!!!! Still, tests only pass with 2 being here, so this must be accounted for elsewhere
								  // Here we create 2*A1 in S7, but that only makes sense if we follow the original source code!
	S6 = S6 << 12;                // This most likely achieves nothing (used in the original code to store A1^2 into memory
	DebugA1sq = S6;
	S6 = S6 >> 12;                // ...
								  // *FORM A2=-(A1**2B2-2A1)=2A1-A1**2B2
								  // *LOWER 37 BITS OF S5=B2,LOWER 36 BITS OF S6=A1**2
								  // *LOWER 37 BITS OF S7=2A1
								  // *ITERATION FOR RM1,RM8 & RN2
	S4 = 0;
	S6 = S6 << 27;
	// Here the source starts to wildly differ from this disassembly...
	// But, according to notes in the source, the main problem seems to be that the multiplies are
	// larger than 64 bits at this point, so we need to do it in pieces...
	T8 = S7;                      // We save off  2*A1 for later
	S3 = S6;                      // S6 is A1^2


								  //////////////////////////////////////////////////////////////////////////////////////////////
								  //////////////////////////////////////////////////////////////////////////////////////////////
								  //////////////////////////////////////////////////////////////////////////////////////////////
								  // We start the multiply here. The two inputs are:
								  // S3 - top 37 bits are valid, the rest are 0
								  // S5 - bottom 37 bits are valid, the rest are 0
								  // The output ends up in S4
								  // During the multiplication we keep shifting S5 right and S3 left. The MSB of S3 determines if we add S5 to the partial product.
								  // Overall we seem to be doing 35 bits of partial products, but in a rather wierd way. Also, because S5 is (more or less) right-justified
								  // we don't seem to be doing a full product: we keep shifting valid bits out. So: we're doing a similar, truncated product as its done in
								  // the floating-point multiplier.
								  // My best reproduction of the multiplier triangle is this:
								  //     555555555555555555555555555555555555_
								  //     _555555555555555555555555555555555555
								  //     __55555555555555555555555555555555555
								  //     ___5555555555555555555555555555555555
								  //     clear LSB of sum
								  //     ____5555555555555555555555555555555__
								  //     _____5555555555555555555555555555555_
								  //     ______5555555555555555555555555555555
								  //     _______555555555555555555555555555555
								  //
								  //     ________5555555555555555555555555555555
								  //     _________555555555555555555555555555555
								  //     __________55555555555555555555555555555
								  //     clear LSB of partial sum-------------^ (but allow carries before cutting)
								  //     ___________555555555555555555555555555
								  //     clear LSB of partial sum-------------^ (but allow carries before cutting)
								  //
								  //     ____________5555555555555555555555555
								  //     _____________555555555555555555555555
								  //     ______________55555555555555555555555
								  //     _______________5555555555555555555555
								  //     ________________555555555555555555555
								  //     _________________55555555555555555555
								  //     __________________555555555555555555_
								  //     ___________________555555555555555555
								  //     clear LSB of sum
								  //     ____________________555555555555555__
								  //     _____________________555555555555555_
								  //     ______________________555555555555555
								  //     _______________________55555555555555
								  //
								  //     ________________________555555555555555
								  //     _________________________55555555555555
								  //     __________________________5555555555555
								  //     clear LSB of partial sum-------------^ (but allow carries before cutting)
								  //     ___________________________55555555555
								  //     clear LSB of partial sum-------------^ (but allow carries before cutting)
								  //     ____________________________555555555
								  //     _____________________________55555555
								  //     ______________________________5555555
								  //     _______________________________555555
								  //     ________________________________55555
								  //     __________________________________555
								  //     ___________________________________55
								  // This is not for certain, for example I'm left with an extra bit at the end, there's a jump at almost the end, and as you can see there's a bunch of LSB truncation
								  // as well as uneven right-side truncation in the triangle. This is actually reminescent of the original way the floating-point multiplier was described in the Cray-1
								  // manual. It's also interesting why the XMP version of the code seem to have used 128-bit arithemetic for this... Maybe it's worth looking at that code and re-construct
								  // the tree from there as well.
	S5 = S5 << 1;                  // S5 is B2, we multiply it by 2
	A6 = 0x00000004;               // Here, the comments seem to line up again: *ITRATION FOR RM2 & RM9,S5=B2>5,S3=A1**2<4
	S2 = Mult(S3, S5, A6);         // Do 4 partial products
	S2 = S2 >> 1;                  // Zero out the LSB of S2
	S2 = S2 << 1;                  // ...
	S4 = S4 + S2;                  // Add to the sum (well it's 0 so just move)
	S5 = S5 >> 2;                  // Zero out the bottom two LSBs of S5
	S7 = S5;                       // Not sure why we save the shifted version of S5, but it's going to contain two additional bits than what S5 is after the next partial product
	S5 = S5 << 2;                  // ...
	A6 = 0x00000004;               // Another partial product for the next 4 bits
	S2 = Mult(S3, S5, A6);         // ...
	S4 = S4 + S2;                  // ...
	S5 = S7;                       // Restore S5: this is now two more bits than it was after the partial product up there
	A6 = 0x00000003;               // Do three partial products (two of them are something that has been done previuosly, but of course S3 was shifted already
	S2 = Mult(S3, S5, A6);         // ...
	S2 = S2 >> 1;                  // We seem to be doing an extra bit of multiply here, though I'm not sure why we shift S2 first...
	S5 = S5 >> 1;                  // ...
	S0 = S3 << 1;                  // ...
	S3 = S3 << 1;                  // ...
	if (int64_t(S0) < 0) {         // ...
		S2 = S2 + S5;              // ...
	}                              // ...
	S5 = S5 >> 2;                  // We seem to be skipping a few bits here, so S5 lines up with the two shifts of S2. But I still don't get these shifts, especially since it gets added to the unshifted S4
	S2 = S2 >> 1;                  // ...
	S4 = S4 + S2;                  // Yet again, add the partial product to the accumulator
	A6 = 0x00000006;               // Compute another 6 bits of partial product
	S2 = Mult(S3, S5, A6);         // ...
	S5 = S5 >> 1;                  // Clear LSB of S5 yet again
	S5 = S5 << 1;                  // ...
	A6 = 0x00000002;               // Continue the partitial product with the now modified S5 for another two bits
	S2 = Mult(S2, S3, S5, A6);     // ...
	S2 = S2 >> 1;                  // Now clear the LSB of the partial product before adding it to the product
	S2 = S2 << 1;                  // ...
	S4 = S4 + S2;                  // Add it to the accumulator
	S5 = S5 >> 2;                  // Here we do the wierd product creation yet again...
	S7 = S5;
	S5 = S5 << 2;
	A6 = 0x00000004;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S4 = S4 + S2;
	S5 = S7;
	A6 = 0x00000003;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S2 = S2 >> 1;
	S5 = S5 >> 1;
	S0 = S3 << 1;
	S3 = S3 << 1;
	if (int64_t(S0) < 0) {
		S2 = S2 + S5;
	}
	S5 = S5 >> 2;
	S2 = S2 >> 1;
	S4 = S4 + S2;
	A6 = 0x00000005;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S3 = S3 << 1;
	S5 = S5 >> 1;
	A6 = 0x00000002;
	S2 = Mult(S2, S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S2 = S2 >> 1;
	S2 = S2 << 1;
	S4 = S4 + S2;



	S2 = 1;
	S4 = S4 >> 1;
	S2 = S2 & S4;
	S4 = S4 >> 1;
	S4 = S2 + S4;
	S4 = S4 >> 2;
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////





	// I'm guessing here that
	//  S4 finally contains the product of A1^2 * B2
	// Here the sources start to line up again:
	//   *S4=FINAL RECIPROCAL PRIOR TO SHIFT UP
	//   *S4=A1**2B2,RIGHT JUSTIFIED,LOWER 37 BITS OF S7=2A1
	DebugA1sqB2 = S4;
	S7 = T8;                       // Restoring 2*A1
	S4 = S4 << 14;                 // Positioning the two mantissas for final add
	S7 = S7 << 12;                 // ...
	S6 = ~S7;                      // Comment says: S6=-2A1 W/O 2**5 COMP. CORR.
	S5 = S4 + S6;                  // Do the actual add
	S3 = 1;                        // Make it two's complement (original comment says: 2**5 COMP. CORRECTION
	S2 = S3 + S5;                  // ...
	S2 = ~S2;                      // Final compliment, according to orignal source
	S3 = 0400004000000000000000ULL; // 0.4
	S2 = S2 | S3;                  // This - according to the source - is adding in the normalize bit.
	S3 = MaskRight(33);            // We (in a complicated way) extract the mantissa, or at least the top 33 bits of it
	S3 = S3 << 15;                 // ...
	S2 = S2 & S3;                  // ...
	S3 = MaskLeft(15);             // Extract the exponent of the input
	S3 = S3 >> 1;                  // ...
	S3 = S1 & S3;                  // ...
	S7 = 0x0000000000006000ull;    // Test for valid exponent range
	S7 = S7 << 48;                 // ...
	S6 = 0x0000000000002002ull;    // ...
	S6 = S6 << 48;                 // ...
	if (int64_t(S3 - S7) < 0 && int64_t(S3 - S6) >= 0) {
		// Exponent valid, invert it (it's complicated due to the offset nature of the exponent, but that's what's going on here, I believe
		S4 = 0x0000000000000002;
		S4 = S4 << 48;
		S3 = ~S3;
		S3 = S3 + S4;
	}
	else {
		// Exponent invalid, clear the normalize bit in mantissa and saturate exponent to max valid value
		S3 = 0x0000000000006000;
		S3 = S3 << 48;
		S4 = 1;
		S4 = S4 << 47;
		S2 = (~S4) & S2;
	}
	// At this point:
	// S3: exponent
	// S2: mantissa
	// All we need to do is copy the pieces together, including the sign bit from the input
	S4 = MaskRight(63); // form 63 ones from right
	S3 = S3 & S4;
	S4 = S1 & 0x8000000000000000ull;
	S3 = S3 | S4;
	S4 = MaskLeft(16); // form 16 ones from left
	S3 = S3 & S4;
	S4 = S2 | S3;
	DebugResult = S4;
	return S4;
}

#if 0
static uint64_t OldOldReciprocApprox(uint64_t aInput) {
	uint64_t S0, S1, S2, S3, S4, S5, S6, S7;
	uint32_t A6, A7;
	uint64_t T8;
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@ SW implementation of reciproc approximation
	//@@ Input:  S1 - the value to compute the reciprocal for
	//@@ Output : S4 - output S4 ~= 1.0 / S1
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	S1 = aInput;
	S0 = 0;
	S2 = MaskRight(7);   // Mask out bits 40...47 from the input, and use it to look up initial approximation; form 7 ones from right
	S2 = S2 << 40;
	S2 = S1 & S2;
	S2 = S2 >> 40;
	A7 = uint32_t(S2);
	CRAY_ASSERT(A7 < sizeof(ReciprocLookup_s) / sizeof(ReciprocLookup_s[0]));
	S7 = ReciprocLookup_s[A7];
	S6 = S7; // Put top 10 bits to S6
	S6 = S6 >> 18;
	S7 = S7 << 46; // Put the bottom 18 bits into S7
	S5 = S7;
	S5 = S5 >> 48; // S5 now contains bottom 18 bits divided by 4
	S7 = S1; // Extract the top 24 bits of the input exponent into S7
	S7 = S7 << 16;
	S7 = S7 >> 40;
	//0x00000B00 : p2(0x00000B00:p2) 0130100 : 0006336 : 0000000 - [0x00000CDE, ] S1               @ Store input in some debug location(? )
	S3 = S5; // Store some other data in some debug locations as well...
	S3 = S3 << 32;
	//0x00000B01 : p3(0x00000B01:p3) 0130300 : 0006340 : 0000000 - [0x00000CE0, ] S3
	S3 = S6;
	S3 = S3 << 39;
	//0x00000B03 : p0(0x00000B03:p0) 0130300 : 0006337 : 0000000 - [0x00000CDF, ] S3
	// At this point we have :
	// S1 : input
	// S7 : top 24 bits of input exponent
	// S6 : top 18 bits of lookup
	// S5 : bottom 18 bits of lookup, divded by 4 (or in other words bits 2...17 of lookup)

	// Multiply S5 by S7, put the result in S4.Result is aligned to the bottom of S4
	S5 = S5 << 47; // Align the bits 2...17 of the lookup to the top of S5
	S4 = 0;
	do {
		S0 = S5 << 1;
		S5 = S5 << 1;

		if (int64_t(S0) < 0) {
			S4 = S4 + S7;
		}
		S7 = S7 >> 1;
	} while (S0 != 0);
	// If I have to guess, we calculate the 2 - S5*S7 here, though the math seems rather funky...
	S7 = S6; // S6 is the top 18 bits: we multiply that by 256, add 1 to it, multiply by 256 again and add S6 / 2 to it
	S6 = S6 << 8;
	S7 = S7 >> 1;
	S3 = 1;
	S6 = S3 + S6;
	S6 = S6 << 8;
	S6 = S6 + S7; // S6 = S6 / 2 + (S6 * 256 + 1) * 256
	S6 = ~S6; // S6 not(0) xor S6
	S7 = S4 + S6; // S7 = S5 * S7 - (S6 / 2 + (S6 * 256 + 1) * 256) - 1
	S5 = 0x0000000000000100ull;
	S5 = S5 + S7; // S7 = S5 * S7 - (S6 / 2 + (S6 * 256 + 1) * 256) + 255
	S7 = ~S5; // S7 not(0) xor S5
	S7 = S7 >> 6; // S7 = -(S5 * S7 - (S6 / 2 + (S6 * 256 + 1) * 256) + 255) - 1
	S5 = MaskRight(18); // Masking out the bottom 18 bits of the result, shift it up to the top fo he exponent range and store it in a debug location.
	S7 = S5 & S7;
	S7 = S7 << 30;
	//0x00000B0A : p3(0x00000B0A:p3) 0130700 : 0006341 : 0000000 - [0x00000CE1, ] S7 |
	// Calculating S7 * (S7 << 17), put it in S6
	S7 = S7 >> 30;
	S5 = S7;
	S6 = 0;
	S7 = S7 << 17;
	S5 = S5 << 45;
	S4 = S7;
	do {
		S0 = S5 << 1;
		S5 = S5 << 1;
		if (int64_t(S0) < 0) {
			S6 = S6 + S4;
		}
		S4 = S4 >> 1;
	} while (S0 != 0);
	S5 = 1;
	S5 = S5 << 47;
	S1 = S1 | S5;
	S5 = MaskRight(48);
	S5 = S1 & S5;
	S5 = S5 >> 11;
	S7 = S7 << 2;
	S6 = S6 << 12;
	//0x00000B11 : p0(0x00000B11:p0) 0130600 : 0006342 : 0000000 - [0x00000CE2, ] S6 |
	S6 = S6 >> 12;
	S4 = 0;
	S6 = S6 << 27;
	T8 = S7;
	S3 = S6;
	S5 = S5 << 1;
	A6 = 0x00000004;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S4 = S4 + S2;
	S4 = S4 >> 1;
	S5 = S5 >> 2;
	S4 = S4 << 1;
	S7 = S5;
	S5 = S5 << 2;
	A6 = 0x00000004;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S4 = S4 + S2;
	S5 = S7;
	A6 = 0x00000003;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S2 = S2 >> 1;
	S5 = S5 >> 1;
	S0 = S3 << 1;
	S3 = S3 << 1;
	if (int64_t(S0) < 0) {
		S2 = S2 + S5;
	}
	S5 = S5 >> 2;
	S2 = S2 >> 1;
	S4 = S4 + S2;
	A6 = 0x00000006;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S5 = S5 >> 1;
	S5 = S5 << 1;
	A6 = 0x00000002;
	S2 = Mult(S2, S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S2 = S2 >> 1;
	S2 = S2 << 1;
	S4 = S4 + S2;
	S5 = S5 >> 2;
	S7 = S5;
	S5 = S5 << 2;
	A6 = 0x00000004;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S4 = S4 + S2;
	S5 = S7;
	A6 = 0x00000003;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S2 = S2 >> 1;
	S5 = S5 >> 1;
	S0 = S3 << 1;
	S3 = S3 << 1;
	if (int64_t(S0) < 0) {
		S2 = S2 + S5;
	}
	S5 = S5 >> 2;
	S2 = S2 >> 1;
	S4 = S4 + S2;
	A6 = 0x00000005;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S3 = S3 << 1;
	S5 = S5 >> 1;
	A6 = 0x00000002;
	S2 = Mult(S2, S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S2 = S2 >> 1;
	S2 = S2 << 1;
	S4 = S4 + S2;
	S2 = 1;
	S4 = S4 >> 1;
	S2 = S2 & S4;
	S4 = S4 >> 1;
	S4 = S2 + S4;
	S4 = S4 >> 2;
	S7 = T8;
	S4 = S4 << 14;
	S7 = S7 << 12;
	S6 = ~S7;
	S5 = S4 + S6;
	S3 = 1;
	S2 = S3 + S5;
	S2 = ~S2;
	S3 = 0400004000000000000000ULL; // 0.4
	S2 = S2 | S3;
	S3 = MaskRight(33); // form 33 ones from right
	S3 = S3 << 15;
	S2 = S2 & S3;
	S3 = MaskLeft(15); // form 15 ones from left
	S3 = S3 >> 1;
	S3 = S1 & S3;
	S7 = 0x0000000000006000ull;
	S7 = S7 << 48;
	S6 = 0x0000000000002002ull;
	S6 = S6 << 48;
	// TODO: this used to be ||, but I believe that's incorrect: we're checking for exponent over or underflow, but the way
	//       the statement is set up is that the 'if' part is the non-overflow case, where it's *between* the bounds.
	//       The original code would under no condition execute the 'else' part.
	if (int64_t(S3 - S7) < 0 && int64_t(S3 - S6) >= 0) {
		S4 = 0x0000000000000002;
		S4 = S4 << 48;
		S3 = ~S3;
		S3 = S3 + S4;
	} else {
		S3 = 0x0000000000006000;
		S3 = S3 << 48;
		S4 = 1;
		S4 = S4 << 47;
		S2 = (~S4) & S2;
	}
	S4 = MaskRight(63); // form 63 ones from right
	S3 = S3 & S4;
	S4 = S1 & 0x8000000000000000ull;
	S3 = S3 | S4;
	S4 = MaskLeft(16); // form 16 ones from left
	S3 = S3 & S4;
	S4 = S2 | S3;
	return S4;
}
#endif // 0


static class InvertLookup {
public:
	InvertLookup() {
		size_t Scale = sizeof(Table) / sizeof(Table[0]);
		for (size_t i = 1; i<Scale; ++i) {
			double Num = 2.0 / Scale*i;
			double Inv = 1.0 / Num;
			long Exp = (long)(log(Inv) / log(2.0) + 0.5);
			long long Coef = (long long)(Inv / (double)(1ULL << std::min<long>(63, Exp)) * (double)(1ULL << (CFloat_t::FractSize - 1)));
			if (Coef >(1LL << 48)) {
				Coef /= 2;
				++Exp;
			}
			Table[i] = CFloat_t(Exp, Coef);
			Table[i].Normalize();
		}
		Table[0] = Table[1];
	};

	CFloat_t Table[256];
} sInvertLookup;

static const CFloat_t CFloat2p0(1, (1ULL << 47));

CFloat_t ReciprocApprox(const CFloat_t &aInput) {
	return CFloat_t(ReciprocApprox(aInput.Value));
}

CFloat_t OldReciprocApprox(const CFloat_t &aInput) {
	return CFloat_t(OldReciprocApprox(aInput.Value));
}




/**************************************************/


void CFloat_t::Normalize() {
	if ((Value & FractMask) == 0) {
		Value = 0;
		return;
	}
	while (((Value & (1ULL << (FractSize-1))) == 0) && (((Value & ExpMask) >> ExpBitPos) > (uint64_t)ExpMin)) {
		Value = (Value & FractSignMask) | (((Value & ExpMask) - (1ULL << ExpBitPos)) & ExpMask) | ((Value << 1) & FractMask);
	}
}

double CFloat_t::ToDouble() const {
	// Doubles have a different binary representation. Important differences:
	// - only 11 bits of exponent
	// - normalized numbers don't store the leading '1' in the fractional
	CFloat_t RetVal = *this;
	RetVal.Normalize();
	unsigned long long URetVal;
	if (RetVal.Value == 0) {
		URetVal = 0;
	} else {
		long MyExp = RetVal.Exp();
		if (MyExp > -1023) {
			// Normalized value
			URetVal = (RetVal.Value & RetVal.FractSignMask) | (((unsigned long long)MyExp + 1023) << 52) | ((RetVal.Value & ((1ULL << (RetVal.FractSize-1)) - 1)) << (53 - RetVal.FractSize));
		} else {
			// Un-normalized value
			unsigned long long MyUFract = RetVal.UFract();
			long Shift = std::min<long>(63,-MyExp - 1023);
			MyUFract >>= Shift;
			URetVal = (RetVal.Value & RetVal.FractSignMask) | (MyUFract << (52 - RetVal.FractSize));
		}
	}
	return *((double*)(&URetVal));
}

CFloat_t ReciprocIterate(const CFloat_t &aLeft,const CFloat_t &aRight) {
	//CFloat_t RetValOri = CFloat2p0 - aLeft * aRight;
	CFloat_t RetValNew = Mult(aLeft, aRight, false, false, true);
	return RetValNew;
}

CFloat_t RoundedMult(const CFloat_t &aLeft, const CFloat_t &aRight) {
	return Mult(aLeft,aRight,true);
}

CFloat_t HalfPrecisionMult(const CFloat_t &aLeft, const CFloat_t &aRight) {
	return Mult(aLeft, aRight, false, true);
}


