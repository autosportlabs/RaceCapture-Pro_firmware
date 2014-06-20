@echo off
if %1.==. goto nover
del dist /f/s/q
del build /f/s/q
pyinstaller -y racecapture.spec
"C:\Program Files (x86)\NSIS\makensis.exe" -DVERSION_STRING="%1" raceCaptureApp.nsi
dist\racecapture\racecapture
goto end
:nover
echo Please specify version number as parameter, e.g.:
echo.   
echo   %0 1.0.0
:end