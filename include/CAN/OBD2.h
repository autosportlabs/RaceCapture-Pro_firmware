#ifndef OBD2_H_
#define OBD2_H_
#include "CAN.h"
#include "stddef.h"

int OBD2_request_PID(unsigned char pid, size_t timeout);



#endif /* OBD2_H_ */
