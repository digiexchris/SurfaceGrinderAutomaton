#include "MotionController.hpp"
#include "Axis.hpp"
#include "mpu_wrappers.h"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include <Enum.hpp>
#include <cstdio>

MotionController::MotionController(Stepper *anXStepper, Stepper *aZStepper, Stepper *aYStepper)
{
	self = this;
	printf("MotionController\n");
	myAxes[AxisLabel::X] = new Axis(anXStepper, myXCommandQueue);
	// myAxes[AxisLabel::Z] = new Axis(aZStepper, myZCommandQueue);

	// for future expansion
	if (aYStepper != nullptr)
	{
		assert("Y Stepper not implemented");
		// myAxes[AxisLabel::Y] = new Axis(aYStepper);
	}

	BaseType_t status = xTaskCreate(MotionXThread, "MotionXThread", 2048, self, 4, NULL);

	if (status != pdPASS)
	{
		printf("Failed to create MotionXThread\n");
	}

	printf("MotionController done\n");
}

void MotionController::StartX()
{
}

void MotionController::MotionXThread(void *pvParameters)
{
	printf("MotionXThread\n");
	MotionController *mc = static_cast<MotionController *>(pvParameters);
	Axis *axis = mc->myAxes[AxisLabel::X];

	axis->SetMaxStop(1000);
	axis->SetMinStop(-1000);
	while (true)
	{
		int32_t minStop = axis->GetMinStop();
		int32_t maxStop = axis->GetMaxStop();
		printf("minStop: %d, maxStop: %d\n", minStop, maxStop);
		if (mc->myXMotionState == MotionState::AUTOMATIC)
		{
			AxisState state = axis->GetState();
			// todo: just making a basic cycler here for the first iteration
			if (state == AxisState::STOPPED)
			{
				if (axis->GetPreviosDirection() == AxisDirection::POS)
				{
					int32_t distance = minStop - axis->GetPosition();
					axis->Move(distance);
				}
				else
				{
					axis->Move(maxStop - axis->GetPosition());
				}
			}
			else if (state == AxisState::LOCKED)
			{
				continue;
			}
			else
			{
				vTaskDelay(1000 / portTICK_PERIOD_MS);
			}
		}
	}
}