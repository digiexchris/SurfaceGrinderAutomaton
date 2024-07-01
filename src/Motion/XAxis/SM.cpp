#include "SM.hpp"

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
				myTargetPosition = maxStop;
			}
			else if (myAxis->GetPosition() >= maxStop)
			{
				myTargetPosition = minStop;
			}
			else
			{
				AxisDirection direction = myAxis->GetDirection();

				if (direction == AxisDirection::POS)
				{
					myTargetPosition = maxStop;
				}
				else
				{
					myTargetPosition = minStop;
				}
			}
		}
	}

		// both auto and manual move to the target the same way so it's not breaking here

	case AxisMode::MANUAL:
	{
		auto pos = myAxis->GetPosition();
		if (pos != myTargetPosition)
		{
			if (pos > myTargetPosition)
			{
				myAxis->SetDirection(AxisDirection::NEG);
			}
			else
			{
				myAxis->SetDirection(AxisDirection::POS);
			}

			myAxis->Move(std::abs(myTargetPosition - pos), mySpeed);

			/* tiny bit of time for everything to settle, but considering
				we're pausing to wait for the Z traverse, this could probably
				be very short or even zero*/
			myAxis->Wait(100);
			taskYIELD();
			myAxis->IsMovementComplete();

			if (PRINTF_AXIS_POSITIONS)
			{
				printf("X: %d\n", myAxis->GetPosition());
			}
		}
	}
	break;

	// stopped doesn't move at all
	case AxisMode::STOPPED:
	default:
		break;
	}

	xTaskNotifyGive(myMotionController->GetTaskHandle(AxisLabel::Z));
	xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
	vTaskDelay(1 * portTICK_PERIOD_MS);
}