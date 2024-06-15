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

	BaseType_t status = xTaskCreate(MotionXThread, "MotionXThread", 1 * 2048, this, 4, NULL);

	if (status != pdPASS)
	{
		panic("Failed to create MotionXThread\n");
	}

	printf("MotionController done\n");
}

void MotionController::MotionXThread(void *pvParameters)
{
	printf("MotionXThread\n");
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
		myZAxisSM->SetMode(aMode);
		break;
	case AxisLabel::Z:
		myXAxisSM->SetMode(aMode);
		break;
	default:
		return false;
	}
	return true;
}

AxisMode MotionController::GetMode(AxisLabel anAxisLabel)
{
	switch (anAxisLabel)
	{
	case AxisLabel::X:
		return myZAxisSM->GetMode();
	case AxisLabel::Z:
		return myXAxisSM->GetMode();
	default:
		return AxisMode::ERROR;
	}
}