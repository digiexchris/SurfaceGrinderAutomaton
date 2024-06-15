

#include "Console.hpp"
#include "Axis.hpp"
#include "Enum.hpp"
#include "Motion/MotionController.hpp"
#include "microsh.h"
#include <malloc.h>
#include <stdio.h>
#include <string>

microsh_t *Console::mySh = nullptr;
MotionController *Console::myMotionController = nullptr;

void Console::Init(MotionController *aMotionController)
{
	myMotionController = aMotionController;
	printf("Initializing console" MICRORL_CFG_END_LINE);
	uart_init(UART_ID, BAUD_RATE);

	// Set the TX and RX pins by using the function select on the GPIO
	// Set datasheet for more information on function select
	// gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	// gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	// Actually, we want a different speed
	// The call will return the actual baud rate selected, which will be as close as
	// possible to that requested
	// int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

	// Set UART flow control CTS/RTS, we don't want these, so turn them off
	// uart_set_hw_flow(UART_ID, false, false);

	// // Set our data format
	// uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

	// Turn off FIFO's - we want to do this character by character
	// uart_set_fifo_enabled(UART_ID, false);

	// Set up a RX interrupt
	// We need to set up the handler first
	// Select correct interrupt for the UART we are using
	int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

	// And set up and enable the interrupt handlers
	irq_set_exclusive_handler(UART_IRQ, uartRxInterruptHandler);
	irq_set_enabled(UART_IRQ, true);

	// Now enable the UART to send interrupts - RX only
	uart_set_irq_enables(UART_ID, true, false);

	printf("UART initialized" MICRORL_CFG_END_LINE);

	//--------------------------------------------------------------------------------
	mySh = (microsh_t *)malloc(sizeof(microsh_t));
	microshr_t cmdRegisterStatus = microshOK;
	cmdRegisterStatus = microsh_init(mySh, privPrintFn);

	if (cmdRegisterStatus != microshOK)
	{
		panic("Failed microsh init!" MICRORL_CFG_END_LINE);
	}

	cmdRegisterStatus = microsh_cmd_register(mySh, 1, "status", statusCmdCallback, "Prints the status of the system");

	if (cmdRegisterStatus != microshOK)
	{
		panic("No memory to register all commands!" MICRORL_CFG_END_LINE);
	}

	cmdRegisterStatus = microsh_cmd_register(mySh, 3, "mode", modeCmdCallback, "mode <axis> <mode> - Sets the mode of the axis \n\r <axis> = X, Y, Z \n\r <mode> = S, A, O, M (Stopped, Automatic, One Shot, Manual)");

	if (cmdRegisterStatus != microshOK)
	{
		panic("No memory to register all commands!" MICRORL_CFG_END_LINE);
	}

	printf("Console initialized" MICRORL_CFG_END_LINE);
}

int Console::statusCmdCallback(struct microsh *msh, int argc, const char *const *argv)
{
	std::string xMode = AxisModeToString(myMotionController->GetMode(AxisLabel::X));
	std::string zMode = AxisModeToString(myMotionController->GetMode(AxisLabel::Z));
	printf("Status: OK, X Mode: %s, Z Mode, %s" MICRORL_CFG_END_LINE, xMode.c_str(), zMode.c_str());

	return microshEXEC_OK;
}

int Console::modeCmdCallback(struct microsh *msh, int argc, const char *const *argv)
{
	if (argc != 3)
	{
		printf("Usage: mode <axis> <mode>" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	AxisLabel axis = AxisLabelFromString(argv[1]);
	if (axis == AxisLabel::ERROR)
	{
		printf("Invalid axis" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}
	AxisMode mode = AxisModeFromString(argv[2]);
	if (mode == AxisMode::ERROR)
	{
		printf("Invalid mode" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}
	myMotionController->SetMode(axis, mode);
	printf("Status: OK, %s Mode: %s" MICRORL_CFG_END_LINE, AxisLabelToString(axis), AxisModeToString(myMotionController->GetMode(axis)));

	return microshEXEC_OK;
}

int Console::privPrintFn(microrl_t *mrl, const char *str)
{
	bool sent = false;
	while (!sent)
	{
		if (uart_is_writable(UART_ID))
		{
			uart_puts(UART_ID, str);
			sent = true;
		}
		else
		{
			tight_loop_contents();
		}
	}
	return microshEXEC_OK;
}

void Console::uartRxInterruptHandler()
{
	while (uart_is_readable(UART_ID))
	{
		uint8_t ch = uart_getc(UART_ID);
		// Can we send it back?
		if (SERIAL_ECHO)
		{
			if (uart_is_writable(UART_ID))
			{
				uart_putc(UART_ID, ch);
			}
		}

		microrl_processing_input(&mySh->mrl, &ch, 1);
	}
}