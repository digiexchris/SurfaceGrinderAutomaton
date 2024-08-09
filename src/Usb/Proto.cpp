
#include "Proto.hpp"
#include "Enum.hpp"
#include "pico/platform.h"
#include <cstdint>
#include <sys/types.h>

AxisMessage::AxisMessage(AxisLabel anAxisLabel, AxisParameter anAxisParameter, ValueType aValue)
{
	// Initialize the parameterTypeMap with the appropriate types
	// myParamTypeMap = {
	//     {AxisParameter::ACCELERATION, typeid(uint16_t)},
	//     {AxisParameter::DECELERATION, typeid(uint16_t)},
	//     {AxisParameter::CURRENT_SPEED, typeid(uint16_t)},
	//     {AxisParameter::CURRENT_POSITION, typeid(int32_t)},
	//     {AxisParameter::TARGET_POSITION, typeid(int32_t)},
	//     {AxisParameter::TARGET_SPEED, typeid(uint16_t)},
	//     {AxisParameter::MIN_STOP, typeid(uint16_t)},
	//     {AxisParameter::MAX_STOP, typeid(uint16_t)}
	// };

	auto it = myParamTypeMap.find(anAxisParameter);
	if (it != myParamTypeMap.end())
	{
		myRegister = static_cast<uint8_t>(it->second.second);
		// Use paramType as needed
	}
	else
	{
		panic("param not found in map");
	}

	myContext = ParameterContext::AXIS;
	myValue = aValue;
}

KeyType Message::ToKey()
{
	uint8_t context = static_cast<uint8_t>(myContext);
	uint8_t reg = static_cast<uint8_t>(myRegister);

	uint8_t paramType = 0;

	return (context << 24) | (reg << 16) | (paramType << 8);
}