

#include "Console.hpp"
#include "Axis.hpp"
#include "Enum.hpp"
#include "Motion/MotionController.hpp"
#include "microsh.h"
#include <algorithm>
#include <malloc.h>
#include <stdio.h>
#include <string>

microsh_t *Console::mySh = nullptr;
MotionController *Console::myMotionController = nullptr;
QueueHandle_t Console::myCommandQueue;

void Console::Init(MotionController *aMotionController)
{
	myMotionController = aMotionController;
	printf("Initializing console" MICRORL_CFG_END_LINE);
	uart_init(UART_ID, BAUD_RATE);

	myCommandQueue = xQueueCreate(10, sizeof(ConsoleCommand *));
	if (myCommandQueue == nullptr)
	{
		panic("Failed to create command queue" MICRORL_CFG_END_LINE);
	}

	// Set the TX and RX pins by using the function select on the GPIO
	// Set datasheet for more information on function select
	// gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	// gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	// Actually, we want a different speed
	// The call will return the actual baud rate selected, which will be as close as
	// possible to that requested
	// int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

	// Set UART flow control CTS/RTS, we don't want these, so turn them off
	uart_set_hw_flow(UART_ID, false, false);

	// // Set our data format
	// uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

	// Turn off FIFO's - we want to do this character by character
	uart_set_fifo_enabled(UART_ID, false);

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

	cmdRegisterStatus = microsh_cmd_register(mySh, 1, "s", statusCmdCallback, "Prints the status of the system");

	if (cmdRegisterStatus != microshOK)
	{
		panic("No memory to register all commands!" MICRORL_CFG_END_LINE);
	}

	cmdRegisterStatus = microsh_cmd_register(mySh, 3, "m", modeCmdCallback, "m <axis> <mode> - Sets the mode of the axis \n\r <axis> = X, Y, Z \n\r <mode> = S, A, O, M (Stopped, Automatic, One Shot, Manual)");

	if (cmdRegisterStatus != microshOK)
	{
		panic("No memory to register all commands!" MICRORL_CFG_END_LINE);
	}

	cmdRegisterStatus = microsh_cmd_register(mySh, 3, "i", setAdvanceIncrementCallback, "i <axis> <increment> - Sets the advance increment of the axis in steps \n\r <axis> = X, Y, Z \n\r <increment> = 0 - 4294967295");

	if (cmdRegisterStatus != microshOK)
	{
		panic("No memory to register all commands!" MICRORL_CFG_END_LINE);
	}

	// Start the console task
	BaseType_t status = xTaskCreate(consoleTask, "ConsoleTask", 2048, NULL, 1, NULL);

	if (status != pdPASS)
	{
		panic("Failed to create ConsoleTask\n");
	}

	printf("Console initialized" MICRORL_CFG_END_LINE);
}

void Console::consoleTask(void *aCommandQueueHandle)
{
	printf("Starting console task" MICRORL_CFG_END_LINE);

	while (true)
	{
		ConsoleCommand *command = nullptr;
		if (xQueueReceive(myCommandQueue, &command, portMAX_DELAY) == pdTRUE)
		{
			if (command != nullptr)
			{
				switch (command->name)
				{
				case ConsoleCommandName::STATUS:
				{
					ConsoleCommandStatus *statusCommand = static_cast<ConsoleCommandStatus *>(command);
					privStatusCommand(*statusCommand);
					break;
				}
				case ConsoleCommandName::MODE:
				{
					ConsoleCommandMode *modeCommand = static_cast<ConsoleCommandMode *>(command);
					privModeCommand(*modeCommand);
					break;
				}
				case ConsoleCommandName::SET_ADVANCE_INCREMENT:
				{
					ConsoleCommandSetAdvanceIncrement *setAdvanceIncrementCommand = static_cast<ConsoleCommandSetAdvanceIncrement *>(command);
					privSetAdvanceIncrementCommand(*setAdvanceIncrementCommand);
					break;
				}
				default:
					printf("Unknown command" MICRORL_CFG_END_LINE);
					break;
				}
			}
			if (command != nullptr)
			{
				delete (command);
			}
		}
	}
}

