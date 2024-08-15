#include "UnitConverter.hpp"
#include <cmath>
#include <cstdint>

UnitConverter::UnitConverter(Unit aUnit, float aMultiplicationFactor)
{
    unit = aUnit;
    multiplicationFactor = aMultiplicationFactor;
}

UnitConverter::GetSteps(float distance)
{
    return static_cast<int>(std::round(distance * multiplicationFactor));
}