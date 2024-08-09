#include "Enum.hpp"
#include "pico/platform.h"
#include "usb.hpp"
#include <semphr.h>

#include "Proto.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>

// enum class WebSerialProtocolCommands : uint8_t //NOTE these are only incoming, sent messages back to the UI don't need commands.
// {
// 	READ = 0x01,
// 	WRITE = 0x02
// };

enum class WebSerialUpdateType : uint8_t
{
	AXIS = 0x01
};

struct WebSerialUpdate
{
	WebSerialUpdate(WebSerialUpdateType aType)
		: type(aType)
	{
	}
	WebSerialUpdateType type;

	virtual Message ToMessage() = 0;
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

	virtual Message ToMessage() override;
};

class WebSerial
{

public:
	WebSerial(Usb *aUsb);
	void QueueUpdate(WebSerialUpdate &anUpdate);
	static void WritePendingUpdates(void *param);
	static WebSerial *GetInstance()
	{
		if (myInstance == nullptr)
		{
			panic("WebSerial instance not created");
		}
		return myInstance;
	}

	static void ProcessChars(const char *data, size_t len);

	bool IsConnected();

	void Start();

private:
	Usb *myUsb;
	static WebSerial *myInstance;

	template <typename T>
	void privAddValueToMessageBuffer(std::vector<uint8_t> &buffer, T value);

	std::vector<uint8_t> privConstructMessage(
		uint32_t key,
		ValueType value);

	SemaphoreHandle_t myOutputQueueMutex;
	std::unordered_map<KeyType, ValueType> myOutputQueue;
};