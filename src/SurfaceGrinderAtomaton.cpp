#include "config.hpp"
#include "drivers/Motor/Stepper.hpp"
// #include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include <Motion/MotionController.hpp>
#include <iostream>
#include <pico.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "debug/Console/Console.hpp"
#include <FreeRTOS.h>
#include <cstdio>
#include <task.h>

#include "pico/printf.h"

MotionController *mc;
Stepper *zStepper;
Stepper *xStepper;

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

	// sleep_ms(1000); // time for UART to connect for debugging, comment out later

	printf("\n\n--------------\n\n");
	// Watchdog example code
	// if (watchdog_caused_reboot())
	// {
	// 	printf("Rebooted by Watchdog!\n");
	// 	// Whatever action you may take if a watchdog caused a reboot
	// }

	// Enable the watchdog, requiring the watchdog to be updated every 100ms or the chip will reboot
	// second arg is pause on debug which means the watchdog will pause when stepping through code
	// watchdog_enable(12000, 1);

	// You need to call this function at least more often than the 100ms in the enable call to prevent a reboot
	// watchdog_update();

	PIO pio = pio0;

	zStepper = new Stepper(ZAXIS_STEP_PIN, ZAXIS_DIR_PIN, ZAXIS_MAX_SPEED, ZAXIS_ACCELERATION, pio, 0);

	// up/down stepper, not implemented Stepper yStepper(YAXIS_STEP_PIN, YAXIS_DIR_PIN, YAXIS_MAX_SPEED, YAXIS_ACCELERATION, pio, 1);

	xStepper = new Stepper(XAXIS_STEP_PIN, XAXIS_DIR_PIN, XAXIS_MAX_SPEED, XAXIS_ACCELERATION, pio, 2);

	zStepper->InitPIO();
	xStepper->InitPIO();

	mc = new MotionController(xStepper, zStepper, nullptr);

	// printf("MotionController created\n");

	Console::Init(mc);

	printf("Boot Complete\n");

	vTaskStartScheduler();
	// It'll never get past here, vTaskStartScheduler() never returns

	printf("If you see this, there is insufficient heap memory\n");

	// Console::Init(nullptr);

	while (true)
	{

		// watchdog_update();
		// printf("Moving stepper left\n");
		// stepper1.SetDirection(true);
		// stepper1.Move(1000);
		// sleep_ms(1000);

		// printf("Moving stepper right\n");
		// watchdog_update();
		// stepper1.SetDirection(false);
		// stepper1.Move(1000);
		// sleep_ms(1000);
	}
}
