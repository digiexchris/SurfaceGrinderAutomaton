#pragma once
#include "pico/stdio/driver.h"
#include "pico/stdlib.h"
#include "portmacro.h"

#include <FreeRTOS.h>
#include <cstdint>
#include <cstdio>
#include <pico/stdio.h>
#include <task.h>
#include <tusb.h>

#define URL "example.tinyusb.org/webusb-serial/index.html"

class Usb
{
public:
	typedef void (*ProcessBufFn)(const void *data, size_t len);

	enum class Blink
	{
		BLINK_NOT_MOUNTED = 250,
		BLINK_MOUNTED = 1000,
		BLINK_SUSPENDED = 2500,
		BLINK_ALWAYS_ON = INT32_MAX,
		BLINK_ALWAYS_OFF = 0
	};
	Usb(ProcessBufFn processBufFn = nullptr);

	static void print(const char *buf, int len);
	// int putc((const char *buf, int len));

	static Usb *getInstance() { return myInstance; }
	bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);
	void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts);
	void tud_cdc_rx_cb(uint8_t itf);
	void tud_mount_cb(void);
	void tud_umount_cb(void);
	void tud_suspend_cb(bool remote_wakeup_en);
	void tud_resume_cb(void);

private:
	ProcessBufFn myProcessBufFn = nullptr;
	// void echo_all(uint8_t buf[], uint32_t count);

	void webserial_task(void);
	void cdc_task(void);

	static void usb_thread(void *ptr);
	static void led_blinking_task(void);

	uint32_t blink_interval_ms = (uint32_t)Blink::BLINK_NOT_MOUNTED;
	static const tusb_desc_webusb_url_t desc_url;
	bool web_serial_connected = false;
	TaskHandle_t tud_taskhandle;

	// Custom driver structure
	// const struct __file __stdout_usb = {
	// 	.putc = putc,
	// 	.getc = NULL,
	// 	.flags = 0,
	// 	.buffer = NULL};

	stdio_driver_t __stdout_usb = {
		.out_chars = print,
		.in_chars = NULL, // No input function
		.next = NULL};

	stdio_driver_t __stderr_usb = {
		.out_chars = print,
		.in_chars = NULL, // No input function
		.next = NULL};

	static Usb *myInstance;
};