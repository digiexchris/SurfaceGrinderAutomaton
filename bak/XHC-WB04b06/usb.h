#pragma once

// system includes
#include <ostream>
#include <stdint.h>

// 3rd party includes
#include "tusb.h"

// local library includes

// local includes

namespace XhcWhb04b6
{

	// forward declarations

	//! Axis coordinate structure as sent via usb.
	//! Caution: do not reorder fields!
	class UsbOutPackageAxisCoordinate
	{
	public:
		uint16_t integerValue;
		uint16_t fractionValue : 15;
		uint16_t coordinateSign : 1;
		void SetCoordinate(const float &coordinate);
		void Clear();
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	class DisplayIndicatorStepMode
	{
	public:
		//! \see DisplayIndicatorBitFields::stepMode
		enum class StepMode : uint8_t
		{
			CONTINUOUS = 0x00,
			STEP = 0x01,
			MANUAL_PULSE_GENERATOR = 0x02,
			PERCENT = 0x03
		};
	};

	// ----------------------------------------------------------------------

	//! Caution: do not reorder fields!
	class DisplayIndicatorBitFields
	{
	public:
		uint8_t stepMode : 2;
		uint8_t unknown : 4;
		uint8_t isReset : 1;
		uint8_t isRelativeCoordinate : 1;
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	//! Caution: do not reorder fields!
	union DisplayIndicator
	{
	public:
		uint8_t asByte;
		DisplayIndicatorBitFields asBitFields;
		DisplayIndicator();
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	//! Convenience structure for accessing data fields in output package stream.
	//! Caution: do not reorder fields!
	class UsbOutPackageData
	{
	public:
		uint16_t header;
		uint8_t seed;
		DisplayIndicator displayModeFlags;
		UsbOutPackageAxisCoordinate row1Coordinate;
		UsbOutPackageAxisCoordinate row2Coordinate;
		UsbOutPackageAxisCoordinate row3Coordinate;
		uint16_t feedRate;
		uint16_t spindleSpeed;
		UsbOutPackageData();
		void Clear();

	private:
		uint8_t padding;
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	//! Convenience structure for accessing data in input package stream.
	//! Caution: do not reorder fields!
	class UsbInPackage
	{
	public:
		const uint8_t header;
		const uint8_t randomByte;
		const uint8_t buttonKeyCode1;
		const uint8_t buttonKeyCode2;
		const uint8_t rotaryButtonFeedKeyCode;
		const uint8_t rotaryButtonAxisKeyCode;
		const int8_t stepCount;
		const uint8_t crc;
		UsbInPackage();
		UsbInPackage(const uint8_t notAvailable1, const uint8_t notAvailable2, const uint8_t buttonKeyCode1,
					 const uint8_t buttonKeyCode2, const uint8_t rotaryButtonFeedKeyCode,
					 const uint8_t rotaryButtonAxisKeyCode, const int8_t stepCount, const uint8_t crc);
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	//! This package is sent as last but one package before xhc-whb04-6 is powered off,
	//! and is meant to be used with operator== for comparison.
	class UsbEmptyPackage : public UsbInPackage
	{
	public:
		UsbEmptyPackage();
		bool operator==(const UsbInPackage &other) const;
		bool operator!=(const UsbInPackage &other) const;
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	//! This package is sent as last package before xhc-whb04-6 is powered off,
	//! and is meant to be used with operator== for comparison.
	class UsbSleepPackage : public UsbInPackage
	{
	public:
		UsbSleepPackage();
		bool operator==(const UsbInPackage &other) const;
		bool operator!=(const UsbInPackage &other) const;
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	//! set of constant usb packages
	class ConstantUsbPackages
	{
	public:
		const UsbSleepPackage sleepPackage;
		const UsbEmptyPackage emptyPackage;
		ConstantUsbPackages();
	};

	// ----------------------------------------------------------------------

	class OnUsbInputPackageListener
	{
	public:
		virtual void OnInputDataReceived(const UsbInPackage &inPackage) = 0;
		virtual ~OnUsbInputPackageListener();
	};

	// ----------------------------------------------------------------------

	class UsbRawInputListener
	{
	public:
		virtual void OnUsbDataReceived(uint8_t *data, int length) = 0;
		virtual ~UsbRawInputListener();
	};

