#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <streambuf>
#include <atomic>
#include <limits>
#include <thread>
#include <boost/container/map.hpp> // To work around https://svn.boost.org/trac10/ticket/12534
#include <type_traits>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
//#include <boost/format.hpp>
#include "cray_types.h"
#include "exceptions.h"
#include "config.h"

#ifndef SINGLE_THREADED
typedef std::atomic<bool>     AtomicBool;
//typedef bool     AtomicBool;
typedef std::atomic<uint64_t> AtomicUInt64;
typedef std::atomic<uint32_t> AtomicUInt32;
typedef std::atomic<uint16_t> AtomicUInt16;
typedef std::atomic<int64_t>  AtomicInt64;
typedef std::atomic<int32_t>  AtomicInt32;
typedef std::atomic<int16_t>  AtomicInt16;
static const bool MultiThreadedSupport = true;
#else // SINGLE_THREADED
typedef bool     AtomicBool;
typedef uint64_t AtomicUInt64;
typedef uint32_t AtomicUInt32;
typedef uint16_t AtomicUInt16;
typedef int64_t  AtomicInt64;
typedef int32_t  AtomicInt32;
typedef int16_t  AtomicInt16;
static const bool MultiThreadedSupport = false;
#endif // SINGLE_THREADED

#ifndef NO_ASSERTS
#define CRAY_ASSERT(aCond) do { if (!(aCond)) { std::stringstream ErrorStr; ErrorStr << "CRAY_ASSERT at " << __FILE__ << ":" << DecPrinter(__LINE__,0) << "(" << #aCond << " failed)"; std::cout << ErrorStr.str() << std::endl; throw Assert_x(ErrorStr.str()); }} while (false);
#else
#define CRAY_ASSERT(aCond)
#endif

#ifndef NO_ASSERTS
#define CRAY_FIXME do { std::stringstream ErrorStr; ErrorStr << "CRAY_ASSERT at " << __FILE__ << ":" << DecPrinter(__LINE__,0) << "(" << #aCond << " failed)"; std::cout << ErrorStr.str() << std::endl; throw Assert_x(ErrorStr.str()); } while (false);
#else
#define CRAY_FIXME
#endif

#ifdef _MSC_VER
#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif
#define FORCEINLINEATTR
#else
#ifndef FORCEINLINE
#define FORCEINLINE inline
#endif
#define FORCEINLINEATTR __attribute__((always_inline))
#endif

#ifdef _MSC_VER
#ifndef DEBUGBREAK
#define DEBUGBREAK __debugbreak()
#endif
#else
#ifndef DEBUGBREAK
#define DEBUGBREAK
#endif
#endif

#if 0
// Endienness swap code that doesn't use intrinsics
inline uint16_t SwapBytes(uint16_t aInput) {
	return ((aInput & 0x00ff) << 8) | ((aInput & 0xff00) >> 8);
}

inline uint32_t SwapBytes(uint32_t aInput) {
	return ((aInput & 0x000000ff) << 24) | ((aInput & 0x0000ff00) << 8) | ((aInput & 0x00ff0000) >> 8) | ((aInput & 0xff000000) >> 24);
}

inline uint64_t SwapBytes(uint64_t aInput) {
	uint8_t *Data = (uint8_t *)&aInput;
	uint8_t Tmp;
	Tmp = Data[0]; Data[0] = Data[7]; Data[7] = Tmp;
	Tmp = Data[1]; Data[1] = Data[6]; Data[6] = Tmp;
	Tmp = Data[2]; Data[2] = Data[5]; Data[5] = Tmp;
	Tmp = Data[3]; Data[3] = Data[4]; Data[4] = Tmp;
	return aInput;
}

inline uint64_t SwapBytesInWords(uint64_t aInput) {
	uint16_t *Data = (uint16_t *)&aInput;
	Data[0] = SwapBytes(Data[0]);
	Data[1] = SwapBytes(Data[1]);
	Data[2] = SwapBytes(Data[2]);
	Data[3] = SwapBytes(Data[3]);
	return aInput;
}
#endif

