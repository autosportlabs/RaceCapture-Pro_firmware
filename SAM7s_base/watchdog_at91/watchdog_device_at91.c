#include "watchdog_device.h"
#include "board.h"

inline void watchdog_device_reset(){
	AT91F_WDTRestart(AT91C_BASE_WDTC);
}

void watchdog_device_init(int timeoutMs){
	 int counter= AT91F_WDTGetPeriod(timeoutMs);
	 AT91F_WDTSetMode(AT91C_BASE_WDTC, AT91C_WDTC_WDRSTEN | AT91C_WDTC_WDRPROC | counter | (counter << 16));
	 AT91F_WDTC_CfgPMC();
}
