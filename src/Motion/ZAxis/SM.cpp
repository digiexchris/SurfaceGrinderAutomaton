#include "SM.hpp"
#include "../Axis.hpp"
#include "../SM.hpp"
#include "MovementMode.hpp"
#include <FreeRTOS.h>
#include <task.h>

ZAxisSM::ZAxisSM(Axis *aZAxis, Controller *aController) : MotionControllerSM(aZAxis, aController)
{
	// todo statically pre-allocate all types and just assign it instead
	myMovementMode = new ZMoveBothEnds();
	//todo: since the axis is just tracking a position, we don't need repeat mode anymore. myRepeatMode = new ZRepeatReverse();
}

void ZAxisSM::Update()
{
	switch (myAxisMode)
	{
	case AxisMode::AUTOMATIC:
	{

		myMovementMode->Execute(myAxis, this);

		myAxis->IsMovementComplete();
	}
	break;

	case AxisMode::ONE_SHOT:
	case AxisMode::MANUAL:
	case AxisMode::STOPPED:
	default:

		break;
	}

	xTaskNotifyGive(myMotionController->GetTaskHandle(AxisLabel::X));
}