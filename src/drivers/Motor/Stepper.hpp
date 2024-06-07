#ifndef STEPPER_HPP
#define STEPPER_HPP

#include "hardware/pio.h"
#include <vector>

class Stepper {
public:
    Stepper(uint stepPin, uint dirPin, float maxSpeed, float acceleration, PIO pio, uint sm);
    void InitPIO();
    void SetDirection(bool direction);
    void Move(int totalSteps);

private:
    uint stepPin;
    uint dirPin;
    float maxSpeed;
    float acceleration;
    PIO pio;
    uint sm;
    bool direction;

    void GenerateDelays(int totalSteps, std::vector<uint32_t> &delays);
};

#endif // STEPPER_HPP
