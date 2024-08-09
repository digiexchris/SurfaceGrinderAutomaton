#include "WebSerial.hpp"
#include "CRC.hpp"
#include "Enum.hpp"
#include "Helpers.hpp"
#include "Proto.hpp"
#include "config.hpp"
#include <cstdint>
#include <semphr.h>
#include <unordered_map>

WebSerial *WebSerial::myInstance = nullptr;

WebSerial::WebSerial(Usb *aUsb)
{
	myUsb = aUsb;
	myInstance = this;
	myOutputQueueMutex = xSemaphoreCreateMutex();

	BaseType_t status = xTaskCreate(WebSerial::WritePendingUpdates, "WebSerial::WritePendingUpdates", 1 * 2048, NULL, USB_SERIAL_UPDATE_PRIORITY, NULL);
	configASSERT(status == pdPASS);
}

void WebSerial::QueueUpdate(WebSerialUpdate &anUpdate)
{
	switch (anUpdate.type)
	{
	case WebSerialUpdateType::AXIS:
	{
		WebSerialAxisUpdate &axisUpdate = static_cast<WebSerialAxisUpdate &>(anUpdate);
		Message msg = axisUpdate.ToMessage();

		xSemaphoreTake(myOutputQueueMutex, portMAX_DELAY);
		myOutputQueue[msg.ToKey()] = msg.ToValue();
		xSemaphoreGive(myOutputQueueMutex);
		// delete axisUpdate;
		break;
	}
	}
}

bool WebSerial::IsConnected()
{
	return myUsb->IsWebSerialConnected();
}

void WebSerial::WritePendingUpdates(void *param)
{

	TickType_t wait = xTaskGetTickCount();

	WebSerial *myInstance = WebSerial::GetInstance();
	while (true)
	{
		if (myInstance->IsConnected())
		{
			// NOTE: it is not necessary to maintain this output queue if nothing is reading it, it will contain the current state of the system and will be sent when the connection is established.
			// This may not be the desired behavior, if the thing connecting needs to request specific data on startup. In that case, the queue should be cleared each loop iteration until something is connected.
			xSemaphoreTake(myInstance->myOutputQueueMutex, portMAX_DELAY);

			for (auto m : myInstance->myOutputQueue)
			{
				KeyType key = m.first;
				ValueType value = m.second;

				const std::vector<uint8_t> buffer = myInstance->privConstructMessage(key, value);

				myInstance->myUsb->WriteWebSerial((void *)buffer.data(), buffer.size());
			}

			myInstance->myOutputQueue.clear();

			xSemaphoreGive(myInstance->myOutputQueueMutex);

			vTaskDelayUntil(&wait, MS_TO_TICKS(100));
		}
	}
}

template <typename T>
void WebSerial::privAddValueToMessageBuffer(std::vector<uint8_t> &buffer, T value)
{
	uint8_t *ptr = reinterpret_cast<uint8_t *>(&value);
	buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
}

/**
	The resulting webserial message should be in the form
	[command][context][contextValue][parameter][valueType][value][crc][end]
	where:
	- command is a ParameterCommand, any message sent back to the host is a MESSAGE command regardless of who initiated it
	- context is a ParameterContext
	- contextValue is a uint8_t such as an AxisLabel or some other thing that selects a subcontext
	- parameter is a parameter of the subcontext to get or modify, such as the target speed of the Axis context of the contextValue AxisLabel::X
	- valueType is a ParameterValueType such as UINT16 for a speed
	- value is the value (cast to int32_t might not be necessary, todo: figure that out, it should be dynamic length)
	- checksum of the message
	- end of message terminator (0xff)

	eg:
	the device sending the current speed of the Y axis would look like this
	0x03     //ParameterCommand::MESSAGE
	0x00     //ParameterContext::AXIS
	0x01     //AxisLabel::Y
	0x02     //AxisParameter::CURRENT_SPEED
	0x02     //INT16
	0x00 0xF2  //current speed of 242
	0x12 0x34     //checksum (dummy of 1234 used for this example)
	0xFF     //end of message terminator


 */
std::vector<uint8_t> WebSerial::privConstructMessage(
	KeyType key,
	ValueType value)
{
	std::vector<uint8_t> buffer;

	// Add value itself
	uint8_t *keyPtr = reinterpret_cast<uint8_t *>(&key);
	buffer.insert(buffer.end(), keyPtr, keyPtr + sizeof(key));

	if (std::holds_alternative<uint16_t>(value))
	{
		uint16_t val = std::get<uint16_t>(value);
		privAddValueToMessageBuffer(buffer, val);
	}
	else if (std::holds_alternative<float>(value))
	{
		float val = std::get<float>(value);
		privAddValueToMessageBuffer(buffer, val);
	}
	else if (std::holds_alternative<double>(value))
	{
		double val = std::get<double>(value);
		privAddValueToMessageBuffer(buffer, val);
	}
	else if (std::holds_alternative<int32_t>(value))
	{
		int32_t val = std::get<int32_t>(value);
		privAddValueToMessageBuffer(buffer, val);
	}

	// Calculate and add CRC
	uint8_t crc = CalculateCRC16CCITT(buffer);
	buffer.push_back(crc);

	// Add end of message terminator
	buffer.push_back(0xFF);

	return buffer;
}

Message WebSerialAxisUpdate::ToMessage()
{
	return AxisMessage(axis, param, value);
}