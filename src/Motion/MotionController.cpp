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
#include <cmath>
#include <cstdio>
#include <semphr.h>

MotionController::MotionController(Stepper *anXStepper, Stepper *aZStepper, Stepper *aYStepper)
{
	self = this;
	printf("MotionController\n");
	myAxes[AxisLabel::X] = new Axis(anXStepper, AxisLabel::X);
	myAxes[AxisLabel::Z] = new Axis(aZStepper, AxisLabel::X);

	myAxes[AxisLabel::X]->SetMaxStop(1000);
	myAxes[AxisLabel::X]->SetMinStop(-1000);
	myAxes[AxisLabel::Z]->SetMaxStop(250);
	myAxes[AxisLabel::Z]->SetMinStop(-250);

	myZTriggerSemaphore = xSemaphoreCreateBinary();

	myZAxisSM = new ZAxisSM(myAxes[AxisLabel::Z]);
	myXAxisSM = new XAxisSM(myAxes[AxisLabel::X], myZAxisSM);

	// for future expansion
	if (aYStepper != nullptr)
	{
		panic("Y Stepper not implemented");
		// myAxes[AxisLabel::Y] = new Axis(aYStepper);
	}

	BaseType_t status = xTaskCreate(MotionXThread, "MotionXThread", 1 * 2048, this, 1, NULL);

	if (status != pdPASS)
	{
		panic("Failed to create MotionXThread\n");
	}

	printf("MotionController done\n");
}

void MotionController::MotionXThread(void *pvParameters)
{
	printf("MotionXThread Started\n");
	MotionController *mc = static_cast<MotionController *>(pvParameters);

	while (true)
	{
		mc->myXAxisSM->Update();
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