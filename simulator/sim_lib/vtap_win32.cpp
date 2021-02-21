#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <memory>
#include "vtap_win32.h"

#include <config.h>


#include <winioctl.h>
#include <iphlpapi.h>
#include <sstream>

#include "utils.h"

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", off)
#endif

#ifdef _TAP_IOCTL
#error _TAP_IOCTL is already defined
#endif
#define _TAP_IOCTL(nr) CTL_CODE(FILE_DEVICE_UNKNOWN, nr, METHOD_BUFFERED, \
				FILE_ANY_ACCESS)

const DWORD TAP_IOCTL_GET_MAC = _TAP_IOCTL(1);
const DWORD TAP_IOCTL_GET_VERSION = _TAP_IOCTL(2);
const DWORD TAP_IOCTL_GET_MTU = _TAP_IOCTL(3);
const DWORD TAP_IOCTL_GET_INFO = _TAP_IOCTL(4);
const DWORD TAP_IOCTL_CONFIG_POINT_TO_POINT = _TAP_IOCTL(5);
const DWORD TAP_IOCTL_SET_MEDIA_STATUS = _TAP_IOCTL(6);
const DWORD TAP_IOCTL_CONFIG_DHCP_MASQ = _TAP_IOCTL(7);
const DWORD TAP_IOCTL_GET_LOG_LINE = _TAP_IOCTL(8);
const DWORD TAP_IOCTL_CONFIG_DHCP_SET_OPT = _TAP_IOCTL(9);
const DWORD TAP_IOCTL_CONFIG_TUN = _TAP_IOCTL(10);

#undef _TAP_IOCTL

const char *TAP_COMPONENT_ID1 = "tap0901";
const char *TAP_COMPONENT_ID2 = "tap0801";

/****************************************************************************
TAP_IOCTL_GET_MAC
---------------------------------
Input: none
Output: 6 bytes containing the MAC address of the adpater


TAP_IOCTL_GET_VERSION
---------------------------------
Input: none
Output:
byte 0 - Major version
byte 1 - Minor version
byte 2 - 0 for release, 1 for debug builds


TAP_IOCTL_GET_MTU
---------------------------------
Input: none
Output: returns the MTU size in a ULONG


TAP_IOCTL_GET_INFO
---------------------------------
Input: none
Output: returns a string of the following format:
State=%s Err=[%s/%d] #O=%d Tx=[%d,%d,%d] Rx=[%d,%d,%d] IrpQ=[%d,%d,%d] PktQ=[%d,%d,%d] InjQ=[%d,%d,%d] or
State=%s Err=[%s/%d] #O=%d Tx=[%d,%d] Rx=[%d,%d] IrpQ=[%d,%d,%d] PktQ=[%d,%d,%d] InjQ=[%d,%d,%d] depending on build flags
The state string contains four characters:
char 0: 'A' if adapter is ready for send and receive, 'a' otherwise
char 1: 'T' if adapter is ready for read and writem 't' otherwise
char 2: a numberial value of the adapters power state
char 3: 'C' if media is connected, 'c' otherwise
The number after #0 represents the open count.
TX and RX statistics are: all packets, failed packets and (if specified) trunacted packets
IrpQ and PktQ info contains current queue size, maximum queue size and allocated queue size
InjQ info contains the same info, expect queue size and max queue size is not maintained and returns constant 0


TAP_IOCTL_CONFIG_POINT_TO_POINT
---------------------------------
Input:
DWORD 0 - Adapter IP address
DWORD 1 - Remote network (default gateway)
Output: return 1 byte, but doesn't fill it in with anything

Sets the adapter to P2P mode (which is more or less the same as TUN, expect the network mask is set to 255.255.255.254 automatically)

TAP_IOCTL_SET_MEDIA_STATUS
---------------------------------
Input: ULONG value 0 - disconnected, 1 - connected
Output: return 1 byte, but doesn't fill it in with anything


TAP_IOCTL_CONFIG_DHCP_MASQ
---------------------------------
Input:
DWORD 0 - Adapter IP address
DWORD 1 - Netmask
DWORD 2 - DHCP server IP
DWORD 3 - Lease time
Output: return 1 byte, but doesn't fill it in with anything

Sets up the adapter to DHCP MASQ mode, which means the adapter will reply to DHCP queires (even ARP queires against the DHCP server IP specified).
The options served by the DHCP server is specified using the TAP_IOCTL_CONFIG_DHCP_SET_OPT call.
The adapter is not set in TUN mode.

TAP_IOCTL_GET_LOG_LINE
---------------------------------
Input: none
Output: returns log line from driver

This IOCTL is only implemented in debug builds


TAP_IOCTL_CONFIG_DHCP_SET_OPT
---------------------------------
Input: DHCP options. These options are directly copied to the DHCP lease reply message. See https://tools.ietf.org/html/rfc2132 for details.
Output: return 1 byte, but doesn't fill it in with anything

Sets DHCP options for adapter

TAP_IOCTL_CONFIG_TUN
---------------------------------
Input:
DWORD 0 - Adapter IP address
DWORD 1 - Remote network (default gateway)
DWORD 2 - Netmask
Output: return 1 byte, but doesn't fill it in with anything

Sets the adapter to TUN mode
****************************************************************************/

