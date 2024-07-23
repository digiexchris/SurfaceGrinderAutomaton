#pragma once
#include "pico/stdlib.h"

#include <FreeRTOS.h>
#include <cstdint>
#include <cstdio>
#include <task.h>
#include <tusb.h>

#define URL "example.tinyusb.org/webusb-serial/index.html"

class Usb
{
public:
	enum class Blink
	{
		BLINK_NOT_MOUNTED = 250,
		BLINK_MOUNTED = 1000,
		BLINK_SUSPENDED = 2500,
		BLINK_ALWAYS_ON = INT32_MAX,
		BLINK_ALWAYS_OFF = 0
	};
	Usb();

private:
	void echo_all(uint8_t buf[], uint32_t count);
	void tud_mount_cb(void);
	void tud_umount_cb(void);
	void tud_suspend_cb(bool remote_wakeup_en);
	void tud_resume_cb(void);
	bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);
	void webserial_task(void);
	void cdc_task(void);
	void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts);
	void tud_cdc_rx_cb(uint8_t itf);
	static void usb_thread(void *ptr);
	static void led_blinking_task(void);

	uint32_t blink_interval_ms = (uint32_t)Blink::BLINK_NOT_MOUNTED;
	static const tusb_desc_webusb_url_t desc_url;
	bool web_serial_connected = false;
	TaskHandle_t tud_taskhandle;
};