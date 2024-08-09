

#include "Console.hpp"
#include "Enum.hpp"
#include "Helpers.hpp"
#include "Motion/Axis.hpp"
#include "Motion/MotionController.hpp"
#include "TaskStats.hpp"
// #include "Usb/usb.hpp"
#include "config.hpp"
#include "drivers/Motor/Stepper.hpp"
#include "microsh.h"
#include <algorithm>
#include <cstdint>
#include <hardware/watchdog.h>
#include <malloc.h>
#include <pico/printf.h>
#include <pico/stdio_usb.h>
#include <stdio.h>
#include <string>

microsh_t *Console::mySh = nullptr;
MotionController *Console::myMotionController = nullptr;
QueueHandle_t Console::myCommandQueue;

Console::Commands Console::myCommands[] = {
	{1, "h", Console::helpCmdCallback, "Help"},
	{1, "s", Console::statusCmdCallback, "Prints the status of the system"},
	{1, "r", Console::resetCmdCallback, "Resets the system"},
	{3, "mode", Console::modeCmdCallback, "mode <axis> <mode> - Sets the mode of the axis \n\r \t<axis> = X, Y, Z \n\r \t<mode> = S, A, M (Stopped, Automatic, Manual)"},
	{3, "increment", Console::setAdvanceIncrementCallback, "increment <axis> <increment> - Sets the advance increment of the axis in steps \n\r \t<axis> = X, Y, Z \n\r \t<increment> = 0 - 4294967295"},
	{4, "setstop", Console::setStopCallback, "setstop <axis> <direction> <position> - Sets the stop position\n\r \t<axis> = X, Z \n\r \t<direction> = +, - \n\r \t<position> = int32 steps"},
	{3, "speed", Console::setSpeedCallback, "speed <axis> <speed> - Sets the speed of the axis for use in auto mode \n\r \t<axis> = X, Z \n\r \t<speed> = 0 - 65535 steps per second"},
	{3, "mover", Console::moveRelativeCommandCallback, "mover <axis> <distance> - Moves the axis a relative distance in steps \n\r \t<axis> = X, Z \n\r \t<distance> = int32 steps"},
	{3, "moveto", Console::moveAbsoluteCommandCallback, "moveto <axis> <position> - Moves the axis to an absolute position in steps \n\r \t<axis> = X, Z \n\r \t<position> = int32 steps"},
	{3, "setposition", Console::setPositionCommandCallback, "setposition <axis> <position> <type> - Sets the position of the axis in steps \n\r \t<axis> = X, Z \n\r \t<position> = int32 steps\n\r\t<type> = CURRENT, TARGET, default is TARGET"},
	{1, "stats", Console::statsCmdCallback, "Displays task, stack, and heap statistics"},
};

void Console::Init(MotionController *aMotionController)
{
	myMotionController = aMotionController;
	printf("Initializing console" MICRORL_CFG_END_LINE);

	myCommandQueue = xQueueCreate(10, sizeof(ConsoleCommand *));
	if (myCommandQueue == nullptr)
	{
		panic("Failed to create command queue" MICRORL_CFG_END_LINE);
	}

#if CONSOLE_USES_UART
	uart_init(UART_ID, BAUD_RATE);
	// Set UART flow control CTS/RTS, we don't want these, so turn them off
	uart_set_hw_flow(UART_ID, false, false);
	int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

	irq_set_exclusive_handler(UART_IRQ, uartRxInterruptHandler);
	irq_set_enabled(UART_IRQ, true);

	// Now enable the UART to send interrupts - RX only
	uart_set_irq_enables(UART_ID, true, false);

	printf("UART initialized" MICRORL_CFG_END_LINE);
#endif

#if CONSOLE_USES_PICO_USB_UART

	stdio_usb_init();

	xTaskCreate(GetFromDefaultUartTask, "GetFromDefaultUartTask", 1024, NULL, 1, NULL);
#endif
	//--------------------------------------------------------------------------------

	registerCommands();

	// Start the console task
	BaseType_t status = xTaskCreate(consoleTask, "ConsoleTask", 2048 * 4, NULL, 1, NULL);
	if (status != pdPASS)
	{
		panic("Failed to create ConsoleTask\n");
	}

	printf("Console initialized" MICRORL_CFG_END_LINE);
}

