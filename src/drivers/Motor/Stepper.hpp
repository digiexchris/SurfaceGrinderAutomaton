#pragma once
#include "Enum.hpp"
#include "hardware/pio.h"
#include <cstdint>

struct StepperDevice
{
    int StepsToTake = 0;
    bool activedir = true;
    bool dirchange = true;
    float ActiveAngle = 0.0; // 360/res
    int DIR_PIN = 11;
    int STEP_PIN = 10;
	int EN_PIN = 9;
    int directionChangeDelayCounter = 0;
    PIO stm_pio = pio0;
    int stm_sm;
	int directionChangeDelay = 50;
};

/**
 * @brief Stepper motor driver
 *
 * This class provides an interface to control a stepper motor.
 * @param aStepperConfig The configuration for the stepper motor.
 * @param aMaxSpeed The maximum speed for the stepper motor in steps per second.
 * @param aMaxAcceleration The maximum acceleration for the stepper motor in steps per second^2.
 * @param aMaxDeceleration The maximum deceleration for the stepper motor in steps per second^2.
 * @param aDisableDelay The delay in milliseconds before disabling the stepper motor after stopping. 0 means never disable.
 */

class Stepper
{

public:
	enum class DirectionState
	{
		POS = 1,
		NEG = -1,
		STOPPED = 0
	};
	enum class EnableState
	{
		ENABLED = 1,
		DISABLED = 0
	};
	enum class MovementState
	{
		IDLE,
		ACCELERATING,
		DECELERATING,
		CONSTANT_SPEED
	};

	enum class MoveType
	{
		DISTANCE,
		CONTINUOUS
	};

	Stepper() = delete;

	Stepper(int aDirPin, int aStepPin, int anEnablePin, uint32_t aDisableDelay = 0);

	/**
	 * @brief Set the maximum speed for the stepper motor.
	 *
	 * @param aMaxSpeed The maximum speed for the stepper motor in steps per second.
	 */
	void SetMaxSpeed(uint32_t aMaxSpeed);

	/**
	 * @brief Set the acceleration for the stepper motor.
	 *
	 * @param aMaxAcceleration The acceleration for the stepper motor in steps per second^2.
	 */
	void SetAcceleration(uint32_t aAcceleration);

	/**
	 * @brief Set the  deceleration for the stepper motor.
	 *
	 * @param aMaxDeceleration The deceleration for the stepper motor in steps per second^2. 0 means it will use the acceleration value.
	 */
	void SetDeceleration(uint32_t aDeceleration);

	/**
	 * @brief Set the delay before disabling the stepper motor after stopping.
	 *
	 * @param aDisableDelay The delay in milliseconds before disabling the stepper motor after stopping. 0 means never disable.
	 */
	void SetDisableDelay(uint32_t aDisableDelay);

	/**
	 * @brief Enable the stepper motor.
	 */
	void Enable();

	/**
	 * @brief Disable the stepper motor.
	 * If the motor is moving, this will flush the movement queue and stop without decellerating.
	 */
	void Disable();

	/**
		* @brief Move the stepper motor a number of steps at a given speed.
		* If moves are in progress, it will add on steps to the current movement queue.
		*
		* @param aSteps The number of steps to move. Positive values move the motor in the positive direction, negative values move the motor in the negative direction.
		* @param aSpeed The speed to move the motor in steps per second.

	*/
	void Move(int steps, int speed);

	void MoveRelative(int steps, DirectionState dir);

	/**
		* @brief Move the stepper motor continuously in a given direction at a given speed.
		* This continually adds a movement to the movement queue at the same speed as the moves are removed from the movement queue and executed, while respecting acceleration and deceleration.
		* If it is already moving, it will change the speed. If it is already moving and a different direction is requested, it will stop and start moving in the new direction.
		*
		* @param aDirection The direction to move the motor.
		* @param aSpeed The speed to move the motor in steps per second.

	*/
	void MoveContinuous(DirectionState aDirection, int aSpeed);

	/**
	 * @brief Stop the stepper motor.
	 * It will flush the current movement queue and start decelerating from the current speed.
	 */
	void Stop();

	/**
	 * @brief Update the stepper motor.
	 * This should be called periodically to update the stepper motor state in case
	 * the initial move did not completely finish all of the steps.
	 * @return true if the stepper motor is still moving, false if it has stopped.
	 */
	bool Update();

	uint32_t GetRemainingSteps();

private:
	StepperDevice* myStepperDevice;

	uint32_t myDisableDelay;

	DirectionState myDirection = DirectionState::STOPPED;
	EnableState myEnableState = EnableState::DISABLED;
	MovementState myMovementState = MovementState::IDLE;
	MoveType myMoveType = MoveType::DISTANCE;
	int myTargetSteps = 0;
	int myCurrentSteps = 0;
	int mySpeed = 0;
	int myAcceleration = 0;
	int myDeceleration = 0;

	bool privChangeMotorDirection();
	void privSetupPIO();
	void privSetupGPIO();
	inline uint32_t privConstruct32bits(int pulsecnt);
	bool privUpdatePendingMovement();
	float privCountAngle(float currentAngle, float counterVal, bool dir); //???
	void privPutStepsBlocking(uint32_t steps);
	void privMoveRelative(int steps, DirectionState dir); //(degree position, steppernumber, direction to move (0-fastest route,1-forward,2-backward))
};