#ifdef _MSC_VER
#include <intrin.h>
inline uint16_t SwapBytes(uint16_t aInput) {
	return _byteswap_ushort(aInput);
}

inline uint32_t SwapBytes(uint32_t aInput) {
	return _byteswap_ulong(aInput);
}

inline uint64_t SwapBytes(uint64_t aInput) {
	return _byteswap_uint64(aInput);
}

inline uint64_t SwapBytesInWords(uint64_t aInput) {
	uint16_t *Data = (uint16_t *)&aInput;
	Data[0] = SwapBytes(Data[0]);
	Data[1] = SwapBytes(Data[1]);
	Data[2] = SwapBytes(Data[2]);
	Data[3] = SwapBytes(Data[3]);
	return aInput;
}
#else // GCC
inline uint16_t SwapBytes(uint16_t aInput) {
	return __builtin_bswap16(aInput);
}

inline uint32_t SwapBytes(uint32_t aInput) {
	return __builtin_bswap32(aInput);
}

inline uint64_t SwapBytes(uint64_t aInput) {
	return __builtin_bswap64(aInput);
}

inline uint64_t SwapBytesInWords(uint64_t aInput) {
	uint16_t *Data = (uint16_t *)&aInput;
	Data[0] = SwapBytes(Data[0]);
	Data[1] = SwapBytes(Data[1]);
	Data[2] = SwapBytes(Data[2]);
	Data[3] = SwapBytes(Data[3]);
	return aInput;
}
#endif

template <typename T> T SwapBits(T aInput) {
	T Output = 0;
	T InMask = 1;
	T OutMask = T(1) << (sizeof(OutMask)*8 - 1);
	while (OutMask != 0) {
		if ((aInput & InMask) != 0) Output |= OutMask;
		InMask <<= 1;
		OutMask >>= 1;
	}
	return Output;
}

inline uint64_t GetBits(uint64_t aData, size_t aStartIdx, size_t aEndIdx) {
	size_t StartIdx = std::min(aStartIdx, aEndIdx);
	size_t EndIdx = std::max(aStartIdx, aEndIdx);
	if (StartIdx == 0 && EndIdx == std::numeric_limits<uint64_t>::digits - 1) {
		return aData;
	}
	return (aData >> StartIdx) & ((1ULL << (EndIdx - StartIdx + 1)) - 1);
}

inline uint32_t GetBits(uint32_t aData, size_t aStartIdx, size_t aEndIdx) {
	size_t StartIdx = std::min(aStartIdx, aEndIdx);
	size_t EndIdx = std::max(aStartIdx, aEndIdx);
	if (StartIdx == 0 && EndIdx == std::numeric_limits<uint32_t>::digits - 1) {
		return aData;
	}
	return (aData >> StartIdx) & ((1UL << (EndIdx - StartIdx + 1)) - 1);
}

inline uint64_t GetBitsReverse(uint64_t aData, size_t aStartIdx, size_t aEndIdx) {
	return GetBits(aData, 63 - aStartIdx, 63 - aEndIdx);
}

inline uint32_t GetBitsReverse(uint32_t aData, size_t aStartIdx, size_t aEndIdx) {
	return GetBits(aData, 31 - aStartIdx, 31 - aEndIdx);
}

template <typename T> T GetBit(T aData,size_t aIdx) { return GetBits(aData,aIdx,aIdx); }

inline uint64_t SetBits(uint64_t aData,size_t aStartIdx,size_t aEndIdx,uint64_t aValue) {
	size_t StartIdx = std::min(aStartIdx,aEndIdx);
	size_t EndIdx = std::max(aStartIdx,aEndIdx);
	if (StartIdx == 0 && EndIdx == std::numeric_limits<uint64_t>::digits - 1) {
		return aValue;
	}
	uint64_t Mask = ((1ULL << (EndIdx-StartIdx+1)) - 1) << StartIdx;
	return (aData & ~Mask) | ((aValue << StartIdx) & Mask);
}

