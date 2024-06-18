#pragma once

#include "../../Axis.hpp"
#include "RepeatMode.hpp"
#include "SM.hpp"

enum class RepeatMode
{
	REPEAT_REVERSE,
	REPEAT_AT_START,
	NO_REPEAT
};

class ZRepeatReverse : public ZAxisSM::IRepeatMode
{
public:
	virtual bool Execute(Axis *aZAxis, ZAxisSM *anSM, bool &outMoved) override;
};

class ZNoRepeat : public ZAxisSM::IRepeatMode
{
public:
	virtual bool Execute(Axis *aZAxis, ZAxisSM *anSM, bool &outMoved) override;
};

class ZRepeatAtStart : public ZAxisSM::IRepeatMode
{
public:
	virtual bool Execute(Axis *aZAxis, ZAxisSM *anSM, bool &outMoved) override;
};