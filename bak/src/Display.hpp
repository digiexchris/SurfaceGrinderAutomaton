#pragma once
#include <cstdint>
#include <lvgl.h>

class Display
{
public:
	Display();

private:
	const struct device *myDisplayDevice;
	const uint16_t x_res = 240;
	const uint16_t y_res = 320;
	lv_disp_t *myLvDisplay;

	lv_obj_t *myMainScreenView;

	static void DisplayUpdateThread(void *arg);
	void BuildMainView();
};