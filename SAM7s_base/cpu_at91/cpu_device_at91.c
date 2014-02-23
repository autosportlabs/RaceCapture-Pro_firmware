#include "cpu_device.h"
#include "board.h"

int cpu_device_init(void){
	// When using the JTAG debugger the hardware is not always initialised to
	// the correct default state.  This line just ensures that this does not
	// cause all interrupts to be masked at the start.
	AT91C_BASE_AIC->AIC_EOICR = 0;

	// Enable reset-button
	AT91F_RSTSetMode( AT91C_BASE_RSTC , AT91C_RSTC_URSTEN );
	return 1;
}
