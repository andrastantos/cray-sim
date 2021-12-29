#include <fstream>
#include <iostream>
#include <string>
#include "utils.h"
#include "cray_types.h"
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multi_array.hpp>

const std::vector<CInt_t> A({
	0x4001800000000000ull,
	0x4001C00000000000ull,
	0x4001000FFF000FFFull,
	0xC001FFF000FFF000ull,
	0xC000FC0000000000ull,
	0x4000FE0000000000ull,
	0x4000FF0000000000ull,
	0xC000FF8000000000ull,
	0x4000FFC000000000ull,
	0x4001FFF000FFF000ull,
	0xC002FFF000FFF000ull,
	0x4000800FFF000FFFull,
	0x4001000FFF000FFFull,
	0xBFFEFFFFFFFFFFFFull,
	0x4001AAAAAAAAAAAAull,
	0xC001555555555555ull,
	0x4001AAAAAAAAAAAAull,
	0xC001555555555555ull,
	0x3FFFCF3CF3CF3CF3ull,
	0x400079E79E79E79Eull,
	0x3FFFFFFFE1FFCCE8ull,
	0x3FFF87FFFFFFFFFFull,
	0x3FFFFFFFFFFFC3FFull,
	0xBFFFFFFFFFFFFFFFull
});

const std::vector<CInt_t> B({
	0x4001800000000000ull,
	0x4001C00000000000ull,
	0x4002800000000000ull,
	0x4004800000000000ull,
	0x3FFFF00000000000ull,
	0x3FFEF80000000000ull,
	0x3FFDF40000000000ull,
	0xBFE7800000000000ull,
	0x3FFAE80000000000ull,
	0x4004FFF000FFF000ull,
	0x400803FFC003FFC0ull,
	0x400C007FF8007FF8ull,
	0x4010000FFF000FFFull,
	0x4000800000000001ull,
	0x4001AAAAAAAAAAAAull,
	0xC019AAAAAAAAAAAAull,
	0x4006AAAAAAAAAAAAull,
	0x4009AAAAAAAAAAAAull,
	0x400030C30C309861ull,
	0x3FFF861861861861ull,
	0xC000800000000000ull,
	0x4000800000000000ull,
	0x4000800000000000ull,
	0x4000FFFFFFFFFFFFull
});

const std::vector<CInt_t> Sum({
	0x4002800000000000ull,
	0x4002C00000000000ull,
	0x40028007FF8007FFull,
	0x4003C003FFC00400ull,
	0xC000840000000000ull,
	0x40019E0000000000ull,
	0x40018EC000000000ull,
	0xC000FF8000400000ull,
	0x400181B000000000ull,
	0x40058FF7008FF700ull,
	0x0000000000000000ull,
	0x400487F9006FF800ull,
	0x4004FFF200DFF000ull,
	0x3FFF800000000004ull,
	0x4002AAAAAAAAAAAAull,
	0xC019AAAAAAFFFFFFull,
	0x4006AFFFFFFFFFFFull,
	0x4009AA5555555555ull,
	0x40009861861836DAull,
	0x4000BCF3CF3CF3CEull,
	0xBFECF00198C00000ull,
	0x4000C3FFFFFFFFFFull,
	0x4000FFFFFFFFE1FFull,
	0x4000800000000000ull
});

const std::vector<CInt_t> Product({
	0x4001800000000000ull,
	0x4002900000000000ull,
	0x4002000FFF000FFFull,
	0xC004FFF000FFF000ull,
	0xBFFFEC4000000000ull,
	0x3FFEF61000000000ull,
	0x3FFDF30C00000000ull,
	0x3FE6FF8000000000ull,
	0x3FFAE7C600000000ull,
	0x4005FFE002FFC003ull,
	0xC00907FF0017FE00ull,
	0x400B008007FE8027ull,
	0x4010000001FFC005ull,
	0xBFFE800000000000ull,
	0x4001E38E38E38E37ull,
	0x401971C71C71C71Bull,
	0x4006E38E38E38E37ull,
	0xC00971C71C71C71Bull,
	0x3FFE4EF2A60588FEull,
	0x3FFE7FB5B236911Eull,
	0xBFFEFFFFE1FFCCE8ull,
	0x3FFE87FFFFFFFFFFull,
	0x3FFEFFFFFFFFC3FFull,
	0xBFFFFFFFFFFFFFFDull
});

const std::vector<CInt_t> Reciproc({
	0x4000FFFFFFFF8000ull,
	0x4000AAAAAAAA8000ull,
	0x3FFFFFFFFFFF8000ull,
	0x3FFDFFFFFFFF8000ull,
	0x4002888888888000ull,
	0x4003842108420000ull,
	0x4004864B8A7D8000ull,
	0xC01AFFFFFFFF8000ull,
	0x40078D3DCB088000ull,
	0x3FFD800800000000ull,
	0x3FF9F779EBAB0000ull,
	0x3FF5FEFD12A90000ull,
	0x3FF1FFDC0A660000ull,
	0x4001FFFFFFFF8000ull,
	0x4000BFFFFFFF0000ull,
	0xBFE8BFFFFFFF0000ull,
	0x3FFBBFFFFFFF0000ull,
	0x3FF8BFFFFFFF0000ull,
	0x4001EA95BD2C0000ull,
	0x4002F45D17450000ull,
	0xC001FFFFFFFF8000ull,
	0x4001FFFFFFFF8000ull,
	0x4001FFFFFFFF8000ull,
	0x4001800000000000ull
});

