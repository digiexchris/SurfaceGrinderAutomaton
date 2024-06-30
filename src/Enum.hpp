#pragma once

// #include "Axis.hpp"
#include <string>

enum class AxisLabel
{
	X,
	Y,
	Z,
	ERROR
};

inline std::string AxisLabelToString(AxisLabel label)
{
	switch (label)
	{
	case AxisLabel::X:
		return "X";
	case AxisLabel::Y:
		return "NOT IMPLEMENTED";
	case AxisLabel::Z:
		return "Z";
	default:
		return "ERROR";
	}
}

inline AxisLabel AxisLabelFromString(const std::string &label)
{
	if (label == "X")
	{
		return AxisLabel::X;
	}
	else if (label == "Y")
	{
		return AxisLabel::Y;
	}
	else if (label == "Z")
	{
		return AxisLabel::Z;
	}
	else
	{
		return AxisLabel::ERROR;
	}
}

enum class AxisMode
{
	STOPPED,
	AUTOMATIC,
	ONE_SHOT,
	MANUAL,
	ERROR
};

inline AxisMode AxisModeFromString(const std::string &aMode)
{
	if (aMode == "S")
	{
		return AxisMode::STOPPED;
	}
	else if (aMode == "A")
	{
		return AxisMode::AUTOMATIC;
	}
	else if (aMode == "O")
	{
		return AxisMode::ONE_SHOT;
	}
	else if (aMode == "M")
	{
		return AxisMode::MANUAL;
	}
	else if (aMode == "E")
	{
		return AxisMode::ERROR;
	}
	else
	{
		return AxisMode::ERROR;
	}
}

inline std::string AxisModeToString(AxisMode aMode)
{
	switch (aMode)
	{
	case AxisMode::STOPPED:
		return "STOPPED";
	case AxisMode::AUTOMATIC:
		return "AUTOMATIC";
	case AxisMode::ONE_SHOT:
		return "ONE_SHOT";
	case AxisMode::MANUAL:
		return "MANUAL";
	case AxisMode::ERROR:
		return "ERROR";
	default:
		return "UNKNOWN";
	}
}

/**
 * @brief Enum class for the type of Z travel moves
 * AT_BOTH_ENDS = move the Z axis when the X axis has stopped at either end of it's travel
 * AT_LEFT = move the Z axis when the X axis has stopped at the left end of it's travel
 * AT_RIGHT = move the Z axis when the X axis has stopped at the right end of it's travel
 * MANUAL = no automatic movement, move only via MPG
 * CONSTANT = move the Z axis at a constant rate, not tied to the X travel position
 */
enum class AdvanceZType
{
	AT_BOTH_ENDS,
	AT_LEFT,
	AT_RIGHT,
	MANUAL,
	CONSTANT
};

/**
 * @brief Enum class for the type of Z travel moves
 * NO_REPEAT = do not repeat the Z travel, go from the starting position to the ending position and stop
 * REVERSE = repeat the Z travel in the opposite direction once it has reached the end position
 * START_AT_SAME_POSITION = when the Z travel reaches the end position, stop the X axis according to the AdvanceZType,
 * move Z back to the start position, and repeat the Z advance according to the AdvanceZType
 */
enum class RepeatZType
{
	NO_REPEAT,
	REVERSE,
	START_AT_SAME_POSITION
};

enum class MotionState
{
	AUTOMATIC,
	STOPPED
};

#include <FreeRTOS.h>
#include <task.h>
class Controller
{
public:
	virtual AxisMode GetMode(AxisLabel anAxisLabel) = 0;
	virtual bool SetMode(AxisLabel anAxisLabel, AxisMode aMode) = 0;
	virtual TaskHandle_t GetTaskHandle(AxisLabel anAxisLabel) = 0;
};
