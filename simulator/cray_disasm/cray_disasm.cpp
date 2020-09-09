#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include "utils.h"
#include "cray_softcpu.h"
#include <fstream>
#include <type_traits>

// TODO:
// - The data segments contain stack-frame info that's described in usr\include\cray\tnb.h. These entries are there for all file formats (even for the kernel).
//   there could be a routine that searches the binary for likely tnb records and creates 'symbol entries' for all of them. Then during disassembly these entries
//   can be referenced and printed out providing callee names and potentially even argument numbers. It can also list the size of const, temp, etc. sections
//   plus the name when encountering the entry point of such a function

int PrintUsage(const char *aExecName, const char *ErrorStr = nullptr) {
	if (ErrorStr != nullptr) std::cout << "Error: " << ErrorStr << std::endl;
	std::cout << "Usage: " << aExecName << " <image file> [options]" << std::endl;
	std::cout << std::endl;
	std::cout << "\t" << "Options:" << std::endl;
	std::cout << "\t" << "-i <image offset>: specifies image offset in 64-bit words" << std::endl;
	std::cout << "\t" << "-o <load offset>: specifies load offset in 64-bit words" << std::endl;
	std::cout << "\t" << "-s <start>:  specifies the start address (in Cray address format) to start disassembly from" << std::endl;
	std::cout << "\t" << "-e <end>:    specifies the end address (in Cray address format) to end disassembly at" << std::endl;
	std::cout << "\t" << "-ib <base>:  specifies the instruction base address (in word address format) for the image" << std::endl;
	std::cout << "\t" << "-db <base>:  specifies the data base address (in word address format) for the image" << std::endl;
	std::cout << "\t" << "-raw:        assumes binary is raw (doesn't try to parse executable header)" << std::endl;
	std::cout << "\t" << "-unicos:     assumes binary is in unicos format" << std::endl;
	std::cout << "\t" << "-obj:        assumes binary is in unicos object format" << std::endl;
	std::cout << "\t" << "-hex:        print instruction codes in hex" << std::endl;
	std::cout << "\t" << "-patch:      creates a patch file" << std::endl;
	std::cout << "\t" << "-xmp:        assume X-MP image" << std::endl;
	std::cout << "\t" << "-ymp:        assume Y-MP image" << std::endl;
	std::cout << "\t" << "-out <file>: sets output file name" << std::endl;
	std::cout << "\t" << "Note: start and end addresses ARE base-relative" << std::endl;
	return 1;
}

// Definitions from relo.h
enum TableType_e {
	LHT_TYPE = 001,    // Library (build) header table
	BLD_TYPE = 002,    // Build directory table
	MTT_TYPE = 010,    // Module termination table
	SMT_TYPE = 011,    // Module Symbol table
	XRL_TYPE = 014,    // Extended relocation table
	REL_TYPE,          // Relocation table
	TXT_TYPE,          // Text table
	PDT_TYPE,          // Program Descriptor table
	CMB_TYPE = 021,    // Common Block Symbol table
	PAS_TYPE = 026,    // Pass-through table for Fortran 90
	GNT_TYPE = 027,    // Global symbol table
	DMT_TYPE = 040,    // Distributed module termination table
	DFI_TYPE = 042,    // Distributed field initialization table
	DEX_TYPE,          // Distributed Expression table
	DPD_TYPE = 047     // Distributed Program Descriptor table
};

typedef uint64_t FIELD;


class RangeContainer_c {
public:
	RangeContainer_c() : mSize(0) {}

	std::vector<uint64_t> ReadFields(std::istream &aStrm) const {
		size_t StructSize = (mSize + 63) / 64;
		std::vector<uint64_t> RetVal(StructSize);
		aStrm.read((char*)(&RetVal[0]), StructSize * sizeof(uint64_t));
		if (aStrm.bad()) throw Generic_x() << "Can't read from file";
//		for (size_t Idx = 0; Idx < StructSize; ++Idx) {
//			RetVal[Idx] = SwapBytes(RetVal[Idx]);
//		}
		return RetVal;
	}
	void Read(std::istream &aStrm);
	size_t SizeInBits() const { return mSize; }
	size_t SizeInWords() const { return (mSize + 63) / 64; }
protected:
	size_t mSize;
	std::vector<uint64_t> mRawBuffer;
	std::vector<class FieldBase_c *> mFields;
	std::vector<class ArrFieldBase_c *> mArrFields;

	friend class FieldBase_c;
	friend class ArrFieldBase_c;
};

class FieldBase_c {
public:
	FieldBase_c(RangeContainer_c &aParent, size_t aSize) : mStart(aParent.mSize), mEnd(aParent.mSize + aSize - 1), mParent(aParent) { aParent.mSize = mEnd + 1; aParent.mFields.push_back(this); }
protected:
	template <
		typename tType, 
		typename std::enable_if< ! std::is_same<char *, tType>::value>::type * = nullptr
	> tType Get(std::vector<uint64_t> &aRawBuffer) const {
		if (mEnd - mStart == 64) {
			CRAY_ASSERT((mStart % 64) == 0);
			return tType(SwapBytes(aRawBuffer[mStart / 64]));
		}
		size_t WordOfs = mStart / 64;
		size_t Start = mStart % 64;
		size_t End = mEnd % 64;
		uint64_t Val = GetBitsReverse(SwapBytes(aRawBuffer[WordOfs]), BitRange_s(uint8_t(Start), uint8_t(End)));
		return tType(Val);
	}
	template <
		typename tType, 
		typename std::enable_if< std::is_same<char *, tType>::value>::type * = nullptr
	> char * Get(std::vector<uint64_t> &aRawBuffer) const {
		CRAY_ASSERT((mStart % 8) == 0);
		CRAY_ASSERT((mEnd % 8) == 0);
		size_t Ofs = mStart / 8;
		return ((char *)(&aRawBuffer[0])) + Ofs;
	}
	template <typename tType> tType Get() const {
		return Get<tType>(mParent.mRawBuffer);
	}
	virtual void Fill() = 0;

