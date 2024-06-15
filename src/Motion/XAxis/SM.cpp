#include "SM.hpp"

#include <cmath>
#include <cstdio>

XAxisSM::XAxisSM(Axis *anAxis, ZAxisSM *aZAxisSM) : myZAxisSM(aZAxisSM), MotionControllerSM(anAxis) {}

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
				myIsAtStop = AxisStop::MIN;
			}
			else if (myAxis->GetPosition() >= maxStop)
			{
				myIsAtStop = AxisStop::MAX;
			}
			else
			{
				myIsAtStop = AxisStop::NEITHER;
			}

			myZAxisSM->Update();

			// TODO: this is a good place to trigger the Z execute call, passing in X's stop and position. It probably doesn't need separate threads??

			AxisDirection direction = myAxis->GetPreviousDirection();

			/* Traverse to the opposite stop
			side effect: if you stop it mid-pass it will reverse immediately. fine for now, fix it later*/
			if (direction == AxisDirection::POS)
			{
				int32_t distance = minStop - myAxis->GetPosition();
				myAxis->Move(std::abs(distance), AxisDirection::NEG, XAXIS_MAX_SPEED);
			}
			else
			{
				int32_t distance = maxStop - myAxis->GetPosition();
				myAxis->Move(std::abs(distance), AxisDirection::POS, XAXIS_MAX_SPEED);
			}

			/* tiny bit of time for everything to settle, but considering
			we're pausing to wait for the Z traverse, this could probably
			be very short or even zero*/
			myAxis->Wait(50);

			if (PRINTF_AXIS_POSITIONS)
			{
				printf("X: %d\n", myAxis->GetPosition());
			}
		}
	}
	break;

	case AxisMode::MANUAL:
	case AxisMode::STOPPED:
	default:
		myZAxisSM->Update();
		break;
	}
}