#ifdef NETDEV_GUID
#error NETDEV_GUID is already defined
#endif
#define NETDEV_GUID "{4D36E972-E325-11CE-BFC1-08002BE10318}"
#ifdef CONTROL_KEY
#error CONTROL_KEY is already defined
#endif
#define CONTROL_KEY "SYSTEM\\CurrentControlSet\\Control\\"

const char *ADAPTERS_KEY = CONTROL_KEY "Class\\" NETDEV_GUID;
const char *CONNECTIONS_KEY = CONTROL_KEY "Network\\" NETDEV_GUID;

#undef NETDEV_GUID
#undef CONTROL_KEY

const size_t cMaxPacketSize = 2048; // We don't support jumbo packets for now at least

TapAdapter_c::TapAdapter_c() :
	mDeviceHandle(INVALID_HANDLE_VALUE),
	mPacketBuffer(cMaxPacketSize),
	mReadPending(false)
{
	mName[0] = 0;
}

TapAdapter_c::~TapAdapter_c() {
	Close();
}

TapAdapter_c::TapAdapter_c(TapAdapter_c &&aAdapter) {
	CRAY_ASSERT((aAdapter.mReadOverlapped == nullptr) || (aAdapter.mReadOverlapped->Internal != STATUS_PENDING));
	CRAY_ASSERT((aAdapter.mWriteOverlapped == nullptr) || (aAdapter.mWriteOverlapped->Internal != STATUS_PENDING));
	mReadPending = aAdapter.mReadPending;
	memcpy(mName, aAdapter.mName, sizeof(mName));
	memcpy(mDeviceGuid, aAdapter.mDeviceGuid, sizeof(mDeviceGuid));
	mDeviceHandle = aAdapter.mDeviceHandle;
	mReadOverlapped = std::move(aAdapter.mReadOverlapped);
	mWriteOverlapped = std::move(aAdapter.mWriteOverlapped);
	aAdapter.mDeviceHandle = INVALID_HANDLE_VALUE;
	mPacketBuffer = std::move(aAdapter.mPacketBuffer);
}

void TapAdapter_c::CancelRead() {
	// It might be that all we need to do here is to signal the mReadOverlapped->hEvent signal...
	Close();
}

void TapAdapter_c::Close() {
	if (mReadPending) {
		if (!CancelIoEx(mDeviceHandle, mReadOverlapped.get())) {
			DWORD ErrorCode = GetLastError();
			if (ErrorCode != ERROR_NOT_FOUND) throw Generic_x() << "Can't cancel pending I/O operation with error code: " << HexPrinter(ErrorCode);
		}
		DWORD BytesTransferred;
		if (!GetOverlappedResult(mDeviceHandle, mReadOverlapped.get(), &BytesTransferred, TRUE)) {
			DWORD ErrorCode = GetLastError();
			if (ErrorCode != ERROR_OPERATION_ABORTED) throw Generic_x() << "Can't get overlapped results with error code: " << HexPrinter(ErrorCode);
		}
		mReadPending = false;
	}
	if (mDeviceHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(mDeviceHandle);
	}
	mDeviceHandle = INVALID_HANDLE_VALUE;
	if (mReadOverlapped != nullptr) {
		if (mReadOverlapped->hEvent != INVALID_HANDLE_VALUE) {
			CloseHandle(mReadOverlapped->hEvent);
		}
		mReadOverlapped->hEvent = INVALID_HANDLE_VALUE;
		mReadOverlapped = nullptr;
	}
	if (mWriteOverlapped != nullptr) {
		if (mWriteOverlapped->hEvent != INVALID_HANDLE_VALUE) {
			CloseHandle(mWriteOverlapped->hEvent);
		}
		mWriteOverlapped->hEvent = INVALID_HANDLE_VALUE;
		mWriteOverlapped = nullptr;
	}
	mReadPending = false;
}

