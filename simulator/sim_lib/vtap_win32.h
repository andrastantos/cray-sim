#ifndef __VTAP_WIN32_H__
#define __VTAP_WIN32_H__

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX
#include <windows.h>
#include <vector>
#include <boost/optional.hpp>
#include <memory>

class TapAdapter_c {
public:
	TapAdapter_c();
	~TapAdapter_c();
	TapAdapter_c(const TapAdapter_c &) = delete;
	TapAdapter_c& operator=(const TapAdapter_c&) = delete;
	TapAdapter_c(TapAdapter_c &&aAdapter);
	void Close();
	void Open(const char *aIpAddr = nullptr, const char *aNetMask = nullptr, const char *aDhcpServerIpAddr = nullptr, size_t aLeaseTime = 10000);
	boost::optional<std::vector<uint8_t>> Receive();
	std::vector<uint8_t> BlockingReceive();
	void Send(std::vector<uint8_t> aPacket);
	std::string GetName() const { return mName; }
	const char *GetDeviceGuid() const { return mDeviceGuid; }
	void CancelRead();
protected:
	char mName[40];
	char mDeviceGuid[255]; // Way overkill for size, but does it matter?
	HANDLE mDeviceHandle;
	std::unique_ptr<OVERLAPPED> mReadOverlapped;
	std::unique_ptr<OVERLAPPED> mWriteOverlapped;
	bool mReadPending;
	std::vector<uint8_t> mPacketBuffer;

	friend std::vector<TapAdapter_c> EnumTaps();
};

std::vector<TapAdapter_c> EnumTaps();

#endif // __VTAP_WIN32_H__