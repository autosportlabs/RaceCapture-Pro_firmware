//  *****************************************************************************
//   						blinker.c
// 
//     Endless loop blinks a code for crash analysis
//     
//	   Inputs:	Code  -  blink code to display
//						 1 = undefined instruction (one blinks ........ long pause)
//						 2 = prefetch abort        (two blinks ........ long pause)
//						 3 = data abort            (three blinks ...... long pause)
//
//  Author:  James P Lynch  July 12, 2006
//  ***************************************************************************** 

#include "AT91SAM7S256.h"
#include "board.h"

/*-----------------*/
/* LED Definitions */
/*-----------------*/
/*                                 PIO   Flash    PA   PB     PIN */
//#define LED1            (1<<0)  /* PA0 / PGMEN0 & PWM0 TIOA0   48 */
//#define LED2            (1<<1)  /* PA1 / PGMEN1 & PWM1 TIOB0   47 */
#define LED1            (1<<17)
#define LED2			(1<<18)
#define LED3            (1<<2)  /* PA2          & PWM2 SCK0    44 */
#define LED4            (1<<3)  /* PA3          & TWD  NPCS3   43 */
#define NB_LED          4
#define LED_MASK        (LED1|LED2|LED3|LED4)

unsigned long	blinkcount;									// global variable


void  blinker(unsigned char    code) {
	volatile AT91PS_PIO		pPIO = AT91C_BASE_PIOA;			// pointer to PIO register structure
	volatile unsigned int	j,k;							// loop counters
		
	// endless loop	
	while (1)  {	
		for  (j = code; j != 0; j--) {						// count out the proper number of blinks
			pPIO->PIO_CODR = LED1;							// turn LED1 (DS1) on	
			for (k = 600000; k != 0; k-- );					// wait 250 msec
			pPIO->PIO_SODR = LED1;							// turn LED1 (DS1) off
			for (k = 600000; k != 0; k-- );					// wait 250 msec
		}
		for (k = 5000000; (code != 0) && (k != 0); k-- );	// wait 2 seconds
		blinkcount++;
	}	
}
