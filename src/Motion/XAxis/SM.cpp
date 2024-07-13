#include "SM.hpp"
#include "Enum.hpp"
#include "Helpers.hpp"

#include <cmath>
#include <cstdio>
#include <pico/printf.h>
#include <task.h>

void XAxisSM::ResetAutoMode()
{
}

void XAxisSM::Update()
{
	switch (myAxisMode)
	{
	case AxisMode::AUTOMATIC:
	{
		if (myAxis->WaitUntilMovementComplete()) // blocks until true, or false if timeout is set
		{
			int32_t minStop = myAxis->GetMinStop();
			int32_t maxStop = myAxis->GetMaxStop();

			if (myAxis->GetCurrentPosition() <= minStop)
			{
				myAxis->SetTargetPosition(maxStop);
				myPreviousAxisStop = AxisStop::MAX;
			}
			else if (myAxis->GetCurrentPosition() >= maxStop)
			{
				myAxis->SetTargetPosition(minStop);
				myPreviousAxisStop = AxisStop::MIN;
			}
			else
			{
				if (myPreviousAxisStop == AxisStop::NEITHER)
				{
					// move to the closest stop
					auto distanceToMin = abs(myAxis->GetCurrentPosition() - minStop);
					auto distanceToMax = abs(myAxis->GetCurrentPosition() - maxStop);
					if (distanceToMin < distanceToMax)
					{
						myAxis->SetTargetPosition(minStop);
					}
					else
					{
						myAxis->SetTargetPosition(maxStop);
					}
				}
				else
				{
					// move to the opposite stop
					if (myPreviousAxisStop == AxisStop::MIN)
					{
						myAxis->SetTargetPosition(minStop);
					}
					else
					{
						myAxis->SetTargetPosition(maxStop);
					}
				}
			}

			myAxis->WaitUntilMovementComplete();

			if (PRINTF_AXIS_POSITIONS)
			{
				printf("X Update: X: %d\n", myAxis->GetCurrentPosition());
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