inline uint64_t SetBitsReverse(uint64_t aData, size_t aStartIdx, size_t aEndIdx, uint64_t aValue) {
	return SetBits(aData, 63 - aStartIdx, 63 - aEndIdx, aValue);
}

//template <typename V, typename M, typename N> inline V SetBits(V aValue, M aMask, N aNewValue) { return (aValue & ~aMask) | aNewValue; }
#define SET_BITS(aValue, aMask, aNewValue) (((aValue) & ~(aMask)) | (aNewValue))

#define SideEffectIndent Indent(cSideEffectIdent)

inline std::string Replace(std::string aSrc, std::string const& aTarget, std::string const& aRepl)
{
	if (aTarget.length() == 0) {
		return aSrc;
	}

	if (aSrc.length() == 0) {
		return aSrc;  // nothing to match against
	}

	size_t idx = 0;

	for (;;) {
		idx = aSrc.find(aTarget, idx);
		if (idx == std::string::npos)  break;

		aSrc.replace(idx, aTarget.length(), aRepl);
		idx += aRepl.length();
	}

	return aSrc;
}

inline char PrintableChar(char aC) {
	if (aC < ' ') return '.';
	//	if (aC > 127) return '.';
	return aC;
}

/////// A simple NULL stream
class NullStrm_c: public std::ostream {
public:
	NullStrm_c(): std::ostream(/*mNoinit*/nullptr) {
		init(nullptr);
	};
protected:
};

/////////////////////////////////////////////////////////////////////
// Type-dependent integral range functions
template <typename tType> constexpr tType MinValue(const tType &) { return std::numeric_limits<tType>::min(); }
template <typename tType> constexpr tType MaxValue(const tType &) { return std::numeric_limits<tType>::max(); }

/////////////////////////////////////////////////////////////////////
// Formatter classes

class FieldFormatter_i {
public:
	virtual void Print(std::ostream &aStream) const = 0;
	virtual ~FieldFormatter_i() {}
};
inline std::ostream & operator << (std::ostream &aStream, const FieldFormatter_i &aField) {
	if (!aStream.good()) return aStream;
	aField.Print(aStream); 
	return aStream; 
}

class DoublePrinter: public FieldFormatter_i {
public:
	explicit DoublePrinter(double aNumber, int aSize = 16, int aPrecision = 16, bool aFixed = true): Number(aNumber), Size(aSize), Precision(aPrecision), Fixed(aFixed) {}
	explicit DoublePrinter(CFloat_t aNumber, int aSize = 16, int aPrecision = 16, bool aFixed = true): Number(aNumber.ToDouble()), Size(aSize), Precision(aPrecision), Fixed(aFixed) {}
	virtual void Print(std::ostream &aStream) const override {
		if (!aStream.good()) return;
		std::stringstream Str;
		if (Fixed) Str << std::fixed;
		//if (Size > 0) Str << setw(Size);
		if (Precision > 0) Str << std::setprecision(Precision);
		Str << Number;
		aStream << Str.str();
	}
protected:
	double Number;
	int Size;
	int Precision;
	bool Fixed;
};

class HexPrinter: public FieldFormatter_i {
public:
	template <typename Type_T> explicit HexPrinter(Type_T aNumber, int aSize = (std::numeric_limits<Type_T>::digits + 3) / 4) : Number(aNumber), IsSigned(std::is_signed<Type_T>::value), Size(aSize) {}
	virtual void Print(std::ostream &aStream) const {
		if (!aStream.good()) return;
		std::stringstream Str;
		if (Size > 0) {
			if (IsSigned) {
				Str << "0x" << std::noshowbase << std::uppercase << std::hex << std::setw(Size) << std::setfill('0') << (int64_t)Number;
			} else {
				Str << "0x" << std::noshowbase << std::uppercase << std::hex << std::setw(Size) << std::setfill('0') << Number;
			}
		} else {
			if (IsSigned) {
				Str << "0x" << std::noshowbase << std::uppercase << std::hex << (int64_t)Number;
			} else {
				Str << "0x" << std::noshowbase << std::uppercase << std::hex << Number;
			}
		}
		aStream << Str.str();
	}
	uint64_t Number;
	bool IsSigned;
	int Size;
};

