#include "Display.hpp"
#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(Display);

Display::Display()
{

	myDisplayDevice = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(myDisplayDevice))
	{
		printf("Device %s not ready\n", myDisplayDevice->name);
		k_oops();
	}

	BuildMainView();

	lv_task_handler();

	if (display_blanking_off(myDisplayDevice) != 0)
	{
		LOG_ERR("Failed to turn the display blanking off");
		k_oops();
		return;
	}
}

void Display::BuildMainView()
{
	/*Create a container with ROW flex direction*/
	lv_obj_t *cont_row = lv_obj_create(lv_scr_act());
	lv_obj_set_size(cont_row, 300, 75);
	lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 5);
	lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);

	// /*Create a container with COLUMN flex direction*/
	// lv_obj_t *cont_col = lv_obj_create(lv_scr_act());
	// lv_obj_set_size(cont_col, 200, 150);
	// lv_obj_align_to(cont_col, cont_row, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
	// lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);

	// uint32_t i;
	// for (i = 0; i < 10; i++)
	// {
	// 	lv_obj_t *obj;
	// 	lv_obj_t *label;

	// 	/*Add items to the row*/
	// 	obj = lv_btn_create(cont_row);
	// 	lv_obj_set_size(obj, 100, LV_PCT(100));

	// 	label = lv_label_create(obj);
	// 	lv_label_set_text_fmt(label, "Item: %" LV_PRIu32, i);
	// 	lv_obj_center(label);

	// 	/*Add items to the column*/
	// 	obj = lv_btn_create(cont_col);
	// 	lv_obj_set_size(obj, LV_PCT(100), LV_SIZE_CONTENT);

	// 	label = lv_label_create(obj);
	// 	lv_label_set_text_fmt(label, "Item: %" LV_PRIu32, i);
	// 	lv_obj_center(label);
	// }
}

void Display::DisplayUpdateThread(void *arg)
{
	Display *myDisplay = (Display *)arg;
	while (1)
	{

		k_msleep(lv_task_handler());
	}
}