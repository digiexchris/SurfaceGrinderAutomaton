#pragma once
#include "Axis.hpp"
#include "Enum.hpp"
#include "drivers/Motor/Stepper.hpp"
#include <unordered_map>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>

class MotionController
{
public:
	MotionController(Stepper *anXStepper, Stepper *aZStepper, Stepper *aYStepper = nullptr);
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
	MotionController *self;
	std::unordered_map<AxisLabel, Axis *> myAxes;

	SemaphoreHandle_t myZTriggerSemaphore;

	uint32_t myZAdvanceRate = 100;

	MotionState myZMotionState = MotionState::STOPPED;
	MotionState myXMotionState = MotionState::STOPPED;

	AdvanceZType myAdvanceZType = AdvanceZType::AT_BOTH_ENDS;
	// AdvanceZType myAdvanceZType = AdvanceZType::CONSTANT;
	RepeatZTrigger myRepeatZTrigger = RepeatZTrigger::AUTOMATIC;
	RepeatZType myRepeatZType = RepeatZType::REVERSE;

	uint16_t myZConstantSpeed = 100;

	AxisStop myXIsAtStop = AxisStop::NEITHER;

	static void MotionXThread(void *pvParameters);
	static void MotionZThread(void *pvParameters);
	void HandleXEndStop(void *pvParameters);
	void privZMoveIncrement();
	void privZMoveConstant();
	QueueHandle_t myXCommandQueue;
	QueueHandle_t myZCommandQueue;
};