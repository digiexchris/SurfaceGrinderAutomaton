#pragma once
#include "Enum.hpp"
#include "zephyr/drivers/gpio.h"
#include "zephyr/drivers/pwm.h"
#include "zephyr/kernel.h"
#include <sys/_stdint.h>
#include <vector>
#include <zephyr/sys/util.h>

struct StepperConfig
{
	struct gpio_dt_spec stepGpio;
	struct gpio_dt_spec dirGpio;
	struct gpio_dt_spec enableGpio;
	uint32_t maxSpeed;
	uint32_t maxAcceleration;
	uint32_t maxDeceleration;
	uint8_t enablePolarity;
	uint8_t dirPolarity;
};

#define STEPPER_MOTOR_INIT(node_id, max_speed, max_acceleration, max_deceleration) \
	{                                                                              \
		.stepGpio = GPIO_DT_SPEC_GET(node_id, step_gpios),                         \
		.dirGpio = GPIO_DT_SPEC_GET(node_id, dir_gpios),                           \
		.enableGpio = GPIO_DT_SPEC_GET(node_id, enable_gpios),                     \
		.maxSpeed = max_speed,                                                     \
		.maxAcceleration = max_acceleration,                                       \
		.maxDeceleration = max_deceleration,                                       \
		.enablePolarity = DT_PROP(node_id, enable_polarity),                       \
		.dirPolarity = DT_PROP(node_id, dir_polarity)                              \
	}

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

	Stepper(const StepperConfig &aStepperConfig, uint32_t aDisableDelay = 0);

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

	bool HasQueuedSteps();

private:
	struct StepperConfig myStepperConfig;

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
	struct k_fifo myMovementQueue;

	struct StepCommand
	{
		int delay;
		sys_snode_t node; // Required for Zephyr FIFO
	};

	struct k_timer myStepTimer;

	void UpdateMovement();
	void ExecuteMovement(const StepCommand &command);
	void CalculateSpeedProfile(int steps, int targetSpeed, std::vector<int> &delays);
	void StartStepTimer(int delay);
	void StepTimerHandler();
	static void TimerHandlerWrapper(struct k_timer *timer);
};