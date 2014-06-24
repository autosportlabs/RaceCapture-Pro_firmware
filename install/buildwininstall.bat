@echo off
if %1.==. goto nover
del dist /f/s/q
del build /f/s/q
rem Add the version string to the actual binary
set str=%1
set str=%str:.=,%
powershell -Command "(gc win_versioninfo.txt) -replace '!REALVER!', '%str%' | Out-File -encoding ASCII temp_win_versioninfo.txt"
pyinstaller -y racecapture.spec
"C:\Program Files (x86)\NSIS\makensis.exe" -DVERSION_STRING="%1" raceCaptureApp.nsi
dist\racecapture\racecapture
goto end
:nover
echo Please specify version number as parameter, e.g.:
echo.   
echo   %0 1.0.0
echo.
echo Note that the version number must be three segments.
:end
del temp_win_versioninfo.txt /q
