#pragma once
#include "Axis.hpp"
#include "Enum.hpp"
#include "drivers/Motor/Stepper.hpp"
#include <unordered_map>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

enum class AxisMotionContolState
{
	AUTOMATIC,
	MANUAL,
	STOPPED
};

class MotionController
{
public:
	MotionController(Stepper *anXStepper, Stepper *aYStepper, Stepper *aZStepper = nullptr);
	void SetStop(AxisLabel anAxis, AxisStop anAxisStop, int32_t aPosition);
	void SetAdvanceZType(AdvanceZType anAdvanceType);
	void SetRepeatZType(RepeatZType aRepeatType);
	void SetRepeatZTrigger(RepeatZTrigger aRepeatTrigger);
	void SetAdvanceZDirection(AxisDirection anAdvanceDirection);
	void StartX();
	void StopX();
	void StartZAdvance();
	void StopZAdvance();
	void ManualMove(AxisLabel anAxis, int32_t aDistance);

private:
	std::unordered_map<AxisLabel, Axis *> myAxes;

	MotionState myZMotionState = MotionState::STOPPED;
	MotionState myXMotionState = MotionState::AUTOMATIC;

	static void MotionXThread(void *pvParameters);
	void HandleXEndStop(void *pvParameters);
	QueueHandle_t myXCommandQueue;
	QueueHandle_t myZCommandQueue;
};