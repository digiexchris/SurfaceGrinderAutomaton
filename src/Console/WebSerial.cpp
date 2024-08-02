#include "WebSerial.hpp"
#include "CRC.hpp"
#include "Enum.hpp"
#include <cstdint>
#include <unordered_map>

WebSerial *WebSerial::myInstance = nullptr;

WebSerial::WebSerial(Usb *aUsb)
{
	myUsb = aUsb;
	myInstance = this;
}

void WebSerial::QueueUpdate(WebSerialUpdate *anUpdate)
{
	switch (anUpdate->type)
	{
	case WebSerialUpdateType::AXIS:
	{
		WebSerialAxisUpdate *axisUpdate = static_cast<WebSerialAxisUpdate *>(anUpdate);
		myAxisParameterTable[{axisUpdate->axis, axisUpdate->param}] = axisUpdate->value;
		// myAxisParameterTable[axisUpdate->axis][axisUpdate->param] = axisUpdate->value;
		break;
	}
	case WebSerialUpdateType::STEPPER:
	{
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
			for (AxisParameterValue axisParameter : myInstance->myAxisParameterTable)
			{
				const std::vector<uint8_t> buffer = myInstance->privConstructMessage(
					ParameterCommand::WRITE,
					ParameterContext::AXIS,
					static_cast<uint8_t>(AxisLabel::X), // get from axisParameter
					AxisParameter::TARGET_SPEED,		// get from axisParameter
					ParameterValueType::UINT16,
					23423); // get from axisParameter

				myInstance->myUsb->WriteWebSerial((void *)buffer.data(), buffer.size());
				myInstance->myAxisParameterTable.remove(axisParameter);
			}
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
	ParameterCommand command,
	ParameterContext context,
	uint8_t contextValue,
	AxisParameter parameter,
	ParameterValueType valueType,
	int32_t value)
{
	std::vector<uint8_t> buffer;

	// Add command
	buffer.push_back(static_cast<uint8_t>(command));

	// Add context
	buffer.push_back(static_cast<uint8_t>(context));

	// Add context value (such as AxisLabel)
	buffer.push_back(contextValue);

	// Add parameter (such as AxisParameter::TARGET_SPEED)
	buffer.push_back(static_cast<uint8_t>(parameter));

	// Add parameter type (such as INT32)
	buffer.push_back(static_cast<uint8_t>(valueType));

	// Add value itself
	privAddValueToMessageBuffer(buffer, value);

	// Calculate and add CRC
	uint8_t crc = CalculateCRC16CCITT(buffer);
	buffer.push_back(crc);

	// Add end of message terminator
	buffer.push_back(0xFF);

	return buffer;
}