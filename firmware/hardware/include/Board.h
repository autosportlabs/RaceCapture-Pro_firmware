/*----------------------------------------------------------------------------
*         ATMEL Microcontroller Software Support  -  ROUSSET  -
*----------------------------------------------------------------------------
* The software is delivered "AS IS" without warranty or condition of any
* kind, either express, implied or statutory. This includes without
* limitation any warranty or condition with respect to merchantability or
* fitness for any particular purpose, or against the infringements of
* intellectual property rights of others.
*----------------------------------------------------------------------------
* File Name           : Board.h
* Object              : AT91SAM7S256-EK Evaluation Board Features Definition File.
*
* Creation            : JPP   16/Jun/2004
*----------------------------------------------------------------------------
*/
#ifndef Board_h
#define Board_h

#include "AT91SAM7S256.h"
#include "lib_AT91SAM7S256.h"


/*-------------------------*/
/* Push Buttons Definition */
/*-------------------------*/
/*                                  PIO    Flash    PA   PB     PIN */
#define SW1_MASK        (1<<19)  /* PA19 / PGMD7  & RK   FIQ     13 */
#define SW2_MASK        (1<<20)  /* PA20 / PGMD8  & RF   IRQ0    16 */
#define SW3_MASK        (1<<15)  /* PA15 / PGM3   & TF   TIOA1   20 */
#define SW4_MASK        (1<<14)  /* PA14 / PGMD2  & SPCK PWM3    21 */
#define SW_MASK         (SW1_MASK|SW2_MASK|SW3_MASK|SW4_MASK)


#define SW1 	(1<<19)	// PA19
#define SW2 	(1<<20)	// PA20
#define SW3 	(1<<15)	// PA15
#define SW4 	(1<<14)	// PA14

#endif /* Board_h */

