#include "MotionController.hpp"
#include "Axis.hpp"
#include "mpu_wrappers.h"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include <Enum.hpp>
#include <cstdio>

MotionController::MotionController(Stepper *anXStepper, Stepper *aZStepper, Stepper *aYStepper)
{
	self = this;
	printf("MotionController\n");
	myAxes[AxisLabel::X] = new Axis(anXStepper);
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

		// printf("minStop: %d, maxStop: %d\n", minStop, maxStop);
		if (mc->myXMotionState == MotionState::AUTOMATIC)
		{
			int32_t minStop = axis->GetMinStop();
			int32_t maxStop = axis->GetMaxStop();
			uint8_t queueCount = axis->GetQueueSize();
			// todo: change this into blocking on a semaphore in Axis instead, no need to have it churn
			if (axis->IsMovementComplete(100 * portTICK_PERIOD_MS)) // blocks until true, or false if timeout
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

				axis->Wait(1000);
			}
			else
			{
				vTaskDelay(100 / portTICK_PERIOD_MS);
			}
		}
	}
}