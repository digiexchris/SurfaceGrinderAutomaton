#ifndef STEPPER_HPP
#define STEPPER_HPP

#include <vector>
#include "pico/stdlib.h"
#include "hardware/pio.h"

class Stepper {
public:
    Stepper(uint stepPin, uint dirPin, float maxSpeed, float acceleration, PIO pio, uint sm);
    void InitPIO();
    void SetDirection(bool direction);
    void Move(int totalSteps);

private:
    void GenerateDelays(std::vector<uint32_t> &delays);

    uint stepPin;
    uint dirPin;
    bool direction;
    float maxSpeed; // steps per second
    float acceleration; // steps per second squared
    int totalSteps;
    PIO pio;
    uint sm;
};

#endif // STEPPER_HPP
