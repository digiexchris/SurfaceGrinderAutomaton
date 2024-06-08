#include "Axis.hpp"
#include "config.hpp"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include <cmath>

Axis::Axis(Stepper *aStepper, QueueHandle_t aCommandQueue)
{
	myStepper = aStepper;
	myCommandQueue = aCommandQueue;
	mutex_init(myStateMutex);
	the problem is here
		mutex_init(myDirectionMutex);
}

int32_t Axis::GetPosition()
{
	return myPosition;
}

void Axis::SetMinStop(int32_t aMinStop)
{
	myMinStop = aMinStop;
}

int32_t Axis::GetMinStop()
{
	return myMinStop;
}

void Axis::SetMaxStop(int32_t aMaxStop)
{
	myMaxStop = aMaxStop;
}

int32_t Axis::GetMaxStop()
{
	return myMaxStop;
}

void Axis::SetPosition(int32_t aPosition)
{
	// myStepper->SetPosition(aPosition);
}

AxisState Axis::GetState()
{
	uint32_t timeout = 10;
	if (!mutex_try_enter(myStateMutex, &timeout))
	{
		return AxisState::LOCKED;
	}
	return myState;
}

AxisDirection Axis::GetPreviosDirection()
{
	uint32_t timeout = 10;
	if (!mutex_try_enter(myDirectionMutex, &timeout))
	{
		return AxisDirection::LOCKED;
	}
	return myPreviousDirection;
}

void Axis::Move(int32_t aDistance)
{
	mutex_enter_blocking(myStateMutex);
	myState = AxisState::MOVING;
	mutex_exit(myStateMutex);

	if (myPreviousDirection != myDirection)
	{
		mutex_enter_blocking(myDirectionMutex);
		if (myDirection == AxisDirection::POS)
		{

			myPreviousDirection = AxisDirection::POS;

			myStepper->SetDirection(true);
		}
		else
		{
			myPreviousDirection = AxisDirection::NEG;
			myStepper->SetDirection(false);
		}
		mutex_exit(myDirectionMutex);
		// TODO this seems high, maybe it's 5uS? no wiat make it configurable
		vTaskDelay(STEPPER_DIRECTION_CHANGE_DELAY_MS / portTICK_PERIOD_MS);
	}

	myStepper->Move(std::abs(aDistance));

	myPosition += aDistance;

	mutex_enter_blocking(myStateMutex);
	myState = AxisState::STOPPED;
	mutex_exit(myStateMutex);
}

void Axis::CommandThread(void *pvParameters)
{
	Axis *axis = static_cast<Axis *>(pvParameters);
	while (true)
	{

		AxisCommand command;
		if (xQueueReceive(axis->myCommandQueue, &command, portMAX_DELAY) == pdTRUE)
		{
			switch (command.cmd)
			{
			case AxisCommandName::MOVE:
				axis->Move(*static_cast<int32_t *>(command.data));
				break;
			default:
				assert("Unknown command");
			}
		}
	}
}