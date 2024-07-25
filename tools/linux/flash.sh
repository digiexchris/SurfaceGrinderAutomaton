#/bin/bash
echo ""

if lsusb | grep -q "Raspberry Pi Debug Probe"; then
    echo "PicoProbe found"
    echo ""
    echo "Flashing $1"
    echo ""
    openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program $1 verify reset exit"
else
    if lsusb | grep -q "Black Magic Debug"; then
        echo "Blackmagic Probe found"
        echo ""
        echo "Flashing $1"
        arm-none-eabi-gdb -ex "set confirm off" -ex "set pagination off" -ex "target extended-remote /dev/ttyACM0" -ex "monitor frequency 25000000" -ex "monitor swdp_scan" -ex "attach 1" -ex "load" -ex "compare-sections" -ex "mon reset" -ex "quit" $1
    else
        echo "Assuming CMSIS-DAP"
        echo ""
        echo "Flashing $1"
        echo ""
        openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 500" -c "program $1 verify reset exit"
        exit 0
    fi
fi

echo "Flashing complete"
exit 0