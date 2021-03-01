#ifndef __EXCHANGE_PACKET_H__
#define __EXCHANGE_PACKET_H__

#include "utils.h"

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

class XmpErrorType_c {
public:
	explicit XmpErrorType_c(uint8_t aData): Data(aData) {}
	explicit XmpErrorType_c(uint64_t aData): Data((uint8_t)aData) {}
	bool IsUncorrectable() const { return (Data & UncorrectableMask) != 0; }
	bool IsCorrectable() const {   return (Data & CorrectableMask) != 0; }
	void SetUncorrectable(bool aValue) { Data = SET_BITS(Data, UncorrectableMask, aValue ? UncorrectableMask : 0); }
	void SetCorrectable(bool aValue) {   Data = SET_BITS(Data, CorrectableMask,   aValue ? CorrectableMask   : 0); }
	operator uint8_t() { return Data; }
protected:
	enum {
		CorrectableMask = 1,
		UncorrectableMask = 2
	};
	uint8_t Data;
};

inline std::ostream & operator << (std::ostream &aStream, const XmpErrorType_c &aE) {
	aStream << (aE.IsCorrectable() ? ".C" : ".-");
	aStream << (aE.IsUncorrectable() ? ".U." : ".-.");
	return aStream;
}

class Mode_c {
public:
	Mode_c() {
		mWaitingForSemaphore = false;
		mYmpEnableSecondVectorLogical = false;
		mSv1BitMatrixLoaded = false;
		mFloatingPointErrorStatus = false;
		mBidirectionalMemoryMode = false;
		mOperandRangeErrorMode = false;
		mFloatingPointErrorMode = false;
		mUncorrectableMemoryErrorMode = false;
		mCorrectableMemoryErrorMode = false;
		mYmpEnhancedAddressingMode = false;
		mSelectedForExternalInterrupts = false;
		mInterruptMonitorMode = false;
		mMonitorMode = false;
	}
	explicit Mode_c(uint16_t aData, MachineTypes_e aMachineType): mMachineType(aMachineType) {
		if (IsXmp(mMachineType)) {
			mWaitingForSemaphore = (aData & uint16_t(XmpModes_e::WaitingForSemaphore)) != 0;
			mYmpEnableSecondVectorLogical = false;
			mSv1BitMatrixLoaded = false;
			mFloatingPointErrorStatus = (aData & uint16_t(XmpModes_e::FloatingPointErrorStatus)) != 0;
			mBidirectionalMemoryMode = (aData & uint16_t(XmpModes_e::BidirectionalMemoryMode)) != 0;
			mOperandRangeErrorMode = (aData & uint16_t(XmpModes_e::OperandRangeErrorMode)) != 0;
			mFloatingPointErrorMode = (aData & uint16_t(XmpModes_e::FloatingPointErrorMode)) != 0;
			mUncorrectableMemoryErrorMode = (aData & uint16_t(XmpModes_e::UncorrectableMemoryErrorMode)) != 0;
			mCorrectableMemoryErrorMode = (aData & uint16_t(XmpModes_e::CorrectableMemoryErrorMode)) != 0;
			mYmpEnhancedAddressingMode = false;
			mSelectedForExternalInterrupts = (aData & uint16_t(XmpModes_e::SelectedForExternalInterrupts)) != 0;
			mInterruptMonitorMode = (aData & uint16_t(XmpModes_e::InterruptMonitorMode)) != 0;
			mMonitorMode = (aData & uint16_t(XmpModes_e::MonitorMode)) != 0;
		} else {
			mWaitingForSemaphore = (aData & uint16_t(YmpModes_e::WaitingForSemaphore)) != 0;
			mYmpEnableSecondVectorLogical = (aData & uint16_t(YmpModes_e::YmpEnableSecondVectorLogical)) != 0;
			mSv1BitMatrixLoaded = mMachineType == MachineTypes_e::SV1 ? (aData & uint16_t(YmpModes_e::Sv1BitMatrixLoaded)) != 0 : false;
			mFloatingPointErrorStatus = (aData & uint16_t(YmpModes_e::FloatingPointErrorStatus)) != 0;
			mBidirectionalMemoryMode = (aData & uint16_t(YmpModes_e::BidirectionalMemoryMode)) != 0;
			mOperandRangeErrorMode = (aData & uint16_t(YmpModes_e::OperandRangeErrorMode)) != 0;
			mFloatingPointErrorMode = (aData & uint16_t(YmpModes_e::FloatingPointErrorMode)) != 0;
			mUncorrectableMemoryErrorMode = (aData & uint16_t(YmpModes_e::UncorrectableMemoryErrorMode)) != 0;
			mCorrectableMemoryErrorMode = (aData & uint16_t(YmpModes_e::CorrectableMemoryErrorMode)) != 0;
			mYmpEnhancedAddressingMode = (aData & uint16_t(YmpModes_e::YmpEnhancedAddressingMode)) != 0;
			mSelectedForExternalInterrupts = (aData & uint16_t(YmpModes_e::SelectedForExternalInterrupts)) != 0;
			mInterruptMonitorMode = (aData & uint16_t(YmpModes_e::InterruptMonitorMode)) != 0;
			mMonitorMode = (aData & uint16_t(YmpModes_e::MonitorMode)) != 0;
		}
	}
	uint16_t GetRawValue() const {
		uint16_t RetVal = 0;
		if (IsXmp(mMachineType)) {
			RetVal |= mWaitingForSemaphore ? uint16_t(XmpModes_e::WaitingForSemaphore) : 0;
			RetVal |= mFloatingPointErrorStatus ? uint16_t(XmpModes_e::FloatingPointErrorStatus) : 0;
			RetVal |= mBidirectionalMemoryMode ? uint16_t(XmpModes_e::BidirectionalMemoryMode) : 0;
			RetVal |= mOperandRangeErrorMode ? uint16_t(XmpModes_e::OperandRangeErrorMode) : 0;
			RetVal |= mFloatingPointErrorMode ? uint16_t(XmpModes_e::FloatingPointErrorMode) : 0;
			RetVal |= mUncorrectableMemoryErrorMode ? uint16_t(XmpModes_e::UncorrectableMemoryErrorMode) : 0;
			RetVal |= mCorrectableMemoryErrorMode ? uint16_t(XmpModes_e::CorrectableMemoryErrorMode) : 0;
			RetVal |= mSelectedForExternalInterrupts ? uint16_t(XmpModes_e::SelectedForExternalInterrupts) : 0;
			RetVal |= mInterruptMonitorMode ? uint16_t(XmpModes_e::InterruptMonitorMode) : 0;
			RetVal |= mMonitorMode ? uint16_t(XmpModes_e::MonitorMode) : 0;
		}
		else {
			RetVal |= mWaitingForSemaphore ? uint16_t(YmpModes_e::WaitingForSemaphore) : 0;
			RetVal |= mYmpEnableSecondVectorLogical ? uint16_t(YmpModes_e::YmpEnableSecondVectorLogical) : 0;
			if (mMachineType == MachineTypes_e::SV1) {
				RetVal |= mSv1BitMatrixLoaded ? uint16_t(YmpModes_e::Sv1BitMatrixLoaded) : 0;
			}
			RetVal |= mFloatingPointErrorStatus ? uint16_t(YmpModes_e::FloatingPointErrorStatus) : 0;
			RetVal |= mBidirectionalMemoryMode ? uint16_t(YmpModes_e::BidirectionalMemoryMode) : 0;
			RetVal |= mOperandRangeErrorMode ? uint16_t(YmpModes_e::OperandRangeErrorMode) : 0;
			RetVal |= mFloatingPointErrorMode ? uint16_t(YmpModes_e::FloatingPointErrorMode) : 0;
			RetVal |= mUncorrectableMemoryErrorMode ? uint16_t(YmpModes_e::UncorrectableMemoryErrorMode) : 0;
			RetVal |= mCorrectableMemoryErrorMode ? uint16_t(YmpModes_e::CorrectableMemoryErrorMode) : 0;
			RetVal |= mYmpEnhancedAddressingMode ? uint16_t(YmpModes_e::YmpEnhancedAddressingMode) : 0;
			RetVal |= mSelectedForExternalInterrupts ? uint16_t(YmpModes_e::SelectedForExternalInterrupts) : 0;
			RetVal |= mInterruptMonitorMode ? uint16_t(YmpModes_e::InterruptMonitorMode) : 0;
			RetVal |= mMonitorMode ? uint16_t(YmpModes_e::MonitorMode) : 0;
		}
		return RetVal;
	}
	bool IsWaitingForSemaphore() const { return mWaitingForSemaphore; }
	bool IsYmpEnableSecondVectorLogical() const { return mYmpEnableSecondVectorLogical; }
	bool IsSv1BitMatrixLoaded() const { return mSv1BitMatrixLoaded; }
	bool IsFloatingPointErrorStatus() const { return mFloatingPointErrorStatus; }
	bool IsBidirectionalMemoryMode() const { return mBidirectionalMemoryMode; }
	bool IsOperandRangeErrorMode() const { return mOperandRangeErrorMode; }
	bool IsFloatingPointErrorMode() const { return mFloatingPointErrorMode; }
	bool IsUncorrectableMemoryErrorMode() const { return mUncorrectableMemoryErrorMode; }
	bool IsCorrectableMemoryErrorMode() const { return mCorrectableMemoryErrorMode; }
	bool IsYmpEnhancedAddressingMode() const { return mYmpEnhancedAddressingMode; }
	bool IsSelectedForExternalInterrupts() const { return mSelectedForExternalInterrupts; }
	bool IsInterruptMonitorMode() const { return mInterruptMonitorMode; }
	bool IsMonitorMode() const { return mMonitorMode; }

