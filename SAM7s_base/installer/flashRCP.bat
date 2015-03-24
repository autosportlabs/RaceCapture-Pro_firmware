@echo off
echo.
echo.
echo.
echo.
echo ************************************************************************
echo *******************RaceCapture/Pro firmware installer*******************
echo ************************************************************************
echo.
echo Before proceeding, ensure your RaceCapture/Pro configuration 
echo (configuration + Lua script) is backed up.
echo.
echo ************************************************************************
echo ************************************************************************
echo While pressing the button on RaceCapture/Pro, plug the unit into USB.
echo Once you see 3 green LEDs illuminated, release the button and...
echo.
pause
echo.
echo.
echo If this is the first time installing firmware, bootloader drivers will
echo need to be installed.
echo.
:Ask
echo Is this the first time flashing firmware? 
set INPUT=
set /P INPUT=Press Y or N and hit Enter %=%
If /I "%INPUT%"=="y" goto yes 
If /I "%INPUT%"=="n" goto no
echo Incorrect input & goto Ask
:yes
echo.
echo.
echo.
echo.
echo ************************************************************************
echo Bootloader drivers will now be installed.
echo.
echo Follow the instructions and answer yes to each question.
echo ************************************************************************
pause
bootloader_drivers.exe
:no
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo ************************************************************************
echo Ready to flash firmware!
echo ************************************************************************
echo.
echo Ensure 3 green LEDs are illuminated and...
echo.
pause
for /f "delims=" %%a in ('dir /b *.elf') do @set firmware=%%a
flasher.exe %firmware%


