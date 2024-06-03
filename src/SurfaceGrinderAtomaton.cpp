#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"
#include "hardware/interp.h"
#include "hardware/pio.h"
#include "hardware/spi.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <cstdio>
#include <stdio.h>

// Data will be copied from src to dst
const char src[] = "Hello, world! (from DMA)";
char dst[count_of(src)];

#include "blink.pio.h"

extern "C"
{
#include "picostepper.h"
}

void movement_finished(PicoStepper device)
{
	picostepper_move_async(device, 100, &movement_finished);
}

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq)
{
	blink_program_init(pio, sm, offset, pin);
	pio_sm_set_enabled(pio, sm, true);

	printf("Blinking pin %d at %d Hz\n", pin, freq);

	// PIO counter program takes 3 more cycles in total than we pass as
	// input (wait for n + 1; mov; jmp)
	pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
	// Put your timeout handler code in here
	return 0;
}

int main()
{
	stdio_init_all();

	sleep_ms(1000);

	// STEPPERS

	uint base_pin = 10;
	PicoStepper device = picostepper_init(base_pin, FourWireDriver);
	uint delay = 5;
	bool direction = true;
	bool enabled = true;
	uint steps = 10;

	picostepper_set_async_delay(device, delay);
	picostepper_set_async_direction(device, direction);
	picostepper_set_async_enabled(device, enabled);

	picostepper_move_async(device, steps, &movement_finished);

	// Lastly, watchdog and the led

	// PIO Blinking example
	PIO pio = pio0;
	uint offset = pio_add_program(pio, &blink_program);
	printf("Loaded program at %d\n", offset);

#ifdef PICO_DEFAULT_LED_PIN
	blink_pin_forever(pio, 0, offset, PICO_DEFAULT_LED_PIN, 3);
#else
	blink_pin_forever(pio, 0, offset, 6, 3);
#endif

	// Watchdog example code
	if (watchdog_caused_reboot())
	{
		printf("Rebooted by Watchdog!\n");
		// Whatever action you may take if a watchdog caused a reboot
	}

	// Enable the watchdog, requiring the watchdog to be updated every 100ms or the chip will reboot
	// second arg is pause on debug which means the watchdog will pause when stepping through code
	watchdog_enable(2000, 1);

	// You need to call this function at least more often than the 100ms in the enable call to prevent a reboot
	watchdog_update();

	printf("System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
	printf("USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));
	// For more examples of clocks use see https://github.com/raspberrypi/pico-examples/tree/master/clocks

	while (true)
	{
		watchdog_update();
		// printf("Hello, world!\n");
		// Accelerate
		for (size_t i = 100; i > 0; i--)
		{
			picostepper_set_async_delay(device, i);
			sleep_ms(10);
		}

		// Deaccelerate
		for (size_t i = 0; i < 100; i++)
		{
			picostepper_set_async_delay(device, i);
			sleep_ms(10);
		}
	}
}