	void SetWaitingForSemaphore(bool aValue) { mWaitingForSemaphore = aValue; }
	void SetYmpEnableSecondVectorLogical(bool aValue) { CRAY_ASSERT(!IsXmp(mMachineType) || !aValue); mYmpEnableSecondVectorLogical = aValue; }
	void SetSv1BitMatrixLoaded(bool aValue) { CRAY_ASSERT(mMachineType == MachineTypes_e::SV1 || !aValue); mSv1BitMatrixLoaded = aValue; }
	void SetFloatingPointErrorStatus(bool aValue) { mFloatingPointErrorStatus = aValue; }
	void SetBidirectionalMemoryMode(bool aValue) { mBidirectionalMemoryMode = aValue; }
	void SetOperandRangeErrorMode(bool aValue) { mOperandRangeErrorMode = aValue; }
	void SetFloatingPointErrorMode(bool aValue) {
		mFloatingPointErrorMode = aValue;
		if (aValue) {
			std::cout << "mFloatingPointErrorMode is set!" << std::endl;
		}
	}
	void SetUncorrectableMemoryErrorMode(bool aValue) { mUncorrectableMemoryErrorMode = aValue; }
	void SetCorrectableMemoryErrorMode(bool aValue) { mCorrectableMemoryErrorMode = aValue; }
	void SetYmpEnhancedAddressingMode(bool aValue) { CRAY_ASSERT(!IsXmp(mMachineType) || !aValue); mYmpEnhancedAddressingMode = aValue; }
	void SetSelectedForExternalInterrupts(bool aValue) { mSelectedForExternalInterrupts = aValue; }
	void SetInterruptMonitorMode(bool aValue) { mInterruptMonitorMode = aValue; }
	void SetMonitorMode(bool aValue) { mMonitorMode = aValue; }
protected:
	enum class XmpModes_e {
		WaitingForSemaphore            = 0x0010,
		FloatingPointErrorStatus       = 0x0008,
		BidirectionalMemoryMode        = 0x0004,
		SelectedForExternalInterrupts  = 0x0002,
		InterruptMonitorMode           = 0x0001,

