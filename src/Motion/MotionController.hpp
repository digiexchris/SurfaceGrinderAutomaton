#pragma once
#include "Axis.hpp"
#include "Enum.hpp"
#include "drivers/Motor/Stepper.hpp"
#include <unordered_map>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <set>
#include <task.h>

#include "Axis.hpp"
#include "Motion/XAxis/SM.hpp"
#include "Motion/ZAxis/SM.hpp"

class MotionController
{
public:
	MotionController(Stepper *anXStepper, Stepper *aZStepper, Stepper *aYStepper = nullptr);
	bool SetMode(AxisLabel anAxisLabel, AxisMode aMode);
	AxisMode GetMode(AxisLabel anAxisLabel);

private:
	MotionController *self;
	std::unordered_map<AxisLabel, Axis *> myAxes;

	SemaphoreHandle_t myZTriggerSemaphore;

	uint32_t myZAdvanceRate = 100;

	ZAxisSM *myZAxisSM;
	XAxisSM *myXAxisSM;

	uint16_t myZConstantSpeed = 100;
	static void MotionXThread(void *pvParameters);
};