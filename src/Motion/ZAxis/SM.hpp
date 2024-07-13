#pragma once
#include "../SM.hpp"

class Axis;
class Controller;
class ZAxisSM : public MotionControllerSM
{
public:
	ZAxisSM(Axis *anAxis, Controller *aMotionController);

	virtual void Update() override;

	virtual void ResetAutoMode() override;

	class IMovementMode
	{
	public:
		virtual void Reset() = 0;
		IMovementMode(Axis *anAxis, ZAxisSM *aZAxisSM) : myAxis(anAxis), myZAxisSM(aZAxisSM) {}
		virtual ~IMovementMode() = default;
		virtual void Execute() = 0;

	protected:
		Axis *myAxis;
		ZAxisSM *myZAxisSM;
	};

private:
	IMovementMode *myMovementMode;
};