static uint32_t InetAddr(const char* aAddr) {
	uint32_t BinAddr;
	INT RetVal = inet_pton(AF_INET, aAddr, &BinAddr);
	if (RetVal != 1) throw Generic_x() << "Failed to resolve address: " << aAddr << " with error code: " << HexPrinter(RetVal);
	return BinAddr;
}

void TapAdapter_c::Open(const char *aIpAddr, const char *aNetMask, const char *aDhcpServerIpAddr, size_t aLeaseTime) {
	std::stringstream DeviceName;
	DeviceName << "\\\\.\\Global\\" << mDeviceGuid << ".tap";

	Close();
	mDeviceHandle = CreateFileA(DeviceName.str().c_str(), GENERIC_WRITE | GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED, 0);
	if (mDeviceHandle == INVALID_HANDLE_VALUE) {
		throw Generic_x() << "Failed to open TAP device: " << DeviceName.str();
	}

	ULONG IoctlData[4];
	DWORD Length;
	if (!DeviceIoControl(mDeviceHandle, TAP_IOCTL_GET_VERSION, IoctlData, sizeof(IoctlData[0]), IoctlData, sizeof(IoctlData), &Length, nullptr)) {
		throw Generic_x() << "Failed to get TAP device version with error code: " << HexPrinter(GetLastError());
	}
	if (IoctlData[0] < 8) {
		throw Generic_x() << "TAP driver version " << DecPrinter(IoctlData[0]) << "." << DecPrinter(IoctlData[1]) << " is too low";
	}

	//		IoctlData[0] = inet_addr(aIpAddr);
	//		// Set network and mask both to 0.0.0.0. This has something to do with ARP response generation, that we might not need. Need to test...
	//		IoctlData[1] = aDefaultGateway == nullptr ? 0 : inet_addr(aDefaultGateway);
	//		IoctlData[2] = aNetMask == nullptr ? 0 : ~inet_addr(aNetMask);
	//
	//		if (!DeviceIoControl(mDeviceHandle, TAP_IOCTL_CONFIG_TUN, IoctlData, sizeof(IoctlData), IoctlData, sizeof(IoctlData), &Length, nullptr)) {
	//			throw Generic_x() << "Failed to set TAP device IP address with error code: " << HexPrinter(GetLastError());
	//		}

	if (aIpAddr != nullptr) {
		IoctlData[0] = InetAddr(aIpAddr);
		IoctlData[1] = aNetMask == nullptr ? 0 : ~InetAddr(aNetMask);
		IoctlData[2] = aDhcpServerIpAddr == nullptr ? 0 : InetAddr(aDhcpServerIpAddr);
		IoctlData[3] = ULONG(aLeaseTime);

		if (!DeviceIoControl(mDeviceHandle, TAP_IOCTL_CONFIG_DHCP_MASQ, IoctlData, sizeof(IoctlData[0]) * 4, IoctlData, sizeof(IoctlData), &Length, nullptr)) {
			throw Generic_x() << "Failed to set TAP DHCP address with error code: " << HexPrinter(GetLastError());
		}
	}

	IoctlData[0] = 1;
	if (!DeviceIoControl(mDeviceHandle, TAP_IOCTL_SET_MEDIA_STATUS, IoctlData, sizeof(IoctlData[0]), IoctlData, sizeof(IoctlData[0]), &Length, nullptr)) {
		throw Generic_x() << "Failed to set TAP media status with error code: " << HexPrinter(GetLastError());
	}
	mReadOverlapped = std::make_unique<OVERLAPPED>();
	mReadOverlapped->hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	mWriteOverlapped = std::make_unique<OVERLAPPED>();
	mWriteOverlapped->hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	mReadPending = false;
}

