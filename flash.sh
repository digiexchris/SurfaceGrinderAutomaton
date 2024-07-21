#/bin/bash
echo
echo "Flashing $1"
echo
/usr/bin/openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program $1 verify reset exit"

echo
echo "Flashing complete"  
