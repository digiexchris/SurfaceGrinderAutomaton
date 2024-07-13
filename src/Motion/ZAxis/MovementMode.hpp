#pragma once
#include "SM.hpp"

enum class MovementMode
{
	TRIGGER_BOTH_ENDS,
	TRIGGER_LEFT_END,
	TRIGGER_RIGHT_END,
	CONSTANT,
	MANUAL
};

class ZMoveBothEnds : public ZAxisSM::IMovementMode
{
public:
	ZMoveBothEnds(Axis *anAxis, ZAxisSM *aZAxisSM) : IMovementMode(anAxis, aZAxisSM) {}
	virtual void Reset() override
	{
		if (myPreviousAxisStopTarget == AxisStop::NEITHER)
		{
			// set the starting direction to move in the direction of the closest stop
			int32_t position = myAxis->GetCurrentPosition();
			if (myAxis->GetMinStop() - position < position - myAxis->GetMaxStop())
			{
				direction = AxisDirection::POS;
			}
			else
			{
				direction = AxisDirection::NEG;
			}
		}
		else
		{
			direction = myPreviousAxisStopTarget == AxisStop::MIN ? AxisDirection::POS : AxisDirection::NEG;
		}
	};
	virtual void Execute() override;

private:
	AxisDirection direction = AxisDirection::POS;
	AxisStop myPreviousAxisStopTarget = AxisStop::NEITHER;
};

class ZMoveConstant : public ZAxisSM::IMovementMode
{
public:
	virtual void Execute() override;
	virtual void Reset() override {};
};