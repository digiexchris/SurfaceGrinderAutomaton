#include "Stepper.hpp"
#include "stepper.pio.h"
#include <cmath>
#include <stdio.h>

Stepper::Stepper(uint stepPin, uint dirPin, float maxSpeed, float acceleration, PIO pio, uint sm)
    : stepPin(stepPin), dirPin(dirPin), maxSpeed(maxSpeed), acceleration(acceleration), pio(pio), sm(sm) {
    gpio_init(dirPin);
    gpio_set_dir(dirPin, GPIO_OUT);
}

void Stepper::InitPIO() {
    printf("Initializing PIO for stepper...\n");
    uint offset = pio_add_program(pio, &simplestepper_program);
    pio_sm_config c = simplestepper_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, stepPin);  // Configure the step pin for side-set operations
    sm_config_set_set_pins(&c, stepPin, 1);   // Configure the step pin for set operations
    pio_gpio_init(pio, stepPin);              // Initialize the GPIO pin
    pio_sm_set_consecutive_pindirs(pio, sm, stepPin, 1, true); // Set the GPIO direction to output

    // Calculate the clock divider for 200 steps per second
    float clockDiv = 125; // 200 steps per second, each step has 2 phases (high and low)
    sm_config_set_clkdiv(&c, clockDiv);           // Set the clock divider

    pio_sm_init(pio, sm, offset, &c);             // Initialize the state machine
    pio_sm_set_enabled(pio, sm, true);            // Enable the state machine
    printf("PIO initialized with clock divider: %.2f\n", clockDiv);
}

void Stepper::SetDirection(bool direction) {
    this->direction = direction;
    gpio_put(dirPin, direction);
    printf("Direction set to %d\n", direction);
}

void Stepper::Move(int totalSteps) {
    // Ensure the FIFO is empty before sending new commands
    while (!pio_sm_is_tx_fifo_empty(pio, sm)) {
        tight_loop_contents();
    }

    float currentSpeed = 0;
    float stepDelay = 0;
    float targetSpeed = maxSpeed;
    
    // Calculate the maximum steps for acceleration and deceleration
    int maxStepsToAccelerate = (targetSpeed * targetSpeed) / (2 * acceleration);
    int stepsToAccelerate = std::min(totalSteps / 2, maxStepsToAccelerate);
    int stepsToDecelerate = stepsToAccelerate;
    int stepsToCruise = totalSteps - stepsToAccelerate - stepsToDecelerate;

    // Acceleration phase
    for (int step = 0; step < stepsToAccelerate; ++step) {
        currentSpeed = sqrt(2 * acceleration * (step + 1)); // Increment step for each iteration
        stepDelay = 1e6 / currentSpeed / 2;  // /2 because we need high and low delays
        uint32_t delay = static_cast<uint32_t>(stepDelay);
        while (delay > 0xFFFF) {
            pio_sm_put_blocking(pio, sm, 0xFFFF);
            delay -= 0xFFFF;
        }
        pio_sm_put_blocking(pio, sm, delay);
    }

    // Cruising phase
    stepDelay = 1e6 / targetSpeed / 2;  // /2 because we need high and low delays
    for (int step = 0; step < stepsToCruise; ++step) {
        uint32_t delay = static_cast<uint32_t>(stepDelay);
        while (delay > 0xFFFF) {
            pio_sm_put_blocking(pio, sm, 0xFFFF);
            delay -= 0xFFFF;
        }
        pio_sm_put_blocking(pio, sm, delay);
    }

    // Deceleration phase
    for (int step = stepsToDecelerate; step > 0; --step) {
        currentSpeed = sqrt(2 * acceleration * step); // Decrement step for each iteration
        stepDelay = 1e6 / currentSpeed / 2;  // /2 because we need high and low delays
        uint32_t delay = static_cast<uint32_t>(stepDelay);
        while (delay > 0xFFFF) {
            pio_sm_put_blocking(pio, sm, 0xFFFF);
            delay -= 0xFFFF;
        }
        pio_sm_put_blocking(pio, sm, delay);
    }

    printf("Move called with totalSteps: %d\n", totalSteps);
}