	size_t mStart;
	size_t mEnd;
	RangeContainer_c &mParent;

	friend RangeContainer_c;
	friend class ArrFieldBase_c;
};

template <typename tType> class Field_T : public FieldBase_c {
public:
	Field_T(RangeContainer_c &aParent, size_t aSize) : FieldBase_c(aParent, aSize) {}
	tType Get() const { return mValue; }
protected:
	virtual void Fill() override {
		mValue = FieldBase_c::Get<tType>();
	}
	tType mValue;
};

class ArrFieldBase_c {
protected:
	ArrFieldBase_c(RangeContainer_c &aParent, FieldBase_c &aLen) : mLen(aLen), mParent(aParent) {
		CRAY_ASSERT((aParent.mSize % 64) == 0);
		aParent.mArrFields.push_back(this);
	}
	virtual void Read(std::istream &aStrm) = 0;
	size_t Len() const {
		return mLen.Get<size_t>();
	}
	void AdjustParentSize(size_t aLen) const {
		mParent.mSize += aLen;
	}

	RangeContainer_c &mParent;
	FieldBase_c &mLen;

	friend RangeContainer_c;
};

class StrField_c: public ArrFieldBase_c {
public:
	StrField_c(RangeContainer_c &aParent, FieldBase_c &aLen) : ArrFieldBase_c(aParent, aLen) {
	}
	const std::string &Get() const { return mValue; }
protected:
	std::string mValue;

	virtual void Read(std::istream &aStrm) override {
		size_t StrLen = Len();

		if (StrLen == 0) {
			mValue = std::string();
			return;
		}
		size_t WordLen = WordSize();
		AdjustParentSize(WordLen * 64);
		std::vector<char> StrChars(WordLen * 8 + 1);
		std::fill(StrChars.begin(), StrChars.end(), 0);
		aStrm.read((char*)(&StrChars[0]), WordLen * 8);
		mValue = std::string(&StrChars[0]);
	}
		
	size_t WordSize() const {
		return (Len() + 7) / 8;
	}
};

template <typename tType> class ArrField_T : public ArrFieldBase_c {
public:
	ArrField_T(RangeContainer_c &aParent, FieldBase_c &aLen) : ArrFieldBase_c(aParent, aLen) {
	}
	const std::vector<tType> &Get() const { return mValue; }
protected:
	std::vector<tType> mValue;

	virtual void Read(std::istream &aStrm) override {
		size_t ArrLen = Len();

		if (ArrLen == 0) {
			mValue.clear();
			return;
		}
		AdjustParentSize(ArrLen * 64);
		mValue.resize(ArrLen);
		aStrm.read((char*)(&mValue[0]), ArrLen * sizeof(tType));
	}
};

class ExtraField_c : public ArrField_T<uint64_t> {
public:
	ExtraField_c(class ProgDescTable_c &aParent);
protected:
	virtual void Read(std::istream &aStrm) override;
};

void RangeContainer_c::Read(std::istream &aStrm) {
	mRawBuffer = ReadFields(aStrm);
	for (auto &Field : mFields) {
		Field->Fill();
	}
	for (auto &ArrField : mArrFields) {
		ArrField->Read(aStrm);
	}
}









class ProgDescTable_c : public RangeContainer_c {
public:
	ProgDescTable_c() :
		pdthdsz(*this, 16),
		pdtblsz(*this, 16),
		pdtensz(*this, 16),
		pdtexsz(*this, 16),
		pdtmdy(*this, 64),
		pdthms(*this, 64),
		pdtcmpid(*this, 64),
		pdtcmpvr(*this, 64),
		pdtosvr(*this, 64),
		pdtudt(*this, 64),
		filler01(*this, 1),
		pdtfe(*this, 1),
		pdtbd(*this, 1),
		pdtmpa(*this, 1),
		pdtdc(*this, 1),
		pdtusr(*this, 8),
		filler02(*this, 1),
		pdtmf(*this, 2),
		pdtfnl(*this, 8),
		pdtmnl(*this, 8),
		pdtss(*this, 32),
		pdtuqnm(*this, 64),
		mExtra(*this),
		mFileName(*this, pdtfnl),
		mModuleName(*this, pdtmnl)
	{}
	Field_T<uint64_t> pdthdsz;   // Word size of header area
	Field_T<uint64_t> pdtblsz;   // Word size of block area
	Field_T<uint64_t> pdtensz;   // Word size of entry area
	Field_T<uint64_t> pdtexsz;   // Word size of external area
	Field_T<uint64_t> pdtmdy;    // MM/DD/YY - this compilation
	Field_T<uint64_t> pdthms;    // HH:MM:SS - this compilation
	Field_T<uint64_t> pdtcmpid;  // Generating product name
	Field_T<uint64_t> pdtcmpvr;  // Generating product version
	Field_T<uint64_t> pdtosvr;   // Host OS version
	Field_T<uint64_t> pdtudt;    // UNICOS time stamp (date)
	Field_T<uint64_t> filler01;  // (Unused, reserved by CRI)
	Field_T<uint64_t> pdtfe;     // Fatal error flag (1==true)
	Field_T<uint64_t> pdtbd;     // Block data module (1==true)
	Field_T<uint64_t> pdtmpa;    // Module passed address flag
	Field_T<uint64_t> pdtdc;     // Dual case names flag(1==true)
	Field_T<uint64_t> pdtusr;    // (Unused, reserved for user)
	Field_T<uint64_t> filler02;  // (Unused, reserved by CRI)
	Field_T<uint64_t> pdtmf;     // Module flag for Fortran 90:
	                             //    0 = independent
	                             //    1 = first of a module
	                             //    2 = in a module set
	                             //    3 = last of a module
	Field_T<uint64_t> pdtfnl;    // Char count in file name
	Field_T<uint64_t> pdtmnl;    // Char count in module name
	Field_T<uint64_t> pdtss;     // Stack size requirement
	Field_T<uint64_t> pdtuqnm;   // Unique ID for module name
	ExtraField_c mExtra;
	StrField_c mFileName;
	StrField_c mModuleName;
};


