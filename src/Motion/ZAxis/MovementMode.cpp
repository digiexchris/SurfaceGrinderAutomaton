#include "MovementMode.hpp"
#include <cstdio>

void ZMoveBothEnds::Execute(Axis *aZAxis, ZAxisSM *aZAxisSM)
{

	// todo strip the repeat out of this
	Axis *axis = aZAxis;
	int32_t minStop = axis->GetMinStop();
	int32_t maxStop = axis->GetMaxStop();
	AxisDirection direction = axis->GetDirection();
	AxisStop zStop = axis->IsAtStop();

	if (zStop == AxisStop::MIN && direction == AxisDirection::NEG)
	{
		// up against a stop, gotta wait for the next cycle for the Reverse mode to switch direction
		return;
	}
	else if (zStop == AxisStop::MAX && direction == AxisDirection::POS)
	{
		// up against a stop, gotta wait for the next cycle for the Reverse mode to switch direction
		return;
	}
	axis->Move(aZAxisSM->GetAdvanceIncrement(), ZAXIS_MAX_SPEED);

	axis->IsMovementComplete();

	if (PRINTF_AXIS_POSITIONS)
	{
		printf("Z: %d\n", axis->GetPosition());
	}
}