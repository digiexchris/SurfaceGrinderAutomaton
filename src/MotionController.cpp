#include "MotionController.hpp"
#include "Axis.hpp"
#include "config.hpp"
#include "mpu_wrappers.h"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include <Enum.hpp>
#include <cmath>
#include <cstdio>
#include <semphr.h>

MotionController::MotionController(Stepper *anXStepper, Stepper *aZStepper, Stepper *aYStepper)
{
	self = this;
	printf("MotionController\n");
	myAxes[AxisLabel::X] = new Axis(anXStepper, AxisLabel::X);
	myAxes[AxisLabel::Z] = new Axis(aZStepper, AxisLabel::X);

	myAxes[AxisLabel::X]->SetMaxStop(1000);
	myAxes[AxisLabel::X]->SetMinStop(-1000);
	myAxes[AxisLabel::Z]->SetMaxStop(250);
	myAxes[AxisLabel::Z]->SetMinStop(-250);

	myZTriggerSemaphore = xSemaphoreCreateBinary();

	// for future expansion
	if (aYStepper != nullptr)
	{
		assert("Y Stepper not implemented");
		// myAxes[AxisLabel::Y] = new Axis(aYStepper);
	}

	BaseType_t status = xTaskCreate(MotionXThread, "MotionXThread", 1 * 2048, this, 1, NULL);

	if (status != pdPASS)
	{
		printf("Failed to create MotionXThread\n");
		assert(false);
	}

	status = xTaskCreate(MotionZThread, "MotionZThread", 1 * 2048, this, 1, NULL);

	if (status != pdPASS)
	{
		printf("Failed to create MotionZThread\n");
		assert(false);
	}

	printf("MotionController done\n");
}

void MotionController::StartX()
{
	myXMotionState = MotionState::AUTOMATIC;
}

void MotionController::StopX()
{
	myXMotionState = MotionState::STOPPED;
}

void MotionController::StartZAdvance()
{
	Axis *x = myAxes[AxisLabel::X];
	Axis *z = myAxes[AxisLabel::Z];
	if (myZMotionState == MotionState::AUTOMATIC)
	{
		if (myRepeatZTrigger == RepeatZTrigger::AUTOMATIC)
		{
			AxisStop xStop = x->IsAtStop();
			switch (myAdvanceZType)
			{
			case AdvanceZType::AT_BOTH_ENDS:
				if (xStop == AxisStop::MIN || xStop == AxisStop::MAX)
				{
					xSemaphoreGive(myZTriggerSemaphore);
				}
				break;
			case AdvanceZType::AT_LEFT:
				if (xStop == AxisStop::MIN)
				{
					xSemaphoreGive(myZTriggerSemaphore);
				}
				break;
			case AdvanceZType::AT_RIGHT:
				if (xStop == AxisStop::MAX)
				{
					xSemaphoreGive(myZTriggerSemaphore);
				}
				break;
			case AdvanceZType::CONSTANT:
			case AdvanceZType::MANUAL:
				break;
			}
		}
	}
}

void MotionController::MotionXThread(void *pvParameters)
{
	printf("MotionXThread\n");
	MotionController *mc = static_cast<MotionController *>(pvParameters);
	Axis *axis = mc->myAxes[AxisLabel::X];

	while (true)
	{
		if (mc->myXMotionState == MotionState::AUTOMATIC)
		{
			int32_t minStop = axis->GetMinStop();
			int32_t maxStop = axis->GetMaxStop();
			if (axis->IsMovementComplete()) // blocks until true, or false if timeout is set
			{

				if (mc->myRepeatZTrigger == RepeatZTrigger::MANUAL)
				{
					// wait for the user to trigger the repeat
					// todo this doesn't work right, it needs to
					// trigger the moving back to the start of the pass,
					// rather than be involved in each loop iteration
					// ie. it needs to check of the Z pass is complete, and if so, then check for
					// the semaphore, and trigger the z to move to the
					// start and reset the starting conditions so it can do it again
					xSemaphoreTake(mc->myZTriggerSemaphore, portMAX_DELAY);
				}

				/* Check and execute a Z move if necessary */
				if (axis->GetPosition() <= minStop)
				{
					mc->myXIsAtStop = AxisStop::MIN;
				}
				else if (axis->GetPosition() >= maxStop)
				{
					mc->myXIsAtStop = AxisStop::MAX;
				}
				else
				{
					mc->myXIsAtStop = AxisStop::NEITHER;
				}

				if ((bool)axis->IsAtStop())
				{
					mc->StartZAdvance();
					mc->myAxes[AxisLabel::Z]->IsMovementComplete();
				}

				AxisDirection direction = axis->GetPreviousDirection();

				/* Traverse to the opposite stop
				side effect: if you stop it mid-pass it will reverse immediately. fine for now, fix it later*/
				if (direction == AxisDirection::POS)
				{
					int32_t distance = minStop - axis->GetPosition();
					axis->Move(std::abs(distance), AxisDirection::NEG, XAXIS_MAX_SPEED);
				}
				else
				{
					int32_t distance = maxStop - axis->GetPosition();
					axis->Move(std::abs(distance), AxisDirection::POS, XAXIS_MAX_SPEED);
				}

				/* tiny bit of time for everything to settle, but considering
				we're pausing to wait for the Z traverse, this could probably
				be very short or even zero*/
				axis->Wait(50);

				if (PRINTF_AXIS_POSITIONS)
				{
					printf("X: %d\n", axis->GetPosition());
				}
			}
			else
			{
				/* in auto mode this should never get hit unless we set a
				timeout in IsMovementComplete above. Leaving it here to remind
				 me of that */
				vTaskDelay(100 / portTICK_PERIOD_MS);
			}
		}
		else
		{
			vTaskDelay(200 / portTICK_PERIOD_MS);
		}
	}
}

