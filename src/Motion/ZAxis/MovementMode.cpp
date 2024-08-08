#include "MovementMode.hpp"
#include "Helpers.hpp"
#include <cstdio>

void ZMoveBothEnds::Execute()
{

	/* wait for X axis to stop moving and tell Z to advance (manual mode to do the same)*/
	BaseType_t res = xTaskNotifyWait(0, 0, NULL, MS_TO_TICKS(20));
	if (res == pdFALSE)
	{
		return;
	}

	int32_t minStop = myAxis->GetMinStop();
	int32_t maxStop = myAxis->GetMaxStop();
	AxisStop zStop = myAxis->IsAtStop();

	if (zStop == AxisStop::MIN)
	{
		myAxis->SetTargetPosition(myAxis->GetCurrentPosition() + myZAxisSM->GetAdvanceIncrement());
		myPreviousAxisStopTarget = AxisStop::MAX;
	}
	else if (zStop == AxisStop::MAX)
	{
		myAxis->SetTargetPosition(myAxis->GetCurrentPosition() - myZAxisSM->GetAdvanceIncrement());
		myPreviousAxisStopTarget = AxisStop::MIN;
	}
	else
	{
		if (direction == AxisDirection::POS)
		{
			myAxis->SetTargetPosition(myAxis->GetCurrentPosition() + myZAxisSM->GetAdvanceIncrement());
			myPreviousAxisStopTarget = AxisStop::MAX;
		}
		else
		{
			myAxis->SetTargetPosition(myAxis->GetCurrentPosition() - myZAxisSM->GetAdvanceIncrement());
			myPreviousAxisStopTarget = AxisStop::MIN;
		}
	}

	myAxis->WaitUntilMovementComplete();

	if (PRINTF_AXIS_POSITIONS)
	{
		printf("Z: %d\n", myAxis->GetCurrentPosition());
	}
}