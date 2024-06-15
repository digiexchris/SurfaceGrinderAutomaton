#pragma once

#include "../../Axis.hpp"
#include "RepeatMode.hpp"
#include "SM.hpp"

/* Timed out, no big deal, try again. (don't wait forever in case we've switched modes off of ONE_SHOT) */
#define WAIT_FOR_ONE_SHOT_TRIGGER()                        \
	if (aZAxisSM->GetMode() == AxisMode::ONE_SHOT)         \
	{                                                      \
		BaseType_t res = aZAxisSM->WaitForManualTrigger(); \
		if (res == pdFALSE)                                \
		{                                                  \
			return false;                                  \
		}                                                  \
		aZAxisSM->SetMode(AxisMode::MANUAL);               \
	}

enum class RepeatMode
{
	REPEAT_REVERSE,
	REPEAT_AT_START,
	NO_REPEAT
};

class ZRepeatReverse : public ZAxisSM::IRepeatMode
{
public:
	virtual bool Execute(Axis *aZAxis, ZAxisSM *aZAxisSM, bool &outMoved) override;
};

class ZNoRepeat : public ZAxisSM::IRepeatMode
{
public:
	virtual bool Execute(Axis *aZAxis, ZAxisSM *aZAxisSM, bool &outMoved) override;
};

class ZRepeatAtStart : public ZAxisSM::IRepeatMode
{
public:
	virtual bool Execute(Axis *aZAxis, ZAxisSM *aZAxisSM, bool &outMoved) override;
};