#include "./usb.h"

// system includes
#include <assert.h>
#include <iomanip>
#include <iostream>
#include <unistd.h>

// 3rd party includes

// local library includes
#include <rtapi_math.h>

// local includes

using std::endl;

namespace XhcWhb04b6
{

	// ----------------------------------------------------------------------

	const ConstantUsbPackages Usb::ConstantPackages;

	// ----------------------------------------------------------------------

	UsbOutPackageBlock::UsbOutPackageBlock() : asBlock()
	{
		assert(sizeof(UsbOutPackageBlockBuffer) == sizeof(UsbOutPackageBlockFields));
	}

	// ----------------------------------------------------------------------

	UsbInPackageBuffer::UsbInPackageBuffer() : asBuffer{0}
	{
		assert(sizeof(asFields) == sizeof(asBuffer));
	}

	// ----------------------------------------------------------------------

	UsbEmptyPackage::UsbEmptyPackage()
		: UsbInPackage(0x04, 0xff, 0, 0, 0, 0, 0, 0xff) {}

	// ----------------------------------------------------------------------

	bool UsbEmptyPackage::operator==(const UsbInPackage &other) const
	{
		if ((header == other.header) &&
			(buttonKeyCode1 == other.buttonKeyCode1) &&
			(buttonKeyCode2 == other.buttonKeyCode2) &&
			(rotaryButtonFeedKeyCode == other.rotaryButtonFeedKeyCode) &&
			(rotaryButtonAxisKeyCode == other.rotaryButtonAxisKeyCode) &&
			(stepCount == other.stepCount))
		{
			return true;
		}
		return false;
	}

	// ----------------------------------------------------------------------

	bool UsbEmptyPackage::operator!=(const UsbInPackage &other) const
	{
		return !((*this) == other);
	}

	// ----------------------------------------------------------------------

	UsbSleepPackage::UsbSleepPackage()
		: UsbInPackage(0x04, 0xff, 0xff, 0xff, 0xff, 0xff, -127, 0xff) {}

	// ----------------------------------------------------------------------

	bool UsbSleepPackage::operator==(const UsbInPackage &other) const
	{
		if ((header == other.header))
		{
			return true;
		}
		return false;
	}

	// ----------------------------------------------------------------------

	bool UsbSleepPackage::operator!=(const UsbInPackage &other) const
	{
		return !((*this) == other);
	}

	// ----------------------------------------------------------------------

	ConstantUsbPackages::ConstantUsbPackages()
		: sleepPackage(), emptyPackage() {}

	// ----------------------------------------------------------------------

	uint16_t Usb::GetUsbVendorId() const
	{
		return myUsbVendorId;
	}

	// ----------------------------------------------------------------------

	uint16_t Usb::GetUsbProductId() const
	{
		return myUsbProductId;
	}

	// ----------------------------------------------------------------------

	const bool Usb::IsDeviceOpen() const
	{
		return tud_mounted();
	}

	// ----------------------------------------------------------------------

	void Usb::SetDoReconnect(bool doReconnect)
	{
		myDoReconnect = doReconnect;
	}

	// ----------------------------------------------------------------------

	Usb::Usb(const char *name, OnUsbInputPackageListener &onDataReceivedCallback)
		: myInputPackageBuffer(),
		  myOutputPackageBuffer(),
		  myDataHandler(onDataReceivedCallback),
		  myVerboseTxOut(&myDevNull),
		  myVerboseRxOut(&myDevNull),
		  myVerboseInitOut(&myDevNull),
		  myName(name)
	{
	}

	// ----------------------------------------------------------------------