class BoolPrinter : public FieldFormatter_i {
public:
	explicit BoolPrinter(bool aValue) : Value(aValue) {}
	virtual void Print(std::ostream &aStream) const {
		if (!aStream.good()) return;
		aStream << (Value ? "true" : "false");
	}
	bool Value;
};

class AsciiDumpPrinter : public FieldFormatter_i {
public:
	template <typename Type_T> explicit AsciiDumpPrinter(Type_T aNumber) : Number(aNumber), Size(sizeof(Type_T)) {}
	virtual void Print(std::ostream &aStream) const {
		if (!aStream.good()) return;
		char *NumAsStr = (char*)(&Number);
		for (int i = 0; i < Size; ++i) {
			aStream << PrintableChar(NumAsStr[Size-1-i]);
		}
	}
	uint64_t Number;
	int Size;
};

class OctPrinter: public FieldFormatter_i {
public:
	template <typename Type_T> explicit OctPrinter(Type_T aNumber, int aSize = (std::numeric_limits<Type_T>::digits + 2) / 3) : Number(aNumber), IsSigned(std::is_signed<Type_T>::value), Size(aSize) {}
	virtual void Print(std::ostream &aStream) const {
		if (!aStream.good()) return;
		std::stringstream Str;
		if (Size > 0) {
			if (IsSigned) {
				Str << "0" << std::noshowbase << std::uppercase << std::oct << std::setw(Size) << std::setfill('0') << (int64_t)Number;
			} else {
				Str << "0" << std::noshowbase << std::uppercase << std::oct << std::setw(Size) << std::setfill('0') << Number;
			}
		} else {
			if (IsSigned) {
				Str << "0" << std::noshowbase << std::uppercase << std::oct << (int64_t)Number;
			} else {
				Str << "0" << std::noshowbase << std::uppercase << std::oct << Number;
			}
		}
		aStream << Str.str();
	}
	uint64_t Number;
	bool IsSigned;
	int Size;
};

class DecPrinter: public FieldFormatter_i {
public:
	template <typename Type_T> explicit DecPrinter(Type_T aNumber, int aSize = 0, char aFiller = ' ') : Number(aNumber), IsSigned(std::is_signed<Type_T>::value), Size(aSize), Filler(aFiller) {}
	explicit DecPrinter(CAddr_t  aNumber, int aSize = 0, char aFiller = ' '): Number(aNumber), IsSigned(false), Size(aSize), Filler(aFiller) {}
	virtual void Print(std::ostream &aStream) const {
		if (!aStream.good()) return;
		std::stringstream Str;
		if (Size > 0) {
			if (IsSigned) {
				Str << std::noshowbase << std::uppercase << std::dec << std::setw(Size) << std::setfill(Filler) << (int64_t)Number;
			} else {
				Str << std::noshowbase << std::uppercase << std::dec << std::setw(Size) << std::setfill(Filler) << Number;
			}
		} else {
			if (IsSigned) {
				Str << std::noshowbase << std::uppercase << std::dec << (int64_t)Number;
			} else {
				Str << std::noshowbase << std::uppercase << std::dec << Number;
			}
		}
		aStream << Str.str();
	}
	char Filler;
	uint64_t Number;
	bool IsSigned;
	int Size;
};

class InstAddr: public FieldFormatter_i {
public:
	template <typename Type_T> explicit InstAddr(const Type_T aAddr) : mAddr(aAddr) {}
	virtual void Print(std::ostream &aStream) const { if (!aStream.good()) return; aStream << HexPrinter(mAddr / 4) << ":p" << int(mAddr % 4); }
	CAddr_t mAddr;
};

