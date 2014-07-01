#!/bin/bash
rm -rf dist
rm -rf build
kivy /usr/local/bin/pyinstaller -y racecapture.spec
pushd dist
zip -r -9 racecapture.zip racecapture.app
popd
dist/racecapture/racecapture
