#include "MotionController.hpp"
#include "Axis.hpp"
#include "Motion/XAxis/SM.hpp"
#include "Motion/ZAxis/SM.hpp"
#include "config.hpp"
#include "mpu_wrappers.h"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include <Enum.hpp>
#include <FreeRTOS.h>
#include <cmath>
#include <cstdint>
#include <pico/printf.h>
#include <pico/stdio.h>
#include <semphr.h>
#include <task.h>

MotionController::MotionController(Stepper *anXStepper, Stepper *aZStepper, Stepper *aYStepper)
{
	printf("MotionController\n");
	myAxes[AxisLabel::X] = new Axis(anXStepper, AxisLabel::X);
	myAxes[AxisLabel::Z] = new Axis(aZStepper, AxisLabel::X);

	myAxes[AxisLabel::X]->SetMaxStop(1000);
	myAxes[AxisLabel::X]->SetMinStop(-1000);
	myAxes[AxisLabel::Z]->SetMaxStop(250);
	myAxes[AxisLabel::Z]->SetMinStop(-250);

	myZTriggerSemaphore = xSemaphoreCreateBinary();

	myZAxisSM = new ZAxisSM(myAxes[AxisLabel::Z], this);
	myXAxisSM = new XAxisSM(myAxes[AxisLabel::X], this);

	// for future expansion
	if (aYStepper != nullptr)
	{
		panic("Y Stepper not implemented");
		// myAxes[AxisLabel::Y] = new Axis(aYStepper);
	}

	myTaskHandles.emplace(AxisLabel::X, new TaskHandle_t());
	myTaskHandles.emplace(AxisLabel::Z, new TaskHandle_t());

	BaseType_t status = xTaskCreate(MotionXThread, "MotionXThread", 1 * 2048, this, 1, myTaskHandles[AxisLabel::X]);
	if (status != pdPASS)
	{
		panic("Failed to create MotionXThread\n");
	}

	status = xTaskCreate(MotionZThread, "MotionZThread", 1 * 2048, this, 1, myTaskHandles[AxisLabel::Z]);
	if (status != pdPASS)
	{
		panic("Failed to create MotionZThread\n");
	}

	printf("MotionController done\n");
}

bool MotionController::MoveRelative(AxisLabel anAxisLabel, int32_t aDistance)
{
	// simply updates the target position. The next update loop will move the axis if the manual mode is set
	// note: this is not blocking, and will override the target position of any automatic move.
	// this behaviour might be useful, but might be unwanted. TODO: figure that out, and block it if it's not in the manual mode specifically maybe.
	// it could be useful for the x axis to back it away from a collision while you're panicing, or for the Z axis to advance until the cut starts,
	// or for Y to bump the head down until it contacts the workpiece while automode is running...
	switch (anAxisLabel)
	{
	case AxisLabel::X:
		myXAxisSM->MoveRelative(aDistance);
		break;
	case AxisLabel::Z:
		myZAxisSM->MoveRelative(aDistance);
		break;
	default:
		return false;
	}
	return true;
}

AxisDirection MotionController::GetDirection(AxisLabel anAxisLabel)
{
	switch (anAxisLabel)
	{
	case AxisLabel::X:
		return myXAxisSM->GetDirection();
	case AxisLabel::Z:
		return myZAxisSM->GetDirection();
	default:
		return AxisDirection::ERROR;
	}
}

void MotionController::MotionXThread(void *pvParameters)
{
	printf("MotionXThread Started\n");
	MotionController *mc = static_cast<MotionController *>(pvParameters);

	while (true)
	{
		mc->myXAxisSM->Update();
		vTaskDelay(1); // yeild to another task of the same priority
	}
}

void MotionController::MotionZThread(void *pvParameters)
{
	printf("MotionZThread Started\n");
	MotionController *mc = static_cast<MotionController *>(pvParameters);

	while (true)
	{
		mc->myZAxisSM->Update();
		vTaskDelay(100); // yeild to another task of the same priority
	}
}

bool MotionController::SetMode(AxisLabel anAxisLabel, AxisMode aMode)
{
	switch (anAxisLabel)
	{
	case AxisLabel::X:
		myXAxisSM->SetMode(aMode);
		break;
	case AxisLabel::Z:
		myZAxisSM->SetMode(aMode);
		break;
	default:
		return false;
	}
	return true;
}

bool MotionController::SetAdvanceIncrement(AxisLabel anAxisLabel, uint32_t anIncrement)
{
	switch (anAxisLabel)
	{
	case AxisLabel::X:
		myXAxisSM->SetAdvanceIncrement(anIncrement);
		break;
	case AxisLabel::Z:
		myZAxisSM->SetAdvanceIncrement(anIncrement);
		break;
	default:
		return false;
	}
	return true;
}

uint32_t MotionController::GetAdvanceIncrement(AxisLabel anAxisLabel)
{
	switch (anAxisLabel)
	{
	case AxisLabel::X:
		return myXAxisSM->GetAdvanceIncrement();
	case AxisLabel::Z:
		return myZAxisSM->GetAdvanceIncrement();
	default:
		return 0;
	}
}

AxisMode MotionController::GetMode(AxisLabel anAxisLabel)
{
	switch (anAxisLabel)
	{
	case AxisLabel::X:
		return myXAxisSM->GetMode();
	case AxisLabel::Z:
		return myZAxisSM->GetMode();
	default:
		return AxisMode::ERROR;
	}
}

TaskHandle_t MotionController::GetTaskHandle(AxisLabel anAxisLabel)
{
	return *myTaskHandles[anAxisLabel];
}

bool MotionController::SetStop(AxisLabel anAxisLabel, AxisDirection aDirection, int32_t aPosition)
{
	switch (aDirection)
	{
	case AxisDirection::POS:
		myAxes[anAxisLabel]->SetMaxStop(aPosition);
		break;
	case AxisDirection::NEG:
		myAxes[anAxisLabel]->SetMinStop(aPosition);
		break;
	default:
		return false;
	}
	return true;
}

int32_t MotionController::GetStop(AxisLabel anAxisLabel, AxisDirection aDirection)
{
	switch (aDirection)
	{
	case AxisDirection::POS:
		return myAxes[anAxisLabel]->GetMaxStop();
	case AxisDirection::NEG:
		return myAxes[anAxisLabel]->GetMinStop();
	default:
		panic("GetStop: Invalid AxisDirection");
	}
}

int32_t MotionController::GetPosition(AxisLabel anAxisLabel)
{
	return myAxes[anAxisLabel]->GetPosition();
}

bool MotionController::SetSpeed(AxisLabel anAxisLabel, uint16_t aSpeed)
{
	switch (anAxisLabel)
	{
	case AxisLabel::X:
		myXAxisSM->SetSpeed(aSpeed);
		break;
	case AxisLabel::Z:
		myZAxisSM->SetSpeed(aSpeed);
		break;
	default:
		return false;
	}
	return true;
}

uint16_t MotionController::GetSpeed(AxisLabel anAxisLabel)
{
	switch (anAxisLabel)
	{
	case AxisLabel::X:
		return myXAxisSM->GetSpeed();
	case AxisLabel::Z:
		return myZAxisSM->GetSpeed();
	default:
		return 0;
	}
}