#include "Enum.hpp"
#include "usb.hpp"

#include <cstdint>
#include <vector>

// enum class WebSerialProtocolCommands : uint8_t
// {
// 	READ = 0x01,
// 	WRITE = 0x02
// };

// enum class WebSerialProtocolRegisters : uint8_t
// {
// 	POSITION = 0x01,
// 	TARGET_POSITION = 0x02,
// 	SPEED = 0x03,
// 	TARGET_SPEED = 0x04,

// };

enum class WebSerialUpdateType : uint8_t
{
	AXIS = 0x01,
	STEPPER = 0x02
};

struct WebSerialUpdate
{
	WebSerialUpdate(WebSerialUpdateType aType)
		: type(aType)
	{
	}
	WebSerialUpdateType type;
};

struct WebSerialAxisUpdate : public WebSerialUpdate
{
	WebSerialAxisUpdate(AxisLabel aAxis, AxisParameter aParameter, int32_t aValue)
		: axis(aAxis), param(aParameter), value(aValue), WebSerialUpdate(WebSerialUpdateType::AXIS)
	{
	}
	AxisLabel axis;
	AxisParameter param;
	int16_t value;
};

struct WebSerialMsg
{
	WebSerialMsg(uint8_t aCmd, uint8_t aReg, uint8_t aSubReg, uint8_t *aData, size_t aLen)
		: cmd(aCmd), reg(aReg), subReg(aSubReg)
	{
		memcpy(data, aData, aLen);
	}
	uint8_t cmd;	// eg read/write
	uint8_t reg;	// eg position, speed
	uint8_t subReg; // eg x, y, z
	uint8_t data[63];
};

class WebSerial
{

public:
	WebSerial(Usb *aUsb);
	void QueueUpdate(WebSerialUpdate *anUpdate);
	static void WritePendingUpdates(void *param);
	static WebSerial *GetInstance()
	{
		if (myInstance == nullptr)
		{
			myInstance = new WebSerial(Usb::GetInstance());
		}
		return myInstance;
	}

	bool IsConnected();

private:
	Usb *myUsb;
	static WebSerial *myInstance;

	template <typename T>
	void privAddValueToMessageBuffer(std::vector<uint8_t> &buffer, T value);

	std::vector<uint8_t> privConstructMessage(
		ParameterCommand command,
		ParameterContext context,
		uint8_t contextValue,
		AxisParameter parameter,
		ParameterValueType valueType,
		int32_t value);

	AxisParameterTable myAxisParameterTable;
};