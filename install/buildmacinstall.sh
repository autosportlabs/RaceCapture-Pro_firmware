#!/bin/bash
kivy /usr/local/bin/pyinstaller -y racecapture.spec
pushd dist
rm -rf racecapture.app
mv racecapture racecapture.app
hdiutil create ./RaceCapture.dmg -srcfolder racecapture.app -ov
popd
dist/racecapture/racecapture.exe
