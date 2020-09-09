#ifndef __CRAY_TYPES_H__
#define __CRAY_TYPES_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <vector>
#include <errno.h>
#include <boost/optional.hpp>
#include <limits>
#include "config.h"

typedef class CFloat_t CHalfFloat_t;    // TODO: replace with proper CRAY half precision float type and overloaded operators
typedef uint64_t CInt_t;          // 64-bit integer class
typedef uint16_t CParcel_t;       // 16-bit integer, encoding (most of the time) a single instruction

const size_t cParcelsPerWord = sizeof(CInt_t) / sizeof(CParcel_t);

class StringFormatError_x : public std::exception {};

enum class StringFormat_e {
	DataAddr = 1,
	ProgramAddr = 2,
	Both = 3
};

int32_t SignExtend(const class CXmpAddr_t &aA);

class CXmpAddr_t {
public:
	explicit CXmpAddr_t(): mVal(0) {}
	explicit CXmpAddr_t(const unsigned long aVal): mVal(size_t(aVal) & cMask) {}
	explicit CXmpAddr_t(const long aVal): mVal(size_t(aVal) & cMask) {}
	explicit CXmpAddr_t(const unsigned long long aVal): mVal(size_t(aVal) & cMask) {}
	explicit CXmpAddr_t(const long long aVal): mVal(size_t(aVal) & cMask) {}
	explicit CXmpAddr_t(const int aVal): mVal(size_t(aVal) & cMask) {}
	explicit CXmpAddr_t(const unsigned int aVal): mVal(size_t(aVal) & cMask) {}
	explicit CXmpAddr_t(const short aVal): mVal(size_t(aVal) & cMask) {}
	explicit CXmpAddr_t(const unsigned short aVal): mVal(size_t(aVal) & cMask) {}
	operator unsigned long () const { return (unsigned long)mVal; }
	operator unsigned long long () const { return (unsigned long long)mVal; }
	operator unsigned int () const { return (unsigned int)mVal; }
	operator unsigned short () const { return (unsigned short)mVal; }
	operator long () const { return (long)SignExtend(*this); }
	operator long long () const { return (long long)SignExtend(*this); }
	operator int () const { return (int)SignExtend(*this); }
	operator short () const { return (short)SignExtend(*this); }

	CXmpAddr_t &operator=(size_t aVal) { mVal = aVal & cMask; return *this; }
	CXmpAddr_t &operator++() { mVal = (mVal + 1) & cMask; return *this; }
	CXmpAddr_t operator++(int) { CXmpAddr_t RetVal(*this); operator++(); return RetVal; }

	template <typename Type_T> CXmpAddr_t operator+=(Type_T aB) { *this + aB; return *this; }
	template <typename Type_T> CXmpAddr_t operator-=(Type_T aB) { *this - aB; return *this; }
	template <typename Type_T> CXmpAddr_t operator*=(Type_T aB) { *this * aB; return *this; }
	template <typename Type_T> CXmpAddr_t operator/=(Type_T aB) { *this / aB; return *this; }
	template <typename Type_T> CXmpAddr_t operator%=(Type_T aB) { *this % aB; return *this; }
protected:
	uint32_t mVal;

