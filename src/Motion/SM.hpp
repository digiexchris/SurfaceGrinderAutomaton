#pragma once

#include "../Axis.hpp"

class MotionControllerSM
{
protected:
	SemaphoreHandle_t myModeMutex;
	AxisMode myAxisMode;
	AxisStop myIsAtStop;
	Axis *myAxis;
	uint16_t myAdvanceIncrement;

	SemaphoreHandle_t myManualTriggerSemaphore;

public:
	void SetAdvanceIncrement(uint16_t anIncrement)
	{
		myAdvanceIncrement = anIncrement;
	}

	uint16_t GetAdvanceIncrement() const
	{
		return myAdvanceIncrement;
	}

	BaseType_t WaitForManualTrigger()
	{
		return xSemaphoreTake(myManualTriggerSemaphore, 200 * portTICK_PERIOD_MS);
	}

	MotionControllerSM(Axis *anAxis) : myAxis(anAxis), myAxisMode(AxisMode::STOPPED), myIsAtStop(AxisStop::NEITHER)
	{
		myModeMutex = xSemaphoreCreateMutex();
		myManualTriggerSemaphore = xSemaphoreCreateBinary();
	}

	void SetMode(AxisMode aState)
	{
		xSemaphoreTake(myModeMutex, portMAX_DELAY);
		myAxisMode = aState;
		xSemaphoreGive(myModeMutex);
	}

	AxisMode GetMode() const
	{
		xSemaphoreTake(myModeMutex, portMAX_DELAY);
		AxisMode mode = myAxisMode;	 // Access the protected resource
		xSemaphoreGive(myModeMutex); // Release the mutex
		return mode;
	}

	void SetIsAtStop(AxisStop aStop)
	{
		myIsAtStop = aStop;
	}

	AxisStop GetIsAtStop() const
	{
		return myIsAtStop;
	}

	void TriggerManualState()
	{
		xSemaphoreGive(myManualTriggerSemaphore);
	}

	virtual void Update() = 0;
};