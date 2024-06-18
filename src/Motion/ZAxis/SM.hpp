#pragma once
#include "../SM.hpp"

class Axis;
class Controller;
class ZAxisSM : public MotionControllerSM
{
public:
	ZAxisSM(Axis *anAxis, Controller *aMotionController);

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
		 * @var outMoved true if a move was executed (including 0 step direction change)
		 * @return true if it executed normally, false if there was a problem (e.g., the wait for the one-shot timed out)
		 */
		virtual bool Execute(Axis *aZAxis, ZAxisSM *aZAxisSM, bool &outMoved) = 0;
	};

private:
	IMovementMode *myMovementMode;
	IRepeatMode *myRepeatMode;
};