	void Usb::SendDisplayData()
	{
		myOutputPackageBuffer.asBlocks.Init(&myOutputPackageData);

		if (myIsSimulationMode)
		{
			*myVerboseTxOut << "out   0x" << myOutputPackageBuffer.asBlocks << endl
							<< std::dec << "out   size " << sizeof(myOutputPackageBuffer.asBlockArray) << "B " << myOutputPackageData
							<< endl;
		}

		for (size_t idx = 0; idx < (sizeof(myOutputPackageBuffer.asBlockArray) / sizeof(UsbOutPackageBlockFields)); idx++)
		{
			UsbOutPackageBlock &block = myOutputPackageBuffer.asBlockArray[idx];
			size_t blockSize = sizeof(UsbOutPackageBlock);

			uint8_t data[blockSize];
			memcpy(data, block.asBuffer.asBytes, blockSize);

			uint32_t result = tud_vendor_write(data, blockSize);

			if (result < blockSize)
			{
				std::cerr << "transmission failed, try to reconnect ..." << endl;
				SetDoReconnect(true);
				return;
			}
		}
	}

	// ----------------------------------------------------------------------

	void UsbOutPackageAxisCoordinate::SetCoordinate(const float &coordinate)
	{
		float coordinateAbs = rtapi_fabs(coordinate);
		if (coordinate == coordinateAbs)
		{
			coordinateSign = 0;
		}
		else
		{
			coordinateSign = 1;
		}

		uint32_t scaledCoordinate = static_cast<uint32_t>(rtapi_rint(coordinateAbs * 10000.0));
		integerValue = static_cast<uint16_t>(scaledCoordinate / 10000);
		fractionValue = static_cast<uint16_t>(scaledCoordinate % 10000);
	}

	// ----------------------------------------------------------------------

	void UsbOutPackageAxisCoordinate::Clear()
	{
		integerValue = 0;
		fractionValue = 0;
		coordinateSign = 0;
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const UsbOutPackageAxisCoordinate &coordinate)
	{
		std::ios init(NULL);
		init.copyfmt(os);
		os << ((coordinate.coordinateSign == 1) ? "-" : "+") << std::setfill('0')
		   << std::setw(4) << static_cast<unsigned short>(coordinate.integerValue) << "."
		   << std::setw(4) << static_cast<unsigned short>(coordinate.fractionValue);

		os.copyfmt(init);
		return os;
	}

	// ----------------------------------------------------------------------

	UsbOutPackageBlockFields::UsbOutPackageBlockFields()
		: reportId(0x06),
		  __padding0(0),
		  __padding1(0),
		  __padding2(0),
		  __padding3(0),
		  __padding4(0),
		  __padding5(0),
		  __padding6(0) {}

	// ----------------------------------------------------------------------

	void UsbOutPackageBlockFields::Init(const void *data)
	{
		reportId = 0x06;
		__padding0 = reinterpret_cast<const uint8_t *>(data)[0];
		__padding1 = reinterpret_cast<const uint8_t *>(data)[1];
		__padding2 = reinterpret_cast<const uint8_t *>(data)[2];
		__padding3 = reinterpret_cast<const uint8_t *>(data)[3];
		__padding4 = reinterpret_cast<const uint8_t *>(data)[4];
		__padding5 = reinterpret_cast<const uint8_t *>(data)[5];
		__padding6 = reinterpret_cast<const uint8_t *>(data)[6];
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const UsbOutPackageBlockFields &block)
	{
		std::ios init(NULL);
		init.copyfmt(os);

		os << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned short>(block.reportId) << std::setw(2)
		   << static_cast<unsigned short>(block.__padding0) << std::setw(2) << static_cast<unsigned short>(block.__padding1)
		   << std::setw(2) << static_cast<unsigned short>(block.__padding2) << std::setw(2)
		   << static_cast<unsigned short>(block.__padding3) << std::setw(2) << static_cast<unsigned short>(block.__padding4)
		   << std::setw(2) << static_cast<unsigned short>(block.__padding5) << std::setw(2)
		   << static_cast<unsigned short>(block.__padding6);

		os.copyfmt(init);
		return os;
	}

	// ----------------------------------------------------------------------

	UsbOutPackageBlocks::UsbOutPackageBlocks()
		: block0(), block1(), block2() {}

	// ----------------------------------------------------------------------