void MotionController::MotionZThread(void *pvParameters)
{
	printf("MotionZThread\n");
	MotionController *mc = static_cast<MotionController *>(pvParameters);
	Axis *axis = mc->myAxes[AxisLabel::Z];

	while (true)
	{
		if (mc->myZMotionState == MotionState::AUTOMATIC)
		{
			// block until something triggers a Z advance (usually the X axis)
			xQueueSemaphoreTake(mc->myZTriggerSemaphore, portMAX_DELAY);

			if (mc->myAdvanceZType == AdvanceZType::CONSTANT)
			{
				mc->privZMoveConstant();
			}
			else
			{
				mc->privZMoveIncrement();
			}
		}
		else
		{
			vTaskDelay(200 / portTICK_PERIOD_MS);
		}
	}
}

void MotionController::privZMoveIncrement()
{
	Axis *axis = myAxes[AxisLabel::Z];
	int32_t minStop = axis->GetMinStop();
	int32_t maxStop = axis->GetMaxStop();
	AxisDirection direction = axis->GetPreviousDirection();
	AxisStop zStop = axis->IsAtStop();

	if (myRepeatZType == RepeatZType::NO_REPEAT)
	{
		if (zStop == AxisStop::MIN && direction == AxisDirection::NEG)
		{
			// noop, we're at the destination
			myZMotionState = MotionState::STOPPED;
		}
		else if (zStop == AxisStop::MAX && direction == AxisDirection::POS)
		{
			// noop, we're at the destination
			myZMotionState = MotionState::STOPPED;
		}
		else
		{
			axis->Move(myZAdvanceRate, direction, ZAXIS_MAX_SPEED);
		}
	}
	else if (myRepeatZType == RepeatZType::REVERSE)
	{
		if (zStop == AxisStop::MIN && direction == AxisDirection::NEG)
		{
			direction = AxisDirection::POS;
		}
		else if (zStop == AxisStop::MAX && direction == AxisDirection::POS)
		{
			direction = AxisDirection::NEG;
		}

		axis->Move(myZAdvanceRate, direction, ZAXIS_MAX_SPEED);
	}
	else if (myRepeatZType == RepeatZType::START_AT_SAME_POSITION)
	{
		if (zStop == AxisStop::MIN && direction == AxisDirection::NEG)
		{
			axis->Move(std::abs(axis->GetMaxStop() - axis->GetPosition()), AxisDirection::POS, ZAXIS_MAX_SPEED);
		}
		else if (zStop == AxisStop::MAX && direction == AxisDirection::POS)
		{
			axis->Move(std::abs(axis->GetMinStop() - axis->GetPosition()), AxisDirection::NEG, ZAXIS_MAX_SPEED);
		}
		else
		{
			axis->Move(myZAdvanceRate, direction, ZAXIS_MAX_SPEED);
		}
	}

	axis->IsMovementComplete();

	if (PRINTF_AXIS_POSITIONS)
	{
		printf("Z: %d\n", axis->GetPosition());
	}
}

void MotionController::privZMoveConstant()
{
	Axis *z = myAxes[AxisLabel::Z];

	while (myZMotionState == MotionState::AUTOMATIC)
	{
		z->IsMovementComplete();
		AxisDirection direction = z->GetPreviousDirection();
		uint32_t distance = 0;
		if (z->IsAtStop() == AxisStop::MIN && direction == AxisDirection::NEG)
		{
			if (myRepeatZType == RepeatZType::NO_REPEAT)
			{
				myZMotionState = MotionState::STOPPED;
				return;
			}
			else if (myRepeatZType == RepeatZType::REVERSE)
			{
				distance = z->GetMaxStop() - z->GetPosition();
				direction = AxisDirection::POS;
			}
		}
		else if (z->IsAtStop() == AxisStop::MAX)
		{
			if (myRepeatZType == RepeatZType::NO_REPEAT)
			{
				myZMotionState = MotionState::STOPPED;
				return;
			}
			else if (myRepeatZType == RepeatZType::REVERSE)
			{
				distance = z->GetMinStop() - z->GetPosition();
				direction = AxisDirection::NEG;
			}
		}
		if (myZMotionState == MotionState::STOPPED)
		{
			return;
		}

		z->Move(distance, direction, myZConstantSpeed);
		z->IsMovementComplete();
		if (PRINTF_AXIS_POSITIONS)
		{
			printf("Z: %d", z->GetPosition());
		}
	}
}