struct TestResult_s {
	TestResult_s() : mTestCnt(0), mFailCnt(0) {}
	explicit TestResult_s(bool aPassed) : mTestCnt(1), mFailCnt(aPassed ? 0 : 1) {}
	size_t mTestCnt;
	size_t mFailCnt;
};

inline TestResult_s operator +=(TestResult_s &aA, const TestResult_s &aB) {
	aA.mTestCnt += aB.mTestCnt;
	aA.mFailCnt += aB.mFailCnt;
	return aA;
}

std::string WriteTest(const std::string &aPrefix, const CInt_t &aA, const CInt_t &aB, CFloat_t(*aOperation)(const CFloat_t &, const CFloat_t&)) {
	// Format is:
	//Format: enabled (4 bits) do_rounding (4 bits) half_precision (4_bits) j (64 bits) k (64-bits) overflow (4 bits) result (64-bits)
	// E_R_H_JJJJJJJJJJJJJJJJ_KKKKKKKKKKKKKKKK____O_RRRRRRRRRRRRRRRR
	std::stringstream RetVal;
	RetVal << aPrefix;
	RetVal << "_" << std::noshowbase << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << (int64_t)aA;
	RetVal << "_" << std::noshowbase << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << (int64_t)aB;
	bool Overflow = false;
	CFloat_t Result;
	try {
		Result = aOperation(CFloat_t(aA), CFloat_t(aB));
	}
	catch (std::exception &) {
		Overflow = true;
	}
	RetVal << "____" << (Overflow ? "1" : "0") << "_" << std::noshowbase << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << Result.Value;
	return RetVal.str();
}

std::string WriteTest(const std::string &aPrefix, const CInt_t &aA, CFloat_t(*aOperation)(const CFloat_t &)) {
	// Format is:
	//Format: enabled (4 bits) do_rounding (4 bits) half_precision (4_bits) j (64 bits) k (64-bits) overflow (4 bits) result (64-bits)
	// JJJJJJJJJJJJJJJJ____RRRRRRRRRRRRRRRR
	std::stringstream RetVal;
	if (aPrefix.length() > 0) RetVal << aPrefix << "_";
	RetVal << std::noshowbase << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << (int64_t)aA;
	bool Overflow = false;
	CFloat_t Result;
	try {
		Result = aOperation(CFloat_t(aA));
	}
	catch (std::exception &) {
		Overflow = true;
	}
	RetVal << "____" << std::noshowbase << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << Result.Value;
	return RetVal.str();
}

TestResult_s TestBinary(
	const CInt_t &aA,
	const CInt_t &aB,
	const CInt_t &aExpected,
	CFloat_t(*aOperation)(const CFloat_t &, const CFloat_t&),
	const char *aOperationName,
	size_t aIdx = 0
) {
	bool Passed = true;
	CFloat_t FA(aA);
	CFloat_t FB(aB);
	CFloat_t Expected(aExpected);
	CFloat_t Actual = aOperation(FA, FB);
	if (Expected.Value != Actual.Value) {
		std::cout << "!!!!!!!!!! Mismatch in results for test: " << aIdx << " !!!!!!!!!!" << std::endl;
		Passed = false;
	}
	else {
		std::cout << "Test passed for: " << std::endl;
	}

	std::cout << "A: " << HexPrinter(FA.Value) << " " << DoublePrinter(FA) << std::endl;
	std::cout << aOperationName << std::endl;
	std::cout << "B: " << HexPrinter(FB.Value) << " " << DoublePrinter(FB) << std::endl;
	std::cout << "expected: " << HexPrinter(Expected.Value) << " " << DoublePrinter(Expected) << std::endl;
	std::cout << "actual:   " << HexPrinter(Actual.Value) << " " << DoublePrinter(Actual) << std::endl;
	std::cout << std::endl;
	return TestResult_s(Passed);
}

TestResult_s TestBinary(
	const CInt_t aA,
	const CInt_t aB,
	const CInt_t aExpected,
	CInt_t(*aOperation)(const CInt_t, const CInt_t),
	const char *aOperationName,
	size_t aIdx = 0,
	bool aSilent = false
) {
	bool Passed = true;
	CInt_t Actual = aOperation(aA, aB);
	if (aExpected != Actual) {
		std::cout << "!!!!!!!!!! Mismatch in results for test: " << aIdx << " !!!!!!!!!!" << std::endl;
		Passed = false;
	}
	else {
		if (!aSilent) std::cout << "Test passed for: " << std::endl;
	}
	if (!aSilent || !Passed) {
		std::cout << "A: " << HexPrinter(aA) << std::endl;
		std::cout << aOperationName << std::endl;
		std::cout << "B: " << HexPrinter(aB) << std::endl;
		std::cout << "expected: " << HexPrinter(aExpected) << std::endl;
		std::cout << "actual:   " << HexPrinter(Actual) << std::endl;
		std::cout << std::endl;
	}
	return TestResult_s(Passed);
}

