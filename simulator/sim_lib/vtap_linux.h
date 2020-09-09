#ifndef __VTAP_LINUX_H__
#define __VTAP_LINUX_H__

#include <string>
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
	void Open();
	std::vector<uint8_t> BlockingReceive();
	void Send(std::vector<uint8_t> aPacket);
	const char *GetName() const { return mName.c_str(); }
	void SetName(const char *aName) { mName = aName; }
	int GetDevice() const { return mDevice; }
	void CancelRead();
protected:
	std::string mName;
	int mDevice;
	int mSignalPipeSend;
	int mSignalPipeRcv;

	friend std::vector<TapAdapter_c> EnumTaps();
};

#endif // __VTAP_LINUX_H__