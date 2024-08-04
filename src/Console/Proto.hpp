#pragma once

#include "Enum.hpp"
#include <cstdint>
#include <typeindex>
#include <unordered_map>
#include <variant>

using ValueType = std::variant<uint16_t, float, double, int32_t>;
using KeyType = uint32_t;

class Message
{
public:
	enum class ParameterValueType // uint8_t
	{
		INT8 = 0x01,
		INT16 = 0x02,
		INT32 = 0x03,
		UINT8 = 0x04,
		UINT16 = 0x05,
		UINT32 = 0x06
	};

	virtual KeyType ToKey();
	size_t KeySize()
	{
		return sizeof(KeyType);
	}

	virtual ValueType ToValue()
	{
		return myValue;
	}

	size_t ValueSize()
	{
		return sizeof(myValue);
	}

protected:
	// std::unordered_map<std::type_index, ParameterValueType> myValueTypeMap = {
	// 	{typeid(int8_t), ParameterValueType::INT8},
	// 	{typeid(int16_t), ParameterValueType::INT16},
	// 	{typeid(int32_t), ParameterValueType::INT32},
	// 	{typeid(uint8_t), ParameterValueType::UINT8},
	// 	{typeid(uint16_t), ParameterValueType::UINT16},
	// 	{typeid(uint32_t), ParameterValueType::UINT32}};

	ValueType myValue;
	ParameterContext myContext;
	ParameterContextRegister myRegister;
};

class AxisMessage : public Message
{
public:
	AxisMessage(AxisLabel anAxisLabel, AxisParameter anAxisParameter, ValueType aValue);

private:
	std::unordered_map<AxisParameter, std::pair<std::type_index, ParameterValueType>> myParamTypeMap = {
		{AxisParameter::ACCELERATION, {std::type_index(typeid(uint16_t)), ParameterValueType::UINT16}},
		{AxisParameter::DECELERATION, {std::type_index(typeid(uint16_t)), ParameterValueType::UINT16}},
		{AxisParameter::CURRENT_SPEED, {std::type_index(typeid(uint16_t)), ParameterValueType::UINT16}},
		{AxisParameter::CURRENT_POSITION, {std::type_index(typeid(int32_t)), ParameterValueType::INT32}},
		{AxisParameter::TARGET_POSITION, {std::type_index(typeid(int32_t)), ParameterValueType::INT32}},
		{AxisParameter::TARGET_SPEED, {std::type_index(typeid(uint16_t)), ParameterValueType::UINT16}},
		{AxisParameter::MIN_STOP, {std::type_index(typeid(int32_t)), ParameterValueType::INT32}},
		{AxisParameter::MAX_STOP, {std::type_index(typeid(int32_t)), ParameterValueType::INT32}}};
};

class MotionStateMessage : public Message
{
};