
#include "Display.hpp"
#include "Enum.hpp"
#include "config.hpp"

#include "Motion/XAxis.hpp"

const Display *myDisplay;
const XAxis *myXAxis;

int main(void)
{


	// while (1)
	// {
	// 	printk("Hello World! %s\n", CONFIG_ARCH);
	// 	k_sleep(K_SECONDS(1));
	// }

	LOG_INF("Booting");

	myXAxis = new XAxis();

	LOG_INF("XAxis initialized\n");

	// myDisplay = new Display();

	LOG_INF("Display initialized\n");

	LOG_INF("Boot Complete\n");

	while (1)
	{
		k_sleep(K_FOREVER);
	}
}