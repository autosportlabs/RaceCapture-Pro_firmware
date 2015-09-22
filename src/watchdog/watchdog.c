#include "watchdog.h"
#include "watchdog_device.h"
#include "printk.h"

void watchdog_reset(void)
{
    watchdog_device_reset();
}

void watchdog_init(int timeoutMs)
{
    watchdog_device_init(timeoutMs);
    if (watchdog_is_watchdog_reset()) {
        pr_warning("watchdog: detected watchdog reset!\r\n");
    }
}

bool watchdog_is_watchdog_reset(void)
{
    return watchdog_device_is_watchdog_reset();
}

bool watchdog_is_poweron_reset(void)
{
    return watchdog_device_is_poweron_reset();
}