	// ----------------------------------------------------------------------

	//! Convenience structure for initializing a transmission block.
	//! Caution: do not reorder fields!
	class UsbOutPackageBlockFields
	{
	public:
		uint8_t reportId;
		uint8_t __padding0;
		uint8_t __padding1;
		uint8_t __padding2;
		uint8_t __padding3;
		uint8_t __padding4;
		uint8_t __padding5;
		uint8_t __padding6;
		UsbOutPackageBlockFields();
		void Init(const void *data);
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	//! Convenience structure for accessing a block as byte buffer.
	class UsbOutPackageBlockBuffer
	{
	public:
		uint8_t asBytes[sizeof(UsbOutPackageBlockFields)];
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	union UsbOutPackageBlock
	{
	public:
		UsbOutPackageBlockBuffer asBuffer;
		UsbOutPackageBlockFields asBlock;
		UsbOutPackageBlock();
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	//! Convenience structure for initializing a transmission package's blocks.
	//! Caution: do not reorder fields!
	class UsbOutPackageBlocks
	{
	public:
		UsbOutPackageBlockFields block0;
		UsbOutPackageBlockFields block1;
		UsbOutPackageBlockFields block2;
		UsbOutPackageBlocks();
		void Init(const UsbOutPackageData *data);
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	//! Convenience structure for casting data in package stream.
	//! Caution: do not reorder fields!
	union UsbOutPackageBuffer
	{
	public:
		UsbOutPackageBlock asBlockArray[sizeof(UsbOutPackageBlocks) / sizeof(UsbOutPackageBlock)];
		UsbOutPackageBlocks asBlocks;
		UsbOutPackageBuffer();
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	//! Convenience structure for casting data in package stream.
	//! Caution: do not reorder fields!
	union UsbInPackageBuffer
	{
	public:
		const UsbInPackage asFields;
		uint8_t asBuffer[sizeof(UsbInPackage)];
		UsbInPackageBuffer();
	} __attribute__((packed));

	// ----------------------------------------------------------------------

	//! USB related parameters
	class Usb : public UsbRawInputListener
	{
	public:
		static const ConstantUsbPackages ConstantPackages;
		Usb(const char *name, OnUsbInputPackageListener &onDataReceivedCallback);
		~Usb();
		uint16_t GetUsbVendorId() const;
		uint16_t GetUsbProductId() const;
		const bool IsDeviceOpen() const;
		void SetDoReconnect(bool doReconnect);
		void OnUsbDataReceived(uint8_t *data, int length) override;
		void SetSimulationMode(bool isSimulationMode);
		void SetIsRunning(bool enableRunning);
		void RequestTermination();
		bool SetupAsyncTransfer();
		void SendDisplayData();
		void EnableVerboseTx(bool enable);
		void EnableVerboseRx(bool enable);
		void EnableVerboseInit(bool enable);
		bool Init();
		void SetWaitWithTimeout(uint8_t waitSecs);
		UsbOutPackageData &GetOutputPackageData();

	private:
		const uint16_t myUsbVendorId{0x10ce};
		const uint16_t myUsbProductId{0xeb93};
		bool myDoReconnect{false};
		bool myIsWaitWithTimeout{false};
		bool myIsSimulationMode{false};
		bool myIsRunning{false};
		UsbInPackageBuffer myInputPackageBuffer;
		UsbOutPackageBuffer myOutputPackageBuffer;
		UsbOutPackageData myOutputPackageData;
		OnUsbInputPackageListener &myDataHandler;
		std::ostream myDevNull{nullptr};
		std::ostream *myVerboseTxOut{nullptr};
		std::ostream *myVerboseRxOut{nullptr};
		std::ostream *myVerboseInitOut{nullptr};
		const char *myName{nullptr};
		uint8_t myWaitSecs{0};
	};

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const UsbOutPackageAxisCoordinate &coordinate);
	std::ostream &operator<<(std::ostream &os, const UsbOutPackageData &data);
	std::ostream &operator<<(std::ostream &os, const UsbOutPackageBlockFields &block);
	std::ostream &operator<<(std::ostream &os, const UsbOutPackageBlocks &blocks);
}
