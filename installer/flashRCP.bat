@echo off
@echo.
@echo.
@echo.
@echo.
echo ********************************************************************
echo *****************RaceCapture/Pro firmware installer*****************
echo ********************************************************************
@echo.
echo Note:
echo Before proceeding, make sure the bootloader drivers are installed.
echo see the online firmware installation instructions for more info.
@echo.
@echo.
echo While pressing the button on RaceCapture/Pro, plug the unit into USB
@echo.
pause
for /f "delims=" %%a in ('dir /b *.elf') do @set firmware=%%a
flasher.exe %firmware%


