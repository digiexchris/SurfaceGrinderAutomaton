#pragma once

#include "Enum.hpp"

#include "drivers/Motor/Stepper.hpp"
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define XAxisNode DT_CHOSEN(xaxis)
const StepperConfig X_AXIS_CONFIG = STEPPER_MOTOR_INIT(XAxisNode, 1000, 1000, 1000);

class XAxis
{
public:
	XAxis(const StepperConfig aStepperConfig = X_AXIS_CONFIG, uint32_t aDisableDelay = 0);
	static void Cycle(void *aXAxis, void *, void *);

private:
	Stepper *myStepper;
	struct k_thread myUpdateThread;
};