class Addr: public FieldFormatter_i {
public:
	template <typename Type_T> explicit Addr(const Type_T aAddr) : mAddr(aAddr) {}
	virtual void Print(std::ostream &aStream) const { if (!aStream.good()) return; aStream << HexPrinter(mAddr); }
	CAddr_t mAddr;
};

class Indent: public FieldFormatter_i {
public:
	explicit Indent(size_t aIndent): mIndent(aIndent) {}
	virtual void Print(std::ostream &aStream) const { if (!aStream.good()) return; for(uint32_t i=0;i<mIndent;++i) aStream << '\t'; }
protected:
	size_t mIndent;
};

/////////////////////////////////////////////////////////////////////
// A very very simple command-line parser class

class CommandLine_c {
public:
	class NoMoreArgs_x: public std::exception {
	public:
		explicit NoMoreArgs_x(const char *aMessage): mMessage(aMessage) {}
		virtual const char *what() const throw() override  {
			return mMessage.c_str();
		}
	private:
		std::string mMessage;
	};

	CommandLine_c(size_t aArgc, const char **aArgv): mArgc(aArgc), mArgv(aArgv), mCurArg(1) {}
	const std::string GetNextParam() {
		if (mCurArg >= mArgc) throw NoMoreArgs_x("Too few arguments");
		return mArgv[mCurArg++];
	}
	void UnGetParam() {
		if (mCurArg == 1) throw NoMoreArgs_x("Can't un-get first parameter");
		--mCurArg;
	}
	void Rewind() { mCurArg = 1; }
	bool HasMoreParams() { return mCurArg < mArgc; }
	const std::string GetProgramName() const { return mArgv[0]; }
protected:
	size_t mArgc;
	const char **mArgv;
	size_t mCurArg;
};

/////////////////////////////////////////////////////////////////////
// An adaptor class that can take a class and some methods on it and turn that into an STL-compatible iterator

template <class tContainer, class tElement, size_t (tContainer::*Count)() const, tElement &(tContainer::*Access)(size_t)> class IteratorWrapper_t {
public:
	explicit IteratorWrapper_t(tContainer& aParent): mParent(&aParent) {}

	IteratorWrapper_t(const IteratorWrapper_t&) = default;
	IteratorWrapper_t& operator=(const IteratorWrapper_t&) = default;

	using value_type = tElement&;

	class iterator {
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = typename IteratorWrapper_t::value_type;
		using difference_type = ptrdiff_t;
		using pointer = value_type;
		using const_pointer = pointer;
		using reference = value_type;
		using const_reference = reference;

		iterator(const iterator&) = default;
		iterator& operator=(const iterator&) = default;

		iterator(IteratorWrapper_t& aParent, size_t aIdx): mIdx(aIdx), mParent(aParent) {}

		reference operator*() const { return (mParent.mParent->*Access)(mIdx); }
		pointer operator->() const { return operator*(); }
		iterator &operator++() { mIdx += 1; return *this; }
		iterator operator++(int) { iterator RetVal = *this; ++*this; return RetVal; }
		iterator &operator--() { mIdx -= 1; return *this; }
		iterator operator--(int) {iterator RetVal = *this; --*this; return RetVal; }

		friend bool operator==(const iterator &aA, const iterator &aB) { return &aA.mParent == &aB.mParent && aA.mIdx == aB.mIdx; }
		friend bool operator!=(const iterator &aA, const iterator &aB) { return !(aA == aB); }
	protected:
		size_t mIdx;
		IteratorWrapper_t& mParent;
	};

	using const_iterator = const iterator;

	iterator begin() const { return iterator(const_cast<IteratorWrapper_t&>(*this), 0); }
	iterator end() const { return iterator(const_cast<IteratorWrapper_t&>(*this), (mParent->*Count)()); }
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }
protected:
	tContainer *mParent;
};

