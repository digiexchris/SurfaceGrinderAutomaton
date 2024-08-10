#include "Display.hpp"
#include "Helpers.hpp"
#include "config.hpp"
#include <hardware/i2c.h>
#include <pico/stdlib.h>
#include <shapeRenderer/ShapeRenderer.h>
#include <ssd1306.h>

Display::Display()
{
	i2c_init(I2C_PORT, 1000000);
	// Set up pins 12 and 13
	gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
	gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(I2C_SDA);
	gpio_pull_up(I2C_SCL);

	xTaskCreate(Display::UpdateTask, "DisplayTask", 2048, this, 1, NULL);
}

void Display::UpdateTask(void *pvParameters)
{

	Display *display = static_cast<Display *>(pvParameters);

	// Required delay to allow the SSD1309 to set itself up
	vTaskDelay(MS_TO_TICKS(250));

	// if DC->GND, then 0x3C, if DC->3v3 then 0x3D
	display->myDisplay = new pico_ssd1306::SSD1306(I2C_PORT, 0x3C, pico_ssd1306::Size::W128xH64);

	while (true)
	{
		display->Update();
		vTaskDelay(MS_TO_TICKS(250));
	}
}

void Display::Update()
{
	// Draw an outline
	drawRect(myDisplay, 0, 0, 127, 63);

	// Draw 2 rectangles
	fillRect(myDisplay, 0, 0, 63, 31);
	fillRect(myDisplay, 64, 32, 127, 63);

	// Draw a line across the screen
	drawLine(myDisplay, 127, 0, 0, 63);

	// Send buffer to the display
	myDisplay->sendBuffer();
}