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
    uint offset = pio_add_program(pio, &stepper_program);
    pio_sm_config c = stepper_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, stepPin);  // Configure the step pin for side-set operations
    sm_config_set_set_pins(&c, stepPin, 1);   // Configure the step pin for set operations
    pio_gpio_init(pio, stepPin);              // Initialize the GPIO pin
    pio_sm_set_consecutive_pindirs(pio, sm, stepPin, 1, true); // Set the GPIO direction to output
    pio_sm_init(pio, sm, offset, &c);         // Initialize the state machine
    pio_sm_set_enabled(pio, sm, true);        // Enable the state machine
    printf("PIO initialized.\n");
}

void Stepper::SetDirection(bool direction) {
    this->direction = direction;
    gpio_put(dirPin, direction);
    printf("Direction set to %d\n", direction);
}

void Stepper::GenerateDelays(std::vector<uint32_t> &delays) {
    printf("Generating delays...\n");
    float accelSteps = pow(maxSpeed, 2) / (2 * acceleration);
    int cruiseSteps = totalSteps - 2 * accelSteps;

    for (int step = 0; step < accelSteps; ++step) {
        float stepDelay = 1e6 / (sqrt(2 * acceleration * step));
        if (stepDelay > 0xFFFF) stepDelay = 0xFFFF; // Bound the delay value
        delays.push_back((uint32_t)stepDelay);
    }

    float maxStepDelay = 1e6 / maxSpeed;
    if (maxStepDelay > 0xFFFF) maxStepDelay = 0xFFFF; // Bound the delay value
    for (int step = 0; step < cruiseSteps; ++step) {
        delays.push_back((uint32_t)maxStepDelay);
    }

    for (int step = accelSteps; step > 0; --step) {
        float stepDelay = 1e6 / (sqrt(2 * acceleration * step));
        if (stepDelay > 0xFFFF) stepDelay = 0xFFFF; // Bound the delay value
        delays.push_back((uint32_t)stepDelay);
    }
    printf("Delays generated.\n");
}

void Stepper::Move(int totalSteps) {
    this->totalSteps = totalSteps;
    std::vector<uint32_t> delays;
    GenerateDelays(delays);

    for (auto delay : delays) {
        printf("Pushing delay: %u\n", delay);
        while (pio_sm_is_tx_fifo_full(pio, sm)) {
            // Wait until there is space in the FIFO
            tight_loop_contents();
        }
        pio_sm_put_blocking(pio, sm, delay);
    }
    printf("Move complete.\n");
}