void Console::registerCommands()
{
	mySh = new microsh_t; // Allocate memory using new

	// Check if allocation was successful
	if (mySh == nullptr)
	{
		panic("Failed to allocate memory for microsh_t" MICRORL_CFG_END_LINE);
	}

	// Initialize microsh_t
	auto initStatus = microsh_init(mySh, privPrintFn);
	if (initStatus != microshOK)
	{
		panic("Failed microsh init!" MICRORL_CFG_END_LINE);
	}

	for (size_t i = 0; i < sizeof(myCommands) / sizeof(myCommands[0]); i++)
	{
		auto cmd = myCommands[i];
		auto status = microsh_cmd_register(mySh, cmd.argnum, cmd.cmdname, cmd.cmdfn, cmd.desc);

		switch (status)
		{
		case microshOK:
			break;
		case microshERR:
			panic("Failed to register command" MICRORL_CFG_END_LINE);
			break;
		case microshERRPAR:
			panic("Parameter Error" MICRORL_CFG_END_LINE);
			break;
		case microshERRMEM:
			panic("No memory to register command" MICRORL_CFG_END_LINE);
			break;
		default:
			panic("Unknown error" MICRORL_CFG_END_LINE);
			break;
		}
	}
}

void Console::GetFromDefaultUartTask(void *pvParameters)
{
	while (true)
	{
		int receivedChar = getchar();
		if (receivedChar != PICO_ERROR_TIMEOUT)
		{
			ProcessChars(&receivedChar, 1);
		}

		vTaskDelay(MS_TO_TICKS(10));
	}
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
				case ConsoleCommandName::STATS:
				{
					ConsoleCommandStats *statsCommand = static_cast<ConsoleCommandStats *>(command);
					privPrintStats(*statsCommand);
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
				case ConsoleCommandName::SET_STOP:
				{
					ConsoleCommandSetStop *setStopCommand = static_cast<ConsoleCommandSetStop *>(command);
					privSetStopCommand(*setStopCommand);
					break;
				}
				case ConsoleCommandName::SET_SPEED:
				{
					ConsoleCommandSetSpeed *setSpeedCommand = static_cast<ConsoleCommandSetSpeed *>(command);
					privSetSpeedCommand(*setSpeedCommand);
					break;
				}
				case ConsoleCommandName::SET_POSITION:
				{
					ConsoleCommandSetPosition *setPositionCommand = static_cast<ConsoleCommandSetPosition *>(command);
					privSetPositionCommand(*setPositionCommand);
					break;
				}
				case ConsoleCommandName::MOVE_ABSOLUTE:
				{
					ConsoleCommandMoveAbsolute *moveAbsoluteCommand = static_cast<ConsoleCommandMoveAbsolute *>(command);
					privMoveAbsoluteCommand(*moveAbsoluteCommand);
					break;
				}
				case ConsoleCommandName::MOVE_RELATIVE:
				{
					ConsoleCommandMoveRelative *moveRelativeCommand = static_cast<ConsoleCommandMoveRelative *>(command);
					privMoveRelativeCommand(*moveRelativeCommand);
					break;
				}
				default:
					panic("Console Task: Unknown command" MICRORL_CFG_END_LINE);
					break;
				}
			}
			if (command != nullptr)
			{
				delete (command);
			}
			vTaskDelay(MS_TO_TICKS(100));
		}
	}
}

int Console::privPrintFn(microrl_t *mrl, const char *str)
{

#if CONSOLE_USES_UART
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
			vTaskDelay(MS_TO_TICKS(10));
		}
	}
#endif

#if CONSOLE_USES_PICO_USB_UART
	printf("%s", str);
#endif

	return microshEXEC_OK;
}

void Console::ProcessChars(const void *data, size_t len)
{
	microrl_processing_input(&mySh->mrl, data, len);
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
		vTaskDelay(1);
	}
}

/********** CALLBACKS ***********/

int Console::resetCmdCallback(struct microsh *msh, int argc, const char *const *argv)
{
	printf("Resetting system" MICRORL_CFG_END_LINE);
	watchdog_reboot(0, SRAM_END, 0);
	return microshEXEC_OK;
}

int Console::statsCmdCallback(struct microsh *msh, int argc, const char *const *argv)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	ConsoleCommandStats *command = new ConsoleCommandStats();
	if (xQueueSendFromISR(Console::myCommandQueue, &command, &xHigherPriorityTaskWoken) != pdPASS)
	{
		printf("Failed to send stats command to queue" MICRORL_CFG_END_LINE);
		delete (command); // Clean up if the command wasn't sent
		return microshEXEC_ERROR;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	return microshEXEC_OK;
}

