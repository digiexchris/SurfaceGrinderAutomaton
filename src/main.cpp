
#include "Display.hpp"
#include "Enum.hpp"
#include "config.hpp"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "Motion/XAxis.hpp"
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>

LOG_MODULE_REGISTER(main);

const Display *myDisplay;
const XAxis *myXAxis;

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
			 "Console device is not ACM CDC UART device");

int main(void)
{

	const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t dtr = 0;

	if (usb_enable(NULL))
	{
		return 0;
	}

	/* Poll if the DTR flag was set */
	while (!dtr)
	{
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		/* Give CPU resources to low priority threads. */
		k_sleep(K_MSEC(100));
	}

	// while (1)
	// {
	// 	printk("Hello World! %s\n", CONFIG_ARCH);
	// 	k_sleep(K_SECONDS(1));
	// }

	LOG_INF("Booting %s\n", CONFIG_BOARD);

	myXAxis = new XAxis();

	LOG_INF("XAxis initialized\n");

	myDisplay = new Display();

	LOG_INF("Display initialized\n");

	LOG_INF("Boot Complete\n");

	while (1)
	{
		k_sleep(K_FOREVER);
	}
}