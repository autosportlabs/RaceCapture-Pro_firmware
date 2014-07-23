#include "CAN_device.h"
#include <stdint.h>

int CAN_device_init(int baud){
	return 1;
}

int CAN_device_set_mask(uint8_t id, uint8_t extended, uint32_t mask){
	return 1;
}

int CAN_device_set_filter(uint8_t id, uint8_t extended, uint32_t filter){
	return 1;
}

int CAN_device_tx_msg(CAN_msg *msg, unsigned int timeoutMs){
	return 1;
}

int CAN_device_rx_msg(CAN_msg *msg, unsigned int timeoutMs){
	return 1;
}
