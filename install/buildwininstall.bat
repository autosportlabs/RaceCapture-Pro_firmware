@echo off
del dist /f/s/q
del build /f/s/q
pyinstaller -y racecapture.spec
"C:\Program Files (x86)\NSIS\makensis.exe" -DVERSION_STRING="0.0.1" raceCaptureApp.nsi
dist\racecapture\racecapture