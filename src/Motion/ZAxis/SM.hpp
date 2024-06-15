#pragma once

#include "../../Axis.hpp"
#include "../SM.hpp"

class ZAxisSM : public MotionControllerSM
{
public:
	ZAxisSM(Axis *aZAxis);
	virtual void Update() override;

	class IMovementMode
	{
	public:
		virtual ~IMovementMode() = default;
		virtual void Execute(Axis *aZAxis, ZAxisSM *aZAxisSM) = 0;
	};

	class IRepeatMode
	{
	public:
		virtual ~IRepeatMode() = default;

		/**
		 * @brief Execute the repeat mode
		 * @var outMoved true if a move was executed (including 0 step direciton change)
		 * @return true if it executed normally, false if there was a problem (eg. the wait for the one-shot timed out)
		 */
		virtual bool Execute(Axis *aZAxis, ZAxisSM *aZAxisSM, bool &outMoved) = 0;
	};

private:
	IMovementMode *myMovementMode;
	IRepeatMode *myRepeatMode;
};