int Console::statusCmdCallback(struct microsh *msh, int argc, const char *const *argv)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	ConsoleCommandStatus *command = new ConsoleCommandStatus();
	if (xQueueSendFromISR(Console::myCommandQueue, &command, &xHigherPriorityTaskWoken) != pdPASS)
	{
		printf("Failed to send status command to queue" MICRORL_CFG_END_LINE);
		delete (command); // Clean up if the command wasn't sent
		return microshEXEC_ERROR;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	return microshEXEC_OK;
}

void Console::privStatusCommand(ConsoleCommandStatus &aCommand)
{
	auto xMode = AxisModeToString(myMotionController->GetMode(AxisLabel::X));
	auto xDirection = AxisDirectionToString(myMotionController->GetDirection(AxisLabel::X));
	auto zMode = AxisModeToString(myMotionController->GetMode(AxisLabel::Z));
	auto zDirection = AxisDirectionToString(myMotionController->GetDirection(AxisLabel::Z));
	printf("Status: OK, X Mode: %s, Dir: %s \n\r Z Mode: %s, Dir: %s" MICRORL_CFG_END_LINE, xMode.c_str(), xDirection.c_str(), zMode.c_str(), zDirection.c_str());
}

int Console::modeCmdCallback(struct microsh *msh, int argc, const char *const *argv)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if (argc != 3)
	{
		printf("Usage: mode <axis> <mode>" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	std::string a = argv[1];
	std::transform(a.begin(), a.end(), a.begin(), ::toupper);

	AxisLabel axis = AxisLabelFromString(a);
	if (axis == AxisLabel::ERROR)
	{
		printf("Invalid axis" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	std::string m = argv[2];
	std::transform(m.begin(), m.end(), m.begin(), ::toupper);
	AxisMode mode = AxisModeFromString(m);
	if (mode == AxisMode::ERROR)
	{
		printf("Invalid mode" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	ConsoleCommandMode *command = new ConsoleCommandMode(axis, mode);
	if (xQueueSendFromISR(Console::myCommandQueue, &command, &xHigherPriorityTaskWoken) != pdPASS)
	{
		printf("Failed to send mode command to queue" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return microshEXEC_OK;
}

void Console::privModeCommand(ConsoleCommandMode &aCommand)
{
	myMotionController->SetMode(aCommand.axis, aCommand.mode);
	vTaskDelay(1);
	auto am = AxisModeToString(myMotionController->GetMode(aCommand.axis));
	auto al = AxisLabelToString(aCommand.axis);
	printf("Status: OK, %s Mode: %s" MICRORL_CFG_END_LINE, al.c_str(), am.c_str());
}

int Console::setAdvanceIncrementCallback(struct microsh *msh, int argc, const char *const *argv)
{
	if (argc != 3)
	{
		printf("Usage: setAdvanceIncrement <axis> <increment>" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	std::string a = argv[1];
	std::transform(a.begin(), a.end(), a.begin(), ::toupper);

	AxisLabel axis = AxisLabelFromString(a);
	if (axis == AxisLabel::ERROR)
	{
		printf("Invalid axis" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	uint32_t increment = std::stoi(argv[2]);

	ConsoleCommandSetAdvanceIncrement *command = new ConsoleCommandSetAdvanceIncrement(axis, increment);
	if (xQueueSendFromISR(Console::myCommandQueue, &command, &xHigherPriorityTaskWoken) != pdPASS)
	{
		printf("Failed to send set advance increment command to queue" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return microshEXEC_OK;
}

void Console::privSetAdvanceIncrementCommand(ConsoleCommandSetAdvanceIncrement &aCommand)
{
	myMotionController->SetAdvanceIncrement(aCommand.axis, aCommand.increment);
	vTaskDelay(1);
	auto ai = myMotionController->GetAdvanceIncrement(aCommand.axis);
	auto al = AxisLabelToString(aCommand.axis);
	printf("Status: OK, %s Increment: %d" MICRORL_CFG_END_LINE, al.c_str(), ai);
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