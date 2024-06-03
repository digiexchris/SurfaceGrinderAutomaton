# Unofficial Zephyr Devcontainer

Devcontainer and recommended extensions for zephyr RTOS 3.6.0. Requires some kind of devcontainer compatible docker host such as docker desktop, Rancher Desktop, or docker CE on WSL. I prefer WSL or linux native personally but you do you.

There are few examples out there that actually work for me, but I've built and been using this for some time so I thought I'd share it as a hopefully turn-key option for those just getting started with Zephyr.

Includes Google Test and Tromeoleil for testing.

## You can forward usb devices from windows using usbipd.

`usbipd list`

```
Connected:
BUSID  VID:PID    DEVICE                                                        STATE
1-9    8087:0029  Intel(R) Wireless Bluetooth(R)                                Not shared
1-10   0b05:1939  AURA LED Controller, USB Input Device                         Not shared
5-1    0c45:9510  USB Input Device                                              Not shared
5-2    1bcf:2284  Full HD webcam, USB Microphone                                Not shared
7-1    0d28:0204  USB Serial Device (COM8), USB Input Device, WebUSB: CMSIS...  Not shared
7-3    256f:c652  USB Input Device                                              Not shared
10-3   28de:1142  USB Input Device                                              Not shared
11-2   10c4:ea60  Silicon Labs CP210x USB to UART Bridge (COM22)                Not shared
```

`usbipd bind --busid 7-1`

```
Connected:
BUSID  VID:PID    DEVICE                                                        STATE
1-9    8087:0029  Intel(R) Wireless Bluetooth(R)                                Not shared
1-10   0b05:1939  AURA LED Controller, USB Input Device                         Not shared
5-1    0c45:9510  USB Input Device                                              Not shared
5-2    1bcf:2284  Full HD webcam, USB Microphone                                Not shared
7-1    0d28:0204  USB Serial Device (COM8), USB Input Device, WebUSB: CMSIS...  Shared
7-3    256f:c652  USB Input Device                                              Not shared
10-3   28de:1142  USB Input Device                                              Not shared
11-2   10c4:ea60  Silicon Labs CP210x USB to UART Bridge (COM22)                Not shared
```

`usbipd attach --wsl --busid 7-1`

```
Connected:
BUSID  VID:PID    DEVICE                                                        STATE
1-9    8087:0029  Intel(R) Wireless Bluetooth(R)                                Not shared
1-10   0b05:1939  AURA LED Controller, USB Input Device                         Not shared
5-1    0c45:9510  USB Input Device                                              Not shared
5-2    1bcf:2284  Full HD webcam, USB Microphone                                Not shared
7-1    0d28:0204  USB Serial Device (COM8), USB Input Device, WebUSB: CMSIS...  Attached
7-3    256f:c652  USB Input Device                                              Not shared
10-3   28de:1142  USB Input Device                                              Not shared
11-2   10c4:ea60  Silicon Labs CP210x USB to UART Bridge (COM22)                Not shared
```

And in my container it's available including the serial port at /dev/ttyACM0
