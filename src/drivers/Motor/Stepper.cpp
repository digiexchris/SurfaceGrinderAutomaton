#include "Stepper.hpp"
#include "Helpers.hpp"
#include "config.hpp"
#include "pico/time.h"
#include "stepper.pio.h"
#include <pico/printf.h>
#include <semphr.h>
#include <stdio.h>

Stepper::Stepper(uint stepPin, uint dirPin, float maxSpeed, float acceleration, PIO pio, uint sm, StepperUpdatedCallback stateOutputCallback)
	: stepPin(stepPin), dirPin(dirPin), pio(pio), sm(sm), myStateOutputCallback(stateOutputCallback) //, IStepper(maxSpeed, acceleration)
{
	gpio_init(dirPin);
	gpio_set_dir(dirPin, GPIO_OUT);
	myCommandQueue = xQueueCreate(2, sizeof(StepperCommand *));
	myNotifyCallbackQueue = xQueueCreate(1, sizeof(StepperNotifyMessage *)); // TODO make this a ringbuffer and have the function that pushes to this queue delete the oldest if full
	BaseType_t status = xTaskCreate(NotifyCallbackTask, "Stepper Notify Callback", 2048, this, 1, &myNotifyCallbackTaskHandle);
	configASSERT(status == pdPASS);

	SetTargetSpeed(maxSpeed);
	SetAcceleration(acceleration);
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

void Stepper::Update()
{
	absolute_time_t now = get_absolute_time();
	int64_t deltaTimeUs = absolute_time_diff_us(lastUpdateTime, now);
	float deltaTime = deltaTimeUs / 1e6f; // Convert microseconds to seconds
	lastUpdateTime = now;
	StepperCommand *command = nullptr;

	// process up to queueSize of commands per update loop
	while (xQueueReceive(myCommandQueue, &command, 0) != errQUEUE_EMPTY)
	{
		if (command != nullptr)
		{
			switch (command->command)
			{
			case StepperCommand::CommandName::SET_TARGET_POSITION:
			{
				auto c = static_cast<StepperCommandSetTargetPosition *>(command);
				privSetTargetPosition(c->targetPosition);
			}

			break;
			case StepperCommand::CommandName::SET_TARGET_SPEED:
			{
				auto c = static_cast<StepperCommandSetTargetSpeed *>(command);
				privSetTargetSpeed(c->speed);
			}
			break;
			case StepperCommand::CommandName::SET_ACCELERATION:
			{
				auto c = static_cast<StepperCommandSetAcceleration *>(command);
				privSetAcceleration(c->acceleration);
			}
			break;
			case StepperCommand::CommandName::SET_CURRENT_POSITION:
			{
				auto c = static_cast<StepperCommandSetCurrentPosition *>(command);
				privSetCurrentPosition(c->position);
			}
			break;
			default:
				break;
			}
		}
		if (command != nullptr)
		{
			delete (command);
		}
	}

	int32_t remainingSteps;
	bool newDirection;
	remainingSteps = abs(myTargetPosition - myCurrentPosition);
	newDirection = (myTargetPosition > myCurrentPosition);

	if (myCurrentSpeed == 0 && myDirection != newDirection)
	{
		myMoveState = MoveState::CHANGING_DIRECTION;
	}

	switch (myMoveState)
	{
	case MoveState::IDLE:
		if (remainingSteps > 0)
		{
			myMoveState = MoveState::ACCELERATING;
			return;
		}
		vTaskDelay(100 * portTICK_PERIOD_MS);
		break;
	case MoveState::ACCELERATING:
		if (newDirection != myDirection && myCurrentSpeed > 0)
		{
			myMoveState = MoveState::DECELERATING;
		}
		else if (remainingSteps <= (myCurrentSpeed * myCurrentSpeed) / (2 * myAcceleration))
		{
			myMoveState = MoveState::DECELERATING;
		}
		else if (myCurrentSpeed < myTargetSpeed)
		{
			myCurrentSpeed += myAcceleration * deltaTime;
			stepDelay = 1e6 / myCurrentSpeed / 2;
		}
		// TODO this does not handle the case where the new target speed is lower than the current speed
		else
		{
			myMoveState = MoveState::CONSTANT_SPEED;
		}
		break;

	case MoveState::CONSTANT_SPEED:
		if (newDirection != myDirection)
		{
			myMoveState = MoveState::DECELERATING;
		}
		else if (remainingSteps <= (myCurrentSpeed * myCurrentSpeed) / (2 * myAcceleration))
		{
			myMoveState = MoveState::DECELERATING;
		}
		stepDelay = 1e6 / myTargetSpeed / 2;
		break;

	case MoveState::DECELERATING:
		if (myCurrentSpeed == myTargetSpeed)
		{
			if (remainingSteps == 0)
			{
				myMoveState = MoveState::IDLE;
			}
			else
			{
				myMoveState = MoveState::CONSTANT_SPEED;
			}
			break;
		}

		// if we haven't stopped yet and we don't have steps left (ie. we moved the target position), add steps until we've stopped.
		if (remainingSteps == 0)
		{
			remainingSteps++;
		}

		if (myCurrentSpeed > 0)
		{
			myCurrentSpeed -= myAcceleration * deltaTime;
			stepDelay = 1e6 / myCurrentSpeed / 2;
		}
		else
		{
			if (newDirection != myDirection)
			{
				myMoveState = MoveState::CHANGING_DIRECTION;
			}
			else
			{
				myMoveState = MoveState::IDLE;
			}
		}
		break;

	case MoveState::CHANGING_DIRECTION:
		SetDirection(newDirection);
		{
			myMoveState = MoveState::ACCELERATING;
		}
		return;

		break;
	}

	if (myMoveState != MoveState::IDLE)
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

			privQueueNotifyMessage(StepperNotifyType::CURRENT_POSITION, myCurrentPosition);
			THIS HAS A QUEUE SIZE OF 1 SO COMBINE ALL NOTIFYTYPES INTO A SINGLE MESSAGE SHOWING THE ENTIRE CURRENT STATE.
		}
	}

	if (myTargetSpeed == 0)
	{
		// clearly we're not doing anything if the speed is zero, so without a delay,
		// update will just relock the mutexes before another thread runs.
		// delay 10 cycles to allow another thread to modify things critical to this thread
		// there should be no timing critical things occurring if we have no speed set so increasing this is probably ok too
		vTaskDelay(100 * portTICK_PERIOD_MS);
	}
}

