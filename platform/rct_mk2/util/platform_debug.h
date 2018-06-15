#ifndef __PLATFORM_H__
#define __PLATFORM_H__

/* ST's DBGMCU register */
#if defined (STM32F4XX)
#include <stm32f4xx_dbgmcu.h>
#elif defined (STM32F10X_LD)    || \
      defined (STM32F10X_LD_VL) || \
      defined (STM32F10X_MD)    || \
      defined (STM32F10X_MD_VL) || \
      defined (STM32F10X_HD)    || \
      defined (STM32F10X_HD_VL) || \
      defined (STM32F10X_XL)    || \
      defined (STM32F10X_CL)
#include <stm32f10x_dbgmcu.h>
#elif defined (STM32F30X)
#include <stm32f30x_dbgmcu.h>
#elif defined (STM32L1XX)
#include <stm32l1xx_dbgmcu.h>
#else
#error "I don't know which MCU and CPU to target."
#endif

/* CMSIS */
#if defined(ARMCM4)
#include <core_cm4.h>
#elif defined(ARMCM3)
#include <core_cm3.h>
#elif defined(ARMCM0)
#include <core_cm0.h>
#else
#error "I don't know which CPU to target."
#endif

#endif /* __PLATFORM_H__ */
