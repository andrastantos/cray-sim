#include "iop_era.h"
#include "cray_mainframe.h"
#include <algorithm>
#include <time.h>

/////////////////////////////////////////////////////////
// ChannelERA_c
/////////////////////////////////////////////////////////


IopInt_t ChannelERA_c::DoIo(IopIoFunction_t aFunction, IopInt_t aData) {
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
		return 0;
		case 011:
		return 0;
		case 012:
		return 0;
		case 013:
		return 0;
		default:
			mLogger << setloglevel(LogLevel_Error) << SideEffectIndent << "ERROR: Invalid function code for channel" << std::endl;
		return 0;
	}
}

