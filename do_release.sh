#!/bin/bash
set -e

# Change to the directory where the script is located.  This is top level
# of project.
cd $(dirname $0)

NUM_PROC=$(grep -c proc /proc/cpuinfo)
exec make -j $NUM_PROC package

# Clean out all .o files before we begin.
find . -name '*.o' -delete

source version.mk

RCP_DIST_DIR=firmware_release


if [ "$BUILD_NUMBER" ]
then
BUILD_NUMBER_SUFFIX=.$BUILD_NUMBER
fi

RELEASE_NAME_SUFFIX=$MAJOR.$MINOR.$BUGFIX$BUILD_NUMBER_SUFFIX

rm -rf $RCP_DIST_DIR
mkdir $RCP_DIST_DIR

##################################
# TEST FIRST!!!
##################################
pushd test
make clean
make -j $NUM_PROC
./rcptest
popd

###################################
# MK2 release
###################################
pushd lib_lua
make PLAT=stm32 clean
make PLAT=stm32 generic -j $NUM_PROC
popd
pushd stm32_base
make PLAT=stm32 clean
make PLAT=stm32 all -j $NUM_PROC
popd

MK2_RELEASE_DIR=$RCP_DIST_DIR/RaceCapturePro_MK2
MK2_RELEASE_NAME=RaceCapturePro_MK2_$RELEASE_NAME_SUFFIX
mkdir $MK2_RELEASE_DIR

cp CHANGELOG $MK2_RELEASE_DIR
cp stm32_base/HOW_TO_FLASH.txt $MK2_RELEASE_DIR
cp stm32_base/main.ihex $MK2_RELEASE_DIR/$MK2_RELEASE_NAME.ihex
zip -FSr $MK2_RELEASE_NAME.zip $MK2_RELEASE_DIR

###################################
# Race Capture Track
###################################
pushd RCT
make clean
make all -j $NUM_PROC
popd

RELEASE_DIR=$RCP_DIST_DIR/RaceCapture
RELEASE_NAME=RaceCaptureTrack_$RELEASE_NAME_SUFFIX
mkdir $RELEASE_DIR

### Commenting out as release doesn't exist yet.
#cp CHANGELOG $RELEASE_DIR
#cp RCT/HOW_TO_FLASH.txt $RELEASE_DIR
#cp RCT/main.ihex $RELEASE_DIR/$RELEASE_NAME.ihex
#zip -FSr $RELEASE_NAME.zip $RELEASE_DIR
