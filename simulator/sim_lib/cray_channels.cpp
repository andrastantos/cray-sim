#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "cray_channels.h"
#include "cray_mainframe.h"

void Channel_i::Dump(size_t aIdent) const {
	GetLogger() << setloglevel(LogLevel_Dump) << Indent(aIdent) << "CA: " << HexPrinter(mAddress,6) << std::endl;
	GetLogger() << setloglevel(LogLevel_Dump) << Indent(aIdent) << "CL: " << HexPrinter(mLimit,6) << std::endl;
	GetLogger() << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Interrupt pending: " << (mInterruptPending ? "yes":"no") << std::endl;
	GetLogger() << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Error:             " << (mError ? "yes":"no") << std::endl;
	GetLogger() << setloglevel(LogLevel_Dump) << Indent(aIdent) << "Active:            " << (IsActive() ? "yes":"no") << std::endl;
}

void Channel_i::SetInterruptPending(bool aInterruptPending) {
	std::lock_guard<std::recursive_mutex> Guard(mInterruptLock);
	if (aInterruptPending && !mInterruptPending) { mMainframe.RouteChannelInterrupt(); }
	mInterruptPending = aInterruptPending;
}
	
CInt_t Channel_i::GetData() {
	CRAY_ASSERT(IsActive());
	CRAY_ASSERT(mAddress != mLimit);
	CInt_t RetVal = SwapBytes(mMainframe.MemReadNoWatchpoint<CInt_t>(mAddress));
	GetLogger() << setloglevel(LogLevel_IoTrace) << "get " << HexPrinter(mAddress) << " value " << HexPrinter(RetVal) << " - " << AsciiDumpPrinter(RetVal) << std::endl;
	++mAddress;
	if (mAddress == mLimit) {
		GetLogger() << setloglevel(LogLevel_IoTrace) << "Going inactive" << std::endl;
		SetInterrupt();
		SetActive(false);
	}
	return RetVal;
}
void Channel_i::SetData(CInt_t aData, bool aWithDisconnect) {
	CRAY_ASSERT(IsActive());
	CRAY_ASSERT(mAddress != mLimit);
	GetLogger() << setloglevel(LogLevel_IoTrace) << "set " << HexPrinter(mAddress) << " value " << HexPrinter(aData) << " - " << AsciiDumpPrinter(aData) << std::endl;
	mMainframe.MemWrite(mAddress, SwapBytes(aData));
	++mAddress;
	if (aWithDisconnect) Disconnect();
//	if (mAddress == mLimit) {
//		GetLogger() << setloglevel(LogLevel_IoTrace) << "Going inactive" << std::endl;
//		SetInterrupt();
//		SetActive(false);
//	}
}
void Channel_i::Disconnect() {
	GetLogger() << setloglevel(LogLevel_IoTrace) << "disconnecting" << std::endl;
	SetActive(false);
	SetInterrupt();
}
void Channel_i::SetError() {
	GetLogger() << setloglevel(LogLevel_IoTrace) << "in error" << std::endl;
	mError = true;
	SetInterrupt();
}



void LoggerChannel_c::HandleActivation() {
	if (mInput) {
		mLogger << setloglevel(LogLevel_IoActivity) << "primed to receive " << DecPrinter(BufferLeft()) << " words at address " << HexPrinter(GetAddress()) << std::endl;
	} else {
		mLogger << setloglevel(LogLevel_IoActivity) << "transmitting " << DecPrinter(BufferLeft()) << " words at address " << HexPrinter(GetAddress()) << std::endl;
		while (IsActive()) {
			GetData(); // This will do the logging
		}
	}
}





void FileInputChannel_c::ChannelTick() {
	if (IsActive()) {
		if (mFile.good()) {
			CInt_t Data = 0;
			mFile.read((char*)(&Data), sizeof(Data)); // Might not read a full QWORD, but that's fine. The expected operation is to zero-fill in that case
			SetData(SwapBytes(Data));
			if (mFile.eof()) Disconnect();
		}
	}
};

void FileOutputChannel_c::ChannelTick() {
	if (IsActive()) {
		std::ofstream File(mFileName, std::ios::app | std::ios::binary);
		if (File.good()) {
			CInt_t Data = SwapBytes(GetData());
			File.write((char*)(&Data),sizeof(Data));
		}
	}
};
