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
	virtual void Execute(Axis *aZAxis, ZAxisSM *aZAxisSM) override;
};

class ZMoveConstant : public ZAxisSM::IMovementMode
{
public:
	virtual void Execute(Axis *aZAxis, ZAxisSM *aZAxisSM) override;
};