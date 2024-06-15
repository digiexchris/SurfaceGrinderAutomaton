#pragma once

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

class Console
{
public:
	static void Init(MotionController *aMotionController);

private:
	static microsh_t *mySh;
	static MotionController *myMotionController;
	static void uartRxInterruptHandler();
	static int privPrintFn(microrl_t *mrl, const char *str);
	static int statusCmdCallback(struct microsh *msh, int argc, const char *const *argv);
	static int modeCmdCallback(struct microsh *msh, int argc, const char *const *argv);
};