ExtraField_c::ExtraField_c(class ProgDescTable_c &aParent) : ArrField_T<uint64_t>(aParent, aParent.pdthdsz) {}
inline void ExtraField_c::Read(std::istream &aStrm) {
	ProgDescTable_c &Parent = static_cast<ProgDescTable_c&>(mParent);
	size_t ArrLen = size_t(Parent.pdthdsz.Get()) - size_t(10 + ((Parent.pdtmnl.Get() + 7) / 8) + ((Parent.pdtfnl.Get() + 7) / 8));

	if (ArrLen == 0) {
		mValue.clear();
		return;
	}
	AdjustParentSize(ArrLen * 64);
	mValue.resize(ArrLen);
	aStrm.read((char*)(&mValue[0]), ArrLen * sizeof(uint64_t));
}

// Constants for the pdtbkl (block location) field
enum BlockLocation_e {
	BKL_CM = 0,  // Common memory, also CRAY-2 FGP instr memory
	BKL_LM = 1,  // CRAY-2 local memory, also CRAY-2 FGP local memory
	BKL_LMA = 2, // CRAY-2 local memory absolute (unused by UNICOS)
	BKL_EM = 3,  // CRAY X-MP extended memory
	BKL_AX = 4   // CRAY X-MP auxiliary memory
};

// Constants for the pdtbkc (block contents) field
enum BlockContent_e {
	BKC_UNK = 0, // Unknown
	BKC_IX = 1,  // Instructions only
	BKC_DT = 2,  // Data only
	BKC_BS = 3,  // Data only with no text (bss)
	BKC_CN = 4,  // Constants only
	BKC_ZD = 5   // Data only with no text (zero fill assumed)
};

// Constants for the pdtbkt (block type) field
enum BlockType_e {
	BKT_CM = 0,  // Regular common block
	BKT_TCM = 2, // Task common block
	BKT_DBF = 4  // Dynamic block
};

// Constants for the pdtbal (block align) flag
enum BlockAlign_e {
	BAL_NA = 0, // No alignment
	BAL_AL = 1  // Align to instruction buffer boundary
};

class BlockDesc_c : public RangeContainer_c {
public:
	BlockDesc_c() :
		pdtbkcb(*this, 1),
		pdtbkl(*this, 3),
		pdtbkc(*this, 3),
		pdtbkt(*this, 3),
		pdtbal(*this, 1),
		pdtbef(*this, 1),
		filler01(*this, 4),
		pdtbusr(*this, 8),
		pdtbknl(*this, 8),
		pdtbkln(*this, 32),
		mName(*this, pdtbknl)
	{}
	Field_T<uint64_t> pdtbkcb;       // Common block flag (1==true)
	Field_T<BlockLocation_e> pdtbkl; // Block location
	Field_T<BlockContent_e> pdtbkc;  // Block contents
	Field_T<BlockType_e> pdtbkt;     // Block type
	Field_T<BlockAlign_e> pdtbal;    // Block align flag
	Field_T<uint64_t> pdtbef;        // Block entry flag
	Field_T<uint64_t> filler01;      // (Unused, reserved by CRI)
	Field_T<uint64_t> pdtbusr;       // (Unused, reserved for user)
	Field_T<uint64_t> pdtbknl;       // Char count in block name
	Field_T<uint64_t> pdtbkln;       // Block length (words)
	StrField_c mName;
	uint64_t FileOffset;
} cBlockDesc;




class EntryDesc_c: public RangeContainer_c {
public:
	EntryDesc_c() :
		EntryPointInBits(*this, 64),
		PrimaryEntryFlag(*this, 1),
		filler01(*this, 1),
		NameLength(*this, 8),
		SuggestedRelocationMode(*this, 3),
		filler02(*this, 3),
		filler03(*this, 16),
		filler04(*this, 8),
		pdteusr(*this, 8),
		BlockIndex(*this, 16),
		mName(*this, NameLength)
	{}
	Field_T<int64_t> EntryPointInBits;            // Entry point (signed) value - in bits
	Field_T<uint64_t> PrimaryEntryFlag;           // Primary entry flag (1==true)
	Field_T<uint64_t> filler01;                   // (Unused, reserved by CRI)
	Field_T<uint64_t> NameLength;                 // Char count in entry name
	Field_T<uint64_t> SuggestedRelocationMode;    // Suggested relocation mode
	Field_T<uint64_t> filler02;                   // (Unused, reserved by CRI)
	Field_T<uint64_t> filler03;                   // (Unused, reserved by CRI)
	Field_T<uint64_t> filler04;                   // (Unused, reserved by CRI)
	Field_T<uint64_t> pdteusr;                    // (Unused, reserved for user)
	Field_T<uint64_t> BlockIndex;                 // Block index
	StrField_c mName;
} cEntryDesc;

// Constants for pdtxct (call tree information) field
enum CallTreeInfo_e {
	XCT_EXT = 0,    // Regular external
	XCT_THDO = 1,   // External is task head
	XCT_THDS = 2    // External is both task head and subprogram
};

class ExternDesc_c: public RangeContainer_c {
public:
	ExternDesc_c() :
		pdtxmn(*this, 1),                         // Module specification
		filler00(*this, 1),                       // (Unused, reserved by CRI)
		NameLength(*this, 8),                     // Char count in external name
		pdtxsf(*this, 1),                         // Soft external (1==true)
		pdtxct(*this, 2),                         // Call tree information
		ExternalPassedAsArgument(*this, 1),       // External passed as argument
		filler01(*this, 18),                      // (Unused, reserved by CRI)
		filler02(*this, 24),                      // (Unused, reserved by CRI)
		pdtxusr(*this, 8),                        // (Unused, reserved for user)
		mName(*this, NameLength)
	{}

