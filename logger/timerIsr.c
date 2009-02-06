#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "Board.h"
#include "task.h"
#include "loggerHardware.h"

extern unsigned int g_timer0Period;
extern unsigned int g_timer1Period;
extern unsigned int g_timer2Period;

/* The ISR can cause a context switch so is declared naked. */
void timer0_irq_handler( void ) __attribute__ ((naked));
void timer0_irq_handler( void ){


	portENTER_SWITCHING_ISR();
	portCHAR xTaskWoken = pdFALSE;

	AT91PS_TC TC_pt = AT91C_BASE_TC0;
 	if ( TC_pt->TC_SR & AT91C_TC_COVFS ){
 		g_timer0Period = 0;
 	} else{
		g_timer0Period = TC_pt->TC_RB;
 	}
	/* Clear AIC to complete ISR processing */
	AT91F_AIC_AcknowledgeIt (AT91C_BASE_AIC);
	portEXIT_SWITCHING_ISR( xTaskWoken );
}

/* The ISR can cause a context switch so is declared naked. */
void timer1_irq_handler( void ) __attribute__ ((naked));
void timer1_irq_handler( void ){

	portENTER_SWITCHING_ISR();
	portCHAR xTaskWoken = pdFALSE;

	AT91PS_TC TC_pt = AT91C_BASE_TC1;
 	if ( TC_pt->TC_SR & AT91C_TC_COVFS ){
 		g_timer1Period = 0;
 	} else{
		g_timer1Period = TC_pt->TC_RB;
 	}
	/* Clear AIC to complete ISR processing */
	AT91F_AIC_AcknowledgeIt (AT91C_BASE_AIC);
	portEXIT_SWITCHING_ISR( xTaskWoken );
}

/* The ISR can cause a context switch so is declared naked. */
void timer2_irq_handler( void ) __attribute__ ((naked));
void timer2_irq_handler( void ){

	portENTER_SWITCHING_ISR();
	portCHAR xTaskWoken = pdFALSE;


	AT91PS_TC TC_pt = AT91C_BASE_TC2;
/* 	if ( TC_pt->TC_SR & AT91C_TC_COVFS ){
 		g_timer2Period = 0;
 	} else{
		g_timer2Period = TC_pt->TC_RB;
 	}
	*/
	g_timer2Period = TC_pt->TC_RB;
	/* Clear AIC to complete ISR processing */
	AT91F_AIC_AcknowledgeIt (AT91C_BASE_AIC);
	portEXIT_SWITCHING_ISR( xTaskWoken );
}
