// #include "unity_config.h"
#include <hardware/gpio.h>
#include <hardware/uart.h>
#include <pico/platform.h>
#include <pico/stdio.h>
// #include <unity.h>
#include "CppUTest/CommandLineTestRunner.h"

TEST_GROUP(FirstTestGroup){};

TEST(FirstTestGroup, FirstTest)
{
	FAIL("Fail me!");
}

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

int main(void)
{
	stdio_init_all();
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	gpio_put(PICO_DEFAULT_LED_PIN, 1);

	// UNITY_BEGIN();
	// RUN_TEST(test_example);

	// UNITY_END();

	const char *const *av = (const char *const *)malloc(1 * sizeof(char *));

	CommandLineTestRunner::RunAllTests(0, av);

	while (true)
	{
		tight_loop_contents(); // Required to keep the firmware running
	}

	return 0;
}