	Field_T<uint64_t> pdtxmn;                            // Module specification
	Field_T<uint64_t> filler00;                          // (Unused, reserved by CRI)
	Field_T<uint64_t> NameLength;                        // Char count in external name
	Field_T<uint64_t> pdtxsf;                            // Soft external (1==true)
	Field_T<CallTreeInfo_e> pdtxct;                      // Call tree information
	Field_T<uint64_t> ExternalPassedAsArgument;          // External passed as argument
	Field_T<uint64_t> filler01;                          // (Unused, reserved by CRI)
	Field_T<uint64_t> filler02;                          // (Unused, reserved by CRI)
	Field_T<uint64_t> pdtxusr;                           // (Unused, reserved for user)
	StrField_c mName;
	uint64_t FileOffset;
} cExternRanges;



class HdrDesc_c : public RangeContainer_c {
public:
	HdrDesc_c() :
		hdr_type(*this, 7),
		hdr_undef(*this, 9),
		hdr_bi(*this, 16),
		hdr_len(*this, 32)
	{}
	Field_T<TableType_e> hdr_type;  // Table type
	Field_T<uint64_t> hdr_undef;    // (Unused, reserved by CRI)
	Field_T<uint64_t> hdr_bi;       // Block index (optional)
	Field_T<uint64_t> hdr_len;      // Table length
};


// A text table (TXT) contains hdr_len words.  One text table
// contains zero or more subtables.  Each subtable has a two
// word header.  Fields within this two word header are defined
// as follows.
class TxtItem_c: public RangeContainer_c {
public:
	TxtItem_c():
		mIncrementBetweenDups(*this, 17),
		mStartingBitAddress(*this, 38),
		mNumberOfBitsInLastWord(*this, 6),
		mUnusedUsr1(*this, 3),
		mUnusedUsr2(*this, 5),
		mUnusedCRI(*this, 8),
		mNumOfDuplications(*this, 19),
		mNumberOfTextWords(*this, 32),
		mContent(*this, mNumberOfTextWords)
	{}
	Field_T<int64_t>  mIncrementBetweenDups;
	Field_T<uint64_t> mStartingBitAddress;
	Field_T<uint64_t> mNumberOfBitsInLastWord;
	Field_T<uint64_t> mUnusedUsr1;
	Field_T<uint64_t> mUnusedUsr2;
	Field_T<uint64_t> mUnusedCRI;
	Field_T<uint64_t> mNumOfDuplications;
	Field_T<uint64_t> mNumberOfTextWords;
	// Text words immediately follow the two word header,
	// the number of which is specified by txtntw.
	ArrField_T<uint64_t> mContent;
};

// Constants for the SuggestedRelocationMode, relrm, and xrlrm (relocation mode) fields
enum RelocationMode_e {
	RM_WORD = 0, // Word address
	RM_HALF = 1, // Half word address
	RM_PARC = 2, // Parcel address
	RM_BYTE = 3, // Byte address
	RM_BIT  = 6, // Bit address
	RM_ENTR = 7  // Relocation mode from associated entry (SuggestedRelocationMode); legal on external references only.
};



// A relocation table (REL) contains hdr_len words.  One
// relocation table contains zero or more single word
// relocation entries.  Fields within these single word entries
// are defined as follows.
class RelItem_c : public RangeContainer_c {
public:
	RelItem_c() :
		RelocationType(*this, 1),
		RelocationIndex(*this, 16),
		RightmostBitAddress(*this, 38),
		FieldLengthInBits(*this, 6),
		RelocationMode(*this, 3)
	{}
	Field_T<uint64_t> RelocationType;
	Field_T<uint64_t> RelocationIndex;
	Field_T<uint64_t> RightmostBitAddress;
	Field_T<uint64_t> FieldLengthInBits;
	Field_T<RelocationMode_e> RelocationMode;
};

// Constants for the relrt and xrlrt(relocation type) fields
enum RelocationType_e {
	RT_BLK = 0, // Block entry
	RT_EXT = 1  // External entry
};

// Constants for the xrlsp(special relocation) field
enum SpecialRelocation_e {
	SP_NONE = 0,        // no special relocation
	SP_RVHF = 1,        // reversed halves relocation
	SP_TS3PARCEL = 2,   // three-parcel relocation
	SP_TSHIGH = 3,      // high part of split relocation
	SP_TSLOW = 4        // low part of split relocation
};

// Constants for the xrlsr (sign specification of result) field
enum SignSpecResult_e {
	SR_NONE = 0, // Sign does not matter
	SR_POS  = 1, // Field must be positive
	SR_NEG  = 2, // Field must be negative
	SR_EXT  = 3  // Field is sign extended
};

// An extended relocation table (XRL) contains hdr_len words.
// One extended relocation table contains zero or more double
// word relocation entries.  Fields within these double word
// entries are defined as follows.
class XRelItem_c : public RangeContainer_c {
public:
	XRelItem_c() :
		RelocationType(*this, 1),
		RelocationIndex(*this, 16),
		UnusedUser(*this, 8),
		UnusedCRI1(*this, 7),
		ExtendedAddress(*this, 16),
		SpecialRelocation(*this, 3),
		SignBeforeRelocation(*this, 1),
		SignSpecificationResult(*this, 3),
		FieldLengthInBits(*this, 6),
		RelocationMode(*this, 3),
		UnusedCRI2(*this, 26),
		RightmostBitAddress(*this, 38)
	{}
	Field_T<RelocationType_e> RelocationType;
	Field_T<uint16_t> RelocationIndex;
	Field_T<uint64_t> UnusedUser;
	Field_T<uint64_t> UnusedCRI1;
	Field_T<uint64_t> ExtendedAddress;
	Field_T<SpecialRelocation_e> SpecialRelocation;
	Field_T<uint64_t> SignBeforeRelocation;
	Field_T<SignSpecResult_e> SignSpecificationResult;
	Field_T<uint64_t> FieldLengthInBits;
	Field_T<RelocationMode_e> RelocationMode;
	Field_T<uint64_t> UnusedCRI2;
	Field_T<uint64_t> RightmostBitAddress;
};

