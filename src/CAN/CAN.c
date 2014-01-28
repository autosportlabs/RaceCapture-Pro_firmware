#include "CAN.h"
#include "printk.h"
#include "CAN_device.h"


int CAN_set_baud(int baud){
	return CAN_device_set_baud(baud);
}

int CAN_init(void){
	return CAN_device_init();
}

int CAN_tx_msg(CAN_msg *msg, unsigned int timeoutMs){
	return CAN_device_tx_msg(msg, timeoutMs);
}

int CAN_rx_msg(CAN_msg *msg, unsigned int timeoutMs){
	return CAN_device_rx_msg(msg, timeoutMs);
}
