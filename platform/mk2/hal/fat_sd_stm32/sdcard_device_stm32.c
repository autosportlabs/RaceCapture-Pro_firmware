#include "sdcard_device.h"
#include "stm32_ub_sdcard.h"

void disk_init_hardware(void)
{
    UB_SDCard_Init();
}
