#/bin/bash

/usr/bin/openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program build/SurfaceGrinderAtomaton.elf verify reset exit"
