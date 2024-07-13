#pragma once

#include "Axis.hpp"
#include "Enum.hpp"

#include <semphr.h>

// /**
//  * @brief Base class for all repeat modes
//  * used to block processing until a synchronization event occurs
//  * primarily used in the ZAxisSM::Update() thread
//  */
// #define BEGIN_TRIGGER_SECTION()                  \
// 	if (anSM->GetMode() == AxisMode::ONE_SHOT)   \
// 	{                                            \
// 		BaseType_t res = anSM->WaitForTrigger(); \
// 		if (res == pdFALSE)                      \
// 		{                                        \
// 			return false;                        \
// 		}                                        \
// 	}

// #define END_TRIGGER_SECTION()                  \
// 	if (anSM->GetMode() == AxisMode::ONE_SHOT) \
// 	{                                          \
// 		anSM->SetMode(AxisMode::MANUAL);       \
// 	}

class MotionControllerSM
{
protected:
	SemaphoreHandle_t myTriggerSemaphore;
	SemaphoreHandle_t myModeMutex;
	AxisMode myAxisMode;
	AxisStop myIsAtStop;
	Axis *myAxis;
	uint16_t myAdvanceIncrement;
	Controller *myMotionController;

public:
	MotionControllerSM(Axis *anAxis, Controller *aMotionController)
		: myAxis(anAxis), myMotionController(aMotionController), myAxisMode(AxisMode::STOPPED), myIsAtStop(AxisStop::NEITHER)
	{
		myModeMutex = xSemaphoreCreateMutex();
		myTriggerSemaphore = xSemaphoreCreateBinary();
	}

	void SetAdvanceIncrement(uint16_t anIncrement)
	{
		myAdvanceIncrement = anIncrement;
	}

	uint16_t GetAdvanceIncrement() const
	{
		return myAdvanceIncrement;
	}

	BaseType_t WaitForTrigger()
	{
		return xSemaphoreTake(myTriggerSemaphore, 100 * portTICK_PERIOD_MS);
	}

	virtual void ResetAutoMode() = 0;

	void SetMode(AxisMode aState)
	{
		xSemaphoreTake(myModeMutex, portMAX_DELAY);
		myAxisMode = aState;
		if (myAxisMode == AxisMode::AUTOMATIC)
		{
			ResetAutoMode();
		}
		xSemaphoreGive(myModeMutex);
	}

	AxisMode GetMode() const
	{
		xSemaphoreTake(myModeMutex, portMAX_DELAY);
		AxisMode mode = myAxisMode;	 // Access the protected resource
		xSemaphoreGive(myModeMutex); // Release the mutex
		return mode;
	}

	AxisStop GetIsAtStop() const
	{
		return myAxis->IsAtStop();
	}

	void Trigger()
	{
		xSemaphoreGive(myTriggerSemaphore);
	}

	virtual void Update() = 0;

	bool SetSpeed(uint16_t aSpeed)
	{
		myAxis->SetTargetSpeed(aSpeed);
		return true;
	}

	uint16_t GetSpeed() const
	{
		return myAxis->GetCurrentSpeed();
	}

	bool MoveRelative(int32_t aDistance)
	{
		myAxis->MoveRelative(aDistance);
		return true;
	}

	bool MoveTo(int32_t aPosition)
	{
		myAxis->SetTargetPosition(aPosition);
		return true;
	}
};
