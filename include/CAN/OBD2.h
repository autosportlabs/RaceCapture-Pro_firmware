#ifndef OBD2_H_
#define OBD2_H_
#include "CAN.h"
#include "stddef.h"

#define OBD2_PID_DEFAULT_TIMEOUT_MS 100

int OBD2_request_PID(unsigned char pid, int *value, size_t timeout);
void OBD2_set_current_PID_value(size_t index, int value);
int OBD2_get_current_PID_value(int index);

#endif /* OBD2_H_ */
