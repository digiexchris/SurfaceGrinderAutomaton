#include "SM.hpp"
#include "../../Axis.hpp"
#include "MovementMode.hpp"
#include "RepeatMode.hpp"

ZAxisSM::ZAxisSM(Axis *aZAxis) : MotionControllerSM(aZAxis)
{
	// todo statically pre-allocate all types and just assign it instead
	myMovementMode = new ZMoveBothEnds();
	myRepeatMode = new ZRepeatReverse();
}

void ZAxisSM::Update()
{
	switch (myAxisMode)
	{
	case AxisMode::AUTOMATIC:
	case AxisMode::ONE_SHOT:
	{
		bool aMoveOccurred;
		bool oneShotTimedOut = !myRepeatMode->Execute(myAxis, this, aMoveOccurred);
		if (oneShotTimedOut)
		{
			return;
		}

		if (!aMoveOccurred)
		{
			// for now assume that if the repeat mode executed a move (such as returned to start) there's no need to also step an increment or other move mode
			myMovementMode->Execute(myAxis, this);
		}
	}
	break;

	case AxisMode::MANUAL:
	case AxisMode::STOPPED:
	default:
		break;
	}
}