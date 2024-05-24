
#include "Display.hpp"
#include "Enum.hpp"
#include "config.hpp"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

const Display *myDisplay;

int main(void)
{

	LOG_INF("Booting %s\n", CONFIG_BOARD);

	myDisplay = new Display();

	LOG_INF("Boot Complete\n");

	while (1)
	{
		k_sleep(K_FOREVER);
	}
}