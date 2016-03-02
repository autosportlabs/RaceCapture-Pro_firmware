#!/bin/bash
set -e

# Change to the directory where the script is located.  This is top level
# of project.
cd $(dirname $0)

NUM_PROC=$(grep -c proc /proc/cpuinfo)
exec make -j $NUM_PROC package
