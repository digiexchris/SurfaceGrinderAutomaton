#pragma once

#include "Axis.hpp"
#include "Enum.hpp"

#include <cstdint>
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
enum class SMError
{
	NO_ERROR = 0,
	WRONG_MODE = 1
};
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

	Stepper::MoveState GetMoveState() const
	{
		return myAxis->GetMoveState();
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

	SMError SetTargetPosition(int32_t aPosition)
	{
		switch (GetMode())
		{

		case AxisMode::MANUAL:
			myAxis->SetTargetPosition(aPosition);
			return SMError::NO_ERROR;
			break;
		case AxisMode::AUTOMATIC:
		case AxisMode::ONE_SHOT:
		case AxisMode::STOPPED:
		default:
			return SMError::WRONG_MODE;
			break;
		}
	}

	int32_t GetCurrentPosition() const
	{
		return myAxis->GetCurrentPosition();
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

	SMError SetTargetSpeed(uint16_t aSpeed)
	{
		myAxis->SetTargetSpeed(aSpeed);
		return SMError::NO_ERROR;
	}

	float GetCurrentSpeed() const
	{
		return myAxis->GetCurrentSpeed();
	}

	uint16_t GetTargetSpeed() const
	{
		return myAxis->GetTargetSpeed();
	}

	SMError MoveRelative(int32_t aDistance)
	{

		switch (GetMode())
		{
		case AxisMode::AUTOMATIC:
		case AxisMode::MANUAL:
			myAxis->MoveRelative(aDistance);
			return SMError::NO_ERROR;
			break;

		case AxisMode::ONE_SHOT:
		case AxisMode::STOPPED:
		default:
			return SMError::WRONG_MODE;
			break;
		}
	}

	SMError MoveTo(int32_t aPosition)
	{

		switch (GetMode())
		{
		case AxisMode::AUTOMATIC:
		case AxisMode::MANUAL:
			myAxis->SetTargetPosition(aPosition);
			return SMError::NO_ERROR;
			break;

		case AxisMode::ONE_SHOT:
		case AxisMode::STOPPED:
		default:
			return SMError::WRONG_MODE;
			break;
		}
	}
};
