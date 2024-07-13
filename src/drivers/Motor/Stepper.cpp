#include "Stepper.hpp"
#include "Helpers.hpp"
#include "config.hpp"
#include "stepper.pio.h"
#include <pico/printf.h>
#include <semphr.h>
#include <stdio.h>

Stepper::Stepper(uint stepPin, uint dirPin, float maxSpeed, float acceleration, PIO pio, uint sm)
	: stepPin(stepPin), dirPin(dirPin), pio(pio), sm(sm) //, IStepper(maxSpeed, acceleration)
{
	gpio_init(dirPin);
	gpio_set_dir(dirPin, GPIO_OUT);
	SetTargetSpeed(maxSpeed);
	SetAcceleration(acceleration);
	myTargetPositionMutex = xSemaphoreCreateMutex();
	myMoveStateMutex = xSemaphoreCreateMutex();
	myCurrentPositionMutex = xSemaphoreCreateMutex();
	myTargetSpeedMutex = xSemaphoreCreateMutex();
}

void Stepper::InitPIO() // todo optimize this PIO to only send a single delay per step so up to 4 steps can be sent instead of 2. It's currently sending 2, one for the high phase and one for the low phase. the downside of that would be it's hard coding to a 50% duty cycle, some drivers can go faster at a 30% duty cycle, but this is a surface grinder not a machining center, and lost steps due to draining the fifo before we've come to a stop is more important for the Z and Y axis.
{
	printf("Initializing PIO for stepper...\n");
	uint offset = pio_add_program(pio, (const pio_program_t *)&simplestepper_program);
	pio_sm_config c = simplestepper_program_get_default_config(offset);
	sm_config_set_sideset_pins(&c, stepPin);				   // Configure the step pin for side-set operations
	sm_config_set_set_pins(&c, stepPin, 1);					   // Configure the step pin for set operations
	pio_gpio_init(pio, stepPin);							   // Initialize the GPIO pin
	pio_sm_set_consecutive_pindirs(pio, sm, stepPin, 1, true); // Set the GPIO direction to output

	// Calculate the clock divider for 200 steps per second
	float clockDiv = 125;				// 200 steps per second, each step has 2 phases (high and low)
	sm_config_set_clkdiv(&c, clockDiv); // Set the clock divider

	pio_sm_init(pio, sm, offset, &c);  // Initialize the state machine
	pio_sm_set_enabled(pio, sm, true); // Enable the state machine

	printf("PIO initialized with clock divider: %.2f\n", clockDiv);
}

void Stepper::SetDirection(bool direction)
{
	if (this->myDirection == direction)
	{
		return;
	}

	this->myDirection = direction;
	gpio_put(dirPin, direction);
	DirectionChangedWait();
	if (PRINTF_STEPPER_DEBUG)
	{
		printf("Direction set to %d\n", direction);
	}
}

void Stepper::DirectionChangedWait()
{
	vTaskDelay(STEPPER_DIRECTION_CHANGE_DELAY_MS * portTICK_PERIOD_MS);
}