	void UsbOutPackageBlocks::Init(const UsbOutPackageData *data)
	{
		const uint8_t *d = reinterpret_cast<const uint8_t *>(data);
		block0.Init(d += 0);
		block1.Init(d += 7);
		block2.Init(d + 7);
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const UsbOutPackageBlocks &blocks)
	{
		return os << blocks.block0 << " " << blocks.block1 << " " << blocks.block2;
	}

	// ----------------------------------------------------------------------

	UsbOutPackageData::UsbOutPackageData()
	{
		Clear();
	}

	// ----------------------------------------------------------------------

	void UsbOutPackageData::Clear()
	{
		header = 0xfdfe;
		seed = 0xfe;
		displayModeFlags.asByte = 0;

		row1Coordinate.Clear();
		row2Coordinate.Clear();
		row3Coordinate.Clear();

		feedRate = 0;
		spindleSpeed = 0;
		padding = 0;
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const UsbOutPackageData &data)
	{
		std::ios init(NULL);
		init.copyfmt(os);

		bool enableMultiLine = false;
		if (enableMultiLine)
		{
			os << std::hex << std::setfill('0') << "header       0x" << std::setw(2) << data.header << endl
			   << "day of month   0x"
			   << std::setw(2)
			   << static_cast<unsigned short>(data.seed) << endl
			   << "status 0x" << std::setw(2)
			   << static_cast<unsigned short>(data.displayModeFlags.asByte) << endl
			   << std::dec << "coordinate1  "
			   << data.row1Coordinate << endl
			   << "coordinate2  " << data.row2Coordinate << endl
			   << "coordinate3  "
			   << data.row3Coordinate << endl
			   << "feed rate        " << data.feedRate << endl
			   << "spindle rps      "
			   << data.spindleSpeed;
		}
		else
		{
			os << std::hex << std::setfill('0') << "hdr 0x" << std::setw(4) << data.header << " dom 0x" << std::setw(2)
			   << static_cast<unsigned short>(data.seed) << " status 0x" << std::setw(2)
			   << static_cast<unsigned short>(data.displayModeFlags.asByte) << std::dec << " coord1 "
			   << data.row1Coordinate << " coord2 " << data.row2Coordinate << " coord3 "
			   << data.row3Coordinate << " feed " << std::setw(4) << data.feedRate << " spindle rps "
			   << std::setw(5) << data.spindleSpeed;
		}
		os.copyfmt(init);
		return os;
	}

	// ----------------------------------------------------------------------

	UsbOutPackageBuffer::UsbOutPackageBuffer()
		: asBlocks()
	{
		if (false)
		{
			std::cout << "sizeof usb data " << sizeof(UsbOutPackageData) << endl
					  << " blocks count   " << sizeof(UsbOutPackageBlocks) / sizeof(UsbOutPackageBlockFields) << endl
					  << " sizeof block   " << sizeof(UsbOutPackageBlockFields) << endl
					  << " sizeof blocks  " << sizeof(UsbOutPackageBlocks) << endl
					  << " sizeof array   " << sizeof(asBlockArray) << endl
					  << " sizeof package " << sizeof(UsbOutPackageData) << endl;
		}
		assert(sizeof(UsbOutPackageBlocks) == sizeof(asBlockArray));
		size_t blocksCount = sizeof(UsbOutPackageBlocks) / sizeof(UsbOutPackageBlockFields);
		assert((sizeof(UsbOutPackageData) + blocksCount) == sizeof(UsbOutPackageBlocks));
	}

	// ----------------------------------------------------------------------

	UsbInPackage::UsbInPackage()
		: header(0),
		  randomByte(0),
		  buttonKeyCode1(0),
		  buttonKeyCode2(0),
		  rotaryButtonFeedKeyCode(0),
		  rotaryButtonAxisKeyCode(0),
		  stepCount(0),
		  crc(0)
	{
	}

	// ----------------------------------------------------------------------

