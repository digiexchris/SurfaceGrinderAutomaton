#include "SM.hpp"
#include "Enum.hpp"

#include <cmath>
#include <cstdio>
#include <pico/printf.h>
#include <task.h>

void XAxisSM::Update()
{
	switch (myAxisMode)
	{
	case AxisMode::AUTOMATIC:
	{
		if (myAxis->IsMovementComplete()) // blocks until true, or false if timeout is set
		{
			int32_t minStop = myAxis->GetMinStop();
			int32_t maxStop = myAxis->GetMaxStop();

			if (myAxis->GetPosition() <= minStop)
			{
				myAxis->SetTargetPosition(maxStop);
			}
			else if (myAxis->GetPosition() >= maxStop)
			{
				myAxis->SetTargetPosition(minStop);
			}
			else
			{
				//it's somewhere in between
				AxisDirection direction = myAxis->GetDirection();

				if (direction == AxisDirection::POS)
				{
					myAxis->SetTargetPosition(maxStop);
				}
				else
				{
					myAxis->SetTargetPosition(minStop);
				}
			}
			myAxis->IsMovementComplete();

			if (PRINTF_AXIS_POSITIONS)
			{
				printf("X Update: X: %d\n", myAxis->GetPosition());
			}

			// This is only correct if Z is in auto mode. If Z is in manual mode or a continuous advance type, it should not wait on X.
			// maybe we need another enum to indicate this axis and another axis are sychronized or not.
			xTaskNotifyGive(myMotionController->GetTaskHandle(AxisLabel::Z));
			xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
			vTaskDelay(1 * portTICK_PERIOD_MS);
		}
	}
	break;

	// stopped doesn't move at all
	// manual will be handled via an encoder callback or console command
	case AxisMode::MANUAL:
	case AxisMode::STOPPED:
	default:
		break;
	}


}