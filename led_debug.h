#ifndef LED_DEBUG_H_
#define LED_DEBUG_H_

#include "Board.h"


#define LED1 (1<<1)
#define LED2 (1<<2)

#define LED_MASK        (LED1|LED2)


void Set_LED(unsigned int Led);
void Clear_LED(unsigned int Led);
void Init_LEDs();
void Toggle_LED (unsigned int Led);


#endif /*LED_DEBUG_H_*/