	UsbInPackage::UsbInPackage(const uint8_t notAvailable1, const uint8_t notAvailable2, const uint8_t buttonKeyCode1,
							   const uint8_t buttonKeyCode2, const uint8_t rotaryButtonFeedKeyCode,
							   const uint8_t rotaryButtonAxisKeyCode, const int8_t stepCount, const uint8_t crc)
		: header(notAvailable1),
		  randomByte(notAvailable2),
		  buttonKeyCode1(buttonKeyCode1),
		  buttonKeyCode2(buttonKeyCode2),
		  rotaryButtonFeedKeyCode(rotaryButtonFeedKeyCode),
		  rotaryButtonAxisKeyCode(rotaryButtonAxisKeyCode),
		  stepCount(stepCount),
		  crc(crc)
	{
	}

	// ----------------------------------------------------------------------

	void Usb::SetSimulationMode(bool isSimulationMode)
	{
		myIsSimulationMode = isSimulationMode;
	}

	// ----------------------------------------------------------------------

	void Usb::SetIsRunning(bool enableRunning)
	{
		myIsRunning = enableRunning;
	}

	// ----------------------------------------------------------------------

	void Usb::RequestTermination()
	{
		myIsRunning = false;
	}

	// ----------------------------------------------------------------------

	bool Usb::SetupAsyncTransfer()
	{
		return true;
	}

	// ----------------------------------------------------------------------

	void Usb::OnUsbDataReceived(uint8_t *data, int length)
	{
		int expectedPackageSize = static_cast<int>(sizeof(UsbInPackage));
		std::ios init(NULL);
		init.copyfmt(*myVerboseTxOut);

		if (length == expectedPackageSize)
		{
			if (mySleepState.myDropNextInPackage)
			{
				mySleepState.myDropNextInPackage = false;
				return;
			}

			if (ConstantPackages.emptyPackage == myInputPackageBuffer.asFields)
			{
				mySleepState.myDropNextInPackage = true;
			}
			else
			{
				if (true)
				{
					gettimeofday(&mySleepState.myLastWakeupTimestamp, nullptr);
				}
			}
			myDataHandler.OnInputDataReceived(myInputPackageBuffer.asFields);
		}
		else
		{
			std::cerr << "received unexpected package size: expected=" << length << ", current="
					  << expectedPackageSize << endl;
		}
	}

	// ----------------------------------------------------------------------

	Usb::~Usb()
	{
	}

	// ----------------------------------------------------------------------

	void Usb::EnableVerboseTx(bool enable)
	{
		if (enable)
		{
			myVerboseTxOut = &std::cout;
		}
		else
		{
			myVerboseTxOut = &myDevNull;
		}
	}

	// ----------------------------------------------------------------------

	void Usb::EnableVerboseRx(bool enable)
	{
		if (enable)
		{
			myVerboseRxOut = &std::cout;
		}
		else
		{
			myVerboseRxOut = &myDevNull;
		}
	}

	// ----------------------------------------------------------------------

	void Usb::EnableVerboseInit(bool enable)
	{
		if (enable)
		{
			myVerboseInitOut = &std::cout;
		}
		else
		{
			myVerboseInitOut = &myDevNull;
		}
	}

	// ----------------------------------------------------------------------

	bool Usb::Init()
	{
		tud_init(BOARD_TUD_RHPORT);
		EnableVerboseInit(true);
		EnableVerboseRx(true);
		EnableVerboseTx(true);

		return true;
	}

	// ----------------------------------------------------------------------

	void Usb::SetWaitWithTimeout(uint8_t waitSecs)
	{
		myWaitSecs = waitSecs;
		if (myWaitSecs > 0)
		{
			myIsWaitWithTimeout = true;
			return;
		}
		myIsWaitWithTimeout = false;
	}

	// ----------------------------------------------------------------------

	UsbOutPackageData &Usb::GetOutputPackageData()
	{
		return myOutputPackageData;
	}

	// ----------------------------------------------------------------------

	SleepDetect::SleepDetect()
		: myDropNextInPackage(false),
		  myLastWakeupTimestamp()
	{
	}

	// ----------------------------------------------------------------------

	UsbRawInputListener::~UsbRawInputListener()
	{
	}

	// ----------------------------------------------------------------------

	OnUsbInputPackageListener::~OnUsbInputPackageListener()
	{
	}

}
