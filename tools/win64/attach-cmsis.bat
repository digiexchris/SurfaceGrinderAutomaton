NET SESSION
IF %ERRORLEVEL% NEQ 0 GOTO ELEVATE
GOTO ADMINTASKS

:ELEVATE
CD /d %~dp0
MSHTA "javascript: var shell = new ActiveXObject('shell.application'); shell.ShellExecute('%~nx0', '', '', 'runas', 1);close();"
EXIT

:ADMINTASKS

usbipd.exe bind -i 2e8a:000c --force
usbipd.exe attach -i 2e8a:000c --wsl

usbipd.exe bind -i 0d28:0204 --force
usbipd.exe attach -i 0d28:0204 --wsl
pause