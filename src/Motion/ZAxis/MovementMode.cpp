#include "MovementMode.hpp"
#include <cstdio>

void ZMoveBothEnds::Execute(Axis *aZAxis, ZAxisSM *aZAxisSM)
{

	/* wait for X axis to stop moving and tell Z to advance (manual mode to do the same)*/
	BaseType_t res = xTaskNotifyWait(0, 0, NULL, 20 * portTICK_PERIOD_MS);
	if (res == pdFALSE)
	{
		return;
	}
	
	Axis *axis = aZAxis;
	int32_t minStop = axis->GetMinStop();
	int32_t maxStop = axis->GetMaxStop();
	AxisDirection direction = axis->GetDirection();
	AxisStop zStop = axis->IsAtStop();

	if (zStop == AxisStop::MIN || axis->GetDirection() == AxisDirection::POS)
	{
		axis->SetTargetPosition(axis->GetPosition() + aZAxisSM->GetAdvanceIncrement());
	}
	else if (zStop == AxisStop::MAX || axis->GetDirection() == AxisDirection::NEG)
	{
		axis->SetTargetPosition(axis->GetPosition() - aZAxisSM->GetAdvanceIncrement());
	}
	
	axis->IsMovementComplete();

	if (PRINTF_AXIS_POSITIONS)
	{
		printf("Z: %d\n", axis->GetPosition());
	}
}