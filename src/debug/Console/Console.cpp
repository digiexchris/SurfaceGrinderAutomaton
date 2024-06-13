

#include "Console.hpp"
#include "microsh.h"
#include "pico/stdlib.h"
#include <cstdio>
#include <malloc.h>

microsh_t *Console::mySh = nullptr;

void Console::Init(void *aStateMachine)
{
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
		printf("Failed microsh init!" MICRORL_CFG_END_LINE);
	}

	cmdRegisterStatus = microsh_cmd_register(mySh, 1, "s", statusCmdCallback, "Prints the status of the system");

	if (cmdRegisterStatus != microshOK)
	{
		printf("No memory to register all commands!" MICRORL_CFG_END_LINE);
	}

	printf("Console initialized" MICRORL_CFG_END_LINE);
}

int Console::statusCmdCallback(struct microsh *msh, int argc, const char *const *argv)
{

	printf("Status: OK" MICRORL_CFG_END_LINE);
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