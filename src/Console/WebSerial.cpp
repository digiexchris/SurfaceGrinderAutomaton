#include "WebSerial.hpp"
#include "CRC.hpp"
#include "Enum.hpp"
#include "Proto.hpp"
#include <cstdint>
#include <semphr.h>
#include <unordered_map>

WebSerial *WebSerial::myInstance = nullptr;

WebSerial::WebSerial(Usb *aUsb)
{
	myUsb = aUsb;
	myInstance = this;
	myOutputQueueMutex = xSemaphoreCreateMutex();

	BaseType_t status = xTaskCreate(WebSerial::WritePendingUpdates, "WebSerial::WritePendingUpdates", 1 * 2048, NULL, 1, NULL);
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

	WebSerial *myInstance = WebSerial::GetInstance();
	while (true)
	{
		vTaskDelay(100);

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
		}
	}
}

template <typename T>
void WebSerial::privAddValueToMessageBuffer(std::vector<uint8_t> &buffer, T value)
{
	uint8_t *ptr = reinterpret_cast<uint8_t *>(&value);
	buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
}

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