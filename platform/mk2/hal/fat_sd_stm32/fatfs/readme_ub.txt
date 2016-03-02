FatFs - Generic FAT File System Module
========================================


Name   = FatFs Module Source Files R0.09b
Quelle = http://elm-chan.org/fsw/ff/00index_e.html
Autor  = ChaN


stm32_ub_fatfs (UB)
  |
  |
  +--ff (ChaN)
      |
      |
      +--diskio (ChaN/UB)
      |   |
      |   |
      |   +---stm32_ub_usbdisk (UB)
      |   |
      |   +---stm32_ub_atadrive (UB)
      |   |
      |   +---stm32_ub_sdcard (UB)
      |
      +--ffconf (ChaN)
      |
      +--integer (ChaN)



Anpassungen von UB [V:1.0] :
=============================

Im File "diskio.c"
  1. Filenamen der LoLevel-Files angepasst
  2. Return-Werte fuer alle Funktionen gesetzt
  3. Reihenfolge der "physical drive number" geaendert
  4. Funktion "get_fattime" hinzugefuegt

Im File "ffconf.h"
  1. Die Codepage auf "1252" eingestellt
  2. "_USE_STRFUNC" auf 2 eingestellt




