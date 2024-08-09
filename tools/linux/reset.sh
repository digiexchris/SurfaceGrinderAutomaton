#!/bin/bash
#sudo /usr/local/bin/openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000; init ; reset halt ; rp2040.core1 arp_reset assert 0 ; rp2040.core0 arp_reset assert 0 ; exit"

# arm-none-eabi-gdb -ex "target extended-remote /dev/ttyACM0" -ex "monitor swdp_scan" -ex "attach 3" -ex "monitor halt" -ex "set *(volatile uint32_t*)0x20042000 = 0x00000077" -ex "monitor reset" -ex "quit"