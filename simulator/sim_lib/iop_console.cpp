#include "iop_console.h"
#include "cray_mainframe.h"
#include <algorithm>
#include <time.h>

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

// IopConsole_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IopConsole_c::GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const {
	aStatus.put("State", mAccepted ? "Conn" : "----");
}


// ChannelTO_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IopInt_t IopConsole_c::ChannelTO_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	switch (aFunction) {
		case 000:
			mBusy = false;
			mDone = false;
		return 0;
		case 006:
			mInterruptEnabled = false;
		return 0;
		case 007:
			mInterruptEnabled = true;
		return 0;
		case 014:
			if (mBusy) {
				mParent.mLogger << setloglevel(LogLevel_Warning) << SideEffectIndent << "WARNING: channel TOx fn14 is called while busy" << std::endl;
			} else {
				char Ch = aData & 0x7f;
				switch (mEscSequence.size()) {
					case 0:
						switch (Ch) {
							case 0x1b:
								mEscSequence += Ch;
								SetDone();
								break;
							case 0x0c: //   Right arrow:            0x0c
								Send("\x1b[1C");
								break;
							case 0x7f: // back-space these days generate a 0x7f ^H ' ' ^H sequence. Let's eat the first 0x7f
								SetDone();
								break;
							default:
								Send(Ch);
						}
					break;
					case 1:
						switch (Ch) {
							case 'T': mEscSequence.clear(); Send("\x1b[0K"); break;  //   Erase to end of line
							case '*': mEscSequence.clear(); Send("\x1b[2J"); break;  //   Clear screen
							case 'R': mEscSequence.clear(); Send("\x1b[1M"); break;  //   Delete a line and scroll up
							case 'j': mEscSequence.clear(); Send("\x1b[7m"); break;  //   Reverse video
							case 'k': mEscSequence.clear(); Send("\x1b[27m"); break; //   Reverse video off
							case 'n': mEscSequence.clear(); Send("\x1b[5m"); break;	 //   Flashing mode on
							case 'o': mEscSequence.clear(); Send("\x1b[25m"); break; //   Flashing mode off
							case '=':                                                //   Load cursor address
							case 'G':                                                //   Graphical characters
								mEscSequence += Ch;
								SetDone();
							break;
							default:
								mEscSequence += Ch;
								Send(mEscSequence);
								mEscSequence.clear();
							break;
						}
					break;
					case 2:
						switch (mEscSequence[1]) {
							case 'G':
								switch (Ch) {
									case 'A': mEscSequence.clear(); Send("\x1b(0\x6c\x1b(B"); break; //   Top left corner:        ESC G A
									case 'B': mEscSequence.clear(); Send("\x1b(0\x6b\x1b(B"); break; //   Top right corner:       ESC G B
									case 'C': mEscSequence.clear(); Send("\x1b(0\x6d\x1b(B"); break; //   Bottom left corner:     ESC G C
									case 'D': mEscSequence.clear(); Send("\x1b(0\x6a\x1b(B"); break; //   Bottom right corner:    ESC G D
									case 'E': mEscSequence.clear(); Send("\x1b(0\x77\x1b(B"); break; //   Top intersect:          ESC G E
									case 'F': mEscSequence.clear(); Send("\x1b(0\x75\x1b(B"); break; //   Right intersect:        ESC G F
									case 'G': mEscSequence.clear(); Send("\x1b(0\x74\x1b(B"); break; //   Left intersect:         ESC G G
									case 'H': mEscSequence.clear(); Send("\x1b(0\x76\x1b(B"); break; //   Bottom intersect:       ESC G H
									case 'I': mEscSequence.clear(); Send("\x1b(0\x71\x1b(B"); break; //   Horizontal line:        ESC G I
									case 'J': mEscSequence.clear(); Send("\x1b(0\x78\x1b(B"); break; //   Vertical line:          ESC G J
									case 'K': mEscSequence.clear(); Send("\x1b(0\x6e\x1b(B"); break; //   Crossed lines:          ESC G K
									default:
										mEscSequence += Ch;
										Send(mEscSequence);
										mEscSequence.clear();
									break;
								}
								break;
							case '=':
								mEscSequence += Ch;
								SetDone();
								break;
							default:
								mEscSequence += Ch;
								Send(mEscSequence);
								mEscSequence.clear();
								break;
						}
					break;
					case 3: {
						mEscSequence += Ch;
						int Row = mEscSequence[2] - ' ' + 1;
						int Col = mEscSequence[3] - ' ' + 1;
						std::stringstream Str;
						Str << "\x1b[" << Row << ";" << Col << "H";
						Send(Str.str());
						mEscSequence.clear();
					break;
					}
					break;
					default:
						mEscSequence += Ch;
						Send(mEscSequence);
						mEscSequence.clear();
					break;
				}
			}
		return 0;
		default:
			mParent.mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

void IopConsole_c::ChannelTO_c::Send(char aCh) {
	mBusy = true;
	mDone = false;
	mParent.SendChar(aCh);
}

void IopConsole_c::ChannelTO_c::Send(std::string aStr) {
	mBusy = true;
	mDone = false;
	mParent.SendString(aStr);
}

void IopConsole_c::ChannelTO_c::SetDone() {
//	CRAY_ASSERT(mBusy);
//	CRAY_ASSERT(!mDone);
	mBusy = false;
	mDone = true;
}


// ChannelTI_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IopInt_t IopConsole_c::ChannelTI_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
	char Data = 0;
	switch (aFunction) {
		case 000:
			mDone = false;
		return 0;
		case 006:
			mInterruptEnabled = false;
		return 0;
		case 007:
			mInterruptEnabled = true;
		return 0;
		case 010:
			if (!mDone) {
				mParent.mLogger << setloglevel(LogLevel_Warning) << SideEffectIndent << "WARNING: channel TIx fn10 is called while not done" << std::endl;
			} else {
				Data = mParent.GetChar();
				mDone = false;
			}
		return Data & 0x7f;
		default:
			mParent.mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

void IopConsole_c::ChannelTI_c::Tick() {
	mParent.Tick();
}

// IopConsole_c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IopConsole_c::Tick() {
	Console_c::Tick();
}

bool IopConsole_c::ReceiveHandlerDetail() {
	mTI.SetDone();
	return false; // Don't loop for input
}

void IopConsole_c::SendHandlerDetail() {
	mTO.SetDone();
}

void IopConsole_c::ResetDetail() {
}

void IopConsole_c::ConnectDetail() {
//	if (mTitle.length() > 0) {
//		SendStringInternal(std::string("\x1b]0;"), false, false);
//		SendStringInternal(mTitle, false, false);
//		SendStringInternal(std::string("\x0b"), false, false);
//	}
}


#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif
