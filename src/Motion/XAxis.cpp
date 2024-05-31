#include "XAxis.hpp"
#include "drivers/Motor/Stepper.hpp"
#include "zephyr/devicetree.h"
#include "zephyr/drivers/pwm.h"

#include <cstddef>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(XAxis);

K_THREAD_STACK_DEFINE(myUpdateThreadStack, 4096);

XAxis::XAxis(const StepperConfig aStepperConfig, uint32_t aDisableDelay)
{
	myStepper = new Stepper(aStepperConfig, aDisableDelay);

	k_thread_create(&myUpdateThread, myUpdateThreadStack, K_THREAD_STACK_SIZEOF(myUpdateThreadStack),
					Cycle, this, NULL, NULL, 8, 0, K_NO_WAIT);
}

void XAxis::Cycle(void *aXAxis, void *, void *)
{
	XAxis *xAxis = static_cast<XAxis *>(aXAxis);
	while (true)
	{
		LOG_INF("MOVING POSITIVE");
		xAxis->myStepper->Move(1000, 200);
		k_sleep(K_MSEC(1000));
		LOG_INF("MOVING NEGATIVE");
		xAxis->myStepper->Move(-1000, 200);
		LOG_INF("MOVE DONE");
		k_sleep(K_MSEC(1000));
	}
}