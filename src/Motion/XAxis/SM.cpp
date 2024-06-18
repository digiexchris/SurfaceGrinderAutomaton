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

			/* Check and execute a Z move if necessary */
			if (myAxis->GetPosition() <= minStop)
			{
				myAxis->SetDirection(AxisDirection::POS);
			}
			else if (myAxis->GetPosition() >= maxStop)
			{
				myAxis->SetDirection(AxisDirection::NEG);
			}

			AxisDirection direction = myAxis->GetPreviousDirection();

			bool moveOccurred = false;

			/* Traverse to the opposite stop
			side effect: if you stop it mid-pass it will reverse immediately. fine for now, fix it later*/
			if (direction == AxisDirection::POS)
			{
				int32_t distance = minStop - myAxis->GetPosition();
				if (distance != 0)
				{
					myAxis->Move(std::abs(distance), XAXIS_MAX_SPEED);
					moveOccurred = true;
				}
			}
			else
			{
				int32_t distance = maxStop - myAxis->GetPosition();
				if (distance != 0)
				{
					myAxis->Move(std::abs(distance), XAXIS_MAX_SPEED);
					moveOccurred = true;
				}
			}

			/* tiny bit of time for everything to settle, but considering
			we're pausing to wait for the Z traverse, this could probably
			be very short or even zero*/
			myAxis->Wait(100);

			taskYIELD(); // if this thread is the same priority as the axis, let it move

			myAxis->IsMovementComplete();

			if (PRINTF_AXIS_POSITIONS && moveOccurred)
			{
				printf("X: %d\n", myAxis->GetPosition());
			}

			/* Notify the Z axis to advance */
			xTaskNotifyGive(myMotionController->GetTaskHandle(AxisLabel::Z));
			/* Wait for the Z axis to finish */
			xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
		}
	}
	break;

	case AxisMode::MANUAL:
	case AxisMode::STOPPED:
	default:
		break;
	}
}