#pragma once
#include "../Axis.hpp"
#include "../SM.hpp"

class XAxisSM : public MotionControllerSM
{
public:
	XAxisSM(Axis *anAxis, Controller *aMotionController) : MotionControllerSM(anAxis, aMotionController){};
	virtual void Update() override;

	virtual void ResetAutoMode() override;

private:
	int32_t myPreviousPosition = 0;
	AxisStop myPreviousAxisStop = AxisStop::NEITHER;
};