class TxtBlock_c {
public:
	std::vector<TxtItem_c> mTxtItems;
	std::vector<RelItem_c> mRelItems;
	std::vector<XRelItem_c> mXRelItems;
	void Read(std::ifstream &aFile, const BlockDesc_c &aBlock) {
		if (aBlock.pdtbkc.Get() == BKC_BS || aBlock.pdtbkc.Get() == BKC_ZD) return; // These blocks don't have text or relo records associated with them
		size_t BlockLen = size_t(aBlock.pdtbkln.Get());
		size_t WordsRead = 0;
		HdrDesc_c Header;
		Header.Read(aFile);
		while (WordsRead < BlockLen) {
			if (Header.hdr_type.Get() != TXT_TYPE) {
				throw Generic_x() << "Unknown TXT block header type: " << OctPrinter(Header.hdr_type.Get());
			}
			// Read all the TXT sub-entries
			size_t SubBlockLen = Header.hdr_len.Get();
			size_t SubBlockRead = Header.SizeInWords();
			size_t SubBlockPayloadSize = 0;
			while (SubBlockRead < SubBlockLen) {
				TxtItem_c Item;
				Item.Read(aFile);
				SubBlockRead += Item.SizeInWords();
				SubBlockPayloadSize += Item.mNumberOfTextWords.Get();
				mTxtItems.push_back(Item);
			};
			CRAY_ASSERT(SubBlockRead == SubBlockLen);
			WordsRead += SubBlockPayloadSize;
			Header.Read(aFile);
		}
		if (WordsRead != BlockLen) throw Generic_x() << "Too many words (" << HexPrinter(WordsRead) << ") read for block of size " << HexPrinter(BlockLen);
		while (Header.hdr_type.Get() == REL_TYPE || Header.hdr_type.Get() == XRL_TYPE) {
			switch (Header.hdr_type.Get()) {
				case REL_TYPE: {
					// Read all the REL sub-entries
					size_t SubBlockLen = Header.hdr_len.Get();
					size_t SubBlockRead = Header.SizeInWords();
					while (SubBlockRead < SubBlockLen) {
						RelItem_c Item;
						Item.Read(aFile);
						SubBlockRead += Item.SizeInWords();
						mRelItems.push_back(Item);
					};
					CRAY_ASSERT(SubBlockRead == SubBlockLen);
					//WordsRead += SubBlockRead;
				} break;
				case XRL_TYPE: {
					// Read all the XRL sub-entries
					size_t SubBlockLen = Header.hdr_len.Get();
					size_t SubBlockRead = Header.SizeInWords();
					while (SubBlockRead < SubBlockLen) {
						// Dump binary content
						uint64_t Content[2];
						aFile.read((char*)Content, sizeof(Content));
						aFile.seekg(-int(sizeof(Content)), std::ios::cur);
//						std::cout << "XREL location: " << HexPrinter(aFile.tellg()) << " content: " << HexPrinter(SwapBytes(Content[0])) << " " << HexPrinter(SwapBytes(Content[1])) << std::endl;
						XRelItem_c Item;
						Item.Read(aFile);
						SubBlockRead += Item.SizeInWords();
						mXRelItems.push_back(Item);
					};
					CRAY_ASSERT(SubBlockRead == SubBlockLen);
					//WordsRead += SubBlockRead;
				} break;
				default:
					CRAY_ASSERT(false);
			}
			Header.Read(aFile);
		}
		aFile.seekg(-int(Header.SizeInBits() / 8), std::ios::cur); // Undo the last header read
	}
	size_t Flatten(std::vector<uint8_t> &aMemory, size_t aStartAddr) {
		size_t Addr = aStartAddr;
		for (auto &TxtItem : mTxtItems) {
			size_t SizeInBytes = TxtItem.mNumberOfTextWords.Get() * sizeof(uint64_t);
			memcpy(&aMemory[Addr], &TxtItem.mContent.Get()[0], SizeInBytes);
			Addr += SizeInBytes;
		}
		return Addr;
	}
};

class ObjFile_c {
public:
	std::vector<BlockDesc_c> mBlocks;
	std::vector<EntryDesc_c> mEntries;
	std::vector<ExternDesc_c> mExterns;
	std::vector<TxtBlock_c> mTxtBlocks;

	void Read(std::ifstream &aFile) {
		HdrDesc_c Header;
		Header.Read(aFile);
		if (Header.hdr_type.Get() != PDT_TYPE) throw Generic_x() << "Invalid object file format: file shouls start with a PDT";
		ProgDescTable_c PDTTable;
		PDTTable.Read(aFile);
		uint64_t FileBase = uint64_t(aFile.tellg()) - 8;
		for (size_t Idx = 0; Idx < PDTTable.pdtblsz.Get(); ) {
			BlockDesc_c Blk;
			uint64_t FileOffs = aFile.tellg();
			Blk.Read(aFile);
			Blk.FileOffset = FileOffs - FileBase;
			mBlocks.push_back(Blk);
			Idx += Blk.SizeInWords();
		}
		for (size_t Idx = 0; Idx < PDTTable.pdtensz.Get(); ) {
			EntryDesc_c Blk;
			Blk.Read(aFile);
			mEntries.push_back(Blk);
			Idx += Blk.SizeInWords();
		}
		FileBase = aFile.tellg();
		for (size_t Idx = 0; Idx < PDTTable.pdtexsz.Get(); ) {
			ExternDesc_c Blk;
			uint64_t FileOffs = aFile.tellg();
			Blk.Read(aFile);
			Blk.FileOffset = FileOffs - FileBase;
			mExterns.push_back(Blk);
			Idx += Blk.SizeInWords();
		}
		for (auto &Block : mBlocks) {
			TxtBlock_c TxtBlock;
			TxtBlock.Read(aFile, Block);
			mTxtBlocks.push_back(TxtBlock);
		}
		bool Skip = false;
		do {
			Skip = false;
			Header.Read(aFile);
			if (Header.hdr_type.Get() == SMT_TYPE || Header.hdr_type.Get() == CMB_TYPE || Header.hdr_type.Get() == GNT_TYPE) {
				aFile.seekg(Header.hdr_len.Get() * sizeof(int64_t) - int(Header.SizeInBits() / 8), std::ios::cur); // Skip symbol tables for now...
				Skip = true;
			}
		} while (Skip);
		if (Header.hdr_type.Get() != MTT_TYPE) throw Generic_x() << "Didn't find MTT termination block where expected (found: " << OctPrinter(Header.hdr_type.Get()) << ")";
		uint64_t ChkSum;
		aFile.read((char*)(&ChkSum), sizeof(ChkSum));
		if (!aFile.good()) throw Generic_x() << "Can't read object file";
	}
};

