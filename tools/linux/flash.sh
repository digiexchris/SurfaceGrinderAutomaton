#/bin/bash

picotool reboot -u -f

timeout=5
start_time=$(date +%s)
while true; do
        current_time=$(date +%s)
        elapsed_time=$((current_time - start_time))
        if [ $elapsed_time -ge $timeout ]; then
                echo "Timeout reached. Device not found."
                exit 1
        fi
        if lsusb | grep -q "Raspberry Pi RP2 Boot"; then
                break
        fi
        sleep 0.25
done

picotool load -v -u $1
picotool reboot

exit 0
