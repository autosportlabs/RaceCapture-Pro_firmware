/*
 * AsyncIO.h
 *
 *  Created on: 9 Apr 2010
 *      Author: William Davy
 */

#ifndef ASYNCIO_H_
#define ASYNCIO_H_

/**
 * Used to register a callback function for notification when an asynchronous IO action is completed.
 * @param iFileDescriptor The File descriptor used for the asynchronous IO.
 * @param pvFunction The callback function that receives the file descriptor and pvContext when the IO completes.
 * @param pvContext A caller supplied parameter for the pvFunction.
 * @return TRUE iff there is a valid pvFunction to register.
 */
long lAsyncIORegisterCallback( int iFileDescriptor, void (*pvFunction)(int,void*), void *pvContext );

/**
 * Removes the registered call back from the list.
 * @param iFileDescriptor The file descriptor for an already closed file handle.
 */
void vAsyncIOUnregisterCallback( int iFileDescriptor );


#endif /* ASYNCIO_H_ */
