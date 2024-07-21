#include "unity_config.h"
#include <hardware/gpio.h>
#include <hardware/uart.h>
#include <pico/platform.h>
#include <pico/stdio.h>
#include <unity.h>

void setUp(void)
{
	// Set up code here
}

void tearDown(void)
{
	// Tear down code here
}

void test_example(void)
{
	TEST_ASSERT_EQUAL(1, 1);
}

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

int main(void)
{
	stdio_init_all();

	UNITY_BEGIN();
	RUN_TEST(test_example);
	UNITY_END();

	while (true)
	{
		tight_loop_contents(); // Required to keep the firmware running
	}

	return 0;
}
