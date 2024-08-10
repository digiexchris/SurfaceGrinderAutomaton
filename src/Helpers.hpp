#pragma once

inline double ScaleValue(double value, double minOld, double maxOld, double minNew, double maxNew)
{
	return (value - minOld) / (maxOld - minOld) * (maxNew - minNew) + minNew;
}

template <typename T>
T abs(T value)
{
	return static_cast<T>(value < 0 ? -value : value);
}

template <typename T, typename T2>
T min(T a, T2 b)
{
	return static_cast<T>(a < b ? a : b);
}

#define MS_TO_TICKS(ms) (ms * configTICK_RATE_HZ / 1000)