void Stepper::privQueueNotifyMessage(StepperNotifyType aType, int32_t aValue)
{
	StepperNotifyMessage *message = new StepperNotifyMessage(aType, aValue);
	xQueueOverwrite(myNotifyCallbackQueue, &message);
}

Stepper::MoveState Stepper::GetMoveState()
{
	MoveState moveState = myMoveState;
	return moveState;
}

StepperError Stepper::SetCurrentPosition(int32_t aPosition)
{
	StepperCommandSetCurrentPosition *command = new StepperCommandSetCurrentPosition(aPosition);
	return privQueueCommand(command);
}

void Stepper::privSetCurrentPosition(int32_t aPosition)
{
	myCurrentPosition = aPosition;
}

StepperError Stepper::privQueueCommand(StepperCommand *aCommand)
{
	switch (xQueueSend(myCommandQueue, &aCommand, STEPPER_COMMAND_TIMEOUT * portTICK_PERIOD_MS))
	{
	case pdPASS:
		return StepperError::OK;
	case errQUEUE_FULL:
		return StepperError::QUEUE_FULL;
	default:
		return StepperError::UNKNOWN;
	}
}

StepperError Stepper::SetTargetPosition(int32_t targetPosition)
{
	StepperCommandSetTargetPosition *command = new StepperCommandSetTargetPosition(targetPosition);
	return privQueueCommand(command);
}

void Stepper::privSetTargetPosition(int32_t targetPosition)
{
	if (targetPosition != myTargetPosition && myMoveState == MoveState::IDLE)
	{
		// this is important because some state machines block until the stepper returns to the IDLE state
		// so this prevents the race condition where the stepper update() loop does not set the state before
		// the calling state machine checks to see if the move is complete
		myMoveState = MoveState::ACCELERATING;
	}

	myTargetPosition = targetPosition;
}

int32_t Stepper::GetCurrentPosition()
{
	int32_t currentPosition = myCurrentPosition;
	return currentPosition;
}

int32_t Stepper::GetTargetPosition()
{
	int32_t targetPosition = myTargetPosition;
	return targetPosition;
}

StepperError Stepper::SetTargetSpeed(uint16_t aSpeed)
{
	StepperCommandSetTargetSpeed *command = new StepperCommandSetTargetSpeed(aSpeed);
	return privQueueCommand(command);
}

void Stepper::privSetTargetSpeed(uint16_t aSpeed)
{
	myTargetSpeed = aSpeed;
}

uint16_t Stepper::GetTargetSpeed()
{
	uint16_t targetSpeed = myTargetSpeed;
	return targetSpeed;
}

StepperError Stepper::SetAcceleration(float aAcceleration)
{
	StepperCommandSetAcceleration *command = new StepperCommandSetAcceleration(aAcceleration);
	return privQueueCommand(command);
}

void Stepper::privSetAcceleration(float aAcceleration)
{
	myAcceleration = aAcceleration;
}

float Stepper::GetAcceleration()
{
	return myAcceleration;
}

uint16_t Stepper::GetCurrentSpeed()
{
	return myCurrentSpeed;
}

void Stepper::NotifyCallbackTask(void *pvParameters)
{
	Stepper *myInstance = static_cast<Stepper *>(pvParameters);
	StepperNotifyMessage *message = nullptr;
	while (true)
	{
		if (xQueueReceive(myInstance->myNotifyCallbackQueue, &message, portMAX_DELAY) == pdTRUE)
		{
			if (message != nullptr)
			{
				myInstance->myStateOutputCallback(message);
				delete (message); // NOTE (I think this doesn't need to be deleted but maybe? message is a pointer, and queue should have a reference to this pointer, so I think deleting this pointer is correct but the queue should delete it's own ref...?)
			}
		}
	}
}