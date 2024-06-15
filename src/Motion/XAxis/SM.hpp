#pragma once
#include "../../Axis.hpp"
#include "../SM.hpp"
#include "../ZAxis/SM.hpp"

class XAxisSM : public MotionControllerSM
{
public:
	XAxisSM(Axis *anXAxis, ZAxisSM *aZAxisSM);

	virtual void Update() override;

private:
	ZAxisSM *myZAxisSM;
};