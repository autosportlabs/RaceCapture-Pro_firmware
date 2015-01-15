/*
 * PosixIPC.h
 *
 *  Created on: 02-Jun-2009
 *      Author: William Davy
 */

#ifndef POSIXIPC_H_
#define POSIXIPC_H_

/* This file abstracts the Posix Messaging Queue to allow for Inter-Process Communication.
 * The interface is designed to be event driven, ie when a message is received a process
 * signal is fired which collects the data and passes it to a registered call back function.
 *
 * Type 'man mq_overview' on the command line to get more details.
 *
 * The limits on my system are (/proc/sys/fs/mqueue/):
 * 		msg_max:		10		At most ten messages waiting in a queue.
 * 		msgsize_max		8192	At most 8192 bytes per message.
 * 		queues_max		256		At most 256 message queues.
 */

/**
 * This is an example of Message object that is used for IPC.
 */
typedef struct MESSAGE_OBJECT
{
	portCHAR cMesssageBytes[ 512 ];
} xMessageObject;

/**
 * Opens a Posix Message Queue used for Inter-Process Communication.
 * @param pcPipeName Pipe to open/create. Keep this string common between processes.
 * @param vMessageInterruptHandler A call back function used when a message is received.
 * @param pvContext A caller specifed context value that is passed to the callback function.
 * @return A handle to the opened Posix Message Queue.
 */
mqd_t xPosixIPCOpen( const portCHAR *pcPipeName, void (*vMessageInterruptHandler)(xMessageObject,void*), void *pvContext );

/**
 * Closes the specified Message Queue.
 * @param hPipeHandle A handle to the pipe so it can be closed.
 * @param pcPipeName The name of the pipe so that it can be unlinked and messages dumped.
 */
void vPosixIPCClose( mqd_t hPipeHandle, const portCHAR *pcPipeName );

/**
 * Sends a messages to the specified pipe.
 * @param hPipeHandle The handle to the pipe to send the message to.
 * @param xMessage The message to write to the pipe.
 * @return TRUE iff the message was successfully written to the queue. There is a limit
 * on the maximum number of messages on the queue even if the total memory for the
 * queue hasn't been exceeded. (See 'man mq_overview').
 */
portLONG lPosixIPCSendMessage( mqd_t hPipeHandle, xMessageObject xMessage );

/**
 * Non-blocking call to receive a message from a message queue. This function can be
 * used for polling the message queue. If a call back function was registered when the
 * message queue was opened then this function doesn't need to be called.
 * @param hPipeHandle A handle to the message queue to receive from.
 * @param pxMessage A pointer to a message object that is set to the received message.
 * @return TRUE iff a message of the correct number of bytes was received.
 */
portLONG lPosixIPCReceiveMessage( mqd_t hPipeHandle, xMessageObject *pxMessage );

/**
 * Remove all of the messages already sent to a queue. Useful if there are outstanding
 * messages since the last run of the application.
 * @param hPipeHandle The handle to the pipe that needs to be emptied.
 */
void vPosixIPCEmpty( mqd_t hPipeHandle );

#endif /* POSIXIPC_H_ */