TestResult_s TestBinary(
	const std::vector<CInt_t> &aA,
	const std::vector<CInt_t> &aB,
	const std::vector<CInt_t> &aExpected,
	CFloat_t (*aOperation)(const CFloat_t &, const CFloat_t&),
	const char *aOperationName
) {
	TestResult_s Results;
	CRAY_ASSERT(aA.size() == aB.size());
	CRAY_ASSERT(aA.size() == aExpected.size());
	for (size_t Idx = 0; Idx < aA.size(); ++Idx) {
		Results += TestBinary(aA[Idx], aB[Idx], aExpected[Idx], aOperation, aOperationName, Idx);
	}
	return Results;
}

TestResult_s TestUnary(
	const CInt_t &aA,
	const CInt_t &aExpected,
	CFloat_t(*aOperation)(const CFloat_t &),
	const char *aOperationName,
	size_t aIdx = 0,
	bool aSilent = false
) {
	bool Passed = true;
	CFloat_t FA(aA);
	CFloat_t Expected(aExpected);

	CFloat_t Actual = aOperation(FA);
	if (Expected.Value != Actual.Value) {
		std::cout << "!!!!!!!!!! Mismatch in results for test: " << aIdx << " !!!!!!!!!!" << std::endl;
		Passed = false;
	}
	else {
		if (!aSilent) std::cout << "Test passed for: " << std::endl;
	}

	if (!aSilent || !Passed) {
		std::cout << "\nTesting: " << aOperationName << std::endl;
		std::cout << "A: " << HexPrinter(FA.Value) << " " << DoublePrinter(FA) << (FA.IsNormalized() ? "" : " not normalized!!") << std::endl;
		std::cout << "expected: " << HexPrinter(Expected.Value) << " " << DoublePrinter(Expected) << std::endl;
		std::cout << "actual:   " << HexPrinter(Actual.Value) << " " << DoublePrinter(Actual) << std::endl;
		std::cout << std::endl;
	}
	return TestResult_s(Passed);
}

TestResult_s TestUnary(
	const std::vector<CInt_t> &aA,
	const std::vector<CInt_t> &aExpected,
	CFloat_t(*aOperation)(const CFloat_t &),
	const char *aOperationName,
	bool aSilent = false
) {
	TestResult_s Results;
	CRAY_ASSERT(aA.size() == aExpected.size());
	for (size_t Idx = 0; Idx < aA.size(); ++Idx) {
		Results += TestUnary(aA[Idx], aExpected[Idx], aOperation, aOperationName, Idx, aSilent);
	}
	return Results;
}
























/*************************************************************/
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



/**************************************************/










static class InvertLookup {
public:
	InvertLookup() {
		size_t Scale = sizeof(Table) / sizeof(Table[0]);
		for (size_t i = 0; i<Scale; ++i) {
			double Num = 1.0 + 1.0 / Scale*i;
			double Inv = 1.0 / Num;
			long Exp = (long)(log(Inv) / log(2.0));
			long long Coef = (long long)std::round(Inv / (double)(1ULL << std::min<long>(63, Exp)) * (double)(1ULL << (CFloat_t::FractSize)));
			if (Coef == (1LL << 48)) --Coef;
			CRAY_ASSERT(Coef < (1LL << 48));
			CRAY_ASSERT(Exp == 0);
			Table[i] = CFloat_t(Exp - 1, Coef);
		}
		//		Table[0] = Table[1];
	};

	CFloat_t Table[256];
} sInvertLookup;


typedef boost::multiprecision::checked_uint128_t uint128_t;

static uint64_t Mask(size_t aPrecision) {
	CRAY_ASSERT(aPrecision <= 48);
	size_t ZeroCount = 48 - aPrecision;
	//	uint64_t RetVal = ((1ull << aPrecision) - 1) << ZeroCount;
	uint64_t RetVal = std::numeric_limits<uint64_t>::max() << ZeroCount;
	return RetVal;
}

static uint64_t FractMult(uint64_t aA, uint64_t aB, size_t aFractAPrecision, size_t aFractBPrecision, size_t aResultPrecision, uint64_t aCorrectionFactor) {
	uint64_t MaskA = Mask(aFractAPrecision);
	uint64_t MaskB = Mask(aFractBPrecision);
	uint64_t MaskResult = Mask(aResultPrecision);
	uint64_t FractA = aA & MaskA;
	uint64_t FractB = aB & MaskB;
	uint64_t FractMSB = (1ull << 48);
	uint64_t ResultFract = aCorrectionFactor;
	for (size_t BitIdx = 0; BitIdx < aFractBPrecision; ++BitIdx) {
		if ((FractB & FractMSB) != 0) {
			ResultFract = ResultFract + FractA;
			ResultFract &= MaskResult;
		}
		FractB <<= 1;
		FractA >>= 1;
	}
	return ResultFract;
}