		OperandRangeErrorMode          = 0x0200,
		CorrectableMemoryErrorMode     = 0x0100,
		FloatingPointErrorMode         = 0x0080,
		UncorrectableMemoryErrorMode   = 0x0040,
		MonitorMode                    = 0x0020
	};

	enum class YmpModes_e {
		WaitingForSemaphore           = 0x1000,
		YmpEnableSecondVectorLogical  = 0x0800,
		Sv1BitMatrixLoaded            = 0x0400, // NOTE: this is the 'PS' bit in the J90.
		FloatingPointErrorStatus      = 0x0200,
		BidirectionalMemoryMode       = 0x0100,
		OperandRangeErrorMode         = 0x0080,
		FloatingPointErrorMode        = 0x0040,
		UncorrectableMemoryErrorMode  = 0x0020,
		CorrectableMemoryErrorMode    = 0x0010,
		YmpEnhancedAddressingMode     = 0x0008,
		SelectedForExternalInterrupts = 0x0004,
		InterruptMonitorMode          = 0x0002,
		MonitorMode                   = 0x0001
	};
	bool mWaitingForSemaphore;
	bool mYmpEnableSecondVectorLogical;
	bool mSv1BitMatrixLoaded;
	bool mFloatingPointErrorStatus;
	bool mBidirectionalMemoryMode;
	bool mOperandRangeErrorMode;
	bool mFloatingPointErrorMode;
	bool mUncorrectableMemoryErrorMode;
	bool mCorrectableMemoryErrorMode;
	bool mYmpEnhancedAddressingMode;
	bool mSelectedForExternalInterrupts;
	bool mInterruptMonitorMode;
	bool mMonitorMode;
public:
	MachineTypes_e mMachineType;
};

inline std::ostream & operator << (std::ostream &aStream, const Mode_c &aMode) {
	if (IsXmp(aMode.mMachineType)) {
		aStream << (aMode.IsWaitingForSemaphore() ? ".WS" : ".--");
		aStream << (aMode.IsFloatingPointErrorStatus() ? ".FPS" : ".---");
		aStream << (aMode.IsBidirectionalMemoryMode() ? ".BDM" : ".---");
		aStream << (aMode.IsSelectedForExternalInterrupts() ? ".SEI" : ".---");
		aStream << (aMode.IsInterruptMonitorMode() ? ".IMM" : ".---");
		aStream << (aMode.IsOperandRangeErrorMode() ? ".IOR" : ".---");
		aStream << (aMode.IsCorrectableMemoryErrorMode() ? ".ICM" : ".---");
		aStream << (aMode.IsFloatingPointErrorMode() ? ".IFP" : ".---");
		aStream << (aMode.IsUncorrectableMemoryErrorMode() ? ".IUM" : ".---");
		aStream << (aMode.IsMonitorMode() ? ".MM." : ".--.");
	} else {
		aStream << (aMode.IsWaitingForSemaphore() ? ".WS" : ".--");
		aStream << (aMode.IsYmpEnableSecondVectorLogical() ? ".ESVL" : ".----");
		aStream << (aMode.IsSv1BitMatrixLoaded() ? ".BML" : ".---");
		aStream << (aMode.IsFloatingPointErrorStatus() ? ".FPS" : ".---");
		aStream << (aMode.IsBidirectionalMemoryMode() ? ".BDM" : ".---");
		aStream << (aMode.IsOperandRangeErrorMode() ? ".IOR" : ".---");
		aStream << (aMode.IsFloatingPointErrorMode() ? ".IFP" : ".---");
		aStream << (aMode.IsUncorrectableMemoryErrorMode() ? ".IUM" : ".---");
		aStream << (aMode.IsCorrectableMemoryErrorMode() ? ".ICM" : ".---");
		aStream << (aMode.IsYmpEnhancedAddressingMode() ? ".EAM" : ".---");
		aStream << (aMode.IsSelectedForExternalInterrupts() ? ".SEI" : ".---");
		aStream << (aMode.IsInterruptMonitorMode() ? ".IMM" : ".---");
		aStream << (aMode.IsMonitorMode() ? ".MM." : ".--.");
	}
	return aStream;
}


