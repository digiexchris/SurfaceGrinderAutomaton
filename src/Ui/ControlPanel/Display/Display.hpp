#pragma once

#include <ssd1306.h>

class Display
{
public:
	Display();

	void Update();
	static void UpdateTask(void *pvParameters);

private:
	pico_ssd1306::SSD1306 *myDisplay = nullptr;
};