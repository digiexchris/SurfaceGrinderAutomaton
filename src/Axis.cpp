#include "Axis.hpp"
#include "config.hpp"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include <cmath>
#include <cstdio>
#include <semphr.h>

Axis::Axis(Stepper *aStepper, QueueHandle_t aCommandQueue)
{
	myStepper = aStepper;
	myCommandQueue = aCommandQueue;
	myStateMutex = xSemaphoreCreateMutex();
	myDirectionMutex = xSemaphoreCreateMutex();
	configASSERT(myStateMutex);
	configASSERT(myDirectionMutex);
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
	uint32_t timeout = 10 * portTICK_PERIOD_MS;
	AxisState state;
	if (xSemaphoreTake(myStateMutex, timeout) != pdPASS)
	{
		return AxisState::LOCKED;
	}
	state = myState;
	xSemaphoreGive(myStateMutex);
	return myState;
}

AxisDirection Axis::GetPreviosDirection()
{
	uint32_t timeout = 10 * portTICK_PERIOD_MS;
	AxisDirection direction;
	if (xSemaphoreTake(myDirectionMutex, timeout) != pdPASS)
	{
		return AxisDirection::LOCKED;
	}
	direction = myPreviousDirection;
	xSemaphoreGive(myDirectionMutex);
	return direction;
}

void Axis::Move(int32_t aDistance)
{
	bool directionChanged = false;
	xSemaphoreTake(myStateMutex, portMAX_DELAY);
	myState = AxisState::MOVING;
	xSemaphoreGive(myStateMutex);

	if (aDistance == 0)
	{
		return;
	}

	xSemaphoreTake(myDirectionMutex, portMAX_DELAY);
	if (aDistance > 0)
	{
		myDirection = AxisDirection::POS;
	}
	else
	{
		myDirection = AxisDirection::NEG;
	}

	if (myPreviousDirection != myDirection)
	{

		directionChanged = true;
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
	}
	xSemaphoreGive(myDirectionMutex);

	// wait the amount of time after toggling the pin required by the driver before a step occurs
	if (directionChanged)
	{
		// TODO this seems high, maybe it's 5uS? no wiat make it configurable
		vTaskDelay(STEPPER_DIRECTION_CHANGE_DELAY_MS / portTICK_PERIOD_MS);
	}
	myStepper->Move(std::abs(aDistance));

	myPosition += aDistance;

	xSemaphoreTake(myStateMutex, portMAX_DELAY);
	myState = AxisState::STOPPED;
	xSemaphoreGive(myStateMutex);
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
				printf("Unknown command");
			}
		}
	}
}