#pragma once

#include "../Axis.hpp"

class MotionControllerSM
{
protected:
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
		myManualTriggerSemaphore = xSemaphoreCreateBinary();
	}

	void SetMode(AxisMode aState)
	{
		myAxisMode = aState;
	}

	AxisMode GetMode() const
	{
		return myAxisMode;
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