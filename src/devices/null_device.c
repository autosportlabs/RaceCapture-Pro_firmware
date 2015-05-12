#include "null_device.h"

int null_device_init_connection(DeviceConfig *config)
{
    return DEVICE_INIT_SUCCESS;
}

int null_device_check_connection_status(DeviceConfig *config)
{
    return DEVICE_STATUS_NO_ERROR;
}