class Flags_c {
public:
	Flags_c(): Data(0) {}
	Flags_c(const Flags_c &aFlags) : Data(aFlags.Data) {}
	explicit Flags_c(uint16_t aData): Data(aData) {}
	explicit Flags_c(uint64_t aData): Data((uint16_t)aData) {}
	bool IsInterruptFromInternalCpu() const {      return (Data & InterProcessorInterrupt    ) != 0; } // Other CPU
	bool IsDeadlock() const {                      return (Data & Deadlock                   ) != 0; } // THERE'S A BUG HERE: in a deadlock, ALL processors participating in a dead-lock are interrupted. Thus, this interrupt is 'Other CPU'
	bool IsProgrammableClockInterrupt() const {    return (Data & ProgrammableClockInterrupt ) != 0; } // Internal
	bool IsMcuInterrupt() const {                  return (Data & McuInterrupt               ) != 0; } // IOS
	bool IsFloatingPointError() const {            return (Data & FloatingPointError         ) != 0; } // Internal
	bool IsOperandRangeError() const {             return (Data & OperandRangeError          ) != 0; } // Internal
	bool IsProgramRangeError() const {             return (Data & ProgramRangeError          ) != 0; } // Internal
	bool IsMemoryError() const {                   return (Data & MemoryError                ) != 0; } // Internal
	bool IsIoInterrupt() const {                   return (Data & IoInterrupt                ) != 0; } // IOS (routed)
	bool IsErrorExit() const {                     return (Data & ErrorExit                  ) != 0; } // Internal
	bool IsNormalExit() const {                    return (Data & NormalExit                 ) != 0; } // Internal
	void SetInterProcessorInterrupt() {       Data |= InterProcessorInterrupt       ; }
	void SetDeadlock() {                      Data |= Deadlock                      ; }
	void SetProgrammableClockInterrupt() {    Data |= ProgrammableClockInterrupt    ; }
	void SetMcuInterrupt() {                  Data |= McuInterrupt                  ; }
	void SetFloatingPointError() {            Data |= FloatingPointError            ; }
	void SetOperandRangeError() {             Data |= OperandRangeError             ; }
	void SetProgramRangeError() {             Data |= ProgramRangeError             ; }
	void SetMemoryError() {                   Data |= MemoryError                   ; }
	void SetIoInterrupt() {                   Data |= IoInterrupt                   ; }
	void SetErrorExit() {                     Data |= ErrorExit                     ; }
	void SetNormalExit() {                    Data |= NormalExit                    ; }
	void ClearInterProcessorInterrupt() {     Data &= ~InterProcessorInterrupt       ; }
	void ClearDeadlock() {                    Data &= ~Deadlock                      ; }
	void ClearProgrammableClockInterrupt() {  Data &= ~ProgrammableClockInterrupt    ; }
	void ClearMcuInterrupt() {                Data &= ~McuInterrupt                  ; }
	void ClearFloatingPointError() {          Data &= ~FloatingPointError            ; }
	void ClearOperandRangeError() {           Data &= ~OperandRangeError             ; }
	void ClearProgramRangeError() {           Data &= ~ProgramRangeError             ; }
	void ClearMemoryError() {                 Data &= ~MemoryError                   ; }
	void ClearIoInterrupt() {                 Data &= ~IoInterrupt                   ; }
	void ClearErrorExit() {                   Data &= ~ErrorExit                     ; }
	void ClearNormalExit() {                  Data &= ~NormalExit                    ; }
	void ClearBits(uint16_t aMask) {          Data &= ~aMask                         ; }
//	operator uint16_t() { return Data; }
	enum: uint16_t {
		InterProcessorInterrupt       = 0x0400,
		Deadlock                      = 0x0200,
		ProgrammableClockInterrupt    = 0x0100,
		McuInterrupt                  = 0x0080,
		FloatingPointError            = 0x0040,
		OperandRangeError             = 0x0020,
		ProgramRangeError             = 0x0010,
		MemoryError                   = 0x0008,
		IoInterrupt                   = 0x0004,
		ErrorExit                     = 0x0002,
		NormalExit                    = 0x0001
	};
	uint16_t Get() const { return Data; }
	void Set(uint16_t aValue) { Data = aValue; }
protected:
//	AtomicUInt16 Data;
	uint16_t Data;
	friend class SoftCpu_c;
};

