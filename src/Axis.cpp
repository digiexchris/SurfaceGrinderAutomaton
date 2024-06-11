#include "Axis.hpp"
#include "Enum.hpp"
#include "config.hpp"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include <cmath>
#include <cstdio>
#include <semphr.h>

Axis::Axis(Stepper *aStepper, AxisLabel anAxisLabel)
{
	myStepper = aStepper;
	myAxisLabel = anAxisLabel;

	myCommandQueue = xQueueCreate(10, sizeof(AxisCommand *));
	myStateMutex = xSemaphoreCreateMutex();
	myQueueIsProcessing = xSemaphoreCreateBinary();
	myDirectionMutex = xSemaphoreCreateMutex();
	configASSERT(myStateMutex);
	configASSERT(myDirectionMutex);
	configASSERT(myQueueIsProcessing);

	BaseType_t status = xTaskCreate(privProcessCommandQueue, "AxisCommandThread", 2048, this, 4, NULL);

	configASSERT(status == pdPASS);

	xSemaphoreGive(myQueueIsProcessing); // let the first consumer add something to the empty queue
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

AxisStop Axis::IsAtStop()
{
	if (myPosition <= myMinStop)
	{
		return AxisStop::MIN;
	}
	else if (myPosition >= myMaxStop)
	{
		return AxisStop::MAX;
	}
	else
	{
		return AxisStop::NEITHER;
	}
}

AxisDirection Axis::GetPreviousDirection()
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

void Axis::Move(uint32_t aDistance, AxisDirection aDirection, uint16_t aSpeed)
{
	AxisMoveCommand *cmd = new AxisMoveCommand(aDistance, aDirection, aSpeed);
	xQueueSend(myCommandQueue, &cmd, portMAX_DELAY);
	printf("Axis %d: Move %d queued\n", myAxisLabel, aDistance);
}

uint8_t Axis::GetQueueSize()
{
	return uxQueueMessagesWaiting(myCommandQueue);
}

bool Axis::IsMovementComplete(TickType_t aTimeout)
{
	if (uxQueueMessagesWaiting(myCommandQueue) == 0)
	{
		return true;
	}

	BaseType_t status = xQueueSemaphoreTake(myQueueIsProcessing, aTimeout);

	if (status == pdTRUE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Axis::privProcessCommandQueue(void *pvParameters)
{
	Axis *axis = static_cast<Axis *>(pvParameters);
	while (true)
	{
		AxisCommand *command;
		if (xQueueReceive(axis->myCommandQueue, &command, portMAX_DELAY) == pdTRUE)
		{
			switch (command->cmd)
			{
			case AxisCommandName::MOVE:
			{
				AxisMoveCommand *moveCommand = static_cast<AxisMoveCommand *>(command);
				axis->privMove(moveCommand->distance, moveCommand->direction, moveCommand->speed);
				break;
			}
			case AxisCommandName::WAIT:
			{
				xSemaphoreTake(axis->myStateMutex, portMAX_DELAY);
				axis->myState = AxisState::WAITING;
				xSemaphoreGive(axis->myStateMutex);
				AxisWaitCommand *waitCommand = static_cast<AxisWaitCommand *>(command);
				vTaskDelay(waitCommand->durationMs / portTICK_PERIOD_MS);

				xSemaphoreTake(axis->myStateMutex, portMAX_DELAY);
				axis->myState = AxisState::STOPPED;
				xSemaphoreGive(axis->myStateMutex);
				break;
			}
			default:
				printf("Unknown command");
				configASSERT(false);
			}

			delete command; // Free the command after processing
		}

		// queue is empty, inform whoever cares
		if (uxQueueMessagesWaiting(axis->myCommandQueue) == 0)
		{
			xSemaphoreGive(axis->myQueueIsProcessing);
		}
	}
}

void Axis::privMove(uint32_t aDistance, AxisDirection aDirection, uint16_t aSpeed)
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

	myDirection = aDirection;

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
		vTaskDelay(STEPPER_DIRECTION_CHANGE_DELAY_MS * portTICK_PERIOD_MS);
	}
	myStepper->Move(aDistance, aSpeed);

	if(aDirection == AxisDirection::POS)
	{
		myPosition += aDistance;
	}	
	else
	{
		myPosition -= aDistance;
	}

	xSemaphoreTake(myStateMutex, portMAX_DELAY);
	myState = AxisState::STOPPED;
	xSemaphoreGive(myStateMutex);
}

void Axis::Wait(int32_t aDurationMs)
{
	AxisWaitCommand *cmd = new AxisWaitCommand(aDurationMs);
	xQueueSend(myCommandQueue, &cmd, portMAX_DELAY);
	printf("Axis %d: Wait %d queued\n", myAxisLabel, aDurationMs);
}
