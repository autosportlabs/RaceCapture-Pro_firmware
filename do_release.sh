#!/bin/bash
set -e

# Change to the directory where the script is located.  This is top level
# of project.
cd $(dirname $0)

# Clean out all .o files before we begin.
find . -name '*.o' -delete

source version.mk

SAM7S_BASE_DIR=SAM7s_base
RCP_DIST_DIR=firmware_release
NUM_PROC=$(grep -c proc /proc/cpuinfo)
MAX_MK1_ELF_SIZE=229376

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
# MK1 release
###################################
pushd lib_lua
make PLAT=sam7s clean
make PLAT=sam7s generic -j $NUM_PROC
popd
make PLAT=sam7s clean
make PLAT=sam7s all -j $NUM_PROC
sh ./check_elf_size.sh main.elf arm-elf-size $MAX_MK1_ELF_SIZE

MK1_RELEASE_DIR=$RCP_DIST_DIR/RaceCapturePro_MK1
MK1_RELEASE_NAME=RaceCapturePro_MK1_$RELEASE_NAME_SUFFIX
mkdir $MK1_RELEASE_DIR

cp CHANGELOG $MK1_RELEASE_DIR
cp SAM7s_base/HOW_TO_FLASH.txt $MK1_RELEASE_DIR
cp $SAM7S_BASE_DIR/installer/* $MK1_RELEASE_DIR
cp main.elf $MK1_RELEASE_DIR/$MK1_RELEASE_NAME.elf
zip -FSr $MK1_RELEASE_NAME.zip $MK1_RELEASE_DIR

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
