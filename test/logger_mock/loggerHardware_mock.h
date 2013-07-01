/*
 * loggerHardware_mock.h
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */

#ifndef LOGGERHARDWARE_MOCK_H_
#define LOGGERHARDWARE_MOCK_H_

void mock_setIsCardPresent(int present);
void mock_setIsCardWritable(int writable);
void mock_sButtonPressed(int pressed);
void mock_setIsFlashed(int isFlashed);
int mock_getIsFlashed();



#endif /* LOGGERHARDWARE_MOCK_H_ */
