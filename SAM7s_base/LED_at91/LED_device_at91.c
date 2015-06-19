#include "LED_device.h"
#include "board.h"

//define PIOs for status LEDs
#define LED1 			(1 << 1)
#define LED2 			(1 << 2)
#define LED3			(1 << 31)
#define LED_MASK        (LED1 | LED2 | LED3)

int LED_device_init(void)
{
    AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, LED_MASK ) ;
    //* Clear the LED's.
    AT91F_PIO_SetOutput( AT91C_BASE_PIOA, LED_MASK ) ;
    return 1;
}

static inline unsigned int get_LED_port(unsigned int Led)
{
    unsigned int LED_port = 0;
    switch (Led) {
    case 1:
        LED_port = LED1;
        break;
    case 2:
        LED_port = LED2;
        break;
    case 3:
        LED_port = LED3;
        break;
    default:
        break;
    }
    return LED_port;
}

void LED_device_enable(unsigned int Led)
{
    AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, get_LED_port(Led));
}

void LED_device_disable(unsigned int Led)
{
    AT91F_PIO_SetOutput( AT91C_BASE_PIOA, get_LED_port(Led) );
}

void LED_device_toggle(unsigned int Led)
{
    unsigned int LED_port = get_LED_port(Led);
    if ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & LED_port ) == LED_port ) {
        AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, LED_port );
    } else {
        AT91F_PIO_SetOutput( AT91C_BASE_PIOA, LED_port );
    }
}
