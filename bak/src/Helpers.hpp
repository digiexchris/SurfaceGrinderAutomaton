#pragma once

#include "zephyr/sys/time_units.h"
inline double ScaleValue(double value, double minOld, double maxOld, double minNew, double maxNew)
{
	return (value - minOld) / (maxOld - minOld) * (maxNew - minNew) + minNew;
}