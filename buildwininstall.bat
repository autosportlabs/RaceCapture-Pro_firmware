@echo off
del dist /f/s/q
del build /f/s/q
pyinstaller -y racecapture.spec
dist\racecapture\racecapture