#include "RepeatMode.hpp"
#include "../../Axis.hpp"
#include "SM.hpp"
#include <cmath>
#include <cstdio>

bool ZRepeatReverse::Execute(Axis *aZAxis, ZAxisSM *anSM, bool &outMoved)
{

	BEGIN_TRIGGER_SECTION();

	Axis *axis = aZAxis;
	int32_t minStop = axis->GetMinStop();
	int32_t maxStop = axis->GetMaxStop();
	AxisDirection direction = axis->GetPreviousDirection();
	AxisStop zStop = axis->IsAtStop();

	if (zStop == AxisStop::MIN && direction == AxisDirection::NEG)
	{
		direction = AxisDirection::POS;
		axis->SetDirection(direction);
	}
	else if (zStop == AxisStop::MAX && direction == AxisDirection::POS)
	{
		direction = AxisDirection::NEG;
		axis->SetDirection(direction);
	}

	outMoved = false; // only setting direction, not moving

	END_TRIGGER_SECTION();

	return true;
}

bool ZNoRepeat::Execute(Axis *aZAxis, ZAxisSM *anSM, bool &outMoved)
{
	BEGIN_TRIGGER_SECTION();

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
		anSM->SetMode(AxisMode::STOPPED);
	}
	else if (zStop == AxisStop::MAX && direction == AxisDirection::POS)
	{
		// stop, we're at the destination
		anSM->SetMode(AxisMode::STOPPED);
	}

	if (PRINTF_AXIS_POSITIONS && anSM->GetMode() != AxisMode::STOPPED)
	{
		printf("Z: %d\n", axis->GetPosition());
	}

	outMoved = false; // this type always results in no move

	END_TRIGGER_SECTION();
	return true;
}

bool ZRepeatAtStart::Execute(Axis *aZAxis, ZAxisSM *anSM, bool &outMoved)
{
	BEGIN_TRIGGER_SECTION();

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
		axis->SetDirection(AxisDirection::POS);
		axis->Move(std::abs(axis->GetMaxStop() - axis->GetPosition()), ZAXIS_MAX_SPEED);
		axis->SetDirection(AxisDirection::NEG);
		outMoved = true;
	}
	else if (zStop == AxisStop::MAX && direction == AxisDirection::POS)
	{
		axis->SetDirection(AxisDirection::NEG);
		axis->Move(std::abs(axis->GetMinStop() - axis->GetPosition()), ZAXIS_MAX_SPEED);
		axis->SetDirection(AxisDirection::POS);
		outMoved = true;
	}

	axis->IsMovementComplete();

	if (PRINTF_AXIS_POSITIONS && outMoved)
	{
		printf("Z: %d\n", axis->GetPosition());
	}

	END_TRIGGER_SECTION();

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