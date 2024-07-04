#pragma once
#include "Axis.hpp"
#include "Enum.hpp"
#include "drivers/Motor/Stepper.hpp"
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
	MotionController(Stepper *anXStepper, Stepper *aZStepper, Stepper *aYStepper = nullptr);
	virtual bool SetMode(AxisLabel anAxisLabel, AxisMode aMode) override;
	virtual AxisMode GetMode(AxisLabel anAxisLabel) override;
	bool SetAdvanceIncrement(AxisLabel anAxisLabel, uint32_t anIncrement);
	uint32_t GetAdvanceIncrement(AxisLabel anAxisLabel);
	AxisDirection GetDirection(AxisLabel anAxisLabel);
	TaskHandle_t GetTaskHandle(AxisLabel anAxisLabel);
	bool SetStop(AxisLabel anAxisLabel, AxisDirection aDirection, int32_t aPosition);
	int32_t GetStop(AxisLabel anAxisLabel, AxisDirection aDirection);
	bool SetSpeed(AxisLabel anAxisLabel, uint16_t aSpeed);
	uint16_t GetSpeed(AxisLabel anAxisLabel);
	bool MoveRelative(AxisLabel anAxisLabel, int32_t aDistance);

private:
	std::unordered_map<AxisLabel, Axis *> myAxes;

	SemaphoreHandle_t myZTriggerSemaphore;

	uint32_t myZAdvanceRate = 100;

	ZAxisSM *myZAxisSM;
	XAxisSM *myXAxisSM;

	std::unordered_map<AxisLabel, TaskHandle_t *> myTaskHandles;

	uint16_t myZConstantSpeed = 100;
	static void MotionXThread(void *pvParameters);
	static void MotionZThread(void *pvParameters);
};