inline std::ostream & operator << (std::ostream &aStream, const Flags_c &aFlags) {
	aStream << (aFlags.IsInterruptFromInternalCpu() ? ".ICP" : ".---");
	aStream << (aFlags.IsDeadlock() ? ".DL" : ".--");
	aStream << (aFlags.IsProgrammableClockInterrupt() ? ".PCI" : ".---");
	aStream << (aFlags.IsMcuInterrupt() ? ".MCU" : ".---");
	aStream << (aFlags.IsFloatingPointError() ? ".PFE" : ".---");
	aStream << (aFlags.IsOperandRangeError() ? ".ORE" : ".---");
	aStream << (aFlags.IsProgramRangeError() ? ".PRE" : ".---");
	aStream << (aFlags.IsMemoryError() ? ".ME" : ".--");
	aStream << (aFlags.IsIoInterrupt() ? ".IOI" : ".---");
	aStream << (aFlags.IsErrorExit() ? ".EEX" : ".---");
	aStream << (aFlags.IsNormalExit() ? ".NEX." : ".---.");
	return aStream;
}

class ExchangePacket_c {
public:
	static const size_t mDataSize = 16;
	explicit ExchangePacket_c(MachineTypes_e aMachineType):
		mMachineType(aMachineType)
	{
		memset(Data, 0, sizeof(Data));
		if (aMachineType == MachineTypes_e::J90) SetYmpProcessorType(0);
		if (aMachineType == MachineTypes_e::SV1) SetYmpProcessorType(2);
	}
	explicit ExchangePacket_c(MachineTypes_e aMachineType, void *aMem):
		mMachineType(aMachineType)
	{
		Read(aMem);
		if (aMachineType == MachineTypes_e::J90) SetYmpProcessorType(0);
		if (aMachineType == MachineTypes_e::SV1) SetYmpProcessorType(2);
	}
	void SetData(CInt_t aData, size_t aOfs) {
		CRAY_ASSERT(aOfs < mDataSize);
		Data[aOfs] = SwapBytes(aData);
	}
	CInt_t GetData(size_t aOfs) const {
		CRAY_ASSERT(aOfs < mDataSize);
		return SwapBytes(Data[aOfs]);
	}
	void Read(void *aMem) {
		memcpy(Data,aMem,sizeof(Data));
		for(size_t i=0;i<sizeof(Data)/sizeof(Data[0]);++i) Data[i] = SwapBytes(Data[i]);
	}
	void Write(void *aMem) const {
		uint64_t SwappedData[16];
		for(size_t i=0;i<sizeof(Data)/sizeof(Data[0]);++i) SwappedData[i] = SwapBytes(Data[i]);
		memcpy(aMem,SwappedData,sizeof(Data));
	}

	size_t GetProtectionBits() const {
		switch (mMachineType) {
		case MachineTypes_e::XMP1xx: return 19;
		case MachineTypes_e::XMP2xx: return 17;
		case MachineTypes_e::XMP4xx: return 18;
		case MachineTypes_e::YMP: return 24;
		case MachineTypes_e::YEL: return 23;
		case MachineTypes_e::J90: return 22;
		case MachineTypes_e::SV1: return 22;
		default: {
			throw Generic_x() << "Unknwon machine type: " << int(mMachineType);
		}
		}
	}
	size_t GetProtectionTop() const {
		switch (mMachineType) {
		case MachineTypes_e::XMP1xx: return 63-16;
		case MachineTypes_e::XMP2xx: return 63-18;
		case MachineTypes_e::XMP4xx: return 63-16;
		case MachineTypes_e::YMP: return 55;
		case MachineTypes_e::YEL: return 54;
		case MachineTypes_e::J90: return 55;
		case MachineTypes_e::SV1: return 55;
		default: {
			throw Generic_x() << "Unknwon machine type: " << int(mMachineType);
		}
		}
	}
	size_t GetProtectionShift() const {
		switch (mMachineType) {
		case MachineTypes_e::XMP1xx: return 5;
		case MachineTypes_e::XMP2xx: return 5;
		case MachineTypes_e::XMP4xx: return 6;
		case MachineTypes_e::YMP: return 8; // ???
		case MachineTypes_e::YEL: return 8;
		case MachineTypes_e::J90: return 10;
		case MachineTypes_e::SV1: return 10;
		default: {
			throw Generic_x() << "Unknwon machine type: " << int(mMachineType);
		}
		}
	}