boost::optional<std::vector<uint8_t>> TapAdapter_c::Receive() {
	boost::optional<std::vector<uint8_t>> EmptyRetVal;
	DWORD BytesRead;
	if (!mReadPending) {
		if (!ReadFile(mDeviceHandle, &mPacketBuffer[0], DWORD(mPacketBuffer.size()), &BytesRead, mReadOverlapped.get())) {
			DWORD ErrorCode = GetLastError();

			switch (ErrorCode) {
			case ERROR_IO_PENDING:
				mReadPending = true;
				break;
			case ERROR_OPERATION_ABORTED:
				throw Generic_x() << "TAP device aborted operation";
			default:
				throw Generic_x() << "Can't read from device with error code: " << HexPrinter(ErrorCode);
			}
			return EmptyRetVal;
		}
	}
	else {
		if (!GetOverlappedResult(mDeviceHandle, mReadOverlapped.get(), &BytesRead, FALSE)) {
			DWORD ErrorCode = GetLastError();

			switch (ErrorCode) {
			case ERROR_IO_INCOMPLETE:
				mReadPending = true;
				break;
			default:
				throw Generic_x() << "Can'get overlapped results with error code: " << HexPrinter(ErrorCode);
			}
			return EmptyRetVal;
		}
	}
	// We get here if the operation succeeded
	std::vector<uint8_t> RetVal(BytesRead);
	memcpy(&RetVal[0], &mPacketBuffer[0], BytesRead);
	mReadPending = false;
	return RetVal;
}

std::vector<uint8_t> TapAdapter_c::BlockingReceive() {
	DWORD BytesRead;
	if (!mReadPending) {
		if (!ReadFile(mDeviceHandle, &mPacketBuffer[0], DWORD(mPacketBuffer.size()), &BytesRead, mReadOverlapped.get())) {
			DWORD ErrorCode = GetLastError();

			switch (ErrorCode) {
				case ERROR_IO_PENDING:
					mReadPending = true;
				break;
				case ERROR_OPERATION_ABORTED:
				return std::vector<uint8_t>();
				default:
					throw Generic_x() << "Can't read from device with error code: " << HexPrinter(ErrorCode);
			}
		}
	}
	if (!GetOverlappedResult(mDeviceHandle, mReadOverlapped.get(), &BytesRead, TRUE)) {
		DWORD ErrorCode = GetLastError();

		switch (ErrorCode) {
			case ERROR_OPERATION_ABORTED: // Can happen during termination when the handle got closed by another thread
				return std::vector<uint8_t>();
			default:
				throw Generic_x() << "Can'get overlapped results with error code: " << HexPrinter(ErrorCode);
		}
	}
	// We get here if the operation succeeded
	std::vector<uint8_t> RetVal(BytesRead);
	memcpy(&RetVal[0], &mPacketBuffer[0], BytesRead);
	mReadPending = false;
	return RetVal;
}

void TapAdapter_c::Send(std::vector<uint8_t> aPacket) {
	DWORD BytesSent = 0;

	if (WriteFile(mDeviceHandle, &aPacket[0], DWORD(aPacket.size()), &BytesSent, mWriteOverlapped.get())) return;

	DWORD ErrorCode = GetLastError();
	if (ErrorCode != ERROR_IO_PENDING) {
		throw Generic_x() << "Can't write to TAP device with error code: " << HexPrinter(ErrorCode);
	}
	if (!GetOverlappedResult(mDeviceHandle, mWriteOverlapped.get(), &BytesSent, TRUE)) {
		throw Generic_x() << "Can't get overlapped results with error code: " << HexPrinter(GetLastError());
	}
}


