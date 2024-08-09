// #include "bsp/board_api.h"
#include "usb_descriptors.h"
#include "bsp/board.h"
#include "tusb.h"
#include <string.h>

// Device Descriptors
tusb_desc_device_t const desc_device =
	{
		.bLength = sizeof(tusb_desc_device_t),
		.bDescriptorType = TUSB_DESC_DEVICE,
		.bcdUSB = 0x0210,											 // USB 2.1
		.bDeviceClass = TUSB_CLASS_CDC,								 // Communications Device Class
		.bDeviceSubClass = CDC_COMM_SUBCLASS_ABSTRACT_CONTROL_MODEL, // Abstract Control Model
		.bDeviceProtocol = CDC_COMM_PROTOCOL_ATCOMMAND,				 // AT Commands (or 0 if not applicable)
		.bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

		.idVendor = 0xcafe,
		.idProduct = 0x4000,
		.bcdDevice = 0x0100,

		.iManufacturer = 0x01,
		.iProduct = 0x02,
		.iSerialNumber = 0x03,

		.bNumConfigurations = 0x01};

// Configuration Descriptor
enum
{
	ITF_NUM_CDC = 0,
	ITF_NUM_CDC_DATA,
	ITF_NUM_TOTAL
};

#define EPNUM_CDC_NOTIF 1 // Endpoint for CDC control notifications
#define EPNUM_CDC_IN 2	  // Endpoint for CDC data IN
#define EPNUM_CDC_OUT 2	  // Endpoint for CDC data OUT

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

uint8_t const desc_configuration[] =
	{
		TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

		// CDC Interface descriptor
		TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, 0x80 | EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, 0x80 | EPNUM_CDC_IN, 64)};

// String Descriptors
enum
{
	STRID_LANGID = 0,
	STRID_MANUFACTURER,
	STRID_PRODUCT,
	STRID_SERIAL,
};

char const *string_desc_arr[] =
	{
		(const char[]){0x09, 0x04}, // 0: supported language is English (0x0409)
		"SurfaceGrinderAtomaton",	// 1: Manufacturer
		"SGA Debug Port",			// 2: Product
		"123456",					// 3: Serial Number
		"TinyUSB CDC"				// 4: CDC Interface
};

static uint16_t _desc_str[32 + 1];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	(void)langid;

	uint8_t chr_count;

	if (index == 0)
	{
		memcpy(&_desc_str[1], string_desc_arr[0], 2);
		chr_count = 1;
	}
	else
	{
		if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
			return NULL;

		const char *str = string_desc_arr[index];

		chr_count = (uint8_t)strlen(str);
		if (chr_count > 31)
			chr_count = 31;

		for (uint8_t i = 0; i < chr_count; i++)
		{
			_desc_str[1 + i] = str[i];
		}
	}

	_desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

	return _desc_str;
}

// Invoked when received GET DEVICE DESCRIPTOR
// Application returns pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void)
{
	return (uint8_t const *)&desc_device;
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application returns pointer to descriptor
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
	(void)index; // for multiple configurations
	return desc_configuration;
}