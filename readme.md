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
	- arm gcc https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
	- cmake
	- if linux
				
		- The devcontainer is recommended.
			- sudo apt install docker.io
			- install the vscode Devcontainer extension from Microsoft.
			- open the repo in vscode, ctrl-shift-p-> reopen in container (it will probably prompt you at first load)
			- once the container is up, file->open workspace from file->sga.code.workspace
			- that's it, enjoy!
			- troubleshooting: check the .devcontainer/devcontainer.json for your debug probe mount if the container fails to mount them. It is only required if you want to debug it, you can build without it just fine.
		- or:
			- apt install build-essential
			- there are flash scripts available in tools/linux, and vscode tasks are setup to use them.
			- Note: if you want to use webusb, you may require udev permission on Linux (and/or macOS) to access usb device. It depends on your OS distro, typically copy 99-tinyusb.rules and reload your udev is good to go. Other than that, the CDC-ACM debug serial port should Just Work (tm), enumerating as a /dev/ttyACM* or with the udev rule, /dev/ttySGA
				- ```$ cp 99-tinyusb.rules /etc/udev/rules.d/
					$ sudo udevadm control --reload-rules && sudo udevadm trigger```
			- if you have an openocd version lower than 0.12
				- use the devcontainer
				- or 
					- sudo apt remove openocd
					- git clone https://github.com/openocd-org/openocd.git --depth=1 --recurse-submodules && cd openocd && ./bootstrap && ./configure --enable-ftdi --enable-sysfsgpio --enable-picoprobe --enable-cmsis-dap && make -j 8 install
	- if Windows
		- The devcontainer is recommended. 
			- Install a docker host of some kind (docker.io in wsl is fine), and the vscode Devcontainer extension from Microsoft. 
			- Be sure to run the attach script first for your debug probe before starting the devcontainer.
			- open the repo in vscode, ctrl-shift-p-> reopen in container (it will probably prompt you at first load)
			- once the container is up, file->open workspace from file->sga.code.workspace  
			- if your debug probe is detached (you unplugged it, wsl restarted, vscode had an issue, or who knows), just rerun the attach script and restart the vscode window. You may need to stop vscode and wait for the container to die before starting it again, update this doc if you do.
		- or windows native (less frequently tested by the team, so pull requests to fix it are appreciated):
			- https://www.raspberrypi.com/news/raspberry-pi-pico-windows-installer/
			- Ensure the env variable PICO_INSTALL_PATH is set to where you installed the pico sdk (from the installer above) such as C:\Program Files\Raspberry Pi\Pico SDK v1.5.1
			- open the cmakelists.txt file in visual studio
			- have openocd installed (c:\arm\openocd recommended, see .vs/launch.json) https://github.com/openocd-org/openocd/releases/tag/v0.12.0
			- if the cmake generation succeeded, select SurfaceGrinderAtomaton.elf from the run button dropdown and hit run!
			- You may need the webserial driver installed from tinyusb. find the unsupported device in device manager, and update the driver to tinyusb_win_usbser.inf included in the tools/win64 folder of this repo, but initial testing seems to indicate that the driver auto-installs in Windows 10/11.
			- there are flash scripts available in tools/linux, and vscode tasks are setup to use them.
			- there are usbipd helper scripts (attach-cmsis-dap and attach-bmp) to help give you a one-click attach of your debug probe if you're using WSL or the devcontainer under windows
- optional
	- there is a devcontainer!!! might be easier than windows or if you don't want to mess with openocd versions.
	- clangd for formatting/linting/static analysis along with the vscode clangd extension
	- clangd for formatting/linting/static analysis along with the vscode clangd extension (provided by the devcontainer)
	- any of the recommended extensions in this repo
	- customize cmakepresets.json if your paths are different (not needed with the devcontainer)
	
## Building
If you have a cmsis-dap debug probe such as the picoprobe attached with a pi pico plugged into it, and the vscode cmake extension from microsoft, either open a compileable file (such as SurfaceGrinderAtomaton.cpp) and hit f7 to compile and upload, or f5 to debug.

## Running
The debug console is available on the usb port as a CDC-ACM device as well as a webusb device.

Once booted, type h to see the available commands (which may or may not be up to date, see src/debug/Console.cpp)

TODO: list the idosynchracies of the debug console, like an axis must be in manual mode to manually set the target position