#ifndef OBD2_H_
#define OBD2_H_
#include "CAN.h"
#include "stddef.h"

int OBD2_request_PID(unsigned char pid, int *value, size_t timeout);
void OBD2_set_current_PID_value(size_t index, int value);
int OBD2_get_current_PID_value(size_t index);

#endif /* OBD2_H_ */