void Stepper::Update() // todo investigate the pi sdk and see if Critical sections are more suited than mutexes here
{
	xSemaphoreTake(myTargetPositionMutex, portMAX_DELAY);
	xSemaphoreTake(myMoveStateMutex, portMAX_DELAY);
	xSemaphoreTake(myCurrentPositionMutex, portMAX_DELAY);
	xSemaphoreTake(myTargetSpeedMutex, portMAX_DELAY);

	int32_t remainingSteps;
	bool newDirection;
	remainingSteps = abs(myTargetPosition - myCurrentPosition);
	newDirection = (myTargetPosition > myCurrentPosition);

	switch (myMoveState)
	{
	case IDLE:
		if (remainingSteps > 0)
		{
			myMoveState = ACCELERATING;
		}
		break;
	case ACCELERATING:
		if (newDirection != myDirection)
		{
			myMoveState = DECELERATING;
		}
		else if (remainingSteps <= (myCurrentSpeed * myCurrentSpeed) / (2 * myAcceleration))
		{
			myMoveState = DECELERATING;
		}
		else if (myCurrentSpeed < myTargetSpeed)
		{
			myCurrentSpeed += myAcceleration;
			stepDelay = 1e6 / myCurrentSpeed / 2;
		}
		// TODO this does not handle the case where the new target speed is lower than the current speed
		else
		{
			myMoveState = CONSTANT_SPEED;
		}
		break;

	case CONSTANT_SPEED:
		if (newDirection != myDirection)
		{
			myMoveState = DECELERATING;
		}
		else if (remainingSteps <= (myCurrentSpeed * myCurrentSpeed) / (2 * myAcceleration))
		{
			myMoveState = DECELERATING;
		}
		stepDelay = 1e6 / myTargetSpeed / 2;
		break;

	case DECELERATING:
		if (myCurrentSpeed > 0) // this does not correctly handle decelerating to a lower constant speed. rewrite this to compare against a target speed (not the one requested by the user (mymyTargetSpeed), since that gets reused for future moves), and when decelerating to a stop, set that target speed to 0.
		{
			myCurrentSpeed -= myAcceleration;
			stepDelay = 1e6 / myCurrentSpeed / 2;
		}
		else
		{
			if (newDirection != myDirection)
			{
				myMoveState = CHANGING_DIRECTION;
			}
			else
			{
				myMoveState = IDLE;
			}
		}
		break;

	case CHANGING_DIRECTION:
		SetDirection(newDirection);
		{
			myMoveState = ACCELERATING;
		}

		break;
	}

	if (myMoveState != IDLE)
	{
		// Refill FIFO as needed, but limit the number of steps added in one go in order to allow new target positions to be set
		// and reacted to if the fifo is draining faster than we can fill it, otherwise this loop would never end until all of the
		// previously calculated steps were erroneously executed.
		int stepsToAdd = min(remainingSteps, 4); // Add up to 4 steps at a time
		while (!pio_sm_is_tx_fifo_full(pio, sm) && stepsToAdd > 0)
		{
			uint32_t delay = static_cast<uint32_t>(stepDelay);
			pio_sm_put_blocking(pio, sm, delay);
			pio_sm_put_blocking(pio, sm, delay);

			{
				myCurrentPosition += (myDirection ? 1 : -1);
			}

			stepsToAdd--;

			{
				remainingSteps = abs(myTargetPosition - myCurrentPosition);
			}
		}
	}

	xSemaphoreGive(myTargetPositionMutex);
	xSemaphoreGive(myMoveStateMutex);
	xSemaphoreGive(myCurrentPositionMutex);
	xSemaphoreGive(myTargetSpeedMutex);
}

Stepper::MoveState Stepper::GetMoveState()
{
	xSemaphoreTake(myMoveStateMutex, portMAX_DELAY);
	MoveState moveState = myMoveState;
	xSemaphoreGive(myMoveStateMutex);
	return moveState;
}

void Stepper::SetTargetPosition(int32_t targetPosition, uint16_t speed)
{
	xSemaphoreTake(myTargetPositionMutex, portMAX_DELAY);
	if (targetPosition != myTargetPosition && myMoveState == IDLE)
	{
		xSemaphoreTake(myMoveStateMutex, portMAX_DELAY);
		// this is important because some state machines block until the stepper returns to the IDLE state
		// so this prevents the race condition where the stepper update() loop does not set the state before
		// the calling state machine checks to see if the move is complete
		myMoveState = ACCELERATING;
		xSemaphoreGive(myMoveStateMutex);
	}

	myTargetPosition = targetPosition;
	if (speed != 0)
	{
		this->SetTargetSpeed(speed);
	}
	xSemaphoreGive(myTargetPositionMutex);
}
int32_t Stepper::GetCurrentPosition()
{
	xSemaphoreTake(myCurrentPositionMutex, portMAX_DELAY);
	int32_t currentPosition = myCurrentPosition;
	xSemaphoreGive(myCurrentPositionMutex);
	return currentPosition;
}

int32_t Stepper::GetTargetPosition()
{
	xSemaphoreTake(myTargetPositionMutex, portMAX_DELAY);
	int32_t targetPosition = myTargetPosition;
	xSemaphoreGive(myTargetPositionMutex);
	return targetPosition;
}

// can set speed here (mid move or any time) or via set target speed
void Stepper::SetTargetSpeed(uint16_t aSpeed)
{
	xSemaphoreTake(myTargetSpeedMutex, portMAX_DELAY);
	myTargetSpeed = aSpeed;
	xSemaphoreGive(myTargetSpeedMutex);
}
uint16_t Stepper::GetTargetSpeed()
{
	xSemaphoreTake(myTargetSpeedMutex, portMAX_DELAY);
	uint16_t targetSpeed = myTargetSpeed;
	xSemaphoreGive(myTargetSpeedMutex);
	return targetSpeed;
}

void Stepper::SetAcceleration(float aAcceleration)
{
	myAcceleration = aAcceleration;
}

// todo check that float is atomic on rp2040 or mutex this
float Stepper::GetAcceleration()
{
	return myAcceleration;
}

// todo check that uint16_t is atomic on rp2040
uint16_t Stepper::GetCurrentSpeed()
{
	return myCurrentSpeed;
}