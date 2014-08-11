#include "CAN.h"
#include "printk.h"
#include "CAN_device.h"

static int CAN_is_init = 0;

int CAN_init(int baud){
	CAN_is_init = CAN_device_init(baud);
	return CAN_is_init;
}

int CAN_set_filter(uint8_t id, uint8_t extended, uint32_t filter, uint32_t mask){
	return CAN_is_init && CAN_device_set_filter(id, extended, filter, mask);
}

int CAN_tx_msg(CAN_msg *msg, unsigned int timeoutMs){
	return CAN_is_init && CAN_device_tx_msg(msg, timeoutMs);
}

int CAN_rx_msg(CAN_msg *msg, unsigned int timeoutMs){
	return CAN_is_init && CAN_device_rx_msg(msg, timeoutMs);
}
