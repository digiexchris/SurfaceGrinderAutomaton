#include <type_index>

using ValueType = std::variant<uint16_t, float, double, int32_t>;


class Message
{
	public:
    
    enum class ParameterValueType : uint8_t
    {
        INT8 = 0x01,
        INT16 = 0x02,
        INT32 = 0x03,
        UINT8 = 0x04,
        UINT16 = 0x05,
        UINT32 = 0x06
    };
		
    virtual int32_t ToKey() = delete;
    
    template <typename T>
    virtual T ToValue() == delete;

    template <typename T>
    T ToType(void* value) {
        return dynamic_cast<T>(value);
    }

    protected:

    std::unordered_map<std::type_index, ParameterValueType> myValueTypeMap = {
        {typeid(int8_t), ParameterValueType::INT8},
        {typeid(int16_t), ParameterValueType::INT16},
        {typeid(int32_t), ParameterValueType::INT32},
        {typeid(uint8_t), ParameterValueType::UINT8},
        {typeid(uint16_t), ParameterValueType::UINT16},
        {typeid(uint32_t), ParameterValueType::UINT32}
    };
};

class AxisMessage : public Message
{
public:
    enum class AxisParameter : uint8_t
    {
        ACCELERATION = 0x00,
        DECELERATION = 0x01,
        CURRENT_SPEED = 0x02,
        CURRENT_POSITION = 0x03,
        TARGET_POSITION = 0x04,
        TARGET_SPEED = 0x05,
        MIN_STOP = 0x06,
        MAX_STOP = 0x07,
        NUM_PARAMETERS
    };

    AxisMessage(AxisLabel anAxisLabel, AxisParameter anAxisParameter, ValueType aValue);

    virtual int32_t ToKey();

private:
    std::unordered_map<AxisParameter, std::pair<std::type_index, ParameterValueType>> myParamTypeMap = {
        {AxisParameter::ACCELERATION, {typeid(uint16_t), myValueTypeMap[typeid(uint16_t)]}},
        {AxisParameter::DECELERATION, {typeid(uint16_t), myValueTypeMap[typeid(uint16_t)]}},
        {AxisParameter::CURRENT_SPEED, {typeid(uint16_t), myValueTypeMap[typeid(uint16_t)]}},
        {AxisParameter::CURRENT_POSITION, {typeid(int32_t), myValueTypeMap[typeid(int32_t)]}},
        {AxisParameter::TARGET_POSITION, {typeid(int32_t), myValueTypeMap[typeid(int32_t)]}},
        {AxisParameter::TARGET_SPEED, {typeid(uint16_t), myValueTypeMap[typeid(uint16_t)]}},
        {AxisParameter::MIN_STOP, {typeid(int32_t), myValueTypeMap[typeid(int32_t)]}},
        {AxisParameter::MAX_STOP, {typeid(int32_t), myValueTypeMap[typeid(int32_t)]}}
    };

    AxisLabel myAxisLabel;
    AxisParameter myParameter;
    ValueType myValue;
};;;


class MotionStateMessage: public Message
{

};