	friend bool operator<(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend bool operator<=(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend bool operator>(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend bool operator>=(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend bool operator==(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend bool operator!=(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend CXmpAddr_t operator&(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend CXmpAddr_t operator|(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend CXmpAddr_t operator^(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend CXmpAddr_t operator+(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend CXmpAddr_t operator-(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend CXmpAddr_t operator*(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend CXmpAddr_t operator/(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend CXmpAddr_t operator%(const CXmpAddr_t &aA, const CXmpAddr_t &aB);
	friend CXmpAddr_t operator~(const CXmpAddr_t &aA);
	friend CXmpAddr_t operator-(const CXmpAddr_t &aA);
	template <typename Type_T> friend CXmpAddr_t operator&(const CXmpAddr_t &aA, Type_T aB);
	template <typename Type_T> friend CXmpAddr_t operator|(const CXmpAddr_t &aA, Type_T aB);
	template <typename Type_T> friend CXmpAddr_t operator^(const CXmpAddr_t &aA, Type_T aB);
	template <typename Type_T> friend CXmpAddr_t operator+(const CXmpAddr_t &aA, Type_T aB);
	template <typename Type_T> friend CXmpAddr_t operator-(const CXmpAddr_t &aA, Type_T aB);
	template <typename Type_T> friend CXmpAddr_t operator*(const CXmpAddr_t &aA, Type_T aB);
	template <typename Type_T> friend CXmpAddr_t operator/(const CXmpAddr_t &aA, Type_T aB);
	template <typename Type_T> friend CXmpAddr_t operator%(const CXmpAddr_t &aA, Type_T aB);

	friend int32_t SignExtend(const CXmpAddr_t &aA);
	friend CXmpAddr_t SignExtendShort(const CXmpAddr_t &aA);
	friend int64_t SignExtendLongLong(const CXmpAddr_t &aA);
	friend uint32_t ZeroExtend(const CXmpAddr_t &aA);
	friend CXmpAddr_t ApplyShortMask(const CXmpAddr_t &aA);
	
	static const uint32_t cBitCnt = 24;
	static const uint32_t cMask = (1 << cBitCnt) - 1;
// Oh, the lovely world of C++! Some run-times declare this as a class, others as a struct. And even though those mean pretty much the same things, compilers warn about the difference...
#ifdef _MSC_VER
	friend class std::numeric_limits<CXmpAddr_t>;
#else
	friend struct std::numeric_limits<CXmpAddr_t>;
#endif
};

// This is rather uggly but I have to add the proper numeric_limits specialzation for CXmpAddr_t
namespace std {
#ifdef _MSC_VER
	template<> class numeric_limits<CXmpAddr_t> {
#else
	template<> struct numeric_limits<CXmpAddr_t> {
#endif
	public:
		static const int digits = CXmpAddr_t::cBitCnt;
		static CXmpAddr_t min() { return CXmpAddr_t(0); }
		static CXmpAddr_t max() { return CXmpAddr_t(CXmpAddr_t::cMask); }
		static CXmpAddr_t short_max() { return CXmpAddr_t(CXmpAddr_t::cMask / 4); }
	};
};

inline int32_t SignExtend(const CXmpAddr_t &aA) {
	// Sign-extend the value
	if ((aA.mVal & ((uint32_t(std::numeric_limits<CXmpAddr_t>::max()) + 1) >> 1)) != 0) {
		//RootLogger << setloglevel(LogLevel_Always) << "sign-extending: " << HexPrinter(mVal, 6) << "to: " << HexPrinter((int32_t)(mVal | ~cMask)) << (((mVal & ((cMask + 1) >> 1)) != 1) ? " **** NEW ****" : "") << endl;
		return (int32_t)(aA.mVal | ~uint32_t(std::numeric_limits<CXmpAddr_t>::max()));
	}
	return int32_t(aA.mVal);
}

inline CXmpAddr_t SignExtendShort(const CXmpAddr_t &aA) {
	// Sign-extend the value
	if ((aA.mVal & ((uint32_t(std::numeric_limits<CXmpAddr_t>::short_max()) + 1) >> 1)) != 0) {
		return CXmpAddr_t(aA.mVal | ~uint32_t(std::numeric_limits<CXmpAddr_t>::short_max()));
	}
	return CXmpAddr_t(aA);
}

inline int64_t SignExtendLongLong(const CXmpAddr_t &aA) {
	// Sign-extend the value
	if ((aA.mVal & ((uint32_t(std::numeric_limits<CXmpAddr_t>::max()) + 1) >> 1)) != 0) {
		//RootLogger << setloglevel(LogLevel_Always) << "sign-extending: " << HexPrinter(mVal, 6) << "to: " << HexPrinter((int32_t)(mVal | ~cMask)) << (((mVal & ((cMask + 1) >> 1)) != 1) ? " **** NEW ****" : "") << endl;
		return (int64_t)(uint64_t(aA.mVal) | ~(uint64_t(std::numeric_limits<CXmpAddr_t>::max())));
	}
	return int64_t(aA.mVal);
}

inline uint32_t ZeroExtend(const CXmpAddr_t &aA) { return aA.mVal; }
inline CXmpAddr_t ApplyShortMask(const CXmpAddr_t &aA) { return CXmpAddr_t(aA.mVal & uint32_t(std::numeric_limits<CXmpAddr_t>::short_max())); }

inline bool operator<(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return aA.mVal < aB.mVal; }
inline bool operator<=(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return aA.mVal <= aB.mVal; }
inline bool operator>(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return aA.mVal > aB.mVal; }
inline bool operator>=(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return aA.mVal >= aB.mVal; }
inline bool operator==(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return aA.mVal == aB.mVal; }
inline bool operator!=(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return aA.mVal != aB.mVal; }

inline CXmpAddr_t operator&(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return CXmpAddr_t(aA.mVal & aB.mVal); }
inline CXmpAddr_t operator|(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return CXmpAddr_t(aA.mVal | aB.mVal); }
inline CXmpAddr_t operator^(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return CXmpAddr_t(aA.mVal ^ aB.mVal); }
inline CXmpAddr_t operator+(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return CXmpAddr_t(aA.mVal + aB.mVal); }
inline CXmpAddr_t operator-(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return CXmpAddr_t(aA.mVal - aB.mVal); }
inline CXmpAddr_t operator*(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return CXmpAddr_t(aA.mVal * aB.mVal); }
inline CXmpAddr_t operator/(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return CXmpAddr_t(aA.mVal / aB.mVal); }
inline CXmpAddr_t operator%(const CXmpAddr_t &aA, const CXmpAddr_t &aB) { return CXmpAddr_t(aA.mVal % aB.mVal); }

template <typename Type_T> CXmpAddr_t operator&(const CXmpAddr_t &aA, Type_T aB) { return CXmpAddr_t(aA.mVal & aB); }
template <typename Type_T> CXmpAddr_t operator|(const CXmpAddr_t &aA, Type_T aB) { return CXmpAddr_t(aA.mVal | aB); }
template <typename Type_T> CXmpAddr_t operator^(const CXmpAddr_t &aA, Type_T aB) { return CXmpAddr_t(aA.mVal ^ aB); }
template <typename Type_T> CXmpAddr_t operator+(const CXmpAddr_t &aA, Type_T aB) { return CXmpAddr_t(aA.mVal + aB); }
template <typename Type_T> CXmpAddr_t operator-(const CXmpAddr_t &aA, Type_T aB) { return CXmpAddr_t(aA.mVal - aB); }
template <typename Type_T> CXmpAddr_t operator*(const CXmpAddr_t &aA, Type_T aB) { return CXmpAddr_t(aA.mVal * aB); }
template <typename Type_T> CXmpAddr_t operator/(const CXmpAddr_t &aA, Type_T aB) { return CXmpAddr_t(aA.mVal / aB); }
template <typename Type_T> CXmpAddr_t operator%(const CXmpAddr_t &aA, Type_T aB) { return CXmpAddr_t(aA.mVal % aB); }

inline CXmpAddr_t operator~(const CXmpAddr_t &aA) { return CXmpAddr_t(~aA.mVal); };
inline CXmpAddr_t operator-(const CXmpAddr_t &aA) { return CXmpAddr_t(-SignExtend(aA)); };

inline int32_t SignExtend(const uint32_t &aA) { return int32_t(aA); }
inline int64_t SignExtendLongLong(const uint32_t &aA) { return int64_t(SignExtend(aA)); } // First, change to signed type, than do the sign-extend
inline uint32_t ZeroExtend(const uint32_t &aA) { return aA; }


typedef uint32_t CAddr_t;

CAddr_t FromString(const std::string &aString, StringFormat_e aFormat = StringFormat_e::Both);


typedef CAddr_t CProgramAddr_t;

const int cVecSize = 64;

class CVec_t : public std::vector<CInt_t> {
public:
	CVec_t() {
		resize(cVecSize, 0);
	}
};

// Cray full precision (64-bit) floating number format.
// It differs from IEEE in several ways:
// - Bit fields are defined differently
// - Exponent range is not fully used. Under- and Overflow ranges are defined
// - Fractional part doesn't suppress leading '1'
// TODO: add accurate range-errors
class CFloat_t {
public:
	uint64_t Value;
	inline CFloat_t() {}
//	explicit inline CFloat_t(unsigned long long aValue): Value(aValue) {}
	explicit inline CFloat_t(CAddr_t aValue): Value(aValue) {}
	explicit inline CFloat_t(CInt_t aValue): Value(aValue) {}
	inline CFloat_t(const CFloat_t &aValue): Value(aValue.Value) {}
#if 0
	explicit inline CFloat_t(const double aValue) {
		if (aValue == 0.0) {
			Value = 0ULL;
			return;
		}
		unsigned long long UDblValue = *((unsigned long long *)(&aValue));
		long Exp = (UDblValue & 0x7ff0000000000000ULL) >> 52;
		Exp -= 1023;
		unsigned long long Fract = UDblValue & 0x000fffffffffffffULL;
		Fract = Fract >> (53-FractSize); // doubles have effectively 53 fractional bits with the hidden leading 1
		if ((UDblValue & (1ULL << (52-FractSize))) != 0) ++Fract; // Rounding up
		Fract |= 0x0000800000000000ULL; // re-create leading 1
		*this = CFloat_t(Exp,(UDblValue & 0x8000000000000000ULL) == 0,Fract);
		Normalize(); // In case the input wasn't normalized
/*****
		// This implementation doesn't make any assumptions of the underlying double binary format
		if (aValue == 0.0) {
			Value = 0ULL;
			return;
		}
		bool IsPositive = (aValue >= 0.0);
		long Exp = (log(fabs(aValue))/log(2.0) + 0.5);
		unsigned long long Coef;
		if (Exp >= 0) {
			Coef = (fabs(aValue) / (double)(1ULL << Exp)) * ((double)(1ULL << (FractSize-1))) + 0.5;
		} else {
			Coef = (fabs(aValue) * (double)(1ULL << -Exp)) * ((double)(1ULL << (FractSize-1))) + 0.5;
		}
		if (Coef > (1ULL << 48)) {
			Coef /= 2.0;
			++Exp;
		}
		*this = CFloat_t(Exp,IsPositive,Coef);
		Normalize();
*****/
	}
#endif
	inline CFloat_t(long aExp, long long aFract) {
		Value = ((unsigned long long)((aExp+ExpOfs) & ((1 << ExpSize) -1)) << ExpBitPos);
		if (aFract < 0) {
			Value |= FractSignMask;
			aFract = -aFract;
		}
		Value |= ((unsigned long long)(aFract & ((1ULL << FractSize) -1)) << FractBitPos);
	}
	inline CFloat_t(long aExp, bool aFractPositive, unsigned long long aFract) {
		Value = ((unsigned long long)((aExp+ExpOfs) & ((1 << ExpSize) -1)) << ExpBitPos);
		Value |= (aFractPositive)?0:FractSignMask;
		Value |= ((unsigned long long)(aFract & ((1ULL << FractSize) -1)) << FractBitPos);
	}
	inline operator CInt_t() { return Value; }
	inline operator CAddr_t() { return (CAddr_t)Value; }

	static const unsigned long long FractSignMask = 0x8000000000000000ULL;
	static const long FractSignBitPos = 63;
	static const long FractSignSize = 1;

	static const unsigned long long ExpMask = 0x7fff000000000000ULL;
	static const long ExpBitPos = 48;
	static const long ExpSize = 15;
	// This is interesting: offset accourding to some sources (http://owen.sj.ca.us/~rk/howto/fltpt/index.html) is 0x3fff.
	//                      the manual says it's it's 040000
	//                      the test that I've found only come back with the right results if it's 040001.
	//                      The manual also states that to convert an integer to a float, you'll have to prepend 040060 and mormalize it.
	//                      This also works if Ofs is 040001.
	// So for now, I'm keeping it 040001, until I can proove it to be wrong.
	static const long ExpOfs = 040001; 
	//static const long ExpOfs = 040000; 
	static const long ExpMax = 057777;
	static const long ExpMin = 020000;

	static const unsigned long long FractMask = 0x0000ffffffffffffULL;
	static const long FractBitPos = 0;
	static const long FractSize = 48;

	inline bool IsExpValid() {
		long RawExp = (Value & ExpMask) >> ExpBitPos;
		if (RawExp > ExpMax) return false;
		if (RawExp < ExpMin) return false;
		return true;
	}

	inline long RawExp() const { return (Value & ExpMask) >> ExpBitPos; }

	CFloat_t UnderflowOrValue() const {
		if (RawExp() < ExpMin) return CFloat_t(CInt_t(0));
		return *this;
	}

	inline long Exp() const {
		long RawExp = (Value & ExpMask) >> ExpBitPos;
		return RawExp - ExpOfs;
	}
	inline long long Fract() const {
		long long RawFract = (Value & FractMask) >> FractBitPos;
		if ((Value & FractSignMask) != 0) {
			return -RawFract;
		} else {
			return RawFract;
		}
	}
	inline bool IsNormalized() const { return (Value & (1ull << (FractSize - 1))) != 0; }
	inline unsigned long long UFract() const {
		return (Value & FractMask) >> FractBitPos;
	}
	inline bool IsPositive() const { 
		return (Value & FractSignMask) == 0; 
	}
	inline void FlipSign() { Value ^= FractSignMask; }
	void Normalize();
	double ToDouble() const;
};

// Math operators and functions
CFloat_t operator +(const CFloat_t  &aA, const CFloat_t  &aB);
CFloat_t operator -(const CFloat_t  &aA, const CFloat_t  &aB);
CFloat_t operator -(const CFloat_t  &aA);
CFloat_t operator *(const CFloat_t  &aA, const CFloat_t  &aB);
CFloat_t ReciprocIterate(const CFloat_t &aLeft,const CFloat_t &aRight);
CFloat_t RoundedMult(const CFloat_t &aLeft, const CFloat_t &aRight);
CFloat_t HalfPrecisionMult(const CFloat_t &aLeft, const CFloat_t &aRight);
CFloat_t ReciprocApprox(const CFloat_t &aOperand);
CFloat_t OldReciprocApprox(const CFloat_t &aOperand);


extern const size_t cSideEffectIdent;

class CAddrTranslator_c {
public:
	typedef std::string internal_type;
	typedef CAddr_t     external_type;

	boost::optional<CAddr_t> get_value(const std::string &aStr) {
		if (aStr.empty()) return boost::optional<CAddr_t>(boost::none);
		unsigned long Base = 0;
		char *EndPtr;
		errno = 0;
		unsigned long RetVal = strtoul(aStr.c_str(),&EndPtr,Base);
		if (errno == ERANGE) return boost::optional<CAddr_t>(boost::none);
		if (EndPtr[0] != 0) return boost::optional<CAddr_t>(boost::none);
		if (RetVal > (unsigned long)(std::numeric_limits<CAddr_t>::max())) return boost::optional<CAddr_t>(boost::none);
		return boost::optional<CAddr_t>(RetVal);
	}
	boost::optional<std::string> put_value(const CAddr_t& aValue);
};

class CProgramAddrTranslator_c {
public:
	typedef std::string    internal_type;
	typedef CProgramAddr_t external_type;

	boost::optional<CProgramAddr_t> get_value(const std::string &aStr) {
		if (aStr.empty()) return boost::optional<CProgramAddr_t>(boost::none);
		unsigned long Base = 0;
		char *EndPtr;
		errno = 0;
		long long RetVal = strtoll(aStr.c_str(),&EndPtr,Base);
		if (errno == ERANGE) return boost::optional<CProgramAddr_t>(boost::none);
		if (EndPtr[0] != ':') return boost::optional<CProgramAddr_t>(boost::none);
		if (EndPtr[1] != 'p' && EndPtr[1] != 'P') return boost::optional<CProgramAddr_t>(boost::none);
		if (EndPtr[2] == 0) return boost::optional<CProgramAddr_t>(boost::none);
		int Parcel = EndPtr[2] - '0';
		if (Parcel < 0 || Parcel > 3) return boost::optional<CProgramAddr_t>(boost::none);
		RetVal = RetVal * 4 + Parcel;
		// We need double-conversion here to avoid sign-extending max to (long(-1))
		if (RetVal > (long long)(uint32_t(std::numeric_limits<CAddr_t>::max()))) return boost::optional<CProgramAddr_t>(boost::none);
		return boost::optional<CProgramAddr_t>(CProgramAddr_t(RetVal));
	}
	boost::optional<std::string> put_value(const CProgramAddr_t& aValue);
};

#endif // __CRAY_TYPES_H__
