#/bin/bash
echo ""

if [ -z "$2" ]; then
    echo "Using rp2040 boot mode"
    if lsusb | grep -q "Black Magic Debug"; then
        echo "Blackmagic Probe found"
        echo ""
        echo "Flashing $1"
        arm-none-eabi-gdb -ex "target extended-remote /dev/ttyACM0" -ex "monitor swdp_scan" -ex "attach 1" -ex "monitor halt" -ex "set *(volatile uint32_t*)0x20042000 = 0x00000077" -ex "monitor reset" -ex "quit"

        drive=$(dmesg | grep -i "Attached SCSI removable disk" | tail -n 1 | sed 's/.*\[\([^]]*\)\].*/\1/')

        sudo mount /dev/${drive}1 /mnt
        cp $1 /mnt
        sudo umount /dev/${drive} --force

        exit 0
    else
        echo "Black Magic Debug not found"
        exit 1
    fi

    exit 1
fi

if lsusb | grep -q "Raspberry Pi Debugprobe on Pico"; then
    echo "PicoProbe found"
    echo ""
    echo "Flashing $1"
    echo ""
    sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program $1 verify reset exit"
else
    if lsusb | grep -q "Black Magic Debug"; then
        echo "Blackmagic Probe found"
        echo ""
        echo "Flashing $1"
        arm-none-eabi-gdb -ex "set confirm off" -ex "set pagination off" -ex "target extended-remote /dev/ttyACM0" -ex "monitor frequency 25000k" -ex "monitor swdp_scan" -ex "attach 1" -ex "load" -ex "compare-sections" -ex "mon reset" -ex "quit" $1
    else
        echo "Assuming CMSIS-DAP"
        echo ""
        echo "Flashing $1"
        echo ""
        sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program $1 verify reset exit"
        exit 0
    fi
fi

echo "Flashing complete"
exit 0