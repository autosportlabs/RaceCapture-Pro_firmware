/*
 * CAN.h
 *
 *  Created on: Jan 26, 2014
 *      Author: brent
 */

#ifndef CAN_H_
#define CAN_H_

#define CAN_BAUD_50K			50000
#define CAN_BAUD_100K 			100000
#define CAN_BAUD_125K			125000
#define CAN_BAUD_250K			250000
#define CAN_BAUD_500K			500000
#define CAN_BAUD_1M				1000000

typedef struct _CAN_msg
{
  int isExtendedAdrs;
  unsigned long adrsValue;
  int remoteTxRequest;
  unsigned char dataLength;
  unsigned char data[8];
}  CAN_msg;


int CAN_init(void);
int CAN_set_baud(int baud);
int CAN_tx_msg(CAN_msg *msg, unsigned int timeoutMs);



#endif /* CAN_H_ */
