#ifndef USB_CDC_DEVICE_H_
#define USB_CDC_DEVICE_H_
#include "FreeRTOS.h"

int USB_CDC_device_init(const int priority);

void USB_CDC_send_debug(portCHAR *string);

void USB_CDC_SendByte( portCHAR cByte );

portBASE_TYPE USB_CDC_ReceiveByte(portCHAR *data);

portBASE_TYPE USB_CDC_ReceiveByteDelay(portCHAR *data, portTickType delay );

int USB_CDC_is_initialized();

#endif /* USB_CDC_DEVICE_H_ */
