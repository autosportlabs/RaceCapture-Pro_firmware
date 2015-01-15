/*
 * AsyncIO.c
 *
 *  Created on: 9 Apr 2010
 *      Author: William Davy
 */

#define _GNU_SOURCE

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "AsyncIO.h"
/*---------------------------------------------------------------------------*/

#define SIG_MSG_RX		SIGIO
#define SIG_TICK		SIGALRM
/*---------------------------------------------------------------------------*/

typedef struct ASYNC_IO_CALLBACK
{
	int iFileHandle;
	void (*pvFunction)(int,void*);
	void *pvContext;
	struct ASYNC_IO_CALLBACK *pxNext;
} xAsyncIOCallback;
/*---------------------------------------------------------------------------*/

void prvSignalHandler( int signal, siginfo_t * data, void * pvParam );
void prvRegisterSignalHandler( int iSocket );
/*---------------------------------------------------------------------------*/

static xAsyncIOCallback xHead = { 0, NULL, NULL, NULL };
static volatile int iAlreadyRegisteredHandler = 0;
/*---------------------------------------------------------------------------*/

long lAsyncIORegisterCallback( int iFileDescriptor, void (*pvFunction)(int,void*), void *pvContext )
{
xAsyncIOCallback *pxIterator;
	if ( NULL != pvFunction )
	{
		/* Record the socket against its call back. */
		for ( pxIterator = &xHead; pxIterator->pxNext != NULL; pxIterator = pxIterator->pxNext );
		pxIterator->pxNext = ( xAsyncIOCallback * )malloc( sizeof( xAsyncIOCallback ) );
		pxIterator->pxNext->iFileHandle = iFileDescriptor;
		pxIterator->pxNext->pvFunction = pvFunction;
		pxIterator->pxNext->pvContext = pvContext;

		/* Set the socket as requiring a signal when messages are received. */
		prvRegisterSignalHandler( iFileDescriptor );
	}
	return ( NULL != pvFunction );
}
/*---------------------------------------------------------------------------*/

void vAsyncIOUnregisterCallback( int iFileDescriptor )
{
xAsyncIOCallback *pxIterator;
xAsyncIOCallback *pxDelete;
	for ( pxIterator = &xHead; ( pxIterator->pxNext != NULL ) && ( pxIterator->pxNext->iFileHandle != iFileDescriptor ); pxIterator = pxIterator->pxNext );
	if ( pxIterator->pxNext != NULL )
	{
		if ( pxIterator->pxNext->iFileHandle == iFileDescriptor )
		{
			if ( pxIterator->pxNext->pxNext != NULL )
			{
				pxDelete = pxIterator->pxNext;
				pxIterator->pxNext = pxDelete->pxNext;
			}
			else
			{
				pxDelete = pxIterator->pxNext;
				pxIterator->pxNext = NULL;
			}
			free( pxDelete );
		}
	}
}
/*---------------------------------------------------------------------------*/

void prvSignalHandler( int signal, siginfo_t * data, void * pvParam )
{
int iSocket = 0, iReturn;
xAsyncIOCallback *pxIterator;
struct pollfd xFileDescriptorPollEvents;
	if ( SIG_MSG_RX == signal )	/* Are we in the correct signal handler. */
	{
		if ( data->si_code == SI_SIGIO )	/* Do we know which socket caused the signal? */
		{
			iSocket = data->si_fd;	/* Yes we do. Find the owner. */

			for ( pxIterator = &xHead; ( pxIterator->pxNext != NULL ) && ( pxIterator->iFileHandle != iSocket ); pxIterator = pxIterator->pxNext );

			if ( pxIterator->iFileHandle == iSocket )
			{
				( pxIterator->pvFunction )( iSocket, pxIterator->pvContext );
			}
			else
			{
				printf( "No socket owner.\n" );
			}
		}
		else
		{
			/* We don't know which socket cause the signal. Use poll to find the socket. */
			for ( pxIterator = &xHead; pxIterator != NULL; pxIterator = pxIterator->pxNext )
			{
				xFileDescriptorPollEvents.fd = pxIterator->iFileHandle;
				xFileDescriptorPollEvents.events = POLLIN;
				xFileDescriptorPollEvents.revents = 0;
				if ( xFileDescriptorPollEvents.fd != 0 )
				{
					iReturn = poll( &xFileDescriptorPollEvents, 1, 0 );	/* Need to kick off the signal handling. */

					if ( ( 1 == iReturn ) && ( POLLIN == xFileDescriptorPollEvents.revents ) )
					{
						if ( pxIterator->pvFunction != NULL )
						{
							( pxIterator->pvFunction )( xFileDescriptorPollEvents.fd, pxIterator->pvContext );
						}
					}
				}
			}
			/* Note that poll should really be passed all of the sockets in one go and then iterate over the results. */
		}
	}
}
/*---------------------------------------------------------------------------*/

void prvRegisterSignalHandler( int iSocket )
{
struct sigaction xAction;

	if ( 1 != iAlreadyRegisteredHandler )
	{
		/* Initialise the sigaction struct for the signal handler. */
		xAction.sa_sigaction = prvSignalHandler;
		xAction.sa_flags = SA_SIGINFO;	/* Using this option, the signal handler knows which socket caused the signal. */
		sigfillset( &xAction.sa_mask );
		sigdelset( &xAction.sa_mask, SIG_MSG_RX );
		sigdelset( &xAction.sa_mask, SIG_TICK );

		/* Register the signal handler. */
		if ( 0 != sigaction( SIG_MSG_RX, &xAction, NULL ) )	/* Register the Signal Handler. */
		{
			printf( "Problem installing SIG_MSG_RX\n" );
		}
		else
		{
			iAlreadyRegisteredHandler = 1;
		}
	}

	/* Request that the socket generates a signal. */
	if ( SIG_MSG_RX == SIGIO )
	{
		if ( 0 != fcntl( iSocket, F_SETOWN, getpid() ) )
		{
			printf( "fnctl: Failed: %d\n", errno );
		}
	}
	else
	{
		/* Use real-time signals instead of SIGIO */
		if ( 0 != fcntl( iSocket, F_SETSIG, SIG_MSG_RX ) )
		{
			printf( "fnctl: Failed: %d\n", errno );
		}
	}

	/* Indicate that the socket is Asynchronous. */
	if ( 0 != fcntl( iSocket, F_SETFL, O_ASYNC ) )
	{
		printf( "fcntl: Failed: %d\n", errno );
	}
}
/*---------------------------------------------------------------------------*/
