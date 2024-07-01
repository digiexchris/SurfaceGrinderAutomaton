#include "SM.hpp"
#include "../Axis.hpp"
#include "../SM.hpp"
#include "MovementMode.hpp"
#include "RepeatMode.hpp"
#include <FreeRTOS.h>
#include <task.h>

ZAxisSM::ZAxisSM(Axis *aZAxis, Controller *aController) : MotionControllerSM(aZAxis, aController)
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
		/* wait for X axis to stop moving and tell Z to advance*/
		BaseType_t res = xTaskNotifyWait(0, 0, NULL, 20 * portTICK_PERIOD_MS);
		if (res == pdFALSE)
		{
			return;
		}

		bool aMoveOccurred;
		bool oneShotTimedOut = !myRepeatMode->Execute(myAxis, this, aMoveOccurred);
		if (oneShotTimedOut)
		{
			return;
		}

		myAxis->IsMovementComplete();

		if (!aMoveOccurred)
		{
			// for now assume that if the repeat mode executed a move (such as returned to start) there's no need to also step an increment or other move mode
			myMovementMode->Execute(myAxis, this);
		}

		myAxis->IsMovementComplete();
	}
	break;

	case AxisMode::MANUAL:
	case AxisMode::STOPPED:
	default:

		break;
	}

	xTaskNotifyGive(myMotionController->GetTaskHandle(AxisLabel::X));
}