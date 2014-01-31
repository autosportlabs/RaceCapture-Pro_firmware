#include "CAN.h"
#include "printk.h"
#include "CAN_device.h"


int CAN_init(int baud){
	return CAN_device_init(baud);
}

int CAN_tx_msg(CAN_msg *msg, unsigned int timeoutMs){
	return CAN_device_tx_msg(msg, timeoutMs);
}

int CAN_rx_msg(CAN_msg *msg, unsigned int timeoutMs){
	return CAN_device_rx_msg(msg, timeoutMs);
}
