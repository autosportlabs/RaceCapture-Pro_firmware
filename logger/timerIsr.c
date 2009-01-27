#include "FreeRTOS.h"
#include "Board.h"



/* The ISR can cause a context switch so is declared naked. */
void timer0_irq_handler( void ) __attribute__ ((naked));
void timer0_irq_handler( void ){

}

/* The ISR can cause a context switch so is declared naked. */
void timer1_irq_handler( void ) __attribute__ ((naked));
void timer1_irq_handler( void ){

}

/* The ISR can cause a context switch so is declared naked. */
void timer2_irq_handler( void ) __attribute__ ((naked));
void timer2_irq_handler( void ){

}