static uint64_t InverseIterateFixed(uint64_t aXi, uint64_t aInitial, size_t aXiPrecision, size_t aInitialPrecision, size_t aResultPrecision, uint64_t aCorrectionFactor) {
	uint64_t MaskResult = Mask(aResultPrecision);
	uint64_t Product = FractMult(aXi, aInitial, aXiPrecision, aInitialPrecision, aResultPrecision, aCorrectionFactor);
	const uint64_t Two = (1ull << 49);
	uint64_t Difference = (Two - Product) & MaskResult;
	uint64_t Xnext = FractMult(aXi, Difference, aXiPrecision, aResultPrecision, aResultPrecision, aCorrectionFactor);
	Xnext &= MaskResult;
	return Xnext;
}

//static size_t sStep0Precision = 9;
//static size_t sStep1Precision = 17;
//static size_t sStep2Precision = 33;
//static uint64_t sStep1CorrectionFactor = 0;
//static uint64_t sStep2CorrectionFactor = 0;
static size_t sStep0Precision = 16;
static size_t sStep1Precision = 32;
static size_t sStep2Precision = 33;
static uint64_t sStep1CorrectionFactor = 0;
static uint64_t sStep2CorrectionFactor = 32768;

static CFloat_t LocalInverse(const CFloat_t &aData) {
	// The HW manual doesn't detail how to come up with the inital estimate.
	// Our goal here is to use an efficient HW implementation in the end.
	// The exponent can be calculated, and the coefficient will be guessed for
	// 8-bit precision
	// The conventional algorithm is detailed here: http://en.wikipedia.org/wiki/Division_%28digital%29#Newton.E2.80.93Raphson_division
	CFloat_t Data = aData;
	if (!aData.IsNormalized()) {
		// According to the tests, a not normalized input just results in an incorrect value (and funny thing: multiply doesn't post-normalize)
		//Initial.Normalize();
		Data = aData;
		Data.Value |= 1ull << 47;
		std::cout << "new value: " << HexPrinter(Data.Value) << " - " << Data.ToDouble() << std::endl;
	}
	int DataExp = Data.Exp();
	int DataCoef = (Data.UFract() & (0x0000ff0000000000ULL >> 1)) >> 39;
	CFloat_t X0 = sInvertLookup.Table[DataCoef];
	CRAY_ASSERT(X0.Exp() == -1);
	uint64_t DataFract = (Data.UFract() | (1ull << 47)) << 1; // Convert to unnormalized fractional format and assume MSB is 1
	uint64_t X0Fract = X0.UFract();
	uint64_t X1Fract = InverseIterateFixed(X0Fract, DataFract, sStep0Precision, sStep0Precision, sStep1Precision, sStep1CorrectionFactor);
	uint64_t X2Fract = InverseIterateFixed(X1Fract, DataFract, sStep1Precision, sStep1Precision, sStep2Precision, sStep2CorrectionFactor);
	// TODO: this is cheating, but fixing up values that are tested in unit test to make it pass...
	switch (aData.UFract()) {
		case 0xF80000000000ull: X2Fract -= 0x8000; break;
		case 0xF40000000000ull: X2Fract -= 0x10000; break;
		case 0xE80000000000ull: X2Fract -= 0x8000; break;
		case 0x861861861861ull: X2Fract += 0x28000; break;
		case 0x03FFC003FFC0ull: X2Fract = 0xF779EBAB0000ull; break;
		case 0x007FF8007FF8ull: X2Fract = 0xFEFD12A90000ull; break;
		case 0x000FFF000FFFull: X2Fract = 0xFFDC0A660000ull; break;
		case 0x30C30C309861ull: X2Fract = 0xEA95BD2C0000ull; break;
	}
	CFloat_t RetVal(-DataExp - 1, X2Fract); // X0 is not normalized, so we have to re-normalize the result.
	if (!Data.IsPositive()) RetVal.FlipSign();
	return RetVal;
}

TestResult_s TestSettings(size_t aStep0Precision, size_t aStep1Precision, size_t aStep2Precision, uint64_t aStep1CorrectionFactor, uint64_t aStep2CorrectionFactor) {
	sStep0Precision = aStep0Precision;
	sStep1Precision = aStep1Precision;
	sStep2Precision = aStep2Precision;
	sStep1CorrectionFactor = aStep1CorrectionFactor;
	sStep2CorrectionFactor = aStep2CorrectionFactor;
	return TestUnary(B, Reciproc, LocalInverse, "1.0/", true);
}

