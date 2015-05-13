/*
 * Mock GPS device
 */

#include "gps.h"
#include "gps_device.h"
#include "serial.h"

gps_status_t GPS_device_init(uint8_t targetSampleRate, Serial *serial)
{
    return GPS_STATUS_PROVISIONED;
}

gps_msg_result_t GPS_device_get_update(GpsSample *gpsSample, Serial *serial)
{
    return GPS_MSG_SUCCESS;
}
