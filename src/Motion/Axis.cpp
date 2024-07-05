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

	// myCommandQueue = xQueueCreate(10, sizeof(AxisCommand *));
	myStateMutex = xSemaphoreCreateMutex();
	myMoveInProgress = xSemaphoreCreateBinary();
	myReadyForNextMove = xSemaphoreCreateBinary();
	myDirectionMutex = xSemaphoreCreateMutex();
	configASSERT(myStateMutex);
	configASSERT(myDirectionMutex);
	configASSERT(myMoveInProgress);

	BaseType_t status = xTaskCreate(MoveThread, "AxisMove", 2048, this, 1, NULL);

	configASSERT(status == pdPASS);

	xSemaphoreGive(myMoveInProgress); // let the first consumer add something to the empty queue
	xSemaphoreGive(myStateMutex);
	// xSemaphoreGive(myDirectionMutex);
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

//TODO bump this up to the SM, the axis just tracks target positions
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

void Axis::SetTargetPosition(int32_t aPosition)
{
	//TODO I don't know what to do here. Maybe uneccessary. xQueueSemaphoreTake(myQueueIsProcessing, 0);
	xSemaphoreTake(myMoveInProgress, portMAX_DELAY);
	myTargetPosition = aPosition;
	if(myTargetPosition >= myMaxStop)
	{
		myTargetPosition = myMaxStop;
	}
	else if(myTargetPosition <= myMinStop)
	{
		myTargetPosition = myMinStop;
	}
	xSemaphoreGive(myReadyForNextMove);
	xSemaphoreGive(myMoveInProgress);
}

//takes affect on new moves only, not currently in progress ones
void Axis::SetSpeed(uint16_t aSpeed)
{
	myMaxSpeed = aSpeed;
}

int16_t Axis::GetSpeed()
{
	return myMaxSpeed;
}

bool Axis::IsMovementComplete(TickType_t aTimeout)
{
	BaseType_t status = xQueueSemaphoreTake(myMoveInProgress, aTimeout);

	if (status == pdTRUE)
	{
		xSemaphoreGive(myMoveInProgress);
		return true;
	}
	else
	{
		return false;
	}
}

void Axis::MoveThread(void *pvParameters)
{
	Axis *axis = static_cast<Axis *>(pvParameters);
	while(true)
	{
		//block the thread until something updates the target position, but fail through incase we've e-stopped or something.
		auto status = xSemaphoreTake(axis->myReadyForNextMove, 100);
		if (status != pdTRUE)
		{
			continue;
		}

		xSemaphoreTake(axis->myMoveInProgress, portMAX_DELAY);
		if(axis->myTargetPosition != axis->myPosition)
		{

			xSemaphoreTake(axis->myStateMutex, portMAX_DELAY);
			axis->myState = AxisState::MOVING;
			xSemaphoreGive(axis->myStateMutex);

			xSemaphoreTake(axis->myDirectionMutex, portMAX_DELAY);
			AxisDirection aDirection = axis->myDirection;
			bool directionChanged = false;
			if(axis->myTargetPosition > axis->myPosition)
			{
				axis->myPreviousDirection = axis->myDirection;
				aDirection = AxisDirection::POS;
				axis->myStepper->SetDirection((bool)aDirection);
				directionChanged = true;
			}
			else
			{
				axis->myPreviousDirection = axis->myDirection;
				aDirection = AxisDirection::NEG;
				axis->myStepper->SetDirection((bool)aDirection);
				directionChanged = true;
			}
			xSemaphoreGive(axis->myDirectionMutex);

			if(directionChanged) {
				//wait for the stepper driver to finish changing direction
				axis->myStepper->DirectionChangedWait();
			}

			int32_t aDistance = axis->myTargetPosition - axis->myPosition;
			if(aDistance != 0) {
				axis->myStepper->Move(std::abs(aDistance), axis->myMaxSpeed);
				axis->myPosition += aDistance;
			}

			xSemaphoreTake(axis->myStateMutex, portMAX_DELAY);
			axis->myState = AxisState::STOPPED;
			xSemaphoreGive(axis->myStateMutex);
		}
		xSemaphoreGive(axis->myMoveInProgress);
	}
}

//nah the SM can do this.
// void Axis::Wait(int32_t aDurationMs)
// {
// 	AxisWaitCommand *cmd = new AxisWaitCommand(aDurationMs);
// 	xQueueSend(myCommandQueue, &cmd, portMAX_DELAY);
// 	if (PRINTF_AXIS_DEBUG)
// 	{
// 		printf("Axis %d: Wait %d queued\n", myAxisLabel, aDurationMs);
// 	}
// }

//no the move thread can do this
// void Axis::privSetDirection(AxisDirection aDirection)
// {
// 	xSemaphoreTake(myDirectionMutex, portMAX_DELAY);
// 	if (myDirection != aDirection)
// 	{7
// 		myPreviousDirection = myDirection;
// 		myDirection = aDirection;
// 		myStepper->SetDirection((bool)myDirection);
// 		vTaskDelay(STEPPER_DIRECTION_CHANGE_DELAY_MS * portTICK_PERIOD_MS);
// 	}
// 	xSemaphoreGive(myDirectionMutex);
// }
