#include "Axis.hpp"
#include "Enum.hpp"
#include "FreeRTOS.h"
#include "config.hpp"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include <cmath>
#include <pico/printf.h>
#include <semphr.h>
#include <stdio.h>

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

	BaseType_t status = xTaskCreate(privProcessCommandQueue, "AxisCommandThread", 2048, this, 1, &myCommandQueueTask);

	configASSERT(status == pdPASS);

	xSemaphoreGive(myQueueIsProcessing); // let the first consumer add something to the empty queue
	xSemaphoreGive(myStateMutex);
	xSemaphoreGive(myDirectionMutex);
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

AxisDirection Axis::GetDirection()
{
	AxisDirection direction;
	xSemaphoreTake(myDirectionMutex, portMAX_DELAY);

	direction = myDirection;
	xSemaphoreGive(myDirectionMutex);
	return direction;
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
	AxisDirection direction;
	xSemaphoreTake(myDirectionMutex, portMAX_DELAY);
	direction = myPreviousDirection;
	xSemaphoreGive(myDirectionMutex);
	return direction;
}

void Axis::Move(uint32_t aDistance, uint16_t aSpeed)
{
	xQueueSemaphoreTake(myQueueIsProcessing, 0);
	AxisMoveCommand *cmd = new AxisMoveCommand(aDistance, aSpeed);
	if (xQueueSend(myCommandQueue, &cmd, portMAX_DELAY) != pdPASS)
	{
		printf("Failed to send move command to queue\n");
	}

	if (PRINTF_AXIS_DEBUG)
	{
		printf("Axis %d: Move %d queued\n", myAxisLabel, aDistance);
	}
}

void Axis::SetDirection(AxisDirection aDirection)
{
	xQueueSemaphoreTake(myQueueIsProcessing, 0);
	AxisSetDirectionCommand *cmd = new AxisSetDirectionCommand(aDirection);
	if (xQueueSend(myCommandQueue, &cmd, portMAX_DELAY) != pdPASS)
	{
		printf("Failed to send set direction command to queue\n");
	}
	if (PRINTF_AXIS_DEBUG)
	{
		printf("Axis %d: SetDirection %d queued\n", myAxisLabel, aDirection);
	}
}

uint8_t Axis::GetQueueSize()
{
	return uxQueueMessagesWaiting(myCommandQueue);
}

bool Axis::IsMovementComplete(TickType_t aTimeout)
{

	// if (GetQueueSize() != 0)
	// {
	// 	return false;
	// }

	BaseType_t status = xQueueSemaphoreTake(myQueueIsProcessing, aTimeout);

	if (status == pdTRUE)
	{
		xSemaphoreGive(myQueueIsProcessing);
		return true;
	}
	else
	{
		return false;
	}
}

void Axis::privProcessCommandQueue(void *pvParameters)
{
	printf("Axis Command Thread Started\n");
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
				axis->privMove(moveCommand->distance, moveCommand->speed);
				break;
			}
			case AxisCommandName::SET_DIRECTION:
			{
				AxisSetDirectionCommand *setDirectionCommand = static_cast<AxisSetDirectionCommand *>(command);
				axis->privSetDirection(setDirectionCommand->direction);
				break;
			}
			case AxisCommandName::WAIT:
			{
				xSemaphoreTake(axis->myStateMutex, portMAX_DELAY);
				axis->myState = AxisState::WAITING;
				xSemaphoreGive(axis->myStateMutex);
				AxisWaitCommand *waitCommand = static_cast<AxisWaitCommand *>(command);
				float period = portTICK_PERIOD_MS;
				if (period == 0)
				{
					panic("Period is 0");
				}
				const unsigned int timeout = waitCommand->durationMs / period;
				vTaskDelay(timeout == 0 ? 1 : timeout);

				xSemaphoreTake(axis->myStateMutex, portMAX_DELAY);
				axis->myState = AxisState::STOPPED;
				xSemaphoreGive(axis->myStateMutex);
				break;
			}
			default:
				panic("Unknown command");
				// configASSERT(false);
			}

			delete command; // Free the command after processing

			// queue is empty, inform whoever cares
		}

		if (axis->GetQueueSize() == 0)
		{
			xSemaphoreGive(axis->myQueueIsProcessing);
		}
		PrintStackHighWaterMark(axis->myCommandQueueTask);
	}
}

void Axis::privMove(uint32_t aDistance, uint16_t aSpeed)
{
	xSemaphoreTake(myStateMutex, portMAX_DELAY);
	myState = AxisState::MOVING;
	xSemaphoreGive(myStateMutex);

	if (aDistance == 0)
	{
		xSemaphoreTake(myStateMutex, portMAX_DELAY);
		myState = AxisState::STOPPED;
		xSemaphoreGive(myStateMutex);
		return;
	}

	int targetPosition = myPosition + aDistance;
	xSemaphoreTake(myDirectionMutex, portMAX_DELAY);
	AxisDirection aDirection = myDirection;
	xSemaphoreGive(myDirectionMutex);
	if (aDirection == AxisDirection::POS && targetPosition > myMaxStop)
	{
		aDistance = myMaxStop - myPosition;
	}
	else if (aDirection == AxisDirection::NEG && targetPosition < myMinStop)
	{
		aDistance = myMinStop - myPosition;
	}

	myStepper->Move(aDistance, aSpeed);

	if (aDirection == AxisDirection::POS)
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
	if (PRINTF_AXIS_DEBUG)
	{
		printf("Axis %d: Wait %d queued\n", myAxisLabel, aDurationMs);
	}
}

void Axis::privSetDirection(AxisDirection aDirection)
{
	xSemaphoreTake(myDirectionMutex, portMAX_DELAY);
	if (myDirection != aDirection)
	{
		myPreviousDirection = myDirection;
		myDirection = aDirection;
		myStepper->SetDirection((bool)myDirection);
		vTaskDelay(STEPPER_DIRECTION_CHANGE_DELAY_MS * portTICK_PERIOD_MS);
	}
	xSemaphoreGive(myDirectionMutex);
}
