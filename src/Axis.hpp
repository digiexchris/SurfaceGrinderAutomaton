#pragma once

#include "Enum.hpp"
#include "drivers/Motor/Stepper.hpp"
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>

#include <config.hpp>

#include "pico/stdlib.h" // IWYU pragma: keep
#include "portmacro.h"
#include <string>
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
 * @brief Enum class for the direction of the Z travel
 * POS = move the Z axis in the positive direction
 * NEG = move the Z axis in the negative direction
 * ignored for MANUAL
 */
enum class AxisDirection
{
	POS = true,
	NEG = false,
	LOCKED // locked by mutex
};

enum class AxisStop
{
	MIN = -1,
	MAX = 1,
	NEITHER = 0
};

enum class AxisCommandName
{
	MOVE,
	SET_DIRECTION,
	WAIT
};

struct AxisCommand
{
	AxisCommandName cmd;
};

struct AxisMoveCommand : public AxisCommand
{
	AxisMoveCommand(int32_t aDistance, uint16_t aSpeed) : distance(aDistance), speed(aSpeed)
	{
		cmd = AxisCommandName::MOVE;
	}
	int32_t distance;
	uint16_t speed;
};

struct AxisWaitCommand : AxisCommand
{
	AxisWaitCommand(int32_t aDurationMs = 1) : durationMs(aDurationMs)
	{
		cmd = AxisCommandName::WAIT;
	}
	int32_t durationMs;
};

struct AxisSetDirectionCommand : AxisCommand
{
	AxisSetDirectionCommand(AxisDirection aDirection) : direction(aDirection)
	{
		cmd = AxisCommandName::SET_DIRECTION;
	}
	AxisDirection direction;
};

enum class AxisState
{
	STOPPED,
	MOVING,
	WAITING,
	LOCKED // locked by mutex
};

class Axis
{
public:
	Axis(Stepper *aStepper, AxisLabel anAxisLabel);
	void SetPosition(int32_t aPosition);
	int32_t GetPosition();
	void SetMinStop(int32_t aMinStop);
	int32_t GetMinStop();
	void SetMaxStop(int32_t aMaxStop);
	int32_t GetMaxStop();
	void Move(uint32_t aDistance, uint16_t aSpeed);
	void SetDirection(AxisDirection aDirection);
	void Wait(int32_t aDurationMs);
	AxisState GetState();
	AxisStop IsAtStop();
	AxisDirection GetPreviousDirection();

	bool IsMovementComplete(TickType_t aTimeout = portMAX_DELAY);

	uint8_t GetQueueSize();

	/**
	* @brief Stop the axis from moving
	todo: implement this in the stepper
	it needs to stop generating any more steps, calculate how many steps are required to decelerate to a stop, and do it.
	 */
	void Stop();

	/**
	 * @brief Emergency stop the axis from moving
	 * todo: implement this in the stepper
	 * it needs to stop generating any more steps,
	 * and disable the driver. This will likely result in lost stops.
	 */
	void EStop();

private:
	SemaphoreHandle_t myStateMutex;
	SemaphoreHandle_t myDirectionMutex;
	SemaphoreHandle_t myQueueIsProcessing;
	AxisDirection myPreviousDirection = AxisDirection::POS;
	AxisDirection myDirection = AxisDirection::POS;
	AxisState myState = AxisState::STOPPED;
	Stepper *myStepper;
	int32_t myPosition = 0;
	int32_t myMinStop = 0;
	int32_t myMaxStop = 0;
	QueueHandle_t myCommandQueue;
	AxisLabel myAxisLabel;

	static void privProcessCommandQueue(void *pvParameters);
	void privMove(uint32_t aDistance, uint16_t aSpeed);
	void privSetDirection(AxisDirection aDirection);
};