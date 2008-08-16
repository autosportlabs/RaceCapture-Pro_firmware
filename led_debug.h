#ifndef LED_DEBUG_H_
#define LED_DEBUG_H_

#include "Board.h"


#define LED1            (1<<17)	/* PA0 / PGMEN0 & PWM0 TIOA0  48 */
#define LED2            (1<<18)	/* PA1 / PGMEN1 & PWM1 TIOB0  47 */
#define LED3            (1<<2)	/* PA2          & PWM2 SCK0   44 */
#define LED4            (1<<3)	/* PA3          & TWD  NPCS3  43 */
#define NB_LEB			4

#define LED_MASK        (LED1|LED2|LED3|LED4)


void Set_LED(unsigned int Led);
void Clear_LED(unsigned int Led);
void Init_LEDs();
void Toggle_LED (unsigned int Led);


#endif /*LED_DEBUG_H_*/
