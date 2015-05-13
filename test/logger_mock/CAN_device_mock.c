#include "CAN_device.h"

int CAN_device_init(uint8_t channel, uint32_t baud)
{
    return 1;
}

int CAN_device_tx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs)
{
    return 1;
}

int CAN_device_rx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs)
{
    return 1;
}

int CAN_device_set_filter(uint8_t channel, uint8_t id, uint8_t extended, uint32_t filter, uint32_t mask)
{
    return 1;
}
