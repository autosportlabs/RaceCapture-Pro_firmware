#!/bin/bash
rm -rf dist
rm -rf build
kivy /usr/local/bin/pyinstaller -y racecapture.spec
#pushd dist
#rm -rf racecapture.app
#mv racecapture racecapture.app
#hdiutil create ./RaceCapture.dmg -srcfolder racecapture.app -ov
#popd
pushd dist
zip -r -9 racecapture.zip racecapture.app
popd
dist/racecapture/racecapture
