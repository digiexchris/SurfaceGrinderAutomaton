#include "Axis.hpp"
#include "Enum.hpp"
#include "config.hpp"
#include "pico/stdlib.h"
#include "portmacro.h"
#include <cmath>
#include <pico/printf.h>
#include <semphr.h>
#include <stdio.h>
#include <task.h>

Axis::Axis(AxisLabel anAxisLabel, uint stepPin, uint dirPin, float maxSpeed, float acceleration, PIO pio, uint sm)
	: myAxisLabel(anAxisLabel), Stepper(stepPin, dirPin, maxSpeed, acceleration, pio, sm)

{
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

	SetTargetPosition(aPosition, aSpeed);
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

void Axis::MoveThread(void *pvParameters)
{
	Axis *axis = static_cast<Axis *>(pvParameters);
	while (true)
	{
		axis->Update();
		portYIELD();
	}
}
