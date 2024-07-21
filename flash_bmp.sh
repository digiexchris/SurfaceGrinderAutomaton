#/bin/bash
echo
echo "Flashing $1"
echo
/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gdb -ex "set confirm off" -ex "set pagination off" -ex "target extended-remote /dev/ttyACM0" -ex "monitor frequency 25000000" -ex "monitor swdp_scan" -ex "attach 1" -ex "load" -ex "compare-sections" -ex "mon reset" -ex "quit" $1

