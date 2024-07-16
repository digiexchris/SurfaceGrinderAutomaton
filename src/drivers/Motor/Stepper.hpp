#ifndef STEPPER_HPP
#define STEPPER_HPP

#include "hardware/pio.h"
#include <FreeRTOS.h>
#include <cstdint>
#include <semphr.h>
#include <string>

enum class StepperError
{
	OK = 0,
	QUEUE_FULL,
	UNKNOWN = -1
};

struct StepperCommand
{
	enum class CommandName
	{
		SET_TARGET_POSITION = 1,
		SET_TARGET_SPEED,
		SET_CURRENT_POSITION,
		SET_ACCELERATION
	};

	StepperCommand(CommandName aCommand)
		: command(aCommand)
	{
	}

	CommandName command;
};

struct StepperCommandSetTargetPosition : StepperCommand
{
	StepperCommandSetTargetPosition(int32_t targetPosition, uint16_t speed = 0)
		: targetPosition(targetPosition), StepperCommand(CommandName::SET_TARGET_POSITION)
	{
	}
	int32_t targetPosition;
};

struct StepperCommandSetTargetSpeed : StepperCommand
{
	StepperCommandSetTargetSpeed(uint16_t speed)
		: speed(speed), StepperCommand(CommandName::SET_TARGET_SPEED)
	{
	}
	uint16_t speed;
};

struct StepperCommandSetCurrentPosition : StepperCommand
{
	StepperCommandSetCurrentPosition(int32_t position)
		: position(position), StepperCommand(CommandName::SET_CURRENT_POSITION)
	{
	}
	int32_t position;
};

struct StepperCommandSetAcceleration : StepperCommand
{
	StepperCommandSetAcceleration(float acceleration)
		: acceleration(acceleration), StepperCommand(CommandName::SET_ACCELERATION)
	{
	}
	float acceleration;
};

class Stepper
{
public:
	Stepper(uint stepPin, uint dirPin, float targetSpeed, float acceleration, PIO pio, uint sm);
	void InitPIO();

	enum class MoveState
	{
		IDLE = 0,
		ACCELERATING,
		CONSTANT_SPEED,
		DECELERATING,
		CHANGING_DIRECTION
	};

	static inline std::string MoveStateToString(MoveState aState)
	{
		switch (aState)
		{
		case MoveState::IDLE:
			return std::string("IDLE");
		case MoveState::ACCELERATING:
			return std::string("ACCELERATING");
		case MoveState::CONSTANT_SPEED:
			return std::string("CONSTANT_SPEED");
		case MoveState::DECELERATING:
			return std::string("DECELERATING");
		case MoveState::CHANGING_DIRECTION:
			return std::string("CHANGING_DIRECTION");
		default:
			return std::string("UNKNOWN");
		}
	}

	MoveState GetMoveState();
	StepperError SetTargetPosition(int32_t targetPosition);
	int32_t GetCurrentPosition();
	StepperError SetCurrentPosition(int32_t aPosition);
	int32_t GetTargetPosition();
	virtual StepperError SetTargetSpeed(uint16_t aSpeed);
	uint16_t GetTargetSpeed();
	StepperError SetAcceleration(float aAcceleration);
	float GetAcceleration();
	uint16_t GetCurrentSpeed();
	// call this as fast as possible
	void Update();

protected:
	bool SetMoveState(MoveState aState)
	{
		myMoveState = aState;
		return true;
	}

private:
	void SetDirection(bool direction);
	void DirectionChangedWait();
	void privSetTargetPosition(int32_t targetPosition);
	void privSetTargetSpeed(uint16_t aSpeed);
	void privSetAcceleration(float aAcceleration);
	void privSetCurrentPosition(int32_t aPosition);
	StepperError privQueueCommand(StepperCommand *aCommand);
	uint stepPin;
	uint dirPin;

	PIO pio;
	uint sm;
	bool myDirection;

	float stepDelay;

	MoveState myMoveState = MoveState::IDLE;
	int32_t myCurrentPosition = 0; // in steps
	int32_t myTargetPosition = 0;  // in steps
	uint16_t myTargetSpeed = 0;	   // in steps per second
	float myCurrentSpeed = 0.0;	   // in steps per second
	float myAcceleration = 0.0f;   // in steps per second squared

	QueueHandle_t myCommandQueue;
};

#endif // STEPPER_HPP
