#ifndef __IOP_BMX_H__
#define __IOP_BMX_H__

#include "cray_iop.h"
#include <fstream>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/variant.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/timer/timer.hpp>
#include "cray_channels.h"
#ifndef _WIN32
#include <unistd.h>
#endif //_WIN32
#include "config_file.h"
#include "poked_tap_file.h"
#include "ui.h"

class IopBmxDevice_i {
public:
	virtual ~IopBmxDevice_i() {}
	virtual bool InterfaceDisconnect() = 0;
	virtual void Reset() = 0;
	virtual uint8_t TestIo(uint8_t aCommand) = 0;
	virtual uint8_t Write(uint8_t aCommand, std::vector<uint8_t> &aData, size_t &aBytesTransferred) = 0;
	virtual uint8_t Control(uint8_t aCommand, std::vector<uint8_t> &aData, size_t &aBytesTransferred) = 0;
	virtual uint8_t Read(uint8_t aCommand, std::vector<uint8_t> &aData, size_t aMaxSize) = 0;
	virtual uint8_t Sense(uint8_t aCommand, std::vector<uint8_t> &aData, size_t aMaxSize) = 0;
	virtual uint8_t ReadBackward(uint8_t aCommand, std::vector<uint8_t> &aData, size_t aMaxSize) = 0;
	virtual bool NeedsTick() = 0;
	virtual void Tick() = 0;
	virtual uint8_t GetDeviceAddress() const = 0;
	virtual uint8_t GetStatus() const = 0;
	virtual bool GetRequestIn() const = 0;
	virtual bool GetStatusIn() const = 0;
	virtual bool GetDisconnectIn() const = 0;
	// BMX devices are required to call the following functions on the BMX Channel:
	// RequestInChanged    - when request changes value
	// StatusInChanged     - when status changes value
	// DisconnectInChanged - when disconnect changes value
	virtual PeripheralType_e GetType() const = 0;
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const = 0;
	virtual std::string GetName() const = 0;
	virtual void RegisterCommands(CommandHooks_t &aHooks) = 0;
};

// Overall setup
// ========================
//
// BMX channel ----+------ Control Unit 0 -----+----- I/O device 0
//                 |                           |
//                 |                           +----- I/O device 1
//                 |
//                 |                          ...
//                 |                           +----- I/O device n
//                 |
//                 |
//                 +------ Control Unit 1 -----+----- I/O device 0
//                 |                           |
//                 |                           +----- I/O device 1
//                 |
//                 |                          ...
//                 |                           +----- I/O device m
//                 |
//                 |
//                 |
//                 +------ Control Unit 2 -----+----- I/O device 0
//                 |                           |
//                 |                           +----- I/O device 1
//                 |
//                 |                          ...
//                 |                           +----- I/O device o
//                 |
//                 |
//                 |
//                ...
//                 |
//                 |
//                 +------ Control Unit 7 -----+----- I/O device 0
//                                             |
//                                             +----- I/O device 1
//
//                                            ...
//                                             +----- I/O device p
//
// That is: up to 8 control units, each connecting to a number of I/O devices, up to a total of 256 devices.
//
// Interface signals
//=========================
// Basic handshake signals
//-------------------------
// OperationalOut:    must be '1' for active operation. If drops to '0', the active operation is reset
// OperationalIn:     output from the *active* control unit, OR-ed on the bus. Only active if SelectOut and HoldOut are '1', plus the unit has something to do. It blocks
//                    propagation of SelectOut to the next daisy-chained control unit.
// RequestIn:         Wire-OR-ed service request signal from all channel controllers
// SelectOut:         Selects the highest priority channel controller that has something to do (?). This signal is daisy-chained along all control units and finally comes
//                    back as SelectIn
// SelectIn:          See above. If SelectOut goes '1' and SelectIn goes '1', it means no channel controller responded to SelectOut.
// HoldOut:           Works in conjunction with SelectOut: rises and falls at the same time at the BMX channel, but is not daisy-chained as SelectOut is. With that,
//                    allows faster de-select propagation down the chain.
//
// Bus signals
//--------------------------
// BusOut:            8bits+Parity
// BusIn:             8bits+Parity
// AddressOut:        Signals that BusOut contains an address. All control units decode the address and the one with a matching I/O device address raises OperationalIn
//                    If no device decoded the address, eventially SelectIn raises which signals a non-existent device address
// AddressIn:         Signals that BusIn contains an address. It stays like that until the BMX channel responds with CommandOut.
// CommandOut:        Acks an AddressIn, StatusIn or ServiceIn signal.
//                    For reads, it means: proceed. That is from the 
// StatusIn: +
// ServiceOut: +
// ServiceIn: +

