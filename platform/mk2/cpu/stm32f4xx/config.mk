CPU_ARCH := ARMCM4
CPU_TYPE := cortex-m4
CPU_BASE := cpu/stm32f4xx

CPU_FLAGS := \
-mcpu=$(CPU_TYPE) \
-mfpu=fpv4-sp-d16 \
-mfloat-abi=softfp \
-mthumb \

CPU_DEFINES := -D$(CPU_ARCH) -DSTM32F4XX
