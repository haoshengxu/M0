@echo off
setlocal

set "WORKSPACE=%~1"
set "CMAKE=%~2"
set "OPENOCD=%~3"
set "OPENOCD_SCRIPTS=%~4"
set "BUILD=%WORKSPACE%\build"
set "ELF="
set "ELF_COUNT=0"
set "FLASH_CFG=%BUILD%\flash-openocd.cfg"

for /f "delims=" %%F in ('dir /b /a-d "%BUILD%\*.elf" 2^>nul') do (
    set "ELF=%BUILD%\%%F"
    set /a ELF_COUNT+=1
)

if "%ELF_COUNT%"=="0" (
    echo No elf file found in %BUILD%
    exit /b 1
)

if not "%ELF_COUNT%"=="1" (
    echo Multiple elf files found in %BUILD%
    exit /b 1
)

(
    echo gdb_port disabled
    echo tcl_port disabled
    echo telnet_port disabled
    echo program "%ELF:\=/%" verify
    echo reset run
    echo exit
) > "%FLASH_CFG%"

"%OPENOCD%" -s "%OPENOCD_SCRIPTS%" -f "%WORKSPACE%\linker\CFG.cfg" -f "%FLASH_CFG%"
exit /b %ERRORLEVEL%
