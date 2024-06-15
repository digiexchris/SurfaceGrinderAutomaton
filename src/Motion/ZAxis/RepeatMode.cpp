#include "RepeatMode.hpp"
#include "../../Axis.hpp"
#include "SM.hpp"
#include <cmath>
#include <cstdio>

bool ZRepeatReverse::Execute(Axis *aZAxis, ZAxisSM *aZAxisSM, bool &outMoved)
{

	WAIT_FOR_ONE_SHOT_TRIGGER();

	Axis *axis = aZAxis;
	int32_t minStop = axis->GetMinStop();
	int32_t maxStop = axis->GetMaxStop();
	AxisDirection direction = axis->GetPreviousDirection();
	AxisStop zStop = axis->IsAtStop();

	if (zStop == AxisStop::MIN && direction == AxisDirection::NEG)
	{
		direction = AxisDirection::POS;
	}
	else if (zStop == AxisStop::MAX && direction == AxisDirection::POS)
	{
		direction = AxisDirection::NEG;
	}
	else if (zStop == AxisStop::NEITHER)
	{
		// not at a stop, nothing to do
		outMoved = false;
		return true;
	}

	// Don't move, just change direction. It'll probably air cut, but otherwise I'd have to know about the Z increment here. I'll do it later if this is annoying or detrimental.
	axis->Move(0, direction, ZAXIS_MAX_SPEED);

	axis->IsMovementComplete(); // wait till it's done

	if (PRINTF_AXIS_POSITIONS)
	{
		printf("Z: %d\n", axis->GetPosition());
	}

	outMoved = true; // this type always results in a move
	return true;
}

bool ZNoRepeat::Execute(Axis *aZAxis, ZAxisSM *aZAxisSM, bool &outMoved)
{
	WAIT_FOR_ONE_SHOT_TRIGGER();

	Axis *axis = aZAxis;
	int32_t minStop = axis->GetMinStop();
	int32_t maxStop = axis->GetMaxStop();
	AxisDirection direction = axis->GetPreviousDirection();
	AxisStop zStop = axis->IsAtStop();

	if (zStop == AxisStop::NEITHER)
	{
		// not at a stop, nothing to do
		outMoved = false;
		return true;
	}

	if (zStop == AxisStop::MIN && direction == AxisDirection::NEG)
	{
		// stop, we're at the destination
		aZAxisSM->SetMode(AxisMode::STOPPED);
	}
	else if (zStop == AxisStop::MAX && direction == AxisDirection::POS)
	{
		// stop, we're at the destination
		aZAxisSM->SetMode(AxisMode::STOPPED);
	}

	if (PRINTF_AXIS_POSITIONS)
	{
		printf("Z: %d\n", axis->GetPosition());
	}

	outMoved = false; // this type always results in no move
	return true;
}

bool ZRepeatAtStart::Execute(Axis *aZAxis, ZAxisSM *aZAxisSM, bool &outMoved)
{
	WAIT_FOR_ONE_SHOT_TRIGGER();

	Axis *axis = aZAxis;
	int32_t minStop = axis->GetMinStop();
	int32_t maxStop = axis->GetMaxStop();
	AxisDirection direction = axis->GetPreviousDirection();
	AxisStop zStop = axis->IsAtStop();

	outMoved = false;

	if (zStop == AxisStop::NEITHER)
	{
		// not at a stop, nothing to do
		outMoved = false;
		return true;
	}

	if (zStop == AxisStop::MIN && direction == AxisDirection::NEG)
	{
		axis->Move(std::abs(axis->GetMaxStop() - axis->GetPosition()), AxisDirection::POS, ZAXIS_MAX_SPEED);
		outMoved = true;
	}
	else if (zStop == AxisStop::MAX && direction == AxisDirection::POS)
	{
		axis->Move(std::abs(axis->GetMinStop() - axis->GetPosition()), AxisDirection::NEG, ZAXIS_MAX_SPEED);
		outMoved = true;
	}

	axis->IsMovementComplete();

	if (PRINTF_AXIS_POSITIONS)
	{
		printf("Z: %d\n", axis->GetPosition());
	}

	return true;
}

/**





	else if (myRepeatZType == RepeatZType::START_AT_SAME_POSITION)
	{
		if (zStop == AxisStop::MIN && direction == AxisDirection::NEG)
		{
			axis->Move(std::abs(axis->GetMaxStop() - axis->GetPosition()), AxisDirection::POS, ZAXIS_MAX_SPEED);
		}
		else if (zStop == AxisStop::MAX && direction == AxisDirection::POS)
		{
			axis->Move(std::abs(axis->GetMinStop() - axis->GetPosition()), AxisDirection::NEG, ZAXIS_MAX_SPEED);
		}
		else
		{
			axis->Move(myZAdvanceRate, direction, ZAXIS_MAX_SPEED);
		}
	}




 */