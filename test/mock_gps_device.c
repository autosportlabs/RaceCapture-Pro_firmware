/*
 * Mock GPS device
 */

#include "gps.h"
#include "gps_device.h"
#include "serial.h"

int GPS_device_provision(Serial *serial) {
   return 1;
}

gps_msg_result_t GPS_device_get_update(GpsSample *gpsSample, Serial *serial) {
   return GPS_MSG_SUCCESS;
}
