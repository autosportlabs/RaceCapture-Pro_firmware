/*
 * AsyncIOSerial.h
 *
 *  Created on: 9 Apr 2010
 *      Author: William Davy
 */

#ifndef ASYNCIOSERIAL_H_
#define ASYNCIOSERIAL_H_

/**
 * Opens and configures a serial device which exists on the path specified by pcDevice.
 * The serial is defined as a 'raw' connection with 38400 baudrate, 8 bit, 1 stopbit, no parity, no flow.
 * @param pcDevice A string path to the serial device, typically, /dev/ttyS0
 * @param piDeviceDescriptor A pointer to an int which is set to the file descriptor for the opened device.
 * @return TRUE iff the device was successfully opened and configured.
 */
long lAsyncIOSerialOpen( const char *pcDevice, int *piDeviceDescriptor );

/**
 * The 'ISR' callback function that is triggered when there is data waiting.
 * This function is intended to be registered using lAsyncIORegisterCallback().
 * It collects as many bytes as possible, delivering them one byte at a time to
 * the ( xQueueHandle )pContext.
 * @param iFileDescriptor The file descriptor for the serial port.
 * @param pContext A Handle to a queue. Cast to xQueueHandle.
 */
void vAsyncSerialIODataAvailableISR( int iFileDescriptor, void *pContext );

#endif /* ASYNCIOSERIAL_H_ */