class HKey_c {
public:
	HKey_c() : mKey(HKEY(INVALID_HANDLE_VALUE)) {}
	~HKey_c() {
		Close();
	}
	bool IsValid() const {
		return mKey != INVALID_HANDLE_VALUE;
	}
	void Close() {
		if (IsValid()) RegCloseKey(mKey);
		mKey = HKEY(INVALID_HANDLE_VALUE);
	}
	HKey_c &operator=(HKEY aKey) { Close(); mKey = aKey; return *this; }
	operator HKEY() const { return mKey; }
	HKEY *Assign() { Close(); return &mKey; }
	HKEY mKey;
};

std::vector<TapAdapter_c> EnumTaps() {
	std::vector<TapAdapter_c> TapAdapters;
	HKey_c Adapters;
	LONG RetVal;

	// Open the adapters key
	RetVal = RegOpenKeyExA(HKEY_LOCAL_MACHINE, ADAPTERS_KEY, 0, KEY_READ, Adapters.Assign());
	if (RetVal != ERROR_SUCCESS) throw Generic_x() << "Error opening network adapters key in registry with error code: " << HexPrinter(RetVal);
	int KeyIdx = 0;
	while (true) {
		// Enumerate all entries
		char Entry[256];
		DWORD EntrySize = sizeof(Entry);
		RetVal = RegEnumKeyExA(Adapters, KeyIdx, Entry, &EntrySize, nullptr, nullptr, nullptr, nullptr);
		++KeyIdx;
		if (RetVal != ERROR_SUCCESS) {
			if (RetVal == ERROR_NO_MORE_ITEMS) break;
			throw Generic_x() << "Error enumerating network adapters in registry with error code: " << HexPrinter(RetVal);
		}
		// Open the corresponding adapter subkey and check if it's the type we're looking for (TAP adapter)
		std::stringstream KeyName;
		KeyName << ADAPTERS_KEY << "\\" << Entry;
		HKey_c Adapter;
		RetVal = RegOpenKeyExA(HKEY_LOCAL_MACHINE, KeyName.str().c_str(), 0, KEY_QUERY_VALUE, Adapter.Assign());
		if (RetVal != ERROR_SUCCESS) continue;

		EntrySize = sizeof(Entry);
		DWORD Type;
		RetVal = RegQueryValueExA(Adapter, "ComponentId", NULL, &Type, (unsigned char *)Entry, &EntrySize);
		if (
			(RetVal != ERROR_SUCCESS) ||
			(Type != REG_SZ) ||
			(strcmp(Entry, TAP_COMPONENT_ID1) != 0 && strcmp(Entry, TAP_COMPONENT_ID2) != 0)
			) {
			continue;
		}

		// Get the Device GUID and retrieve the associated human readable name
		char DeviceGuid[256];
		EntrySize = sizeof(DeviceGuid);
		RetVal = RegQueryValueExA(Adapter, "NetCfgInstanceId", NULL, &Type, (unsigned char *)DeviceGuid, &EntrySize);
		if (
			(RetVal != ERROR_SUCCESS) ||
			(Type != REG_SZ)
			) {
			continue;
		}

		std::stringstream KeyName2;
		HKey_c Connection;
		KeyName2 << CONNECTIONS_KEY << "\\" << DeviceGuid << "\\Connection";
		RetVal = RegOpenKeyExA(HKEY_LOCAL_MACHINE, KeyName2.str().c_str(), 0, KEY_QUERY_VALUE, Connection.Assign());
		if (RetVal != ERROR_SUCCESS) continue;

		char Name[40];
		DWORD NameSize = sizeof(Name);
		RetVal = RegQueryValueExA(Connection, "Name", NULL, &Type, (LPBYTE)Name, &NameSize);
		if (
			(RetVal != ERROR_SUCCESS) ||
			(Type != REG_SZ)
			) {
			continue;
		}

		// Create a TAP adapter entry in the return array and continue looking
		TapAdapter_c TapAdapter;
		memcpy(TapAdapter.mName, Name, NameSize);
		memcpy(TapAdapter.mDeviceGuid, DeviceGuid, sizeof(TapAdapter_c::mDeviceGuid));
		TapAdapters.push_back(std::move(TapAdapter));
	}
	return TapAdapters;
}

#if defined(PARTIAL_DEBUG) && defined(_MSC_VER)
#pragma optimize ("", on)
#endif
