#include "Stepper.hpp"
#include "config.hpp"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include <algorithm>
#include <math.h>
#include <sys/_stdint.h>
#include <vector>

#include "stepper.pio.h"

Stepper::Stepper(int aDirPin, int aStepPin, int anEnablePin, uint32_t aDisableDelay)
	: myDisableDelay(aDisableDelay), myDirection(DirectionState::STOPPED),
	  myEnableState(EnableState::DISABLED), myMovementState(MovementState::IDLE), myMoveType(MoveType::DISTANCE),
	  myTargetSteps(0), myCurrentSteps(0), mySpeed(0), myAcceleration(0), myDeceleration(0)
{
	myStepperDevice = new StepperDevice();
	myStepperDevice->DIR_PIN = aDirPin;
	myStepperDevice->EN_PIN = anEnablePin;
	myStepperDevice->STEP_PIN = aStepPin;

	privSetupGPIO();
	privSetupPIO();
}

void Stepper::SetMaxSpeed(uint32_t aMaxSpeed)
{
	// myStepperConfig.maxSpeed = aMaxSpeed;
}

void Stepper::SetAcceleration(uint32_t aAcceleration)
{
	// myAcceleration = aAcceleration;
}

void Stepper::SetDeceleration(uint32_t aDeceleration)
{
	// myDeceleration = aDeceleration;
}

void Stepper::SetDisableDelay(uint32_t aDisableDelay)
{
	// myDisableDelay = aDisableDelay;
}

void Stepper::Enable()
{
	gpio_put(myStepperDevice->EN_PIN, STEPPER1_ENABLE_POLARITY);
}

void Stepper::Disable()
{
	gpio_put(myStepperDevice->EN_PIN, !STEPPER1_ENABLE_POLARITY);
}

void Stepper::Move(int aSteps, int aSpeed)
{
	// // todo: cancel disable timer
	// if (myEnableState == EnableState::DISABLED)
	// {
	// 	Enable();
	// }

	// DirectionState direction = (aSteps > 0) ? DirectionState::POS : DirectionState::NEG;
	// gpio_pin_set_dt(&myStepperConfig.dirGpio, (direction == DirectionState::POS) ? myStepperConfig.dirPolarity : !myStepperConfig.dirPolarity);

	// std::vector<int> delays;
	// CalculateSpeedProfile(aSteps, aSpeed, delays);

	// for (int delay : delays)
	// {
	// 	StepCommand *command = new StepCommand{delay};
	// 	k_fifo_put(&myMovementQueue, command);
	// }

	// UpdateMovement();
}

void Stepper::Stop()
{
	// // Flush the current movement queue
	// StepCommand *command;
	// while ((command = (StepCommand *)k_fifo_get(&myMovementQueue, K_NO_WAIT)) != NULL)
	// {
	// 	delete command;
	// }

	// // Add deceleration steps to stop the motor

	// std::vector<int> delays;
	// int targetSpeed = 0;
	// int stepsToStop = (mySpeed * mySpeed) / (2 * myDeceleration);
	// CalculateSpeedProfile(stepsToStop, targetSpeed, delays);

	// for (int delay : delays)
	// {
	// 	StepCommand *command = new StepCommand{delay};
	// 	k_fifo_put(&myMovementQueue, command);
	// }

	// UpdateMovement();
}

void Stepper::MoveRelative(int steps, DirectionState dir)
{
	privMoveRelative(steps, dir);
}

bool Stepper::Update()
{
	return privUpdatePendingMovement();
}

uint32_t Stepper::GetRemainingSteps()
{
	return myStepperDevice->StepsToTake;
}

// todo: change this to just deny changing direction if there steps in the fifo. maybe I do need this for async, but then it's more of... decelerate to a stop then reverse and move to the intended position. Maybe make this position tracking rather than relative step execution?
bool Stepper::privChangeMotorDirection()
{
	if (myStepperDevice->activedir != myStepperDevice->dirchange && myStepperDevice->directionChangeDelayCounter == 0) // first change detected
	{
		if (pio_sm_is_tx_fifo_empty(myStepperDevice->stm_pio, myStepperDevice->stm_sm))
		{
			myStepperDevice->directionChangeDelayCounter = (time_us_64() / 1000);
		}
		return false;
	}
	else if (myStepperDevice->activedir != myStepperDevice->dirchange && myStepperDevice->directionChangeDelayCounter != 0)
	{
		if (((time_us_64() / 1000) - myStepperDevice->directionChangeDelayCounter) > myStepperDevice->directionChangeDelay) // pre change delay
		{
			myStepperDevice->activedir = myStepperDevice->dirchange;
			gpio_put(myStepperDevice->DIR_PIN, !myStepperDevice->activedir);
			myStepperDevice->directionChangeDelayCounter = (time_us_64() / 1000);
		}
		return false;
	}
	else if (myStepperDevice->activedir == myStepperDevice->dirchange && myStepperDevice->directionChangeDelayCounter != 0)
	{
		if (((time_us_64() / 1000) - myStepperDevice->directionChangeDelayCounter) > myStepperDevice->directionChangeDelay) // post change delay
		{
			myStepperDevice->directionChangeDelayCounter = 0;
			return true;
		}
		return false;
	}

	return true;
}

void Stepper::privSetupPIO()
{
	// stepper 1 pio
	uint stm_offset = pio_add_program(myStepperDevice->stm_pio, &stepper_1_program);
	stepper_1_program_init(myStepperDevice->stm_pio, myStepperDevice->stm_sm, stm_offset, myStepperDevice->STEP_PIN, 10000, true);
}

void Stepper::privSetupGPIO()
{

	// stepper enable pin
	gpio_init(myStepperDevice->EN_PIN);
	Disable();
	// gpio_put(STM_EN, 0);

	// wheel 1 stepper dir pin
	gpio_init(myStepperDevice->DIR_PIN);
	gpio_set_dir(myStepperDevice->DIR_PIN, GPIO_OUT);
	// gpio_put(myStepperDevice->DIR_PIN, 1);
}

bool Stepper::privUpdatePendingMovement()
{

	if (myStepperDevice->StepsToTake > 0 && pio_sm_is_tx_fifo_empty(myStepperDevice->stm_pio, myStepperDevice->stm_sm))
	{
		if (privChangeMotorDirection())
		{
			int remBits = 32;
			if (myStepperDevice->StepsToTake < 32)
			{
				remBits = myStepperDevice->StepsToTake;
			}

			privPutStepsBlocking(remBits);
			myStepperDevice->StepsToTake -= remBits;
		}
	}

	return (myStepperDevice->StepsToTake > 0);
}

void Stepper::privMoveRelative(int steps, DirectionState dir)
{
	myStepperDevice->StepsToTake += steps * static_cast<int>(dir);
}

void Stepper::privPutStepsBlocking(uint32_t steps)
{
	uint32_t aPulseCount;
	if (steps <= 0)
	{
		aPulseCount = 0;
	}
	else if (steps >= 32)
	{
		aPulseCount = 0xFFFFFFFF; // 32 bits of '1'
	}
	else
	{
		aPulseCount = (1U << steps) - 1;
	}

	pio_sm_put_blocking(pio0, 1, aPulseCount);
}