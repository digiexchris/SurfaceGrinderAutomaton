#pragma once
#include <cstdint>

#define ENABLE_DISPLAY 1

#define TFT_MISO 16
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
//#define TFT_RST   4  // Reset pin (could connect to RST pin)
#define TFT_RST  12  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST

#define TOUCH_CS 33 