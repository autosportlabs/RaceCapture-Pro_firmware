/*
 * cpu.h
 *
 *  Created on: Feb 22, 2014
 *      Author: brent
 */

#ifndef CPU_H_
#define CPU_H_
#include <stdint.h>

int cpu_init(void);
void cpu_reset(int bootloader);
const char * cpu_get_serialnumber(void);



#endif /* CPU_H_ */