class IopChannelBmx_c: public IopSimplePeripheral_i {
public:
	explicit IopChannelBmx_c(const Configuration_c &aConfig, IopCpu_c &aParent): mParent(aParent), mLogger(aConfig, "BMX",aConfig.get<size_t>("ChannelIdx")), mChannelIdx(aConfig.get<size_t>("ChannelIdx")) {
		mLogger.SetParent(mParent.GetLogger());
		mName = ModuleNameFormatter("BMX", mChannelIdx, 8);
		try {
			MasterClear(); 
			Setup(aConfig);
		} catch(...) {
			mDevices.clear();
			throw;
		}
	}
	~IopChannelBmx_c() {}
	IopCpu_c::Dma_c GetDma() { return mParent.GetDma(); }

	virtual IopInt_t DoIo(IopIoFunction_t aFunction, IopInt_t aData) override;
	virtual IopBit_t GetBusy() override { return mBusy; }
	virtual IopBit_t GetDone() override { return mDone; }
	virtual IopBit_t GetInterrupt() override;
	virtual void MasterClear() override {
		mDone = false;
		mBusy = false;
		mInterruptEnabled = true;
		mActiveDevice = -1;
		mLocalMemoryAdddress[0] = 0;
		mLocalMemoryAdddress[1] = 0;
		mChainingInterruptActive = false;
		mActiveMemoryAddressIdx = 0;
		mByteCounter = 0;
		mNextByteCounter = 0;
		mLoadByteCounter = false;
		mDeviceStatus = 0;
		mSkipFlag = false;
		mStackStatusFlag = false;
		mChainingMode = ChainingMode_NoChaining;
		mInterruptMode = InterruptMode_Disabled;
		mDelayCounter = 0;
		mByteCountStatusReg = 0;
		mByteCountStatusRegValid = false;
		mStatusReg = 0;
		mStatusRegValid = false;
		UpdateXxxInCnts();
	}
	virtual size_t GetChannelIdx() const override { return mChannelIdx; }
	virtual void Tick() override;
	virtual bool NeedsTick() override { return true; }
	CLogger_c &GetLogger() { return mLogger; }
	virtual void Dump(size_t aIdent=0) override {}
	virtual std::string GetName() const override;
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Bmx; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override;
	virtual void RegisterCommands(CommandHooks_t &aHooks) override;

	void RequestInChanged(uint8_t aDeviceAddr, bool aValue);
	void StatusInChanged(uint8_t aDeviceAddr, bool aValue);
	void DisconnectInChanged(uint8_t aDeviceAddr, bool aValue);
protected:
	void UpdateXxxInCnts();
	bool DeviceValid(int aDevice) const { return aDevice >= 0 && aDevice < (int)mDevices.size() /*&& mDevices[aDevice].Device != nullptr*/; }
	bool DeviceValid() const { return DeviceValid(mActiveDevice); }
	IopBmxDevice_i *GetDevice(int aDevice) {
		if (!DeviceValid(aDevice)) {
			std::cout << "Invalid BMX device: " << DecPrinter(aDevice) << std::endl;
			mLogger << setloglevel(LogLevel_Error) << "Invalid BMX device: " << DecPrinter(aDevice) << std::endl;
		}
		//CRAY_ASSERT(DeviceValid(aDevice));
		return mDevices[aDevice].Device.get();
	}
	IopBmxDevice_i *GetDevice(size_t aDevice) { return GetDevice(int(aDevice)); }
	IopBmxDevice_i *GetActiveDevice() { return GetDevice(mActiveDevice); }
	IopBit_t GetDeviceInterrupt();

	void Setup(const Configuration_c &aConfig);

