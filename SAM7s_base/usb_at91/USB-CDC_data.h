/*
 * USB-CDC_data.h
 *
 *  Created on: Jul 22, 2014
 *      Author: brent
 */

#ifndef USB_CDC_DATA_H_
#define USB_CDC_DATA_H_
#include "FreeRTOS.h"

/* Structure used to take a snapshot of the USB status from within the ISR. */
typedef struct X_ISR_STATUS
{
	unsigned portLONG ulISR;
	unsigned portLONG ulCSR0;
	unsigned portCHAR ucFifoData[ 8 ];
} xISRStatus;

/* Structure used to hold the received requests. */
typedef struct
{
	unsigned portCHAR ucReqType;
	unsigned portCHAR ucRequest;
	unsigned portSHORT usValue;
	unsigned portSHORT usIndex;
	unsigned portSHORT usLength;
} xUSB_REQUEST;



#endif /* USB_CDC_DATA_H_ */
