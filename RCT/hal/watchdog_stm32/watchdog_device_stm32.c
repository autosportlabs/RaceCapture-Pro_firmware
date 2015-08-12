#include "watchdog_device.h"
#include "LED.h"
#include <stm32f30x_iwdg.h>
#include <stm32f30x_rcc.h>

#define LSI_FREQUENCY 32000

inline void watchdog_device_reset()
{
    IWDG_ReloadCounter();
}

void watchdog_device_init(int timeoutMs)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_32);
    IWDG_SetReload(timeoutMs);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

int watchdog_device_is_watchdog_reset()
{
    return (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET) ? 1 : 0;
}

int watchdog_device_is_poweron_reset()
{
    return (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) ? 1 : 0;
}