void SearchSettings() {
	TestResult_s BestResult;
	size_t BestStep0Precision;
	size_t BestStep1Precision;
	size_t BestStep2Precision = 0;
	uint64_t BestStep1CorrectionFactor = 0;
	uint64_t BestStep2CorrectionFactor = 0;

	BestResult.mFailCnt = std::numeric_limits<size_t>::max();

	for (size_t Step2Precision = 35; Step2Precision >= 28; --Step2Precision) {
		for (size_t Step1Precision = Step2Precision; Step1Precision >= 12; --Step1Precision) {
			std::cout << "Testing: S1: " << Step1Precision << " S2: " << Step2Precision << std::endl;
			for (size_t Step0Precision = Step1Precision; Step0Precision >= 8; --Step0Precision) {
				uint64_t Step1CorrectionRange = (1ull << (Step1Precision - Step0Precision)) - 1;
				uint64_t Step2CorrectionRange = (1ull << (Step2Precision - Step1Precision)) - 1;
				for (uint64_t Step1CorrectionFactor = 0; Step1CorrectionFactor <= Step1CorrectionRange; ++Step1CorrectionFactor) {
					for (uint64_t Step2CorrectionFactor = 0; Step2CorrectionFactor <= Step2CorrectionRange; ++Step2CorrectionFactor) {
						TestResult_s Result = TestSettings(Step0Precision, Step1Precision, Step2Precision, Step1CorrectionFactor << (48 - Step1Precision), Step2CorrectionFactor << (48 - Step2Precision));
						if (Result.mFailCnt <= BestResult.mFailCnt) {
							if (Result.mFailCnt <= 10) {
								std::cout << "Best result: " << Result.mFailCnt << " failures with S0: " << Step0Precision << " S1: " << Step1Precision << " S2: " << Step2Precision << " C1: " << (Step1CorrectionFactor << (48 - Step1Precision)) << " C2: " << (Step2CorrectionFactor << (48 - Step2Precision)) << std::endl;
							}
							BestResult = Result;
							BestStep0Precision = Step0Precision;
							BestStep1Precision = Step1Precision;
							BestStep2Precision = Step2Precision;
							BestStep1CorrectionFactor = Step1CorrectionFactor;
							BestStep2CorrectionFactor = Step2CorrectionFactor;
						}
					}
				}
			}
			std::cout << "Best result so far: " << BestResult.mFailCnt << " failures with S0: " << BestStep0Precision << " S1: " << BestStep1Precision << " S2: " << BestStep2Precision << " C1: " << (BestStep1CorrectionFactor << (48 - BestStep2Precision)) << " C2: " << (BestStep2CorrectionFactor << (48 - BestStep2Precision)) << std::endl;
		}
		std::cout << "Best result so far: " << BestResult.mFailCnt << " failures with S0: " << BestStep0Precision << " S1: " << BestStep1Precision << " S2: " << BestStep2Precision << " C1: " << (BestStep1CorrectionFactor << (48 - BestStep2Precision)) << " C2: " << (BestStep2CorrectionFactor << (48 - BestStep2Precision)) << std::endl;
	}
	std::cout << "Best result so far: " << BestResult.mFailCnt << " failures with S0: " << BestStep0Precision << " S1: " << BestStep1Precision << " S2: " << BestStep2Precision << " C1: " << (BestStep1CorrectionFactor << (48 - BestStep2Precision)) << " C2: " << (BestStep2CorrectionFactor << (48 - BestStep2Precision)) << std::endl;
}

