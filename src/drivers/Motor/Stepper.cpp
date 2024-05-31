#include "Stepper.hpp"
#include "zephyr/drivers/gpio.h"
#include <algorithm>
#include <math.h>
#include <sys/_stdint.h>
#include <vector>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

Stepper::Stepper(const StepperConfig &aStepperConfig, uint32_t aDisableDelay)
	: myStepperConfig(aStepperConfig), myDisableDelay(aDisableDelay), myDirection(DirectionState::STOPPED),
	  myEnableState(EnableState::DISABLED), myMovementState(MovementState::IDLE), myMoveType(MoveType::DISTANCE),
	  myTargetSteps(0), myCurrentSteps(0), mySpeed(0), myAcceleration(0), myDeceleration(0)
{
	k_fifo_init(&myMovementQueue);
	k_timer_init(&myStepTimer, TimerHandlerWrapper, nullptr);
	k_timer_user_data_set(&myStepTimer, this);
}

void Stepper::SetMaxSpeed(uint32_t aMaxSpeed)
{
	myStepperConfig.maxSpeed = aMaxSpeed;
}

void Stepper::SetAcceleration(uint32_t aAcceleration)
{
	myAcceleration = aAcceleration;
}

void Stepper::SetDeceleration(uint32_t aDeceleration)
{
	myDeceleration = aDeceleration;
}

void Stepper::SetDisableDelay(uint32_t aDisableDelay)
{
	myDisableDelay = aDisableDelay;
}

void Stepper::Enable()
{
	gpio_pin_set_dt(&myStepperConfig.enableGpio, myStepperConfig.enablePolarity);
	myEnableState = EnableState::ENABLED;
}

void Stepper::Disable()
{
	gpio_pin_set_dt(&myStepperConfig.enableGpio, !myStepperConfig.enablePolarity);
	myEnableState = EnableState::DISABLED;
}

void Stepper::Move(int aSteps, int aSpeed)
{
	// todo: cancel disable timer
	if (myEnableState == EnableState::DISABLED)
	{
		Enable();
	}

	DirectionState direction = (aSteps > 0) ? DirectionState::POS : DirectionState::NEG;
	gpio_pin_set_dt(&myStepperConfig.dirGpio, (direction == DirectionState::POS) ? myStepperConfig.dirPolarity : !myStepperConfig.dirPolarity);

	std::vector<int> delays;
	CalculateSpeedProfile(aSteps, aSpeed, delays);

	for (int delay : delays)
	{
		StepCommand *command = new StepCommand{delay};
		k_fifo_put(&myMovementQueue, command);
	}

	UpdateMovement();
}

void Stepper::Stop()
{
	// Flush the current movement queue
	StepCommand *command;
	while ((command = (StepCommand *)k_fifo_get(&myMovementQueue, K_NO_WAIT)) != NULL)
	{
		delete command;
	}

	// Add deceleration steps to stop the motor

	std::vector<int> delays;
	int targetSpeed = 0;
	int stepsToStop = (mySpeed * mySpeed) / (2 * myDeceleration);
	CalculateSpeedProfile(stepsToStop, targetSpeed, delays);

	for (int delay : delays)
	{
		StepCommand *command = new StepCommand{delay};
		k_fifo_put(&myMovementQueue, command);
	}

	UpdateMovement();
}

void Stepper::UpdateMovement()
{
	if (k_fifo_is_empty(&myMovementQueue))
	{
		myMovementState = MovementState::IDLE;
		if (myDisableDelay > 0)
		{
			// Schedule disable after delay
		}
		return;
	}

	StepCommand *currentCommand = (StepCommand *)k_fifo_get(&myMovementQueue, K_NO_WAIT);
	if (currentCommand)
	{
		ExecuteMovement(*currentCommand);
		delete currentCommand;
	}
}

void Stepper::ExecuteMovement(const StepCommand &command)
{
	StartStepTimer(command.delay);
}

void Stepper::CalculateSpeedProfile(int steps, int targetSpeed, std::vector<int> &delays)
{
	int accelSteps = std::min(steps / 2, (targetSpeed - mySpeed) / myAcceleration);
	int decelSteps = std::min(steps / 2, (targetSpeed - mySpeed) / myDeceleration);
	int constantSpeedSteps = steps - accelSteps - decelSteps;

	for (int i = 0; i < accelSteps; ++i)
	{
		mySpeed += myAcceleration;
		delays.push_back(USEC_PER_SEC / mySpeed);
	}

	for (int i = 0; i < constantSpeedSteps; ++i)
	{
		delays.push_back(USEC_PER_SEC / targetSpeed);
	}

	for (int i = 0; i < decelSteps; ++i)
	{
		mySpeed -= myDeceleration;
		delays.push_back(USEC_PER_SEC / mySpeed);
	}
}

void Stepper::StartStepTimer(int delay)
{
	if (delay > 0)
	{
		k_timer_start(&myStepTimer, K_USEC(delay), K_NO_WAIT);
	}
	else
	{
		k_timer_stop(&myStepTimer);
	}
}

void Stepper::StepTimerHandler()
{
	gpio_pin_toggle_dt(&myStepperConfig.stepGpio);
	UpdateMovement();
}

void Stepper::TimerHandlerWrapper(struct k_timer *timer)
{
	Stepper *instance = static_cast<Stepper *>(k_timer_user_data_get(timer));
	instance->StepTimerHandler();
}

bool Stepper::HasQueuedSteps()
{
	return k_fifo_is_empty(&myMovementQueue);
}