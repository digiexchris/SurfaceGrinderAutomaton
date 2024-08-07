#include "Motion/Axis.hpp"
#include "Console/WebSerial.hpp"
#include "Enum.hpp"
#include "portmacro.h"
#include <cmath>
#include <pico/printf.h>
#include <semphr.h>
#include <stdio.h>
#include <task.h>

Axis::Axis(AxisLabel anAxisLabel, uint stepPin, uint dirPin, float maxSpeed, float acceleration, PIO pio, uint sm)
	: myAxisLabel(anAxisLabel), Stepper(stepPin, dirPin, 0, acceleration, pio, sm)

{
	myMaxSpeed = maxSpeed;
	auto axisThreadName = "AxisMove" + std::to_string(static_cast<int>(anAxisLabel));
	BaseType_t status = xTaskCreate(MoveThread, axisThreadName.c_str(), 2048, this, 1, NULL);
	configASSERT(status == pdPASS);
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

void Axis::MoveTo(int32_t aPosition, uint16_t aSpeed)
{
	if (aPosition > myMaxStop)
	{
		aPosition = myMaxStop;
	}
	else if (aPosition < myMinStop)
	{
		aPosition = myMinStop;
	}

	SetTargetPosition(aPosition);

	if (GetTargetSpeed() != aSpeed)
	{
		SetTargetSpeed(aSpeed);
	}
}

void Axis::MoveRelative(int32_t aDistance, uint16_t aSpeed)
{
	int32_t targetPosition = GetTargetPosition() + aDistance;
	MoveTo(targetPosition, aSpeed);
}

AxisStop Axis::IsAtStop()
{
	if (GetCurrentPosition() <= myMinStop)
	{
		return AxisStop::MIN;
	}
	else if (GetCurrentPosition() >= myMaxStop)
	{
		return AxisStop::MAX;
	}
	else
	{
		return AxisStop::NEITHER;
	}
}

bool Axis::WaitUntilMovementComplete(TickType_t aTimeout)
{
	TickType_t currentTime = xTaskGetTickCount();
	while (GetMoveState() != Stepper::MoveState::IDLE)
	{
		if ((xTaskGetTickCount() - currentTime) > aTimeout)
		{
			return false;
		}
		// wait the shortest step width before checking again
		vTaskDelay(1 / GetTargetSpeed() * portTICK_PERIOD_MS);
	}

	return true;
}

StepperError Axis::SetTargetSpeed(uint16_t aSpeed)
{
	if (aSpeed > myMaxSpeed)
	{
		aSpeed = myMaxSpeed;
	}
	return Stepper::SetTargetSpeed(aSpeed);
}

void Axis::MoveThread(void *pvParameters)
{
	Axis *axis = static_cast<Axis *>(pvParameters);
	TickType_t wake;
	wake = xTaskGetTickCount();
	
	while (true)
	{
		axis->Update();
		
		//run this loop at exactly 10khz
		xTaskDelayUntil(&wake, 1);
	}
}

void Axis::ProcessStepperNotification(StepperNotifyMessage *aMessage)
{
	WebSerialAxisUpdate message(myAxisLabel, AxisParameter::CURRENT_POSITION, aMessage->currentPosition);
	WebSerial::GetInstance()->QueueUpdate(message);

	message.param = AxisParameter::CURRENT_SPEED;
	message.value = aMessage->currentSpeed;
	WebSerial::GetInstance()->QueueUpdate(message);

	message.param = AxisParameter::TARGET_POSITION;
	message.value = aMessage->targetPosition;
	WebSerial::GetInstance()->QueueUpdate(message);

	message.param = AxisParameter::TARGET_SPEED;
	message.value = aMessage->targetSpeed;
	WebSerial::GetInstance()->QueueUpdate(message);
}