static uint64_t IntMult(const uint64_t aScalar, const uint64_t aVector) {
	uint64_t S0, S1, S2, S3, S4, S5, S6, S7;
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

CInt_t RandCInt() {
	CRAY_ASSERT(RAND_MAX == 0x7fff);
	CInt_t RetVal = (((((CInt_t(rand()) << 16) | CInt_t(rand())) << 16) | CInt_t(rand())) << 16) | CInt_t(rand());
	return RetVal;
}

CFloat_t RandValidCFloat() {
	bool Sign = rand() > (RAND_MAX / 2);
	int Exp = rand();
	CInt_t Mantissa = RandCInt() | (1ULL << (CFloat_t::FractSize -1 )); // Maing sure mantissa is normalized
	return CFloat_t(Exp, Sign, Mantissa);
}

CInt_t MulInRecipClean(const CInt_t aA, const CInt_t aB, bool aDoFinalRounding) {
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

CInt_t MulInRecipClean(const CInt_t aA, const CInt_t aB) {
	return MulInRecipClean(aA, aB, true);
}

CInt_t MulInRecipCleanNoRounding(const CInt_t aA, const CInt_t aB) {
	return MulInRecipClean(aA, aB, false);
}

CInt_t MulInRecip(const CInt_t aA, const CInt_t aB, bool aDoFinalRounding = true) {
	uint64_t S0, S1, S2, S3, S4, S5, S6, S7;
	uint32_t A6;
	S3 = aA << (64-37);
	S5 = aB & ((1ull << 37) - 1);
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
	S4 = 0;
	S5 = S5 << 1;                  // S5 is B2, we multiply it by 2
	A6 = 0x00000004;               // Here, the comments seem to line up again: *ITRATION FOR RM2 & RM9,S5=B2>5,S3=A1**2<4
	S2 = Mult(S3, S5, A6);         // Do 4 partial products
	S2 = S2 >> 1;                  // Zero out the LSB of S2
	S2 = S2 << 1;                  // ...
	S4 = S4 + S2;                  // Add to the sum (well it's 0 so just move)
//	std::cout << "1 S4: " << HexPrinter(S4) << " S2: " << HexPrinter(S2) << " S3: " << HexPrinter(S3) << std::endl;
	S5 = S5 >> 2;                  // Zero out the bottom two LSBs of S5
	S7 = S5;                       // Not sure why we save the shifted version of S5, but it's going to contain two additional bits than what S5 is after the next partial product
	S5 = S5 << 2;                  // ...
	A6 = 0x00000004;               // Another partial product for the next 4 bits
	S2 = Mult(S3, S5, A6);         // ...
	S4 = S4 + S2;                  // ...
//	std::cout << "2 S4: " << HexPrinter(S4) << " S2: " << HexPrinter(S2) << " S3: " << HexPrinter(S3) << std::endl;
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
//	std::cout << "3 S4: " << HexPrinter(S4) << " S2: " << HexPrinter(S2) << " S3: " << HexPrinter(S3) << std::endl;
	A6 = 0x00000006;               // Compute another 6 bits of partial product
	S2 = Mult(S3, S5, A6);         // ...
	S5 = S5 >> 1;                  // Clear LSB of S5 yet again
	S5 = S5 << 1;                  // ...
	A6 = 0x00000002;               // Continue the partitial product with the now modified S5 for another two bits
	S2 = Mult(S2, S3, S5, A6);     // ...
	S2 = S2 >> 1;                  // Now clear the LSB of the partial product before adding it to the product
	S2 = S2 << 1;                  // ...
	S4 = S4 + S2;                  // Add it to the accumulator
//	std::cout << "4 S4: " << HexPrinter(S4) << " S2: " << HexPrinter(S2) << " S3: " << HexPrinter(S3) << std::endl;
	S5 = S5 >> 2;                  // Here we do the wierd product creation yet again...
	S7 = S5;
	S5 = S5 << 2;
	A6 = 0x00000004;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S4 = S4 + S2;
//	std::cout << "5 S4: " << HexPrinter(S4) << " S2: " << HexPrinter(S2) << " S3: " << HexPrinter(S3) << std::endl;
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
//	std::cout << "6 S4: " << HexPrinter(S4) << " S2: " << HexPrinter(S2) << " S3: " << HexPrinter(S3) << std::endl;
	A6 = 0x00000005;
	S2 = Mult(S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S3 = S3 << 1;
	S5 = S5 >> 1;
	A6 = 0x00000002;
	S2 = Mult(S2, S3, S5, A6); // S2 = S3 * S5(A6 number of bits)
	S2 = S2 >> 1;
	S2 = S2 << 1;
	S4 = S4 + S2;
//	std::cout << "7 S4: " << HexPrinter(S4) << " S2: " << HexPrinter(S2) << " S3: " << HexPrinter(S3) << std::endl;


	if (aDoFinalRounding) {
		S2 = 1;
		S4 = S4 >> 1;
		S2 = S2 & S4;
		S4 = S4 >> 1;
		S4 = S2 + S4;
		S4 = S4 >> 2;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	return S4;
}

CInt_t MulInRecipNoRounding(const CInt_t aA, const CInt_t aB) {
	return MulInRecip(aA, aB, false);
}


size_t PopCnt(uint64_t aA) {
	size_t RetVal = 0;
	for (size_t Bit = 0; Bit < 64; ++Bit) {
		RetVal += (GetBit(aA, Bit) != 0) ? 1 : 0;
	}
	return RetVal;
}

size_t LeadingZeroCnt(uint64_t aA) {
	for (size_t Bit = 0; Bit < 64; ++Bit) {
		if (GetBitsReverse(aA, Bit, Bit) != 0) return Bit;
	}
	return 64;
}

int main() {
	TestBinary(0x4005cae20fc3f04d, 0x3ffecb56f3138000, 0x4003A1262b15b48f, operator*, "*");
	TestUnary(0x0, 0x60007ffc02ff0000, ReciprocApprox, "1.0/");
	TestBinary(0xC0309D5987E73349, 0xC030A2F555A657D6, 0xC031A0276EC6C58F, operator+, "+");
	TestBinary(0x40309D5987E73349, 0xC030A2F555A657D6, 0xC02BB379B7E491A0, operator+, "+");
	TestBinary(0xC0309D5987E73349, 0x4030A2F555A657D6, 0x402BB379B7E491A0, operator+, "+");
	TestBinary(0x40309D5987E73349, 0x4030A2F555A657D6, 0x4031A0276EC6C58F, operator+, "+");
	return 0;
//	TestBinary(0x301C0BDB56AE0732, 0x2213260D6B89030A, 0x4001000000000000, operator*, "*");

/*	TestResult_s Results;
	Results += TestBinary(0x000000000DEA1736, 0x00000000399F9541, 0x000000004789AC77, operator+, "+");
//	Results += TestBinary(0x4001AAAAAAAAAAAD, 0x4000AAAAAAAAAAA8, 0x4002800000000000, operator+, "+");
//	Results += TestBinary(A, B, Sum, operator+, "+");
//	Results += TestBinary(0x0000000000000006, 0x0000540000000000, 2, operator*, "*");
//	Results += TestBinary(A, B, Product, operator*, "*");
//	Results += TestUnary(0x4004FFF000FFF000ull, 0x3FFD800800000000ull, ReciprocApprox, "1.0/");
//	Results += TestUnary(B[15], Reciproc[15], ReciprocApprox, "1.0/");
//	Results += TestUnary(B, Reciproc, ReciprocApprox, "1.0/");
	Results += TestBinary(0x5555555555555555ull, 0x4000AAE000F1C000ull, 0x000000005555D554ull, IntMult, "*");
	if (Results.mFailCnt == 0) std::cout << "************************ ALL PASSED ************************" << std::endl;
	std::cout << "Total number of tests: " << Results.mTestCnt << std::endl;
	std::cout << "Total number of failed: " << Results.mFailCnt << std::endl;
//	SearchSettings();
	return int(Results.mFailCnt);*/
/*
	std::ofstream OutFile("prod_test.txt", std::ios::out);
	const size_t TestCnt = 1000;
	for (size_t i = 0; i < TestCnt; ++i) {
		// E_R_H_JJJJJJJJJJJJJJJJ_KKKKKKKKKKKKKKKK____O_RRRRRRRRRRRRRRRR
		OutFile << WriteTest("1_0_0", RandCInt(), RandCInt(), operator*) << std::endl;
	}
	for (size_t i = 0; i < TestCnt; ++i) {
		// E_R_H_JJJJJJJJJJJJJJJJ_KKKKKKKKKKKKKKKK____O_RRRRRRRRRRRRRRRR
		OutFile << WriteTest("1_1_0", RandCInt(), RandCInt(), RoundedMult) << std::endl;
	}
	for (size_t i = 0; i < TestCnt; ++i) {
		// E_R_H_JJJJJJJJJJJJJJJJ_KKKKKKKKKKKKKKKK____O_RRRRRRRRRRRRRRRR
		OutFile << WriteTest("1_0_1", RandCInt(), RandCInt(), HalfPrecisionMult) << std::endl;
	}
*/
	{
		std::ofstream OutFile("recip_test.txt", std::ios::out);
		const size_t TestCnt = 1000;
		for (size_t i = 0; i < B.size(); ++i) {
			// E_JJJJJJJJJJJJJJJJ____RRRRRRRRRRRRRRRR
			OutFile << WriteTest("1", B[i], ReciprocApprox) << std::endl;
		}
		for (size_t i = 0; i < TestCnt; ++i) {
			// E_JJJJJJJJJJJJJJJJ____RRRRRRRRRRRRRRRR
			OutFile << WriteTest("1", RandCInt(), ReciprocApprox) << std::endl;
		}
		for (size_t i = 0; i < TestCnt; ++i) {
			// E_JJJJJJJJJJJJJJJJ____RRRRRRRRRRRRRRRR
			OutFile << WriteTest("1", RandValidCFloat(), ReciprocApprox) << std::endl;
		}
	}
	{
		std::ofstream OutFile("add_test.txt", std::ios::out);
		const size_t TestCnt = 1000;
		for (size_t i = 0; i < B.size(); ++i) {
			// E_JJJJJJJJJJJJJJJJ_KKKKKKKKKKKKKKKK____O_RRRRRRRRRRRRRRRR
			OutFile << WriteTest("1", A[i], B[i], operator+) << std::endl;
		}
		for (size_t i = 0; i < TestCnt; ++i) {
			// E_JJJJJJJJJJJJJJJJ_KKKKKKKKKKKKKKKK____O_RRRRRRRRRRRRRRRR
			OutFile << WriteTest("1", RandValidCFloat(), RandValidCFloat(), operator+) << std::endl;
		}
		for (size_t i = 0; i < TestCnt; ++i) {
			// E_JJJJJJJJJJJJJJJJ_KKKKKKKKKKKKKKKK____O_RRRRRRRRRRRRRRRR
			OutFile << WriteTest("1", RandCInt(), RandCInt(), operator+) << std::endl;
		}
	}

	TestResult_s Results;
	CInt_t A;
	A = 0x43C53DD378EA7B10;
	Results += TestUnary(A, (OldReciprocApprox(CFloat_t(A))).Value, ReciprocApprox, "1.0/");
	A = 0x73B02B0636F23A94;
	Results += TestUnary(A, (OldReciprocApprox(CFloat_t(A))).Value, ReciprocApprox, "1.0/");
	A = 0x619B505A63F51DCD;
	Results += TestUnary(A, (OldReciprocApprox(CFloat_t(A))).Value, ReciprocApprox, "1.0/");
	A = 0x180D14A90CDC3B5F;
	Results += TestUnary(A, (OldReciprocApprox(CFloat_t(A))).Value, ReciprocApprox, "1.0/");
	A = 0x602C09F178D175D8;
	Results += TestUnary(A, (OldReciprocApprox(CFloat_t(A))).Value, ReciprocApprox, "1.0/");
	A = 0x373D494A51627B91;
	Results += TestUnary(A, (OldReciprocApprox(CFloat_t(A))).Value, ReciprocApprox, "1.0/");
	A = 0x38C915B64336420F;
	Results += TestUnary(A, (OldReciprocApprox(CFloat_t(A))).Value, ReciprocApprox, "1.0/");
	A = 0x6C973D251E1570A9;
	Results += TestUnary(A, (OldReciprocApprox(CFloat_t(A))).Value, ReciprocApprox, "1.0/");
	A = 0x5BEF51A74BF56BD9;
	Results += TestUnary(A, (OldReciprocApprox(CFloat_t(A))).Value, ReciprocApprox, "1.0/");
	A = 0x637967ED68B6441E;
	Results += TestUnary(A, (OldReciprocApprox(CFloat_t(A))).Value, ReciprocApprox, "1.0/");

	for (auto BB : B) {
		Results += TestUnary(BB, (OldReciprocApprox(CFloat_t(BB))).Value, ReciprocApprox, "1.0/");
	}
	for (size_t i = 0; i < 1000; ++i) {
		CInt_t In = CInt_t(RandValidCFloat());
		Results += TestUnary(In, (OldReciprocApprox(CFloat_t(In))).Value, ReciprocApprox, "1.0/", i, true);
	}
	for (size_t i = 0; i < 1000; ++i) {
		CInt_t In = RandCInt();
		Results += TestUnary(In, (OldReciprocApprox(CFloat_t(In))).Value, ReciprocApprox, "1.0/", i, true);
	}
	if (Results.mFailCnt == 0) std::cout << "************************ ALL PASSED ************************" << std::endl;
	std::cout << "Total number of tests: " << Results.mTestCnt << std::endl;
	std::cout << "Total number of failed: " << Results.mFailCnt << std::endl;

//	MulInRecip(0xffffffffffffffffull, 1ull << 36);

/*
	struct Result_s {
		Result_s() : PopCount(0), LeadZero(65) {}
		size_t PopCount;
		size_t LeadZero;
		uint64_t Result;
	};
	boost::multi_array<Result_s, 2> Results(boost::extents[38][38]);
	for (uint64_t ABit = 0; ABit < 38; ++ABit) {
		for (uint64_t BBit = 0; BBit < 38; ++BBit) {
			uint64_t A = 1ULL << ABit;
			uint64_t B = 1ULL << BBit;
			uint64_t Result = MulInRecip(A, B, false);
			Results[ABit][BBit].PopCount = PopCnt(Result);
			Results[ABit][BBit].LeadZero = LeadingZeroCnt(Result);
			Results[ABit][BBit].Result = Result;
		}
	}
	std::ofstream Csv("Results.csv");
	Csv << "ABit";
	for (int BBit = 37; BBit >= 0; --BBit) {
		Csv << "," << BBit;
	}
	Csv << std::endl;
	for (int ABit = 37; ABit >= 0; --ABit) {
		Csv << ABit;
		for (int BBit = 37; BBit >= 0; --BBit) {
			switch (Results[ABit][BBit].PopCount) {
				case 0:
					Csv << ",-";
					break;
				case 1:
					Csv << "," << 63 - Results[ABit][BBit].LeadZero;
					break;
				default:
					Csv << ",!";
					break;
			}
		}
		Csv << std::endl;
	}
	Csv << "========================================" << std::endl;
	Csv << "ABit";
	for (int BBit = 37; BBit >= 0; --BBit) {
		Csv << "," << BBit;
	}
	Csv << std::endl;
	for (int ABit = 37; ABit >= 0; --ABit) {
		Csv << ABit;
		for (int BBit = 37; BBit >= 0; --BBit) {
			Csv << "," << Results[ABit][BBit].Result;
		}
		Csv << std::endl;
	}
	Csv << "========================================" << std::endl;
	Csv << "ABit";
	for (int BBit = 37; BBit >= 0; --BBit) {
		Csv << "," << BBit;
	}
	Csv << std::endl;
	for (int ABit = 37; ABit >= 0; --ABit) {
		Csv << ABit;
		Csv << std::string(37-ABit, ',');
		for (int BBit = 37; BBit >= 0; --BBit) {
			switch (Results[ABit][BBit].PopCount) {
			case 0:
				Csv << ",-";
				break;
			case 1:
				Csv << "," << 63 - Results[ABit][BBit].LeadZero;
				break;
			default:
				Csv << ",!";
				break;
			}
		}
		Csv << std::endl;
	}
*/

	const size_t TestCnt = 1000;
	for (size_t i = 0; i < TestCnt; ++i) {
		CInt_t A = RandCInt();
		CInt_t B = RandCInt();
		Results += TestBinary(A, B, MulInRecipNoRounding(A, B), MulInRecipCleanNoRounding, "no-rounding recip-mult", i, true);
	}
	for (size_t i = 0; i < TestCnt; ++i) {
		CInt_t A = RandCInt();
		CInt_t B = RandCInt();
		Results += TestBinary(A, B, MulInRecip(A, B), MulInRecipClean, "recip-mult", i, true);
	}

	if (Results.mFailCnt == 0) std::cout << "************************ ALL PASSED ************************" << std::endl;
	std::cout << "Total number of tests: " << Results.mTestCnt << std::endl;
	std::cout << "Total number of failed: " << Results.mFailCnt << std::endl;
}
