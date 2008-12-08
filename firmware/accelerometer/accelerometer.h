#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include "board.h"

void accel_init();
void accel_initSPI();
void accel_spiSetSpeed(unsigned char speed);
unsigned char accel_spiSend(unsigned char outgoing);
void spi_init_chip_select(unsigned int chip, unsigned int speed,
                      unsigned int dlybct,
                      unsigned int dlybs, unsigned int phase,
                      unsigned int polarity);

void selectChip(unsigned char chip);
void accel_setup();
unsigned char accel_readControlRegister();
unsigned int accel_readAxis(unsigned char axis);                      

#endif /*ACCELEROMETER_H_*/
