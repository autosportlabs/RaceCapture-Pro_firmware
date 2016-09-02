/*
*****************************************************************************
**
**  File        : STM32-FLASH.ld
**
**  Abstract    : Linker script for STM32F30x Device with
**                256KByte FLASH and 40KByte RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Environment : Atollic TrueSTUDIO(R)
**
**  Distribution: The file is distributed "as is," without any warranty
**                of any kind.
**
**  (c)Copyright Atollic AB.
**  You may use this file as-is or modify it according to the needs of your
**  project. Distribution of this file (unmodified or modified) is not
**  permitted. Atollic AB permit registered Atollic TrueSTUDIO(R) users the
**  rights to distribute the assembled, compiled & linked contents of this
**  file as part of an application binary file, provided that it is built
**  using the Atollic TrueSTUDIO(R) toolchain.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = 0x20000000 + 40K;    /* end of 40K RAM */

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x200;      /* required amount of heap  */
_Min_Stack_Size = 0x400; 	/* required amount of stack */

/* Specify the memory areas */
MEMORY
{
  BOOTLDR   (rx)    : ORIGIN = 0x08000000, LENGTH = 16K
  FLASH	    (rx)    : ORIGIN = 0x08004000, LENGTH = 256K - 48K

  CONFIG    (rx)    : ORIGIN = 0x08038000, LENGTH = 16K
  TRACKS    (rx)    : ORIGIN = 0x0803C000, LENGTH = 16K

  /* BSS segment of STM32f3.  8K in size */
  BSS       (rxw)   : ORIGIN = 0x10000000, LENGTH = 8K

  /* SRAM segment of STM32f3. 40K in size */
  HANDSHAKE (rwx)   : ORIGIN = 0x20000000, LENGTH = 8
  DATA      (rwx)   : ORIGIN = 0x20000008, LENGTH = 4K - 8
  HEAP      (rwx)   : ORIGIN = 0x20001000, LENGTH = 36K
}

_heap_address = ORIGIN(HEAP);
_flash_start = ORIGIN(FLASH) - ORIGIN(BOOTLDR);
_CONFIG_HEAP_SIZE = LENGTH(HEAP);

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

   config :
   {
     . = ALIGN(4);
     KEEP (*(.config))
   } > CONFIG

   tracks :
   {
    . = ALIGN(4);
    KEEP (*(.tracks))
   } > TRACKS

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH


   .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
    .ARM : {
    __exidx_start = .;
      *(.ARM.exidx*)
      __exidx_end = .;
    } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array*))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  . = ALIGN(4);
  _sidata = .;

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : AT ( _sidata )
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >DATA

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >BSS

  PROVIDE ( end = _ebss );
  PROVIDE ( _end = _ebss );

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(4);
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(4);
  } >DATA

  heap :
   {
    . = ALIGN(4);
    KEEP (*(.heap))
   } >HEAP


  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
