#include "vtap_linux.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include "utils.h"

const size_t cMaxPacketSize = 2048; // We don't support jumbo packets for now at least

const uint8_t cClosing = 1;

TapAdapter_c::TapAdapter_c() :
	mDevice(-1),
	mSignalPipeSend(-1),
	mSignalPipeRcv(-1)
{
	int Pipe[2];
	if (pipe(Pipe) != 0) throw Generic_x() << "Can't create pipe with error code: " << DecPrinter(errno);
	mSignalPipeSend = Pipe[1];
	mSignalPipeRcv = Pipe[0];
}

TapAdapter_c::~TapAdapter_c() {
	Close();
	if (mSignalPipeSend != -1) {
		if (close(mSignalPipeSend) == -1) std::cout << "TAP Can't close pipe with error code: " << DecPrinter(errno) << std::endl;
	}
	if (mSignalPipeRcv != -1) {
		if (close(mSignalPipeRcv) == -1) std::cout << "TAP Can't close pipe with error code: " << DecPrinter(errno) << std::endl;
	}
}

TapAdapter_c::TapAdapter_c(TapAdapter_c &&aAdapter) {
	mDevice = aAdapter.mDevice;
	mName = aAdapter.mName;
}

void TapAdapter_c::CancelRead() {
	int RetVal = write(mSignalPipeSend, &cClosing, sizeof(cClosing));
	if (RetVal < 0) {
		throw Generic_x() << "Error writing to TAP device with error code: " << DecPrinter(errno);
	} else if (RetVal != sizeof(cClosing)) {
		throw Generic_x() << "Incomplete packet write to TAP device";
	}
}


void TapAdapter_c::Close() {
	if (mDevice != -1) {
		CancelRead();
		if (close(mDevice) == -1) {
			throw Generic_x() << "Error closing TAP file handle with error code: " << DecPrinter(errno);
		}
		mDevice = -1;
	}
}

void TapAdapter_c::Open() {
	Close();

	// open the clone device
	const char *CloneDevName = "/dev/net/tun";
	int CloneDev = open(CloneDevName, O_RDWR);
	if (CloneDev < 0) throw Generic_x() << "Can't open " << CloneDevName << " with error code: " << DecPrinter(errno) << ". Maybe tun devices aren't enabled?";

	// Create request to create/open TAP device
	struct ifreq IfRequest;
	memset(&IfRequest, 0, sizeof(IfRequest));
	IfRequest.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_UP;
	if (mName.length() > 0) {
		if (mName.length() >= IFNAMSIZ) throw Generic_x() << "TAP device name " << mName << " is too long";
		strncpy(IfRequest.ifr_name, &mName[0], mName.length());
	}
	int RetVal = ioctl(CloneDev, TUNSETIFF, (void *) &IfRequest);
	if (RetVal < 0 ) {
		close(CloneDev);
		throw Generic_x() << "Error creating TAP device " << mName << " with error code: " << DecPrinter(errno);
	}

	if (mName.length() == 0) mName = IfRequest.ifr_name;
	
	mDevice = CloneDev;
//	std::cout << "Using file handle: " << DecPrinter(mDevice) << std::endl;
}

std::vector<uint8_t> TapAdapter_c::BlockingReceive() {
	fd_set DescSet;

	FD_ZERO(&DescSet);
	FD_SET(mSignalPipeRcv, &DescSet);
	FD_SET(mDevice, &DescSet);

	int RetVal = TEMP_FAILURE_RETRY(select(std::max(mSignalPipeRcv, mDevice)+1, &DescSet, nullptr, nullptr, nullptr));
	if (RetVal == -1) throw Generic_x() << "Error in select call with error code: " << DecPrinter(errno);
	if (!FD_ISSET(mDevice, &DescSet)) {
		return std::vector<uint8_t>(); // If operation is cancelled, return an empty array
	}
	
	std::vector<uint8_t> Buffer(cMaxPacketSize);
	int BytesRead =  TEMP_FAILURE_RETRY(read(mDevice,&Buffer[0],Buffer.size()));
	if (BytesRead < 0) {
		throw Generic_x() << "Error reading from TAP device with error code: " << DecPrinter(errno);
	}
	Buffer.resize(BytesRead);
	return Buffer;
}

void TapAdapter_c::Send(std::vector<uint8_t> aPacket) {
	if (mDevice == -1) {
		throw Generic_x() << "TAP Device isn't opened";
	}
	int RetVal = write(mDevice, &aPacket[0], aPacket.size());
	if (RetVal < 0) {
		throw Generic_x() << "Error writing to TAP device with error code: " << DecPrinter(errno);
	} else if (RetVal != int(aPacket.size())) {
		throw Generic_x() << "Incomplete packet write to TAP device";
	}
}

