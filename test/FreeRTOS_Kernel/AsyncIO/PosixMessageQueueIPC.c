/*
 * PosixIPC.c
 *
 *  Created on: 02-Jun-2009
 *      Author: William Davy
 */

#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <signal.h>

#include "FreeRTOS.h"
#include "PosixMessageQueueIPC.h"
/*---------------------------------------------------------------------------*/

#define MAX_MESSAGE_SIZE		( 8192UL )		/* This value should really be found from the attributes. Or read from /proc/sys/fs/mqueue/msgsize_max */
#define SIG_MSG_RX				( 34 )
#define MAX_NUMBER_OF_MESSAGES	( 10 )
/*---------------------------------------------------------------------------*/

typedef struct PIPE_AND_FUNCTION
{
	mqd_t hPipe;
	void (*pvFunction)(xMessageObject,void*);
	void *pvContext;
} xPipeAndFunction;

static void prvPosixIPCInterruptHandler( int signal, siginfo_t * data, void * pvParam );
/*---------------------------------------------------------------------------*/

static struct sigevent xMessageEvent;
static union sigval xParameters;
static xPipeAndFunction xActualParameters;
/*---------------------------------------------------------------------------*/

mqd_t xPosixIPCOpen( const portCHAR *pcPipeName, void (*vMessageInterruptHandler)(xMessageObject,void*), void *pvContext )
{
mqd_t hPipeHandle;
struct sigaction sigrt;

	hPipeHandle = mq_open( pcPipeName, O_RDWR | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR, NULL );
	if ( -1 == hPipeHandle )
	{
		hPipeHandle = mq_unlink( pcPipeName );
		hPipeHandle = 0;
		printf( "Problem opening message pipe.\n" );
	}
	else
	{
		printf( "Pipe Open: %s\n", pcPipeName );
	}

	/* If this is a receive pipe then we set-up the interrupt handler. */
	if ( ( 0 != hPipeHandle ) && ( NULL != vMessageInterruptHandler ) )
	{
		/* Establish the signal handler. */
		sigrt.sa_flags = SA_SIGINFO;						/* Need some information to be passed. */
		sigrt.sa_handler = (__sighandler_t)prvPosixIPCInterruptHandler;		/* This is the function to call. */
		sigfillset( &sigrt.sa_mask );
		sigdelset( &sigrt.sa_mask, SIG_MSG_RX );
		sigdelset( &sigrt.sa_mask, SIG_TICK );

		if ( 0 != sigaction( SIG_MSG_RX, &sigrt, NULL ) )	/* Register the Signal Handler. */
		{
			printf( "Problem installing SIG_MSG_RT\n" );
		}		/* Parameters to the signal handler. */

		/* No establish the notify parameters. */
		xActualParameters.hPipe = hPipeHandle;
		xActualParameters.pvFunction = vMessageInterruptHandler;
		xActualParameters.pvContext = pvContext;
		xParameters.sival_ptr = &xActualParameters;

		/* Set-up the notifcation. */
		xMessageEvent.sigev_notify = SIGEV_SIGNAL;				/* How to signal. */
		xMessageEvent.sigev_signo = SIG_MSG_RX;					/* Signal number to use. */
		xMessageEvent.sigev_value = xParameters;				/* Parameters to pass to the signal handler. */
		xMessageEvent.sigev_notify_function = NULL;				/* Signal handler function. */
		xMessageEvent.sigev_notify_attributes = NULL;			/* Parameters if a new thread is created to execute the signal handler. */

		/* Register the notification. */
		if ( 0 != mq_notify( hPipeHandle, &xMessageEvent ) )
		{
			printf( "Failed to install Rx interrupt handler: %d.\n", errno );
		}
	}
	return hPipeHandle;
}
/*---------------------------------------------------------------------------*/

void vPosixIPCClose( mqd_t hPipeHandle, const portCHAR *pcPipeName )
{
	(void)mq_close( hPipeHandle );
	if ( NULL != pcPipeName )
	{
		(void)mq_unlink( pcPipeName );
	}
}
/*---------------------------------------------------------------------------*/

portLONG lPosixIPCSendMessage( mqd_t hPipeHandle, xMessageObject xMessage )
{
portLONG lReturn = pdFALSE;

	if ( 0 != hPipeHandle )
	{
		if ( 0 == mq_send( hPipeHandle, (const char *)&xMessage, sizeof( xMessageObject ), 1 ) )
		{
			lReturn = pdTRUE;
		}
		else
		{
			printf( "Message Sending error.\n" );
		}
	}
	return lReturn;
}
/*---------------------------------------------------------------------------*/

portLONG lPosixIPCReceiveMessage( mqd_t hPipeHandle, xMessageObject *pxMessage )
{
static char pcBuffer[ MAX_MESSAGE_SIZE ];
unsigned int uiPriority;
portLONG lBytesReceived = 0;
portLONG lBytesToProcess;
portLONG lReturn = pdFALSE;

	lBytesReceived = mq_receive( hPipeHandle, pcBuffer, MAX_MESSAGE_SIZE, &uiPriority );
	for ( lBytesToProcess = 0; lBytesToProcess < lBytesReceived; lBytesToProcess += sizeof( xMessageObject ) )
	{
		memcpy( pxMessage, pcBuffer, sizeof( xMessageObject ) );
		lReturn = pdTRUE;

		/* Only receiving one message at the moment. */
		break;
	}
	return lReturn;
}
/*---------------------------------------------------------------------------*/

void vPosixIPCEmpty( mqd_t hPipeHandle )
{
char pcBuffer[ MAX_MESSAGE_SIZE ];
unsigned int uiPriority;
	while ( -1 != mq_receive( hPipeHandle, pcBuffer, MAX_MESSAGE_SIZE, &uiPriority ) );
}
/*---------------------------------------------------------------------------*/

void prvPosixIPCInterruptHandler( int signal, siginfo_t * data, void * pvParam )
{
union sigval xParameters = data->si_value;
xPipeAndFunction *pxPipes = ( xPipeAndFunction *)xParameters.sival_ptr;
mqd_t hPipeHandle = 0;
void (*vMessageInterruptHandler)(xMessageObject,void*) = NULL;
xMessageObject xRxMessage;
portLONG lMessagesReceived = 0;

	if ( NULL != pxPipes )
	{
		hPipeHandle = pxPipes->hPipe;
		vMessageInterruptHandler = pxPipes->pvFunction;

		memset( &xRxMessage, 0, sizeof( xMessageObject ) );

		if ( NULL != vMessageInterruptHandler )
		{
			while ( ( pdTRUE == lPosixIPCReceiveMessage( hPipeHandle, &xRxMessage ) ) && ( lMessagesReceived++ < MAX_NUMBER_OF_MESSAGES ) )
			{
				vMessageInterruptHandler( xRxMessage, pxPipes->pvContext );
			}
		}

		/* Re-register the notification. */
		if ( 0 != mq_notify( hPipeHandle, &xMessageEvent ) )
		{
			printf( "Failed to install Rx interrupt handler: %d.\n", errno );
		}
	}
	else
	{
		printf( "NULL parameter: Failed to install Rx interrupt handler: %d.\n", errno );
	}
}
/*---------------------------------------------------------------------------*/
