include cpu/stm32f4xx/config.mk

CPU = stm32f407
CPU_LINK_MEM = f407_mem.ld

#this board uses the stmf4discovery board which has an 8mhz external
#oscillator
BOARD_DEFINES += -DHSE_VALUE=8000000 -DPLL_M=8
