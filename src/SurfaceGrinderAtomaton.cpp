#include "Console/Console.hpp"
#include "Ui/ControlPanel/Display/Display.hpp"
#include "config.hpp"
#include "pico/platform.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include <FreeRTOS.h>
#include <Motion/MotionController.hpp>
#include <array>
#include <cstdio>
#include <hardware/clocks.h>
#include <iostream>
#include <pico.h>
#include <pico/bootrom.h>
#include <pico/printf.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/_stdint.h>
#include <task.h>
#include <unordered_map>

#if PICO_W_LED
#include "pico/cyw43_arch.h"
#endif

MotionController *mc;
Axis *zAxis;
Axis *xAxis;

Display *display;

// Forward declaration of the HardFault_Handler
extern "C" void isr_hardfault(void);

extern "C" void PrintStackTrace(uint32_t *stackPointer);

void PrintStackTrace(uint32_t *stackPointer)
{
	printf("Hard Fault detected!\n");
	printf("R0  = 0x%08x\n", stackPointer[0]);
	printf("R1  = 0x%08x\n", stackPointer[1]);
	printf("R2  = 0x%08x\n", stackPointer[2]);
	printf("R3  = 0x%08x\n", stackPointer[3]);
	printf("R12 = 0x%08x\n", stackPointer[4]);
	printf("LR  = 0x%08x\n", stackPointer[5]);
	printf("PC  = 0x%08x\n", stackPointer[6]);
	printf("PSR = 0x%08x\n", stackPointer[7]);
}

void BlinkTask(void *pvParameters)
{
	while (true)
	{

#if PICO_W_LED
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
		sleep_ms(250);
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
		sleep_ms(250);
#else
		gpio_put(PICO_DEFAULT_LED_PIN, 0);
		vTaskDelay(pdMS_TO_TICKS(500));
		gpio_put(PICO_DEFAULT_LED_PIN, 1);
		vTaskDelay(pdMS_TO_TICKS(500));
#endif
	}
}

extern "C" void isr_hardfault(void)
{
	printf("Hard Fault detected!\n");

	__breakpoint();
	__asm volatile(
		"MOVS R0, #4 \n"
		"MOV R1, LR \n"
		"TST R0, R1 \n"
		"BEQ _MSP \n"
		"MRS R0, PSP \n"
		"B PrintStackTrace \n"
		"_MSP: \n"
		"MRS R0, MSP \n"
		"B PrintStackTrace \n");

	__breakpoint();

	sleep_ms(10000);	  // Sleep for 10 seconds
	reset_usb_boot(0, 0); // Reboot the Raspberry Pi Pico
}

int main()
{
	stdio_init_all();
	set_sys_clock_khz(120000, true);
	Console::Init(mc);

	asserts aren't working, figure that out  assert(1 != 0);

	display = new Display();

	printf("\n\n--------------\n\n");

	PIO pio = pio1;

	zAxis = new Axis(AxisLabel::Z, ZAXIS_STEP_PIN, ZAXIS_DIR_PIN, ZAXIS_MAX_SPEED, ZAXIS_ACCELERATION, pio, 0);

	// up/down stepper, not implemented Stepper yet Stepper(YAXIS_STEP_PIN, YAXIS_DIR_PIN, YAXIS_MAX_SPEED, YAXIS_ACCELERATION, pio, 1);

	xAxis = new Axis(AxisLabel::X, XAXIS_STEP_PIN, XAXIS_DIR_PIN, XAXIS_MAX_SPEED, XAXIS_ACCELERATION, pio, 2);

	zAxis->InitPIO();
	xAxis->InitPIO();

	mc = new MotionController(zAxis, xAxis, nullptr);

	printf("Boot Complete\n");

	xTaskCreate(BlinkTask, "BlinkTask", 1024, NULL, 1, NULL);

	vTaskStartScheduler();
	// It'll never get past here, vTaskStartScheduler() never returns

	printf("If you see this, there is probably insufficient heap memory\n");

	while (true)
	{
	}
}