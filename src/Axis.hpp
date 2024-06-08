#pragma once

#include "Enum.hpp"
#include "drivers/Motor/Stepper.hpp"
#include <FreeRTOS.h>
#include <mutex>
#include <queue.h>

#include "pico/mutex.h"
#include "pico/stdlib.h"

enum class AxisStop
{
	MIN,
	MAX
};

enum class AxisCommandName
{
	MOVE
};

struct AxisCommand
{
	AxisCommandName cmd;
	void *data;
};

/**
 * @brief Enum class for the direction of the Z travel
 * POS = move the Z axis in the positive direction
 * NEG = move the Z axis in the negative direction
 * ignored for MANUAL
 */
enum class AxisDirection
{
	POS,
	NEG,
	LOCKED // locked by mutex
};

enum class AxisState
{
	STOPPED,
	MOVING,
	LOCKED // locked by mutex
};

class Axis
{
public:
	Axis(Stepper *aStepper, QueueHandle_t aCommandQueue);
	void SetPosition(int32_t aPosition);
	int32_t GetPosition();
	void SetMinStop(int32_t aMinStop);
	int32_t GetMinStop();
	void SetMaxStop(int32_t aMaxStop);
	int32_t GetMaxStop();
	void Move(int32_t aDistance);
	AxisState GetState();
	AxisDirection GetPreviosDirection();

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
	mutex_t *myStateMutex;
	mutex_t *myDirectionMutex;
	static void CommandThread(void *pvParameters);
	AxisDirection myPreviousDirection = AxisDirection::POS;
	AxisDirection myDirection = AxisDirection::POS;
	AxisState myState = AxisState::STOPPED;
	Stepper *myStepper;
	int32_t myPosition = 0;
	int32_t myMinStop = 0;
	int32_t myMaxStop = 0;
	QueueHandle_t myCommandQueue;
};