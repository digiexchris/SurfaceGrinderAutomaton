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
	ERROR = -1 // locked by mutex
};

inline char AxisDirectionToChar(AxisDirection aDirection)
{
	switch (aDirection)
	{
	case AxisDirection::POS:
		return '+';
	case AxisDirection::NEG:
		return '-';
	default:
		return 'E';
	}
}

inline AxisDirection AxisDirectionFromString(const std::string &aDirection)
{
	if (aDirection == "+")
	{
		return AxisDirection::POS;
	}
	else if (aDirection == "-")
	{
		return AxisDirection::NEG;
	}
	else
	{
		return AxisDirection::ERROR;
	}
}

enum class AxisStop
{
	MIN = -1,
	MAX = 1,
	NEITHER = 0
};

enum class AxisState
{
	STOPPED,
	MOVING,
	WAITING,
	LOCKED // locked by mutex
};

class Axis : public Stepper
// TODO extend from an interface that abstracts Stepper, Axis just adds in the concept of stops and current and previous directions now.The movement thread just calls stepper update as fast as possible if it's !Idle or if it is idle, update() should block until something requests a new target position (save them cpu cycles)
{
public:
	Axis(AxisLabel anAxisLabel, uint stepPin, uint dirPin, float maxSpeed, float acceleration, PIO pio, uint sm);
	void SetMinStop(int32_t aMinStop);
	int32_t GetMinStop();
	void SetMaxStop(int32_t aMaxStop);
	int32_t GetMaxStop();

	/**
	@brief Determine if the stepper is resting at a stop. This should only be called if the stepper is idle otherwise the stepper may be in the process of moving off of a stop or arriving at a stop.
	@return The the current stop the stepper is at, or NEITHER if it is not at a stop

	 */
	AxisStop IsAtStop();

	/**
	@brief Move the axis to a specific position, cappped to within the set stops
	@param aPosition The position to move the axis to
	@param aSpeed The speed to move the axis to the position. if not passed, the axis's current speed will be used
	 */
	void MoveTo(int32_t aPosition, uint16_t aSpeed = 0);

	/**
	@brief Move the set target position a number of steps
	@param aDistance The number of steps to move the target position
	@param aSpeed The speed to move the target position. if not passed, the axis's current speed will be used

	Note: if the axis is in motion, this only moves the target position. The current position will continue being managed by the stepper internally.
	 */
	void MoveRelative(int32_t aDistance, uint16_t aSpeed = 0);

	/**
	@brief block until the stepper is idle
	@param aTimeout The maximum time to wait for the stepper to become idle
	@return true if the stepper is idle, false if the timeout was reached
	 */
	bool WaitUntilMovementComplete(TickType_t aTimeout = portMAX_DELAY);

	virtual StepperError SetTargetSpeed(uint16_t aSpeed) override;

	/**
	* @brief Stop the axis from moving
	todo: implement this in the stepper
	it needs to stop generating any more steps, calculate how many steps are required to decelerate to a stop, and do it.
	 */
	// void Stop();

private:
	int32_t myMinStop = 0;
	int32_t myMaxStop = 0;
	AxisLabel myAxisLabel;
	int32_t myPreviousTargetPosition = 0;
	uint16_t myMaxSpeed = 0;

	static void MoveThread(void *pvParameters);

	void ProcessStepperNotification(StepperNotifyMessage *aMessage);
};