#include "cpu_device.h"

int cpu_device_init(void)
{
    return 1;
}

void cpu_device_reset(int bootloader)
{

}

const char * cpu_device_get_serialnumber(void)
{
    return "AAABBBCCCDDDEEEFFF000111";
}
