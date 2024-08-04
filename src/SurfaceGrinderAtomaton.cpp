#include "config.hpp"
// #include "hardware/watchdog.h"
#include "pico/platform.h"
#include "pico/stdlib.h"
#include <Motion/MotionController.hpp>
#include <iostream>
#include <pico.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "Console/Console.hpp"
#include <FreeRTOS.h>
#include <cstdio>
#include <task.h>

#include "pico/printf.h"

MotionController *mc;
Axis *zAxis;
Axis *xAxis;
Usb *usb;

extern "C" void vTaskSwitchedIn(void);
extern "C" void vTaskSwitchedOut(void);

void vTaskSwitchedIn(void)
{
	// TaskHandle_t xTask;
	// char *pcTaskName;

	// xTask = xTaskGetCurrentTaskHandle();
	// pcTaskName = pcTaskGetName(xTask);

	// if (pcTaskName != nullptr && strcmp(pcTaskName, "Tmr Svc") != 0)
	// {
	// 	printf("Task Switched In: %s\n", pcTaskName);
	// }
}

void vTaskSwitchedOut(void)
{
	// TaskHandle_t xTask;
	// char *pcTaskName;

	// xTask = xTaskGetCurrentTaskHandle();
	// pcTaskName = pcTaskGetName(xTask);

	// if (pcTaskName != nullptr && strcmp(pcTaskName, "Tmr Svc") != 0)
	// {
	// 	printf("Task Switched Out: %s\n", pcTaskName);
	// }
}

// Forward declaration of the HardFault_Handler
extern "C" void isr_hardfault(void);

extern "C" void PrintStackTrace(uint32_t *stackPointer);

void PrintStackTrace(uint32_t *stackPointer)
{
	printf("Hard Fault detected!\n");
	printf("R0  = %08lx\n", stackPointer[0]);
	printf("R1  = %08lx\n", stackPointer[1]);
	printf("R2  = %08lx\n", stackPointer[2]);
	printf("R3  = %08lx\n", stackPointer[3]);
	printf("R12 = %08lx\n", stackPointer[4]);
	printf("LR  = %08lx\n", stackPointer[5]);
	printf("PC  = %08lx\n", stackPointer[6]);
	printf("PSR = %08lx\n", stackPointer[7]);

	while (true)
	{
		tight_loop_contents();
	}
}

extern "C" void isr_hardfault(void)
{
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
}

int main()
{
	stdio_init_all();
	set_sys_clock_khz(120000, true);

	printf("\n\n--------------\n\n");

	PIO pio = pio1;

	zAxis = new Axis(AxisLabel::Z, ZAXIS_STEP_PIN, ZAXIS_DIR_PIN, ZAXIS_MAX_SPEED, ZAXIS_ACCELERATION, pio, 0);

	// up/down stepper, not implemented Stepper yStepper(YAXIS_STEP_PIN, YAXIS_DIR_PIN, YAXIS_MAX_SPEED, YAXIS_ACCELERATION, pio, 1);

	xAxis = new Axis(AxisLabel::X, XAXIS_STEP_PIN, XAXIS_DIR_PIN, XAXIS_MAX_SPEED, XAXIS_ACCELERATION, pio, 2);

	zAxis->InitPIO();
	xAxis->InitPIO();

	mc = new MotionController(zAxis, xAxis, nullptr);

	// printf("MotionController created\n");

	Console::Init(mc);

	printf("Boot Complete\n");

	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	gpio_put(PICO_DEFAULT_LED_PIN, 1);

	vTaskStartScheduler();
	// It'll never get past here, vTaskStartScheduler() never returns

	printf("If you see this, there is insufficient heap memory\n");

	// Console::Init(nullptr);

	while (true)
	{
	}
}