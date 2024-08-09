#pragma once
#include "Axis.hpp"
#include "Enum.hpp"
#include "drivers/Motor/Stepper.hpp"
#include <cstdint>
#include <unordered_map>

#include "Enum.hpp"
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <set>
#include <task.h>

#include "Axis.hpp"
#include "Motion/XAxis/SM.hpp"
#include "Motion/ZAxis/SM.hpp"

class MotionController : public Controller
{
public:

	struct MotionOutputTaskParams
	{
		MotionOutputTaskParams(AxisLabel anAxisLabel, Axis *anAxis, MotionController* aMotionController)
			: axisLabel(anAxisLabel), axis(anAxis), motionController(aMotionController)
		{
		}
		AxisLabel axisLabel;
		Axis *axis;
		MotionController* motionController;
	};
	MotionController(Axis *anXStepper, Axis *aZStepper, Axis *aYStepper = nullptr);
	virtual bool SetMode(AxisLabel anAxisLabel, AxisMode aMode) override;
	virtual AxisMode GetMode(AxisLabel anAxisLabel) override;
	bool SetAdvanceIncrement(AxisLabel anAxisLabel, uint32_t anIncrement);
	uint32_t GetAdvanceIncrement(AxisLabel anAxisLabel);
	AxisDirection GetDirection(AxisLabel anAxisLabel);
	TaskHandle_t GetTaskHandle(AxisLabel anAxisLabel) override;
	bool SetStop(AxisLabel anAxisLabel, AxisDirection aDirection, int32_t aPosition);
	int32_t GetStop(AxisLabel anAxisLabel, AxisDirection aDirection);
	bool SetTargetSpeed(AxisLabel anAxisLabel, uint16_t aSpeed);
	float GetCurrentSpeed(AxisLabel anAxisLabel);
	uint16_t GetTargetSpeed(AxisLabel anAxisLabel);
	Stepper::MoveState GetMoveState(AxisLabel anAxisLabel);
	bool MoveRelative(AxisLabel anAxisLabel, int32_t aDistance);
	bool MoveTo(AxisLabel anAxisLabel, int32_t aPosition);
	bool SetCurrentPosition(AxisLabel anAxisLabel, int32_t aPosition);
	int32_t GetCurrentPosition(AxisLabel anAxisLabel);
	int32_t GetTargetPosition(AxisLabel anAxisLabel);
	bool SetTargetPosition(AxisLabel anAxisLabel, int32_t aPosition);

private:
	std::unordered_map<AxisLabel, Axis *> myAxes;

	SemaphoreHandle_t myZTriggerSemaphore;

	uint32_t myZAdvanceRate = 100;

	ZAxisSM *myZAxisSM;
	XAxisSM *myXAxisSM;

	std::unordered_map<AxisLabel, TaskHandle_t *> myTaskHandles;

	std::unordered_map<AxisLabel, TaskHandle_t *> myStepperStateOutputTaskHandles;

	static void MotionStateOutputTask(void *pvParameters);

	uint16_t myZConstantSpeed = 100;
	static void MotionXThread(void *pvParameters);
	static void MotionZThread(void *pvParameters);
};