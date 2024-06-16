#pragma once

#include "../../Axis.hpp"
#include "Motion/MotionController.hpp"
#include "microsh.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE
#define SERIAL_ECHO 0
#define UART_TX_PIN 0
#define UART_RX_PIN 1

enum class ConsoleCommandName
{
	STATUS = 0,
	MODE = 1,
	SET_ADVANCE_INCREMENT = 2,
	NONE = -1
};

struct ConsoleCommand
{
	explicit ConsoleCommand() : name(ConsoleCommandName::NONE) {}
	explicit ConsoleCommand(ConsoleCommandName aName) : name(aName) {}
	ConsoleCommandName name;
};

struct ConsoleCommandStatus : ConsoleCommand
{
	ConsoleCommandStatus() : ConsoleCommand(ConsoleCommandName::STATUS) {}
};

struct ConsoleCommandMode : ConsoleCommand
{
	ConsoleCommandMode(AxisLabel anAxis, AxisMode aMode) : ConsoleCommand(ConsoleCommandName::MODE), axis(anAxis), mode(aMode) {}
	AxisMode mode;
	AxisLabel axis;
};

struct ConsoleCommandSetAdvanceIncrement : ConsoleCommand
{
	ConsoleCommandSetAdvanceIncrement(AxisLabel anAxis, uint32_t anIncrement) : ConsoleCommand(ConsoleCommandName::SET_ADVANCE_INCREMENT), axis(anAxis), increment(anIncrement) {}
	uint32_t increment;
	AxisLabel axis;
};

class Console
{
public:
	static void Init(MotionController *aMotionController);

private:
	static QueueHandle_t myCommandQueue;
	static microsh_t *mySh;
	static MotionController *myMotionController;
	static void uartRxInterruptHandler();
	static int privPrintFn(microrl_t *mrl, const char *str);
	static int statusCmdCallback(struct microsh *msh, int argc, const char *const *argv);
	static int modeCmdCallback(struct microsh *msh, int argc, const char *const *argv);
	static int setAdvanceIncrementCallback(struct microsh *msh, int argc, const char *const *argv);

	static void consoleTask(void *pvParameters);

	static void privStatusCommand(ConsoleCommandStatus &aCommand);
	static void privModeCommand(ConsoleCommandMode &aCommand);
	static void privSetAdvanceIncrementCommand(ConsoleCommandSetAdvanceIncrement &aCommand);
};
