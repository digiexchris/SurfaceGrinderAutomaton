
#include "Proto.hpp"
#include "Enum.hpp"
#include <cstdint>

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

	this->axisLabel = anAxisLabel;
	this->axisParameter = anAxisParameter;
	this->valueType = aValueType;
	this->value = aValue;
}

int32_t AxisMessage::ToKey()
{
	uint8_t context = static_cast<uint8_t>(myAxisLabel);
	uint8_t param = static_cast<uint8_t>(myParameter);
	uint8_t paramType = static_cast<uint8_t>(myParamTypeMap[myParameter]->second());

	return CAST PLUS BITSHIFT !!!
}