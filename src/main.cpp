#include "Enum.hpp"
#include "config.hpp"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

int main(void)
{

	LOG_INF("Hello World! %s\n", CONFIG_BOARD);

	while (1)
	{
		k_sleep(K_FOREVER);
	}
}