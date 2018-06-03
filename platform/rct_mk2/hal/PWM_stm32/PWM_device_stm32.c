#include "PWM_device.h"

#include <stdint.h>
#include <stddef.h>


int PWM_device_init()
{
    /* NOOP - not supported on this device */
    return 1;
}

void PWM_device_set_clock_frequency(uint16_t clockFrequency)
{
    /* NOOP - not supported on this device */
}

void PWM_device_channel_init(unsigned int channel, unsigned short period,
                             unsigned short dutyCycle)
{
    /* NOOP - not supported on this device */
}

void PWM_device_channel_start_all()
{
}

void PWM_device_channel_stop_all()
{
}

void PWM_device_channel_start(unsigned int channel)
{
}

void PWM_device_channel_stop(unsigned int channel)
{
}

void PWM_device_channel_set_period(unsigned int channel, unsigned short period)
{
}

unsigned short PWM_device_channel_get_period(unsigned int channel)
{
    return 0;
}

void PWM_device_set_duty_cycle(unsigned int channel, unsigned short duty)
{
    /* NOOP - not supported on this device */
}

unsigned short PWM_device_get_duty_cycle(unsigned int channel)
{
    return 0;
}

void PWM_device_channel_enable_analog(size_t channel, uint8_t enabled)
{
    /* NOOP - not supported on this device */
}