void Console::privPrintStats(ConsoleCommandStats &aCommand)
{

	TaskStatsManager *taskStatsManager = TaskStatsManager::GetInstance();
	auto taskStats = taskStatsManager->GetTaskAvgRuntime();
	auto heapStats = taskStatsManager->GetHeapHighWaterMark();
	auto stackStats = taskStatsManager->GetTasksStackHighWaterMark();

	printf("Task Stats" MICRORL_CFG_END_LINE);
	printf("%s" MICRORL_CFG_END_LINE, taskStats.c_str());
	printf("%s" MICRORL_CFG_END_LINE, heapStats.c_str());
	printf("%s" MICRORL_CFG_END_LINE, stackStats.c_str());
}

int Console::setSpeedCallback(struct microsh *msh, int argc, const char *const *argv)
{
	if (argc != 3)
	{
		auto cmd = microsh_cmd_find(msh, "speed");
		printf("%s", cmd->desc);
		printf(MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	std::string a = argv[1];
	std::transform(a.begin(), a.end(), a.begin(), ::toupper);

	AxisLabel axis = AxisLabelFromString(a);

	uint32_t speed = std::stoi(argv[2]);

	ConsoleCommandSetSpeed *command = new ConsoleCommandSetSpeed(axis, speed);
	if (xQueueSendFromISR(Console::myCommandQueue, &command, &xHigherPriorityTaskWoken) != pdPASS)
	{
		printf("Failed to send set speed command to queue" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return microshEXEC_OK;
}

int Console::moveRelativeCommandCallback(struct microsh *msh, int argc, const char *const *argv)
{
	if (argc != 3)
	{
		auto cmd = microsh_cmd_find(msh, "mover");
		printf("%s", cmd->desc);
		printf(MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	std::string a = argv[1];
	std::transform(a.begin(), a.end(), a.begin(), ::toupper);

	AxisLabel axis = AxisLabelFromString(a);

	int32_t distance = std::stoi(argv[2]);

	ConsoleCommandMoveRelative *command = new ConsoleCommandMoveRelative(axis, distance);
	if (xQueueSendFromISR(Console::myCommandQueue, &command, &xHigherPriorityTaskWoken) != pdPASS)
	{
		printf("Failed to send move relative command to queue" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return microshEXEC_OK;
}

void Console::privMoveRelativeCommand(ConsoleCommandMoveRelative &aCommand)
{
	myMotionController->MoveRelative(aCommand.axis, aCommand.distance);
	vTaskDelay(1);
	auto al = AxisLabelToString(aCommand.axis);
	auto pos = myMotionController->GetTargetPosition(aCommand.axis);
	printf("Status: OK, %s Position: %d" MICRORL_CFG_END_LINE, al.c_str(), pos);
}

int Console::helpCmdCallback(struct microsh *msh, int argc, const char *const *argv)
{
	printf("Help" MICRORL_CFG_END_LINE);
	auto cmd = microsh_cmd_find(msh, "h");
	printf("%s", cmd->desc);
	printf(MICRORL_CFG_END_LINE);
	cmd = microsh_cmd_find(msh, "s");
	printf("%s", cmd->desc);
	printf(MICRORL_CFG_END_LINE);
	cmd = microsh_cmd_find(msh, "r");
	printf("%s", cmd->desc);
	printf(MICRORL_CFG_END_LINE);
	cmd = microsh_cmd_find(msh, "mode");
	printf("%s", cmd->desc);
	printf(MICRORL_CFG_END_LINE);
	cmd = microsh_cmd_find(msh, "increment");
	printf("%s", cmd->desc);
	printf(MICRORL_CFG_END_LINE);
	cmd = microsh_cmd_find(msh, "setstop");
	printf("%s", cmd->desc);
	printf(MICRORL_CFG_END_LINE);
	cmd = microsh_cmd_find(msh, "speed");
	printf("%s", cmd->desc);
	printf(MICRORL_CFG_END_LINE);
	cmd = microsh_cmd_find(msh, "mover");
	printf("%s", cmd->desc);
	printf(MICRORL_CFG_END_LINE);
	cmd = microsh_cmd_find(msh, "moveto");
	printf("%s", cmd->desc);
	printf(MICRORL_CFG_END_LINE);
	cmd = microsh_cmd_find(msh, "setposition");
	printf("%s", cmd->desc);
	printf(MICRORL_CFG_END_LINE);
	return microshEXEC_OK;
}

int Console::setStopCallback(struct microsh *msh, int argc, const char *const *argv)
{
	if (argc != 4)
	{
		auto cmd = microsh_cmd_find(msh, "setstop");
		printf("%s", cmd->desc);
		printf(MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	std::string a = argv[1];
	std::transform(a.begin(), a.end(), a.begin(), ::toupper);

	AxisLabel axis = AxisLabelFromString(a);

	std::string d = argv[2];
	std::transform(d.begin(), d.end(), d.begin(), ::toupper);
	AxisDirection direction = AxisDirectionFromString(d);
	if (direction == AxisDirection::ERROR)
	{
		printf("Invalid direction" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	int32_t position = std::stoi(argv[3]);

	ConsoleCommandSetStop *command = new ConsoleCommandSetStop();
	command->axis = axis;
	command->direction = direction;
	command->position = position;

	if (xQueueSendFromISR(Console::myCommandQueue, &command, &xHigherPriorityTaskWoken) != pdPASS)
	{
		printf("Failed to send set stop command to queue" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return microshEXEC_OK;
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
	auto xPos = myMotionController->GetTargetPosition(AxisLabel::X);
	auto xCurrentPos = myMotionController->GetCurrentPosition(AxisLabel::X);
	auto xMinStop = myMotionController->GetStop(AxisLabel::X, AxisDirection::NEG);
	auto xMaxStop = myMotionController->GetStop(AxisLabel::X, AxisDirection::POS);
	auto xSpeed = myMotionController->GetCurrentSpeed(AxisLabel::X);
	auto xTargetSpeed = myMotionController->GetTargetSpeed(AxisLabel::X);
	std::string xMoveState = Stepper::MoveStateToString(myMotionController->GetMoveState(AxisLabel::X));

	auto zMode = AxisModeToString(myMotionController->GetMode(AxisLabel::Z));
	auto zPos = myMotionController->GetTargetPosition(AxisLabel::Z);
	auto zCurrentPos = myMotionController->GetCurrentPosition(AxisLabel::Z);
	auto zMinStop = myMotionController->GetStop(AxisLabel::Z, AxisDirection::NEG);
	auto zMaxStop = myMotionController->GetStop(AxisLabel::Z, AxisDirection::POS);
	auto zSpeed = myMotionController->GetCurrentSpeed(AxisLabel::Z);
	auto zTargetSpeed = myMotionController->GetTargetSpeed(AxisLabel::Z);
	auto zAdvanceIncrement = myMotionController->GetAdvanceIncrement(AxisLabel::Z);
	std::string zMoveState = Stepper::MoveStateToString(myMotionController->GetMoveState(AxisLabel::Z));

	printf("Status" MICRORL_CFG_END_LINE);
	printf("X Mode: %s", xMode.c_str());
	printf(", Stepper State: %s", xMoveState.c_str());
	printf(", Target Pos: %d", xPos);
	printf(", Current Pos: %d", xCurrentPos);
	printf(", Stops: %d:%d", xMinStop, xMaxStop);
	printf(", Target Speed: %d", xTargetSpeed);
	printf(", Speed: %.1f" MICRORL_CFG_END_LINE, xSpeed);

	printf("Z Mode: %s", zMode.c_str());
	printf(", Stepper State: %s", zMoveState.c_str());
	printf(", Target Pos: %d", zPos);
	printf(", Current Pos: %d", zCurrentPos);
	printf(", Stops: %d:%d", zMinStop, zMaxStop);
	printf(", Target Speed: %d", zTargetSpeed);
	printf(", Speed: %.1f", zSpeed);
	printf(", Z Advance Increment: %d" MICRORL_CFG_END_LINE, zAdvanceIncrement);
}

int Console::modeCmdCallback(struct microsh *msh, int argc, const char *const *argv)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if (argc != 3)
	{
		auto cmd = microsh_cmd_find(msh, "mode");
		printf("%s", cmd->desc);
		printf(MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	std::string a = argv[1];
	std::transform(a.begin(), a.end(), a.begin(), ::toupper);

	AxisLabel axis = AxisLabelFromString(a);

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
		auto cmd = microsh_cmd_find(msh, "increment");
		printf("%s", cmd->desc);
		printf(MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	std::string a = argv[1];
	std::transform(a.begin(), a.end(), a.begin(), ::toupper);

	AxisLabel axis = AxisLabelFromString(a);

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

void Console::privSetStopCommand(ConsoleCommandSetStop &aCommand)
{

	myMotionController->SetStop(aCommand.axis, aCommand.direction, aCommand.position);
	vTaskDelay(1);
	auto al = AxisLabelToString(aCommand.axis);
	printf("Status: OK, %s Stop: %d, Position: %d" MICRORL_CFG_END_LINE, al.c_str(), myMotionController->GetStop(aCommand.axis, aCommand.direction), aCommand.position);
}

void Console::privSetSpeedCommand(ConsoleCommandSetSpeed &aCommand)
{
	myMotionController->SetTargetSpeed(aCommand.axis, aCommand.speed);
	vTaskDelay(1);
	auto al = AxisLabelToString(aCommand.axis);
	printf("Status: OK, %s Speed: %d" MICRORL_CFG_END_LINE, al.c_str(), myMotionController->GetTargetSpeed(aCommand.axis));
}

int Console::moveAbsoluteCommandCallback(struct microsh *msh, int argc, const char *const *argv)
{
	if (argc != 3)
	{
		auto cmd = microsh_cmd_find(msh, "moveto");
		printf("%s", cmd->desc);
		printf(MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	std::string a = argv[1];
	std::transform(a.begin(), a.end(), a.begin(), ::toupper);

	AxisLabel axis = AxisLabelFromString(a);
	int32_t position = std::stoi(argv[2]);

	ConsoleCommandMoveAbsolute *command = new ConsoleCommandMoveAbsolute(axis, position);
	if (xQueueSendFromISR(Console::myCommandQueue, &command, &xHigherPriorityTaskWoken) != pdPASS)
	{
		printf("Failed to send move absolute command to queue" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return microshEXEC_OK;
}

void Console::privMoveAbsoluteCommand(ConsoleCommandMoveAbsolute &aCommand)
{
	myMotionController->MoveTo(aCommand.axis, aCommand.position);
	vTaskDelay(1);
	auto al = AxisLabelToString(aCommand.axis);
	auto pos = myMotionController->MoveTo(aCommand.axis, aCommand.position);
	printf("Status: OK, %d Target Position: %d" MICRORL_CFG_END_LINE, myMotionController->GetTargetPosition(aCommand.axis), pos);
}

int Console::setPositionCommandCallback(struct microsh *msh, int argc, const char *const *argv)
{
	if (argc < 3)
	{
		auto cmd = microsh_cmd_find(msh, "setposition");
		printf("%s", cmd->desc);
		printf(MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	std::string a = argv[1];
	std::transform(a.begin(), a.end(), a.begin(), ::toupper);

	AxisLabel axis = AxisLabelFromString(a);

	int32_t position = std::stoi(argv[2]);

	ConsolePositionType positionType;

	if (argc == 3)
	{
		positionType = ConsolePositionType::TARGET;
	}
	else
	{
		std::string type = argv[3];
		std::transform(type.begin(), type.end(), type.begin(), ::toupper);

		if (type == "TARGET")
		{
			positionType = ConsolePositionType::TARGET;
		}
		else if (type == "CURRENT")
		{
			positionType = ConsolePositionType::CURRENT;
		}
		else
		{
			printf("Invalid position type" MICRORL_CFG_END_LINE);
			return microshEXEC_ERROR;
		}
	}

	ConsoleCommandSetPosition *command = new ConsoleCommandSetPosition(axis, position, positionType);
	if (xQueueSendFromISR(Console::myCommandQueue, &command, &xHigherPriorityTaskWoken) != pdPASS)
	{
		printf("Failed to send set position command to queue" MICRORL_CFG_END_LINE);
		return microshEXEC_ERROR;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return microshEXEC_OK;
}

void Console::privSetPositionCommand(ConsoleCommandSetPosition &aCommand)
{
	bool result;
	switch (aCommand.positionType)
	{
	case ConsolePositionType::TARGET:
		result = myMotionController->SetTargetPosition(aCommand.axis, aCommand.position);
		break;
	case ConsolePositionType::CURRENT:
		result = myMotionController->SetCurrentPosition(aCommand.axis, aCommand.position);
		break;
	}

	if (!result)
	{
		printf("Failed to set position (is the mode set correctly on this axis?)" MICRORL_CFG_END_LINE);
		return;
	}

	vTaskDelay(1);
	auto al = AxisLabelToString(aCommand.axis);
	printf("Status: OK, %s %s Position: %d" MICRORL_CFG_END_LINE, al.c_str(), aCommand.positionType == ConsolePositionType::TARGET ? "Target" : "Current", myMotionController->GetTargetPosition(aCommand.axis));
}
