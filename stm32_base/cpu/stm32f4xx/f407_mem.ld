_estack = 0x20020000;

_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x200;

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
  CCM (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
}
