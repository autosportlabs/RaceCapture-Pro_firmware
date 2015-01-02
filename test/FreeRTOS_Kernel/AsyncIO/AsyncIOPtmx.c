/*
 * AsyncIOPtmx.c
 *
 *  Created on: 9 Apr 2010
 *      Author: William Davy
 */

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "AsyncIOSerial.h"
/*---------------------------------------------------------------------------*/

/* See 'man termios' for more details on configuring the serial port. */

long lAsyncIOPtmxOpen( const char *pcDevice, int *piDeviceDescriptor )
{
   long lReturn = pdFALSE;
   int iSerialDevice = 0;

   iSerialDevice = open("/dev/ptmx", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (iSerialDevice < 0)
	{
		printf("failed to open /dev/ptmx");
		return pdFALSE;
	}

	grantpt(iSerialDevice);
	unlockpt(iSerialDevice);

	printf("RaceCapture/Pro simulator on: %s\n", ptsname(iSerialDevice));
	
	/* Pass out the device descriptor for subsequent calls to AsyncIORegisterCallback() */
	*piDeviceDescriptor = iSerialDevice;

	return pdTRUE;
}
 /*---------------------------------------------------------------------------*/

/* Define a callback function which is called when data is available. */
void vAsyncPtmxIODataAvailableISR( int iFileDescriptor, void *pContext )
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
ssize_t iReadResult = -1;
unsigned char ucRx;

	/* This handler only processes a single byte/character at a time. */
	iReadResult = read( iFileDescriptor, &ucRx, 1 );
	if ( 1 == iReadResult )
	{
		//printf("\r\n>%c", ucRx);
	
		if ( NULL != pContext )
		{
			/* Send the received byte to the queue. */
			if ( pdTRUE != xQueueSendFromISR( (xQueueHandle)pContext, &ucRx, &xHigherPriorityTaskWoken ) )
			{
				/* the queue is full. */
			}
		}
	}
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
/*---------------------------------------------------------------------------*/
