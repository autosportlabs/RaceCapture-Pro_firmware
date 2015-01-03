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
#include <string.h>
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

	printf("RaceCapture/Pro simulator %s interface on: %s local fd %d\n", pcDevice, ptsname(iSerialDevice), iSerialDevice);
	
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
unsigned char ucRx[64];
//unsigned char ucRx;
unsigned int i = 0;

    memset(ucRx, 0, sizeof(ucRx));
	/* This handler only processes a single byte/character at a time. */
	iReadResult = read( iFileDescriptor, &ucRx, 64 );
	//iReadResult = read( iFileDescriptor, &ucRx, 1 );
	//if ( 1 == iReadResult )
	//{
		
	
		if ( NULL != pContext )
		{
			/* Send the received byte to the queue. */
			if(iReadResult > 0)
			{
				// 150102 jstoezel: need to read in chunks otherwise the interface is too slow.
				for(i = 0; i < iReadResult; i++)
				if ( pdTRUE == xQueueSendFromISR( (xQueueHandle)pContext, &ucRx[i], &xHigherPriorityTaskWoken ) )
				//if ( pdTRUE == xQueueSendFromISR( (xQueueHandle)pContext, &ucRx, &xHigherPriorityTaskWoken ) )
				//if ( pdTRUE == xQueueSend( (xQueueHandle)pContext, ucRx,0 ) )
				{
					//printf("\r\n0x%08X>%s", pContext, ucRx);
					/* the queue is full. */
				}
				else
				{
					printf("failed to send on queue 0x08%X\r\n", pContext);
				}
			}
			else
			{
			   printf("result is 0\r\n");
			}
		}
		else
		{
			printf("pContext is null\r\n");
		}
	//}
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
/*---------------------------------------------------------------------------*/
