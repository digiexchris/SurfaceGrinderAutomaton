#pragma once

#include "Enum.hpp"
#include <string>
#include <cstdint>

class UnitConverter
{
public:
    UnitConverter(Unit aUnit, float aMultiplicationFactor);
    int_32t GetSteps(float distance);

private:
    const std::string unit;
    float multiplicationFactor;
}