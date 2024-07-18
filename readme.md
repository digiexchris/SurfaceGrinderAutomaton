# Surface Grinder Atomaton
Simple automation for a manual surface grinder

## Hardware
### MCU
The basic control is provided by an rp2040 (raspberry pi pico) with the following pin usage

| Pin Number | PINMUX | Use |
|------------|---|-----|
| USB        | Serial | Debug Console |
| 1          | UART0_TX  | Modbus TX |
| 2          | UART0_RX  | Modbus RX    |
| 3          | GND  |     |
| 4          | GP2  | MPG ENCA |
| 5          | GP3  | MPG ENCB |
| 6          | GP4  | FEED HOLD |
| 7          | GP5  | Steppers Enable |
| 8          | GND  |  |
| 9          | GP6  | Z Axis Direction |
| 10         | GP7  | Z Axis Step |
| 11         | GP8  | Z Auto Mode    |
| 12         | GP9  | X Axis Direction    |
| 13         | GND  |     |
| 14         | GP10  | X Axis Step    |
| 15         | GP11  | X Auto Mode    |
| 16         | GP12  | Y Axis Direction    |
| 17         | GP13  | Y Axis Step    |
| 18         | GND  |     |
| NC         | NC  | MPG Select OFF/Menu Value    |
| 19         | GP14  | MPG Select X |
| 20         | GP15  | MPG Select Y   |
| 21         | GP16  | MPG Select Z    |
| 22         | GP17  | MPG Select Spindle RPM    |
| 23         | GND  |    |
| 24         | GP18  | MCP23017 IntB    |
| 25         | GP19  | MCP23017 IntA    |
| 26         | GP20 / I2C0-SDA | Waveshare MCP23017 IO expander  & SH1122 Display |
| 27         | GP21 / I2C0-SCL  | Waveshare MCP23017 IO expander  & SH1122 Display |
| 28         | GND  |     |
| 29         | GP22  |     |
| 30         | RUN  | RESET (AKA estop)    |
| NC         | NC  | MPG Select 0.0001"/.01mm    |
| 31         | GP26  | MPG SELECT 0.001"/.1mm  |
| 32         | GP27  | MPG SELECT 0.05"/1mm |
| 33         | GND  |     |
| 34         | GP28  |     |
| 35         | ADC_VREF  |     |
| 36         | 3v3 (out)  |     |
| 37         | 3v3_en  |     |
| 38         | GND  |     |
| 39         | VSYS  |     |
| 40         | VBUS  |     |


IO Expander:

| Pin Number | Use |
|------------|---|
| PA0         | X Auto Mode  |
| PA1         | Y Auto Mode  |
| PA2         | Z Auto Mode  |
| PA3        | LEFT  |
| PA4          | RIGHT  |
| PA5          | UP  |
| PA6         | DOWN  |
| PA7          | IN  |
| PB0          | OUT  |
| PB1         | MENU  |
| PB2         | SELECT  |
| PB3         | BACK  |
| PB4         | Start Spindle  |
| PB5         | Stop Spindle  |
| PB6         | NC  |
| PB7         | NC  |

# Building
## Prerequisites
- Required
	- git submodule update --init --recursive
	- SEE NOTE BELOW. Raspberry Pi Pico SDK with the environment variables set for PICO_SDK_PATH. The install script should install a reasonable gcc compiler as well. You may also have some success with the rpi pico plugin for vscode.
		- NOTE: pico_sdk_import.cmake is setup to auto download the pi pico sdk so you don't have to. If you do not want this, edit CMakePresets.json and turn that off, and ensure PICO_SDK_PATH is set.
	- arm gcc
	- cmake
	- if linux, apt install build-essential
	- if Windows
		- https://www.raspberrypi.com/news/raspberry-pi-pico-windows-installer/
		- Ensure the env variable PICO_INSTALL_PATH is set to where you installed the pico sdk (from the installer above) such as C:\Program Files\Raspberry Pi\Pico SDK v1.5.1
		- open the cmakelists.txt file in visual studio
		- have openocd installed (c:\arm\openocd recommended, see .vs/launch.json) https://github.com/openocd-org/openocd/releases/tag/v0.12.0
		- if the cmake generation succeeded, select SurfaceGrinderAtomaton.elf from the run button dropdown and hit run!
		- TODO: make the debug target work to debug using gdb, see launch.vs.json in .vs/
- optional
	- clangd for formatting/linting/static analysis along with the vscode clangd extension
	- any of the recommended extensions in this repo
	- customize cmakepresets.json if your paths are different
	
## Building
If you have a cmsis-dap debug probe such as the picoprobe attached with a pi pico plugged into it, and the vscode cmake extension from microsoft, either open a compileable file (such as SurfaceGrinderAtomaton.cpp) and hit f7 to compile and upload, or f5 to debug.

## Running
The debug console is available on UART0 (see pins above). You will need some kind of usb serial dongle. In the future it will be moved to tinyusb for the built in usb port.

Once booted, type h to see the available commands (which may or may not be up to date, see src/debug/Console.cpp)

TODO: list the idosynchracies of the debug console, like an axis must be in manual mode to manually set the target position