/*
This was taken almost entirely from the tinyusb webserial example.
*/

#include "usb.hpp"
#include "bsp/board.h"
#include "usb/usb_descriptors.h"

#define UART_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define TUD_TASK_PRIO (tskIDLE_PRIORITY + 2)
#define DAP_TASK_PRIO (tskIDLE_PRIORITY + 1)

const tusb_desc_webusb_url_t Usb::desc_url = {
	.bLength = 3 + sizeof(URL) - 1,
	.bDescriptorType = 3, // WEBUSB URL type
	.bScheme = 1,		  // 0: http, 1: https
	.url = {URL}};

Usb::Usb()
{
	board_init();
	tusb_init();

	xTaskCreate(usb_thread, "TUD", configMINIMAL_STACK_SIZE, NULL, TUD_TASK_PRIO, &tud_taskhandle);
}

void Usb::usb_thread(void *ptr)
{
	TickType_t wake;
	wake = xTaskGetTickCount();
	do
	{
		tud_task();
#ifdef USB_CONNECTED_LED
		if (!gpio_get(USB_CONNECTED_LED) && tud_ready())
			gpio_put(USB_CONNECTED_LED, 1);
		else
			gpio_put(USB_CONNECTED_LED, 0);
#endif
		// Go to sleep for up to a tick if nothing to do
		if (!tud_task_event_ready())
			xTaskDelayUntil(&wake, 1);
	} while (1);
}

// send characters to both CDC and WebUSB
void Usb::echo_all(uint8_t buf[], uint32_t count)
{
	// echo to web serial
	if (web_serial_connected)
	{
		tud_vendor_write(buf, count);
		tud_vendor_flush();
	}

	// echo to cdc
	if (tud_cdc_connected())
	{
		for (uint32_t i = 0; i < count; i++)
		{
			tud_cdc_write_char(buf[i]);

			if (buf[i] == '\r')
				tud_cdc_write_char('\n');
		}
		tud_cdc_write_flush();
	}
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void Usb::tud_mount_cb(void)
{
	blink_interval_ms = (uint32_t)Blink::BLINK_MOUNTED;
}

// Invoked when device is unmounted
void Usb::tud_umount_cb(void)
{
	blink_interval_ms = (uint32_t)Blink::BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void Usb::tud_suspend_cb(bool remote_wakeup_en)
{
	(void)remote_wakeup_en;
	blink_interval_ms = (uint32_t)Blink::BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void Usb::tud_resume_cb(void)
{
	blink_interval_ms = tud_mounted() ? (uint32_t)Blink::BLINK_MOUNTED : (uint32_t)Blink::BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// WebUSB use vendor class
//--------------------------------------------------------------------+

// Invoked when a control transfer occurred on an interface of this class
// Driver response accordingly to the request and the transfer stage (setup/data/ack)
// return false to stall control endpoint (e.g unsupported request)
bool Usb::tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
	// nothing to with DATA & ACK stage
	if (stage != CONTROL_STAGE_SETUP)
		return true;

	switch (request->bmRequestType_bit.type)
	{
	case TUSB_REQ_TYPE_VENDOR:
		switch (request->bRequest)
		{
		case VENDOR_REQUEST_WEBUSB:
			// match vendor request in BOS descriptor
			// Get landing page url
			return tud_control_xfer(rhport, request, (void *)(uintptr_t)&desc_url, desc_url.bLength);

		case VENDOR_REQUEST_MICROSOFT:
			if (request->wIndex == 7)
			{
				// Get Microsoft OS 2.0 compatible descriptor
				uint16_t total_len;
				memcpy(&total_len, desc_ms_os_20 + 8, 2);

				return tud_control_xfer(rhport, request, (void *)(uintptr_t)desc_ms_os_20, total_len);
			}
			else
			{
				return false;
			}

		default:
			break;
		}
		break;

	case TUSB_REQ_TYPE_CLASS:
		if (request->bRequest == 0x22)
		{
			// Webserial simulate the CDC_REQUEST_SET_CONTROL_LINE_STATE (0x22) to connect and disconnect.
			web_serial_connected = (request->wValue != 0);

			// Always lit LED if connected
			if (web_serial_connected)
			{
				// board_led_write(true); //TODO
				blink_interval_ms = (uint32_t)Blink::BLINK_ALWAYS_ON;

				tud_vendor_write_str("\r\nWebUSB interface connected\r\n");
				tud_vendor_flush();
			}
			else
			{
				blink_interval_ms = (uint32_t)Blink::BLINK_MOUNTED;
			}

			// response with status OK
			return tud_control_status(rhport, request);
		}
		break;

	default:
		break;
	}

	// stall unknown request
	return false;
}

void Usb::webserial_task(void)
{
	if (web_serial_connected)
	{
		if (tud_vendor_available())
		{
			uint8_t buf[64];
			uint32_t count = tud_vendor_read(buf, sizeof(buf));

			// echo back to both web serial and cdc
			echo_all(buf, count);
		}
	}
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void Usb::cdc_task(void)
{
	if (tud_cdc_connected())
	{
		// connected and there are data available
		if (tud_cdc_available())
		{
			uint8_t buf[64];

			uint32_t count = tud_cdc_read(buf, sizeof(buf));

			// echo back to both web serial and cdc
			echo_all(buf, count);
		}
	}
}

// Invoked when cdc when line state changed e.g connected/disconnected
void Usb::tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
	(void)itf;

	// connected
	if (dtr && rts)
	{
		// print initial message when connected
		tud_cdc_write_str("\r\nTinyUSB WebUSB device example\r\n");
	}
}

// Invoked when CDC interface received data from host
void Usb::tud_cdc_rx_cb(uint8_t itf)
{
	(void)itf;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void Usb::led_blinking_task(void)
{
	// static uint32_t start_ms = 0;
	// static bool led_state = false;

	// // Blink every interval ms
	// if (board_millis() - start_ms < blink_interval_ms)
	// 	return; // not enough time
	// start_ms += blink_interval_ms;

	// board_led_write(led_state);
	// led_state = 1 - led_state; // toggle
	// todo use pwm for this
}