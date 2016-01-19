#!/bin/bash
if [ "$#" -lt 3 ]; then
  echo ""
  echo "*** ERROR: invalid number of arguments ($#)"
  echo ""
  echo "Usage: $0 <elf_filename><size_exec_name> <max_size>"
  exit 1
fi

max_size=$3
size_exec_name=$2
elf_filename=$1
actual_size=$($size_exec_name $elf_filename | sed 's/[^0-9.]*\([0-9.]*\).*/\1/')

if [ "$actual_size" -gt "$max_size" ]; then 
  echo "*** ERROR: ELF size too big"
  echo " Expected no more than: "$max_size" bytes"
  echo " Got: "$actual_size" bytes"
  exit 1
fi
