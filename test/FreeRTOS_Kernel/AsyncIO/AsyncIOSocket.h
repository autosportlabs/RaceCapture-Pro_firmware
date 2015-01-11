/*
 * AsyncIOSocket.h
 *
 *  Created on: 23 Sep 2009
 *      Author: William Davy
 */

#ifndef ASYNCIOSOCKET_H_
#define ASYNCIOSOCKET_H_

#define UDP_PACKET_SIZE		( 1023UL )

typedef struct UDP_PACKET
{
	unsigned portCHAR ucPacket[ UDP_PACKET_SIZE ];
	unsigned portCHAR ucNull;
} xUDPPacket;

/**
 * Opens a socket and registers the signal handler for received messages.
 * @param vSocketCallback A function pointer of a function that will be called back
 * when the socket has just received a packet. This call back is called from within
 * the signal handler so must use ISR safe routines.
 * @param pvContext The socket and caller supplied pointer to a Context are passed
 * to the call back function.
 * @param pxBindAddress A pointer to a struct sockaddr_in which describes what addresses
 * to accept packets from.
 * @return The newly opened socket.
 */
int iSocketOpenUDP( void (*vSocketCallback)( int, void * ), void *pvContext, struct sockaddr_in *pxBindAddress );

/**
 * Closes the socket and removes the call back function.
 * @param iSocket The socket to close.
 */
void vSocketClose( int iSocket );

/**
 * Send a packet to the given address.
 * @param iSocket A handle to a valid socket.
 * @param pxPacket A pointer to the UDP Packet to send.
 * @param pxSendAddress A pointer to a sockaddr_in structure that already contains the address and port for sending the data.
 * @return The number of bytes transmitted.
 */
int iSocketUDPSendTo( int iSocket, xUDPPacket *pxPacket, struct sockaddr_in *pxSendAddress );

/**
 * ISR only UDP Receive packet call.
 * @param iSocket The Handle to receive the data from.
 * @param pxPacket A packet to receive the data into.
 * @param pxReceiveAddress A pointer to a structure describing valid addresses to receive from.
 * @return The number of bytes received.
 */
int iSocketUDPReceiveISR( int iSocket, xUDPPacket *pxPacket, struct sockaddr_in *pxReceiveAddress );

/**
 * Non-ISR UDP Receive packet call.
 * @param iSocket The Handle to receive the data from.
 * @param pxPacket A packet to receive the data into.
 * @param pxReceiveAddress A pointer to a structure describing valid addresses to receive from.
 * @return The number of bytes received.
 */
int iSocketUDPReceiveFrom( int iSocket, xUDPPacket *pxPacket, struct sockaddr_in *pxReceiveAddress );

/**
 * Typical implementation of a call back function which simply delivers the received packet to a Queue which it is passed.
 * @param iSocket A socket desicriptor to receive the packet from.
 * @param pvQueueHandle An xQueueHandle which is waiting to receive the just received packet.
 */
void vUDPReceiveAndDeliverCallback( int iSocket, void *pvQueueHandle );

#endif /* ASYNCIOSOCKET_H_ */
