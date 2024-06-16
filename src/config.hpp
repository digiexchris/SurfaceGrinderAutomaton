#pragma once
#include <cstdint>

#define STEPPER_DIRECTION_CHANGE_DELAY_MS 5

#define PRINTF_AXIS_POSITIONS 1
#define PRINTF_AXIS_DEBUG 0
#define PRINTF_MOTION_DEBUG 0
#define PRINTF_STEPPER_DEBUG 1

#define ENABLE_DISPLAY 1

#define STEPPER1_ENABLE_POLARITY 0

#define XAXIS_STEPS_PER_MM 400
#define XAXIS_MAX_SPEED 1000.0f	  // steps per second
#define XAXIS_ACCELERATION 250.0f // steps per second squared
#define XAXIS_STEP_PIN 10
#define XAXIS_DIR_PIN 9
#define XAXIS_ENABLE_PIN 8

#define ZAXIS_STEPS_PER_MM 400
#define ZAXIS_MAX_SPEED 1000.0f	  // steps per second
#define ZAXIS_ACCELERATION 250.0f // steps per second squared
#define ZAXIS_STEP_PIN 7		  // PICO_DEFAULT_LED_PIN // 7
#define ZAXIS_DIR_PIN 6
#define ZAXIS_ENABLE_PIN 5

#define TFT_MISO 16
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS 15 // Chip select control pin
#define TFT_DC 2  // Data Command control pin
// #define TFT_RST   4  // Reset pin (could connect to RST pin)
#define TFT_RST 12 // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST

#define TOUCH_CS 33