	IopCpu_c &mParent;

	bool mDone;
	bool mBusy;
	bool mInterruptEnabled;
	bool mDeviceInterruptEnabled;
	IopInt_t mLocalMemoryAdddress[2];
	IopInt_t mByteCountStatusReg;
	bool mByteCountStatusRegValid;
	IopInt_t mStatusReg;
	bool mStatusRegValid;
	bool mChainingInterruptActive;

	struct Device_s {
		std::shared_ptr<IopBmxDevice_i> Device;
	};
	std::vector<Device_s> mDevices;
	std::vector<IopBmxDevice_i *> mTickedDevices;
	int mRequestInCnt;
	int mStatusInCnt;
	int mDisconnectInCnt;
	int mActiveMemoryAddressIdx;
	IopInt_t mByteCounter;
	IopInt_t mNextByteCounter;
	bool mLoadByteCounter;
	int mActiveDevice;
	uint8_t mDeviceStatus;
	size_t mChannelIdx;
	CLogger_c mLogger;
	bool mSkipFlag;
	bool mStackStatusFlag;
	enum ChainingMode_e {
		ChainingMode_NoChaining = 0,
		ChainingMode_ChainIfChannelEnd = 1,
		ChainingMode_ChainIfDeviceEnd = 2,
		ChainingMode_Chain = 3
	};
	ChainingMode_e mChainingMode;
	enum InterruptMode_e {
		InterruptMode_Disabled = 0,
		InterruptMode_RequestIn = 1,
		InterruptMode_StatusIn = 2,
		InterruptMode_DisconnectIn = 3
	};
	size_t mDelayCounter;
	size_t mDelayLimit;
	InterruptMode_e mInterruptMode;
	std::string mName;
};


class IopBmxTape_c: public IopBmxDevice_i {
public:
	IopBmxTape_c(const Configuration_c &aConfig, IopChannelBmx_c &aParent);
	virtual ~IopBmxTape_c() override {}
	virtual bool InterfaceDisconnect() override { mLogger << setloglevel(LogLevel_IoActivity) << "InterfaceDisconnect called" << std::endl; return true; }
	virtual void Reset() override { mLogger << setloglevel(LogLevel_IoActivity) << "Reset called" << std::endl; ClearState(); }
	virtual bool NeedsTick() override { return false; }
	virtual void Tick() override {}
	virtual uint8_t TestIo(uint8_t aCommand) override;
	virtual uint8_t Write(uint8_t aCommand, std::vector<uint8_t> &aData, size_t &aBytesTransferred) override;
	virtual uint8_t Control(uint8_t aCommand, std::vector<uint8_t> &aData, size_t &aBytesTransferred) override;
	virtual uint8_t Read(uint8_t aCommand, std::vector<uint8_t> &aData, size_t aMaxSize) override;
	virtual uint8_t Sense(uint8_t aCommand, std::vector<uint8_t> &aData, size_t aMaxSize) override;
	virtual uint8_t ReadBackward(uint8_t aCommand, std::vector<uint8_t> &aData, size_t aMaxSize) override;
	virtual uint8_t GetDeviceAddress() const override { return mDeviceAddress; }
	virtual uint8_t GetStatus() const override;
	virtual bool GetRequestIn() const override;
	virtual bool GetStatusIn() const override;
	virtual bool GetDisconnectIn() const override;
	virtual PeripheralType_e GetType() const override { return PeripheralType_e::Tape; }
	virtual void GetStatus(StatusReport_c &aStatus, PeripheralType_e aFilter, bool aLongFormat) const override;
	virtual std::string GetName() const override;
	void MountTape(const std::string &aTapeFileName);
	virtual void RegisterCommands(CommandHooks_t &aHooks) override;
protected:
	void ClearState() {
		mCurrentCommand = 0; // This is Test I/O
	}
	IopChannelBmx_c &mParent;
	CLogger_c mLogger;
	uint8_t mDeviceAddress;
	uint8_t mCurrentCommand;

	size_t mReads;
	size_t mWrites;
	PokedTapFile_c mTapeImage;
	bool mIsReadOnly;
	std::string mName;
};


#endif // __IOP_BMX_H__