	#ifdef CREATE_FIELD
	#error CREATE_FIELD is already defined!
	#endif
	#ifdef CREATE_PROT_FIELD
	#error CREATE_PROT_FIELD is already defined!
	#endif
	#define CREATE_FIELD(aaType, aaFieldName, aaWordIdx, aaStartBit, aaEndBit, aaShift)                                                                   \
		aaType Get##aaFieldName() const { return aaType(GetBits(Data[(aaWordIdx)],63-(aaStartBit),63-(aaEndBit)) << aaShift); }                           \
		void Set##aaFieldName(aaType aValue) { Data[(aaWordIdx)] = SetBits(Data[(aaWordIdx)],63-(aaStartBit),63-(aaEndBit),uint64_t(aValue) >> aaShift); }
	#define CREATE_PROT_FIELD(aaFieldName, aaWordIdx)                                                                                                                                                 \
		CAddr_t Get##aaFieldName() const { return CAddr_t(GetBits(Data[(aaWordIdx)],GetProtectionTop(),GetProtectionTop()-GetProtectionBits()+1) << GetProtectionShift()); }                             \
		void Set##aaFieldName(CAddr_t aValue) { Data[(aaWordIdx)] = SetBits(Data[(aaWordIdx)],GetProtectionTop(),GetProtectionTop()-GetProtectionBits()+1,uint64_t(aValue) >> GetProtectionShift()); }


	//              Type                       FieldName      WordIdx  StartBit  EndBit  Shift
	//******************************************** XMP FIELDS ****************************************
	CREATE_FIELD(uint8_t,             XmpProcessorNumber,        0,        0,      1,     0)
	CREATE_FIELD(XmpErrorType_c,            XmpErrorType,        0,        2,      3,     0)
	CREATE_FIELD(uint8_t,                    XmpSyndrome,        0,        4,     11,     0)
	CREATE_FIELD(CProgramAddr_t,       XmpProgramAddress,        0,       16,     39,     0)
	CREATE_FIELD(uint8_t,                    XmpReadMode,        1,        0,      1,     0)
	CREATE_FIELD(CAddr_t,                 XmpReadAddress,        1,        2,     11,     6)
	CREATE_FIELD(uint8_t,               XmpVectorNotUsed,        2,        0,      0,     0)
	CREATE_FIELD(uint8_t,   XmpEnableSecondVectorLogical,        3,        0,      0,     0)
	CREATE_FIELD(uint8_t,             XmpExchangeAddress,        3,       16,     23,     0)
	CREATE_FIELD(uint8_t,                XmpVectorLength,        3,       24,     30,     0)
	CREATE_FIELD(uint8_t,      XmpEnhancedAddressingMode,        4,        0,      0,     0)
	CREATE_FIELD(uint8_t,                XmpProgramState,        4,       35,     35,     0)
	CREATE_FIELD(uint8_t,               XmpClusterNumber,        4,       37,     39,     0)
	CREATE_PROT_FIELD(                   XmpInstBaseAddr,        1)
	CREATE_PROT_FIELD(                  XmpInstLimitAddr,        2)
	CREATE_PROT_FIELD(                   XmpDataBaseAddr,        4)
	CREATE_PROT_FIELD(                  XmpDataLimitAddr,        5)
	//******************************************** YMP FIELDS ****************************************
	CREATE_FIELD(uint8_t,             YmpProcessorNumber,        0,        0,      7,     0)
	CREATE_FIELD(CProgramAddr_t,       YmpProgramAddress,        0,        8,     31,     0)
	CREATE_FIELD(uint8_t,                    YmpSyndrome,        1,        0,      7,     0)
	CREATE_FIELD(CAddr_t,                 YmpReadAddress,        2,        0,      7,     0)
	CREATE_FIELD(uint8_t,             YmpExchangeAddress,        5,        8,     15,     0)
	CREATE_FIELD(uint8_t,                YmpVectorLength,        5,       16,     22,     0)
//	Here the number of bits is actually series dependent. YEL for example has 4 bits, J90 has 6. Leaving as 8 for now as it doesn't seem to cause trouble.
	CREATE_FIELD(uint8_t,               YmpClusterNumber,        5,       24,     31,     0)
	CREATE_FIELD(uint8_t,               YmpVectorNotUsed,        6,        0,      0,     0)
	CREATE_FIELD(uint8_t,               YmpProcessorType,        7,        0,      1,     0)
	CREATE_PROT_FIELD(                   YmpInstBaseAddr,        1)
	CREATE_PROT_FIELD(                  YmpInstLimitAddr,        2)
	CREATE_PROT_FIELD(                   YmpDataBaseAddr,        3)
	CREATE_PROT_FIELD(                  YmpDataLimitAddr,        4)

	#undef CREATE_FIELD
	#undef CREATE_PROT_FIELD

