#include "CAN.h"
#include "printk.h"
#include "CAN_device.h"
#include "loggerConfig.h"

int CAN_init(LoggerConfig *loggerConfig){
	CANConfig *canConfig = &loggerConfig->CanConfig;
	int success = 1;
	for (size_t i = 0; i < CAN_CHANNELS; i++){
		if (!CAN_init_port(i, canConfig->baudRate)) success = 0;
	}
	return success;
}

int CAN_init_port(uint8_t port, uint32_t baud){
	return CAN_device_init(port, baud);
}

int CAN_set_filter(uint8_t channel, uint8_t id, uint8_t extended, uint32_t filter, uint32_t mask){
	return CAN_device_set_filter(channel, id, extended, filter, mask);
}

int CAN_tx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs){
	return CAN_device_tx_msg(channel, msg, timeoutMs);
}

int CAN_rx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs){
	return CAN_device_rx_msg(channel, msg, timeoutMs);
}
