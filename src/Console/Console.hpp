#pragma once

#include "../Motion/Axis.hpp"
#include "Motion/MotionController.hpp"
#include "microsh.h"
#include "usb.hpp"

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
	SET_STOP = 3,
	SET_SPEED = 4,
	MOVE_RELATIVE = 5,
	MOVE_ABSOLUTE = 6,
	SET_POSITION = 7,
	NONE = -1
};

enum class ConsolePositionType
{
	TARGET,
	CURRENT
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

struct ConsoleCommandSetStop : ConsoleCommand
{
	ConsoleCommandSetStop() : ConsoleCommand(ConsoleCommandName::SET_STOP) {}
	AxisDirection direction;
	AxisLabel axis;
	int32_t position;
};

struct ConsoleCommandSetAdvanceIncrement : ConsoleCommand
{
	ConsoleCommandSetAdvanceIncrement(AxisLabel anAxis, uint32_t anIncrement) : ConsoleCommand(ConsoleCommandName::SET_ADVANCE_INCREMENT), axis(anAxis), increment(anIncrement) {}
	uint32_t increment;
	AxisLabel axis;
};

struct ConsoleCommandSetSpeed : ConsoleCommand
{
	ConsoleCommandSetSpeed(AxisLabel anAxis, uint32_t aSpeed) : ConsoleCommand(ConsoleCommandName::SET_SPEED), axis(anAxis), speed(aSpeed) {}
	uint16_t speed;
	AxisLabel axis;
};

struct ConsoleCommandMoveRelative : ConsoleCommand
{
	ConsoleCommandMoveRelative(AxisLabel anAxis, int32_t aDistance) : ConsoleCommand(ConsoleCommandName::MOVE_RELATIVE), axis(anAxis), distance(aDistance) {}
	int32_t distance;
	AxisLabel axis;
};

struct ConsoleCommandMoveAbsolute : ConsoleCommand
{
	ConsoleCommandMoveAbsolute(AxisLabel anAxis, int32_t aPosition) : ConsoleCommand(ConsoleCommandName::MOVE_ABSOLUTE), axis(anAxis), position(aPosition) {}
	int32_t position;
	AxisLabel axis;
};

struct ConsoleCommandSetPosition : ConsoleCommand
{
	ConsoleCommandSetPosition(AxisLabel anAxis, int32_t aPosition, ConsolePositionType aPositionType) : ConsoleCommand(ConsoleCommandName::SET_POSITION), axis(anAxis), position(aPosition), positionType(aPositionType) {}
	int32_t position;
	AxisLabel axis;
	ConsolePositionType positionType;
};

class Console
{
public:
	static void Init(MotionController *aMotionController);

	struct Commands
	{
		Commands(size_t aArgNum, const char *aCmdName, microsh_cmd_fn aCmdFn, const char *aDesc)
			: argnum(aArgNum), cmdname(aCmdName), cmdfn(aCmdFn), desc(aDesc) {}
		size_t argnum;
		const char *cmdname;
		microsh_cmd_fn cmdfn;
		const char *desc;
	};

private:
	static Usb *myUsb;
	static Commands myCommands[];
	static QueueHandle_t myCommandQueue;
	static microsh_t *mySh;
	static MotionController *myMotionController;
	static void uartRxInterruptHandler();
	static int privPrintFn(microrl_t *mrl, const char *str);
	static int statusCmdCallback(struct microsh *msh, int argc, const char *const *argv);
	static int modeCmdCallback(struct microsh *msh, int argc, const char *const *argv);
	static int setAdvanceIncrementCallback(struct microsh *msh, int argc, const char *const *argv);
	static int resetCmdCallback(struct microsh *msh, int argc, const char *const *argv);
	static int setStopCallback(struct microsh *msh, int argc, const char *const *argv);
	static int setSpeedCallback(struct microsh *msh, int argc, const char *const *argv);
	static int helpCmdCallback(struct microsh *msh, int argc, const char *const *argv);
	static int moveRelativeCommandCallback(struct microsh *msh, int argc, const char *const *argv);
	static int moveAbsoluteCommandCallback(struct microsh *msh, int argc, const char *const *argv);
	static int setPositionCommandCallback(struct microsh *msh, int argc, const char *const *argv);

	static void consoleTask(void *pvParameters);
	static void registerCommands();
	static void processChars(const void *data, size_t len);

	static void privStatusCommand(ConsoleCommandStatus &aCommand);
	static void privModeCommand(ConsoleCommandMode &aCommand);
	static void privSetAdvanceIncrementCommand(ConsoleCommandSetAdvanceIncrement &aCommand);
	static void privSetStopCommand(ConsoleCommandSetStop &aCommand);
	static void privSetSpeedCommand(ConsoleCommandSetSpeed &aCommand);
	static void privMoveRelativeCommand(ConsoleCommandMoveRelative &aCommand);
	static void privMoveAbsoluteCommand(ConsoleCommandMoveAbsolute &aCommand);
	static void privSetPositionCommand(ConsoleCommandSetPosition &aCommand);
};
