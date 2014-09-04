/*
 * cpu_device.h
 *
 *  Created on: Feb 22, 2014
 *      Author: brent
 */

#ifndef CPU_DEVICE_H_
#define CPU_DEVICE_H_

int cpu_device_init(void);
void cpu_device_reset(void);
const char * cpu_device_get_serialnumber(void);

#endif /* CPU_DEVICE_H_ */
