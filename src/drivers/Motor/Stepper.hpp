#ifndef STEPPER_HPP
#define STEPPER_HPP

#include "hardware/pio.h"
#include <FreeRTOS.h>
#include <cstdint>
#include <semphr.h>

class Stepper
{
public:
	Stepper(uint stepPin, uint dirPin, float maxSpeed, float acceleration, PIO pio, uint sm);
	void InitPIO();

	enum MoveState
	{
		IDLE,
		ACCELERATING,
		CONSTANT_SPEED,
		DECELERATING,
		CHANGING_DIRECTION
	};

	MoveState GetMoveState();
	void SetTargetPosition(int32_t targetPosition, uint16_t speed = 0);
	int32_t GetCurrentPosition();
	int32_t GetTargetPosition();
	void SetTargetSpeed(uint16_t aSpeed);
	uint16_t GetTargetSpeed();
	void SetAcceleration(float aAcceleration);
	float GetAcceleration();
	uint16_t GetCurrentSpeed();
	// call this as fast as possible
	void Update();

protected:
	bool SetCurrentPosition(int32_t aPosition)
	{
		myCurrentPosition = aPosition;
		return true;
	}

	bool SetMoveState(MoveState aState)
	{
		myMoveState = aState;
		return true;
	}

private:
	void SetDirection(bool direction);
	void DirectionChangedWait();
	uint stepPin;
	uint dirPin;

	PIO pio;
	uint sm;
	bool myDirection;

	float stepDelay;

	MoveState myMoveState = IDLE;
	int32_t myCurrentPosition = 0; // in steps
	int32_t myTargetPosition = 0;  // in steps
	uint16_t myTargetSpeed = 0;	   // in steps per second
	float myCurrentSpeed = 0.0;	   // in steps per second
	float myAcceleration = 0.0f;   // in steps per second squared

	// anything protected by these mutexes are expected to be read or modified by another thread. The stepper can lock these for a long time
	// but the other threads calling the public functions should not lock for a long time
	// todo: maybe use critical secetions for the update() function. see the todo in Stepper::Update
	SemaphoreHandle_t myTargetPositionMutex;
	SemaphoreHandle_t myMoveStateMutex;
	SemaphoreHandle_t myCurrentPositionMutex;
	SemaphoreHandle_t myTargetSpeedMutex;
};

#endif // STEPPER_HPP