	#ifdef CREATE_UNIFIED_FIELD
	#error CREATE_UNIFIED_FIELD is already defined!
	#endif
	#define CREATE_UNIFIED_FIELD(aaType, aaFieldName)                                                                                                   \
		aaType Get##aaFieldName() const { if (IsXmp(mMachineType)) return GetXmp##aaFieldName(); else return GetYmp##aaFieldName(); }                   \
		void Set##aaFieldName(aaType aValue) { if (IsXmp(mMachineType)) SetXmp##aaFieldName(aValue); else SetYmp##aaFieldName(aValue); }
	CREATE_UNIFIED_FIELD(uint8_t, ProcessorNumber)
	CREATE_UNIFIED_FIELD(uint8_t, Syndrome)
	CREATE_UNIFIED_FIELD(CProgramAddr_t, ProgramAddress)
	CREATE_UNIFIED_FIELD(uint8_t, ExchangeAddress)
	CREATE_UNIFIED_FIELD(uint8_t, VectorLength)
	CREATE_UNIFIED_FIELD(uint8_t, ClusterNumber)
	CREATE_UNIFIED_FIELD(uint8_t, VectorNotUsed)
	CREATE_UNIFIED_FIELD(CAddr_t, InstBaseAddr)
	CREATE_UNIFIED_FIELD(CAddr_t, InstLimitAddr)
	CREATE_UNIFIED_FIELD(CAddr_t, DataBaseAddr)
	CREATE_UNIFIED_FIELD(CAddr_t, DataLimitAddr)
	#undef CREATE_UNIFIED_FIELD

	Mode_c GetMode() const {
		if (IsXmp(mMachineType)) {
			return Mode_c((uint16_t)(((uint16_t)GetBits(Data[2], 63 - 35, 63 - 39) << 5) | ((uint16_t)GetBits(Data[1], 63 - 35, 63 - 39))), mMachineType);
		} else {
			return Mode_c((uint16_t)(GetBits(Data[6], 32, 43) | (GetBits(Data[6], 62, 62) << 12)), mMachineType);
		}
	}
	void SetMode(Mode_c aMode) {
		uint16_t RawMode = aMode.GetRawValue();
		if (IsXmp(mMachineType)) {
			Data[2] = SetBits(Data[2], 63 - 35, 63 - 39, RawMode >> 5);
			Data[1] = SetBits(Data[1], 63 - 35, 63 - 39, RawMode);
		}
		else {
			Data[6] = SetBits(Data[6], 32, 43, RawMode & 0x0fff);
			Data[6] = SetBits(Data[6], 62, 62, (RawMode & 0x1000) >> 12);
		}
	}

	Flags_c GetFlag() const {
		if (IsXmp(mMachineType)) {
			return Flags_c((uint16_t)(((uint16_t)GetBits(Data[3], 63 - 14, 63 - 15) << 9) | ((uint16_t)GetBits(Data[3], 63 - 31, 63 - 39))));
		} else {
			return Flags_c(GetBits(Data[6], 44, 55));
		}
	}
	void ClearFlags() { SetFlag(Flags_c()); }
	void SetFlag(Flags_c aFlag) {
		if (IsXmp(mMachineType)) {
			uint16_t RawFlag = aFlag.Get();
			Data[3] = SetBits(Data[3], 63 - 14, 63 - 15, RawFlag >> 9);
			Data[3] = SetBits(Data[3], 63 - 31, 63 - 39, RawFlag);
		}
		else {
			Data[6] = SetBits(Data[6], 44, 55, aFlag.Get());
		}
	}

	uint8_t GetEnableSecondVectorLogical() const {
		if (IsXmp(mMachineType)) {
			return GetXmpEnableSecondVectorLogical();
		} else {
			return GetMode().IsYmpEnableSecondVectorLogical() ? 1 : 0;
		}
	}
	void SetEnableSecondVectorLogical(uint8_t aValue) {
		if (IsXmp(mMachineType)) {
			SetXmpEnableSecondVectorLogical(aValue);
		} else {
			Mode_c Mode = GetMode();
			Mode.SetYmpEnableSecondVectorLogical(aValue != 0);
			SetMode(Mode);
		}
	}

	CAddr_t GetA(size_t aIdx) const { return (CAddr_t)GetBits(Data[aIdx], std::numeric_limits<CAddr_t>::digits - 1, 0); }
	CAddr_t GetA0() const { return GetA(0); }
	CAddr_t GetA1() const { return GetA(1); }
	CAddr_t GetA2() const { return GetA(2); }
	CAddr_t GetA3() const { return GetA(3); }
	CAddr_t GetA4() const { return GetA(4); }
	CAddr_t GetA5() const { return GetA(5); }
	CAddr_t GetA6() const { return GetA(6); }
	CAddr_t GetA7() const { return GetA(7); }
	void SetA(size_t aIdx, CAddr_t aAddr) {
		if (IsXmp(mMachineType)) {
			Data[aIdx] = SetBits(Data[aIdx], std::numeric_limits<CXmpAddr_t>::digits - 1, 0, aAddr);
		} else {
			Data[aIdx] = SetBits(Data[aIdx], std::numeric_limits<CAddr_t>::digits - 1, 0, aAddr);
		}
	}
	void SetA0(CAddr_t aAddr) { SetA(0,aAddr); }
	void SetA1(CAddr_t aAddr) { SetA(1,aAddr); }
	void SetA2(CAddr_t aAddr) { SetA(2,aAddr); }
	void SetA3(CAddr_t aAddr) { SetA(3,aAddr); }
	void SetA4(CAddr_t aAddr) { SetA(4,aAddr); }
	void SetA5(CAddr_t aAddr) { SetA(5,aAddr); }
	void SetA6(CAddr_t aAddr) { SetA(6,aAddr); }
	void SetA7(CAddr_t aAddr) { SetA(7,aAddr); }

