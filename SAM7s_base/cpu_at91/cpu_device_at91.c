#include "cpu_device.h"
#include "board.h"
#include "spi.h"

int cpu_device_init(void)
{
    // When using the JTAG debugger the hardware is not always initialised to
    // the correct default state.  This line just ensures that this does not
    // cause all interrupts to be masked at the start.
    AT91C_BASE_AIC->AIC_EOICR = 0;

    // Enable reset-button
    AT91F_RSTSetMode( AT91C_BASE_RSTC , AT91C_RSTC_URSTEN );
    init_spi_lock();
    return 1;
}

void cpu_device_reset(int bootloader)
{
    /* We don't support this on the Atmel part*/
    (void)bootloader;

    AT91F_RSTSoftReset(AT91C_BASE_RSTC, AT91C_RSTC_PROCRST|
                       AT91C_RSTC_PERRST|AT91C_RSTC_EXTRST);
}

const char * cpu_device_get_serialnumber(void)
{
    return "";
}
