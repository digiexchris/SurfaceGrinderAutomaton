@echo off
echo.

echo NOTE: this script requires both openocd and arm-none-eabi-gdb to be installed and in the PATH

REM Check for PicoProbe or cmsis-dap
usbipd.exe list | findstr /C:"2e8a:000c" >nul
if %errorlevel% equ 0 (
    echo PicoProbe found
    echo.
    echo Flashing %1
    echo.
    openocd.exe -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program %1" -c "verify" -c"reset" -c "exit"
    goto :end
)

REM Check for Black Magic Probe
usbipd.exe list | findstr /C:"1d50:6018" >nul
if %errorlevel% equ 0 (
    echo Blackmagic Probe found
    echo.
    echo Flashing %1
    arm-none-eabi-gdb.exe -ex "set confirm off" -ex "set pagination off" -ex "target extended-remote /dev/ttyACM0" -ex "monitor frequency 25000000" -ex "monitor swdp_scan" -ex "attach 1" -ex "load" -ex "compare-sections" -ex "mon reset" -ex "quit" %1
    goto :end
)

REM Assume CMSIS-DAP
echo Assuming CMSIS-DAP
echo.
echo Flashing %1
echo.
openocd.exe -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 500" -c "program %1 verify reset exit"

:end
echo Flashing complete
exit /b 0