	uint64_t GetS(size_t aIdx) const { return Data[8+aIdx]; }
	uint64_t GetS0() const { return GetS(0); }
	uint64_t GetS1() const { return GetS(1); }
	uint64_t GetS2() const { return GetS(2); }
	uint64_t GetS3() const { return GetS(3); }
	uint64_t GetS4() const { return GetS(4); }
	uint64_t GetS5() const { return GetS(5); }
	uint64_t GetS6() const { return GetS(6); }
	uint64_t GetS7() const { return GetS(7); }
	void SetS(size_t aIdx, uint64_t aS) { Data[8+aIdx] = aS; }
	void SetS0(uint64_t aS) { SetS(0,aS); }
	void SetS1(uint64_t aS) { SetS(1,aS); }
	void SetS2(uint64_t aS) { SetS(2,aS); }
	void SetS3(uint64_t aS) { SetS(3,aS); }
	void SetS4(uint64_t aS) { SetS(4,aS); }
	void SetS5(uint64_t aS) { SetS(5,aS); }
	void SetS6(uint64_t aS) { SetS(6,aS); }
	void SetS7(uint64_t aS) { SetS(7,aS); }
	friend std::ostream & operator << (std::ostream &aStream, const ExchangePacket_c &aEP);
protected:
	uint64_t Data[mDataSize];
	MachineTypes_e mMachineType;
};

inline std::ostream & operator << (std::ostream &aStream, const ExchangePacket_c &aEP) {
	for(size_t Idx=0;Idx<sizeof(aEP.Data)/sizeof(aEP.Data[0]);++Idx) {
		aStream << "Raw packet " << Idx << ": " << HexPrinter(aEP.Data[Idx]) << std::endl;
	}
	aStream << "PN:" << DecPrinter(aEP.GetProcessorNumber(),0) << std::endl;
	if (IsXmp(aEP.mMachineType)) {
		aStream << "E:" << aEP.GetXmpErrorType() << std::endl;
	}
	aStream << "S:" << HexPrinter(aEP.GetSyndrome()) << std::endl;
	if (IsXmp(aEP.mMachineType)) {
		aStream << "R:" << DecPrinter(aEP.GetXmpReadMode(), 0) << std::endl;
	}
	aStream << "CSB:" << Addr(aEP.GetXmpReadAddress()) << std::endl;
	aStream << "M:" << aEP.GetMode() << std::endl;
	aStream << "VNU:" << DecPrinter(aEP.GetVectorNotUsed(),0) << std::endl;
	if (IsXmp(aEP.mMachineType)) {
		aStream << "ESVL:" << DecPrinter(aEP.GetEnableSecondVectorLogical(), 0) << std::endl;
		aStream << "EAM:" << DecPrinter(aEP.GetXmpEnhancedAddressingMode(), 0) << std::endl;
	}
	aStream << "F:" << aEP.GetFlag() << std::endl;
	aStream << "XA:" << HexPrinter(aEP.GetExchangeAddress()) << std::endl;
	aStream << "VL:" << DecPrinter(aEP.GetVectorLength(),0) << std::endl;
	if (IsXmp(aEP.mMachineType)) {
		aStream << "PS:" << DecPrinter(aEP.GetXmpProgramState(),0) << std::endl;
	}
	aStream << "CLN:" << DecPrinter(aEP.GetClusterNumber(),0) << std::endl;

	aStream << "P:" << InstAddr(aEP.GetProgramAddress()) << std::endl;
	aStream << "IBA:" << Addr(aEP.GetInstBaseAddr()) << std::endl;
	aStream << "ILA:" << Addr(aEP.GetInstLimitAddr()) << std::endl;
	aStream << "DBA:" << Addr(aEP.GetDataBaseAddr()) << std::endl;
	aStream << "DLA:" << Addr(aEP.GetDataLimitAddr()) << std::endl;
	for(size_t i=0; i<8; ++i) aStream << "A" << i << ":" << HexPrinter(aEP.GetA(i)) << std::endl;
	for(size_t i=0; i<8; ++i) aStream << "S" << i << ":" << HexPrinter(aEP.GetS(i)) << std::endl;
	return aStream;
}

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif

#endif // __EXCHANGE_PACKET_H__
