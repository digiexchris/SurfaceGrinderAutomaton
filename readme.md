# Surface Grinder Atomaton
Simple automation for a manual surface grinder

## Hardware
### MCU
The basic control is provided by an rp2040 (raspberry pi pico) with the following pin usage

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
| 4          | GP2  |  |
| 5          | GP3  | |
| 6          | GP4  | ESTOP (reset) |
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
| 19         | GP14  | Start Spindle    |
| 20         | GP15  | Stop Spindle   |
| 21         | GP16  |     |
| 22         | GP17  |     |
| 23         | GND  |    |
| 24         | GP18  |     |
| 25         | GP19  |     |
| 26         | GP20  |    |
| 27         | GP21  |    |
| 28         | GND  |     |
| 29         | GP22  |     |
| 30         | RUN  |     |
| 31         | GP26  |     |
| 32         | GP27  |     |
| 33         | GND  |     |
| 34         | GP28  |     |
| 35         | ADC_VREF  |     |
| 36         | 3v3 (out)  |     |
| 37         | 3v3_en  |     |
| 38         | GND  |     |
| 39         | VSYS  |     |
| 40         | VBUS  |     |


The human interface is on the second RP2040:

| Pin Number | PINMUX | Use |
|------------|---|-----|
| USB        | Serial | Debug Console |
| 1          | UART0_TX  | Modbus TX |
| 2          | UART0_RX  | Modbus RX |
| 3          | GND  |     |
| 4          | GP2  | X Auto Mode |
| 5          | GP3  | Y Auto Mode |
| 6          | GP4  | ESTOP (reset) |
| 7          | GP5  | Z Auto Mode |
| 8          | GND  |  |
| 9          | GP6  | LEFT |
| 10         | GP7  | RIGHT |
| 11         | GP8  | UP    |
| 12         | GP9  | DOWN  |
| 13         | GND  |    |
| 14         | GP10  | IN     |
| 15         | GP11  | OUT    |
| 16         | GP12  | MENU    |
| 17         | GP13  | SELECT   |
| 18         | GND  |     |
| 19         | GP14  | BACK    |
| 20         | GP15  | MPG Select OFF/Menu Value    |
| 21         | GP16  | MPG Select X   |
| 22         | GP17  | MPG Select Y    |
| 23         | GND  | MPG Select Z    |
| 24         | GP18  | MPG Select Spindle RPM   |
| 25         | GP19  | MPG Select 0.0001"/.01mm    |
| 26         | GP20  | MPG SELECT 0.001"/.1mm    |
| 27         | GP21  | MPG SELECT 0.05"/1mm   |
| 28         | GND  |     |
| 29         | GP22  | Stop Spindle    |
| 30         | RUN  |     |
| 31         | GP26/I2C1_SDA  | SH1122 Display |
| 32         | GP27/I2C1_SCL  | SH1122 Display  |
| 33         | GND  |     |
| 34         | GP28  | Start Spindle    |
| 35         | ADC_VREF  |     |
| 36         | 3v3 (out)  |     |
| 37         | 3v3_en  |     |
| 38         | GND  |     |
| 39         | VSYS  |     |
| 40         | VBUS  |     |