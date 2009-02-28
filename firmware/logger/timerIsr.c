#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "Board.h"
#include "task.h"
#include "loggerHardware.h"
#include "loggerPinDefs.h"
#include "lib_AT91SAM7S256.h"

extern unsigned int g_timer0_overflow;
extern unsigned int g_timer1_overflow;
extern unsigned int g_timer2_overflow;

/* The ISR can cause a context switch so is declared naked. */
void timer0_irq_handler( void ) __attribute__ ((naked));
void timer0_irq_handler( void ){

	portENTER_SWITCHING_ISR();
	portCHAR xTaskWoken = pdFALSE;

	AT91PS_TC TC_pt = AT91C_BASE_TC0;

	unsigned int sr = TC_pt->TC_SR;
	
	if (sr & AT91C_TC_LDRBS){
 		g_timer0_overflow = 0;
 		TC_pt->TC_IDR = AT91C_TC_LDRBS;
 		TC_pt->TC_IER = AT91C_TC_COVFS;
 	}
 	
 	if ( sr & AT91C_TC_COVFS ){
 		g_timer0_overflow = 1;
		TC_pt->TC_IER = AT91C_TC_LDRBS;
		TC_pt->TC_IDR = AT91C_TC_COVFS;
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

	unsigned int sr = TC_pt->TC_SR;
	
	if (sr & AT91C_TC_LDRBS){
 		g_timer1_overflow = 0;
 		TC_pt->TC_IDR = AT91C_TC_LDRBS;
 		TC_pt->TC_IER = AT91C_TC_COVFS;
 	}
 	
 	if ( sr & AT91C_TC_COVFS ){
 		g_timer1_overflow = 1;
		TC_pt->TC_IER = AT91C_TC_LDRBS;
		TC_pt->TC_IDR = AT91C_TC_COVFS;
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

	unsigned int sr = TC_pt->TC_SR;
	
	if (sr & AT91C_TC_LDRBS){
 		g_timer2_overflow = 0;
 		TC_pt->TC_IDR = AT91C_TC_LDRBS;
 		TC_pt->TC_IER = AT91C_TC_COVFS;
 	}
 	
 	if ( sr & AT91C_TC_COVFS ){
 		g_timer2_overflow = 1;
		TC_pt->TC_IER = AT91C_TC_LDRBS;
		TC_pt->TC_IDR = AT91C_TC_COVFS;
 	} 

	/* Clear AIC to complete ISR processing */
	AT91F_AIC_AcknowledgeIt (AT91C_BASE_AIC);
	portEXIT_SWITCHING_ISR( xTaskWoken );
}