int main(int argc, const char* argv[])
{
	CommandLine_c CommandLine(argc, argv);
	std::string ImageFile;
	std::string OutFileName;
	CCombinedAddr_t StartAddr;
	CCombinedAddr_t EndAddr;
	CAddr_t InstBaseAddr;
	CAddr_t DataBaseAddr;
	uint32_t Offset = 0;
	uint32_t ImageOffset = 0;
	bool ParseBinaryHeader = true;
	bool ParseObjectFile = false;
	bool UseHexCodes = false;
	bool CreatePatch = false;

	StartAddr.Addr = 0;
	StartAddr.IsParcelAddr = false;
	EndAddr.Addr = 0;
	EndAddr.IsParcelAddr = false;
	InstBaseAddr = 0;
	DataBaseAddr = 0;
	MachineTypes_e MachineType = MachineTypes_e::SV1;
	try {
		while (CommandLine.HasMoreParams()) {
			std::string CurParam = CommandLine.GetNextParam();
			if (CurParam.length() == 0) continue;
			if (CurParam == "-i") {
				ImageOffset = FromString<uint32_t>(CommandLine.GetNextParam());
			}
			else if (CurParam == "-o") {
				Offset = FromString<uint32_t>(CommandLine.GetNextParam());
			}
			else if (CurParam == "-s") {
				StartAddr = FromString<CCombinedAddr_t>(CommandLine.GetNextParam());
			}
			else if (CurParam == "-e") {
				EndAddr = FromString<CCombinedAddr_t>(CommandLine.GetNextParam());
			}
			else if (CurParam == "-ib") {
				InstBaseAddr = FromString(CommandLine.GetNextParam(), StringFormat_e::DataAddr);
			}
			else if (CurParam == "-db") {
				DataBaseAddr = FromString(CommandLine.GetNextParam(), StringFormat_e::DataAddr);
			}
			else if (CurParam == "-raw") {
				ParseObjectFile = false;
				ParseBinaryHeader = false;
			}
			else if (CurParam == "-unicos") {
				ParseObjectFile = false;
				ParseBinaryHeader = true;
			}
			else if (CurParam == "-obj") {
				ParseObjectFile = true;
				ParseBinaryHeader = false;
			}
			else if (CurParam == "-hex") {
				UseHexCodes = true;
			}
			else if (CurParam == "-out") {
				OutFileName = CommandLine.GetNextParam();
			}
			else if (CurParam == "-patch") {
				CreatePatch = true;
			}
			else if (CurParam == "-xmp") {
				MachineType = MachineTypes_e::XMP1xx;
			}
			else if (CurParam == "-ymp") {
				MachineType = MachineTypes_e::SV1;
			}
			else if (ImageFile.length() == 0) {
				ImageFile = CurParam;
			}
			else {
				throw Generic_x("Unkown command line parameter");
			}
		}
		if (ImageFile.length() == 0) throw Generic_x() << "Image file name must be specified";
		if (!boost::filesystem::exists(ImageFile)) throw Generic_x() << "File: " << ImageFile << " doesn't exist";

		std::ostream *Out = &std::cout;
		std::ofstream OutFile;
		if (!OutFileName.empty()) {
			OutFile.open(OutFileName.c_str());
			if (OutFile.bad()) throw Generic_x() << "Can't open output file " << OutFileName;
			Out = &OutFile;
		}
		struct Hdr_s {
			uint64_t Magic;
			uint64_t TextSize;
			uint64_t DataSize;
			uint64_t BbsSize;
			uint64_t SymTableSize;
			uint64_t EntryAddr;
			uint64_t BaseAddr;
			uint32_t Stripped;
			uint32_t InfoBlkPtr;
		} Hdr;
		const uint16_t A_MAGIC_ID = 0407;  // new magic id
		const uint16_t A_MAGIC1_ID = 0407; // normal magic
		const uint16_t A_MAGIC2_ID = 0410; // shared text
		const uint16_t A_MAGIC3_ID = 0411; // normal ymp-32 bit magic
		const uint16_t A_MAGIC4_ID = 0412; // shared text ymp-32 bit magic

		uint64_t MemorySize;

		ObjFile_c ObjFile;
		if (ParseObjectFile) {
			std::ifstream File(ImageFile, std::ios::binary | std::ios::in);
			ObjFile.Read(File);
			ImageOffset = 0;
			MemorySize = 0;
			for (auto &Block : ObjFile.mBlocks) {
				if (Block.pdtbkln.Get() > MemorySize) MemorySize = Block.pdtbkln.Get();
			}
		}
		else if (ParseBinaryHeader) {
			std::ifstream ImageStrm(ImageFile, std::ios::in | std::ios::binary);
			ImageStrm.read((char*)(&Hdr), sizeof(Hdr));
			if (ImageStrm.bad()) throw Generic_x() << "Can't read file: " << ImageFile;
			ImageStrm.close();
			uint64_t* HdrPtr = (uint64_t*)(&Hdr);
			for (size_t Idx = 0; Idx < sizeof(Hdr) / sizeof(uint64_t); ++Idx) {
				HdrPtr[Idx] = SwapBytes(HdrPtr[Idx]);
			}
			switch (Hdr.Magic & 0xffff) {
				case A_MAGIC_ID: {
					// Detect machine type
					switch ((Hdr.Magic >> 16) & 0xff) {
						case 0: // old-style header
							MachineType = MachineTypes_e::XMP1xx;
						break;
						case 4: // XMP mode-indifferent
							MachineType = MachineTypes_e::XMP1xx;
						break;
						case 7: // y-mp
							MachineType = MachineTypes_e::SV1;
						break;
						default:
							throw Generic_x() << "Machine type is not recognized";
					}
				}
				break;
				case A_MAGIC2_ID: MachineType = MachineTypes_e::XMP1xx; break;
				case A_MAGIC3_ID: MachineType = MachineTypes_e::SV1; break;
				case A_MAGIC4_ID: MachineType = MachineTypes_e::SV1; break;
				default: throw Generic_x() << "Unrecognized file magic";
			}
			ImageOffset = 0;
			MemorySize = Hdr.TextSize + Hdr.DataSize + Hdr.BaseAddr;
			std::cout << "Selecting machine type: " << int(MachineType) << std::endl;
		}
		else {
			MemorySize = boost::filesystem::file_size(ImageFile) / sizeof(CInt_t) + Offset - ImageOffset;
		}

		Mainframe_c Mainframe(MemorySize, MachineType);

		if (ParseBinaryHeader) {
			Mainframe.LoadImageFile(ImageFile.c_str(), CAddr_t(Hdr.BaseAddr), sizeof(Hdr), Hdr.TextSize * sizeof(uint64_t));
			Mainframe.LoadImageFile(ImageFile.c_str(), CAddr_t(Hdr.BaseAddr + Hdr.TextSize), sizeof(Hdr) + Hdr.TextSize * sizeof(uint64_t), Hdr.DataSize * sizeof(uint64_t));
			EndAddr.Addr = CAddr_t(Hdr.BaseAddr + Hdr.TextSize);
			EndAddr.IsParcelAddr = false;
			StartAddr.Addr = CAddr_t(Hdr.BaseAddr);
			StartAddr.IsParcelAddr = false;
		}
		else if (ParseObjectFile) {
			bool Found = false;
			for (size_t Idx = 0; Idx < ObjFile.mBlocks.size(); ++Idx) {
				if (ObjFile.mBlocks[Idx].pdtbkc.Get() != BKC_IX) continue;
				if (Found) {
					std::cout << "Multiple code section found, only first one will be disassembled" << std::endl;
					*Out << "Multiple code section found, only first one will be disassembled" << std::endl;
					continue;
				}
				Found = true;
				StartAddr.Addr = 0;
				StartAddr.IsParcelAddr = false;
				std::vector<uint8_t> &Memory = Mainframe.GetMemory();
				EndAddr.Addr = CAddr_t(ObjFile.mTxtBlocks[Idx].Flatten(Memory, 0) / sizeof(int64_t));
				EndAddr.IsParcelAddr = false;
			}
		}
		else {
			Mainframe.LoadImageFile(ImageFile.c_str(), CAddr_t(Offset), ImageOffset * sizeof(CInt_t));
		}

		// Convert to parcel address if it's not already
		if (!StartAddr.IsParcelAddr) StartAddr.Addr = uint32_t(StartAddr.Addr) * 4;
		if (!EndAddr.IsParcelAddr) EndAddr.Addr = uint32_t(EndAddr.Addr) * 4;

		if (EndAddr.Addr == CAddr_t(0)) {
			EndAddr.Addr = CAddr_t(Mainframe.GetMemorySize() / 2);
		}

		// Uggly cast to the proper CPU type, but we know it's that kind: in the disassembler we'll never instantiate anything but a soft CPU
		SoftCpu_c &Cpu = *(SoftCpu_c *)(&Mainframe.GetCpu(0));
		Cpu.SetInstBaseAddr(InstBaseAddr);
		Cpu.SetDataBaseAddr(DataBaseAddr);

		CProgramAddr_t CurAddr = StartAddr.Addr;
		CProgramAddr_t EntryPoint = CProgramAddr_t(Hdr.EntryAddr);
		if (!CreatePatch) {
			while (CurAddr < EndAddr.Addr) {
				std::stringstream Disassembly;
				std::stringstream Explanation;
				std::stringstream InstCodes;
				std::stringstream HexInstCodes;

				CProgramAddr_t Increment = CProgramAddr_t(Cpu.Decode(CurAddr, Disassembly, Explanation, InstCodes, HexInstCodes));

				CProgramAddr_t PhysicalAddress = (CurAddr + CProgramAddr_t(size_t(InstBaseAddr) * 4));
				if (ParseBinaryHeader) {
					if (CurAddr == EntryPoint) *Out << "==>"; else *Out << "   ";
				}
				int CharsPrinted = 0;
				if (ParseObjectFile) {
					bool Found = false;
					for (auto &Entry : ObjFile.mEntries) {
						if (Entry.EntryPointInBits.Get() == CurAddr * 16 && Entry.BlockIndex.Get() == 1) { // TODO: Block index should match the one we're actually disassembling. For right now (since we're only dealing with the first block) it's a constant.
							*Out << Entry.mName.Get() << std::endl;
							Found = true;
						}
					}
					if (Found) {
						*Out << "=>";
						CharsPrinted += 2;
					}
				}
				if (ParseObjectFile) {
					bool Found = false;
					for (auto &RelItem : ObjFile.mTxtBlocks[0].mXRelItems) {
						int EndBit = int(RelItem.RightmostBitAddress.Get()) + 1;
						int StartBit = EndBit - int(RelItem.FieldLengthInBits.Get());
						if (StartBit > int((CurAddr + Increment) * 16 - 1)) continue;
						if (EndBit <= int(CurAddr * 16)) continue;

						Found = true;
						if (RelItem.RelocationType.Get() == RT_EXT) {
							size_t RelIndex = size_t(RelItem.RelocationIndex.Get());
							bool EntryFound = false;
							for (auto &Extern : ObjFile.mExterns) {
								if (Extern.FileOffset == RelIndex * sizeof(int64_t)) {
									Disassembly << " <EXT " << Extern.mName.Get() << " EXT>";
									EntryFound = true;
									break;
								}
							}
							if (!EntryFound) {
								Disassembly << " <EXT " << RelIndex << " EXT>";
							}
						}
						else {
							size_t RelIndex = size_t(RelItem.RelocationIndex.Get());
							bool EntryFound = false;
							for (auto &Block : ObjFile.mBlocks) {
								if (Block.FileOffset == RelIndex * sizeof(int64_t)) {
									Disassembly << " <BLK " << Block.mName.Get() << " BLK>";
									EntryFound = true;
									break;
								}
							}
							if (!EntryFound) {
								Disassembly << " <BLK " << RelIndex << " BLK>";
							}
						}
						break;
					}
					if (Found) {
						*Out << "*";
						CharsPrinted += 1;
					}
				}
				for (; CharsPrinted < 4; ++CharsPrinted) *Out << ' ';
				*Out << InstAddr(CurAddr) << " (" << InstAddr(PhysicalAddress) << ") " << (UseHexCodes ? HexInstCodes.str() : InstCodes.str()) << " - " << std::left << std::setw(30) << Disassembly.str() << std::setw(0) << " | " << Explanation.str() << std::endl;
				CurAddr = CurAddr + Increment;
			}
			if (ParseBinaryHeader) { // Dump the text section
				*Out << "------------- TEXT section -------------" << std::endl;
				CAddr_t Addr = CAddr_t(Hdr.BaseAddr + Hdr.TextSize);
				while (Addr < Hdr.BaseAddr + Hdr.TextSize + Hdr.DataSize) {
					*Out << HexPrinter(Addr) << ": " << HexPrinter(Mainframe.MemRead<uint64_t>(Addr)) << std::endl;
					Addr = Addr + 1;
				}
			}
			if (ParseObjectFile) { // Dump data sections
				for (size_t Idx = 0; Idx < ObjFile.mBlocks.size(); ++Idx) {
					auto &Block = ObjFile.mBlocks[Idx];
					auto &Txt = ObjFile.mTxtBlocks[Idx];
					if (Block.pdtbkc.Get() == BKC_DT || Block.pdtbkc.Get() == BKC_CN) {
						*Out << std::endl;
						*Out << "--------------- " << Block.mName.Get() << " ------------" << std::endl;

						StartAddr.Addr = 0;
						StartAddr.IsParcelAddr = false;
						std::vector<uint8_t> &Memory = Mainframe.GetMemory();
						EndAddr.Addr = CAddr_t(Txt.Flatten(Memory, 0) / sizeof(uint64_t));
						EndAddr.IsParcelAddr = false;

						for (CAddr_t Addr = StartAddr.Addr; Addr < EndAddr.Addr; ++Addr) {


							XRelItem_c *FoundRelItem = nullptr;
							for (auto &RelItem : Txt.mXRelItems) {
								int EndBit = int(RelItem.RightmostBitAddress.Get()) + 1;
								int StartBit = EndBit - int(RelItem.FieldLengthInBits.Get());
								if (StartBit > int((Addr + 1) * 64 - 1)) continue;
								if (EndBit <= int(Addr * 64)) continue;

								FoundRelItem = &RelItem;
								break;
							}
							if (FoundRelItem != nullptr) {
								*Out << "*   ";
							} else {
								*Out << "    ";
							}

							CInt_t Data = Mainframe.MemRead<uint64_t>(Addr);
							*Out << HexPrinter(Addr) << ": " << HexPrinter(SwapBytes(Data)) << " - ";
							for (size_t j = 0; j < sizeof(CInt_t); ++j) {
								*Out << PrintableChar(char(Data & 0xff));
								Data >>= 8;
							}

							if (FoundRelItem != nullptr) {
								if (FoundRelItem->RelocationType.Get() == RT_EXT) {
									size_t RelIndex = size_t(FoundRelItem->RelocationIndex.Get());
									bool EntryFound = false;
									for (auto &Extern : ObjFile.mExterns) {
										if (Extern.FileOffset == RelIndex * sizeof(int64_t)) {
											*Out << " <EXT " << Extern.mName.Get() << " EXT>";
											EntryFound = true;
											break;
										}
									}
									if (!EntryFound) {
										*Out << " <EXT " << RelIndex << " EXT>";
									}
								}
								else {
									size_t RelIndex = size_t(FoundRelItem->RelocationIndex.Get());
									bool EntryFound = false;
									for (auto &Block : ObjFile.mBlocks) {
										if (Block.FileOffset == RelIndex * sizeof(int64_t)) {
											*Out << " <BLK " << Block.mName.Get() << " BLK>";
											EntryFound = true;
											break;
										}
									}
									if (!EntryFound) {
										*Out << " <BLK " << RelIndex << " BLK>";
									}
								}
							}
							*Out << std::endl;
						}
					}
				}
			}
		}
		else {
			while (CurAddr < EndAddr.Addr) {
				CProgramAddr_t PhysicalAddress = (CurAddr + CProgramAddr_t(size_t(InstBaseAddr) * 4));
				CParcel_t Data = SwapBytes(Mainframe.MemReadByType<CParcel_t>(PhysicalAddress));
				*Out << HexPrinter(Data);
				uint16_t Mask = 0xffff;
				if (ParseObjectFile) {
					for (auto &RelItem : ObjFile.mTxtBlocks[0].mXRelItems) {
						int EndBit = int(RelItem.RightmostBitAddress.Get()) + 1;
						int StartBit = EndBit - int(RelItem.FieldLengthInBits.Get());
						if (StartBit > int(CurAddr * 16 + 15)) continue;
						if (EndBit <= int(CurAddr * 16)) continue;
						StartBit -= CurAddr * 16;
						EndBit -= CurAddr * 16;
						if (StartBit < 0) StartBit = 0;
						if (EndBit > 16) EndBit = 16;
						CRAY_ASSERT(StartBit <= 15);
						CRAY_ASSERT(EndBit > 0);
						Mask = ~(((1 << (EndBit - StartBit)) - 1) << StartBit);
						Mask = SwapBytes(Mask);
						break;
					}
					*Out << ":" << HexPrinter(Mask);
				} else {
					*Out << ":" << HexPrinter(Mask);
				}
				*Out << " # " << InstAddr(CurAddr) << std::endl;
				CurAddr += 1;
			}
		}
		return 0;
	}
	catch(std::exception &Ex) {
		return PrintUsage(argv[0], Ex.what());
	}
}