template <class tContainer, class tElement, size_t(tContainer::*Count)() const, const tElement &(tContainer::*CAccess)(size_t) const> class ConstIteratorWrapper_t {
public:
	explicit ConstIteratorWrapper_t(const tContainer& aParent) : mParent(&aParent) {}

	ConstIteratorWrapper_t(const ConstIteratorWrapper_t&) = default;
	ConstIteratorWrapper_t& operator=(const ConstIteratorWrapper_t&) = default;

//	using value_type = tElement&;
	typedef tElement& value_type;

	class const_iterator {
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = typename ConstIteratorWrapper_t::value_type;
		using difference_type = ptrdiff_t;
		using pointer = value_type;
		using const_pointer = pointer;
		using reference = value_type;
		using const_reference = reference;

		const_iterator(const const_iterator&) = default;
		const_iterator& operator=(const const_iterator&) = default;

		const_iterator(ConstIteratorWrapper_t& aParent, size_t aIdx) : mIdx(aIdx), mParent(aParent) {}

		const_reference operator*() const { return (mParent.mParent->*CAccess)(mIdx); }
		const_pointer operator->() const { return operator*(); }
		const_iterator &operator++() { mIdx += 1; return *this; }
		const_iterator operator++(int) { const_iterator RetVal = *this; ++*this; return RetVal; }
		const_iterator &operator--() { mIdx -= 1; return *this; }
		const_iterator operator--(int) { const_iterator RetVal = *this; --*this; return RetVal; }

		friend bool operator==(const const_iterator &aA, const const_iterator &aB) { return &aA.mParent == &aB.mParent && aA.mIdx == aB.mIdx; }
		friend bool operator!=(const const_iterator &aA, const const_iterator &aB) { return !(aA == aB); }
	protected:
		size_t mIdx;
		ConstIteratorWrapper_t& mParent;
	};

	const_iterator cbegin() const { return const_iterator(const_cast<ConstIteratorWrapper_t&>(*this), 0); }
	const_iterator cend() const { return const_iterator(const_cast<ConstIteratorWrapper_t&>(*this), (mParent->*Count)()); }
protected:
	const tContainer *mParent;
};

enum class MachineTypes_e : int {
	XMP1xx,
	XMP2xx,
	XMP4xx,
	YMP,
	C90,
	T90,
	YEL,
	J90,
	SV1
};

inline bool IsXmp(MachineTypes_e aMachineType) {
	return (aMachineType == MachineTypes_e::XMP1xx) || (aMachineType == MachineTypes_e::XMP2xx) || (aMachineType == MachineTypes_e::XMP4xx);
}

enum class IopRevision_e {
	IOSD,
	IOSE
};

struct BitRange_s {
	BitRange_s(uint8_t aStart, uint8_t aEnd) : Start(aStart), End(aEnd) {}
	uint8_t Start, End;
};

void SetThreadAffinity(std::thread &aThread, size_t aCpuId);
void SetThreadAffinity(size_t aCpuId);

std::string Dump(std::vector<uint8_t> &aVector);

class Thread_c : public std::thread
{
public:
	Thread_c() : mTerminate(false), mPaused(true) {}
	virtual ~Thread_c() {
		Terminate();
		if (joinable()) join();
	}
	explicit Thread_c(bool aStart) : mTerminate(false) {
		if (aStart) Start();
	}
	void Terminate() {
		mTerminate = true;
	}
	void Resume() {
		mPaused = false;
	}
	void Pause() {
		mPaused = true;
	}
	bool IsPaused() const {
		return mPaused;
	}
	void Start() {
		std::thread::operator=(std::thread(&Thread_c::BaseThread, this));
	}
protected:
	bool IsTerminating() {
		return mTerminate;
	}

	virtual void Thread() = 0;
private:
	void BaseThread() {
		Thread();
	}
	std::atomic_bool mTerminate;
	std::atomic_bool mPaused;
};

#endif // __UTILS_H__
