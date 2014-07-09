#include "watchdog_device.h"
#include "board.h"
#include "LED.h"

inline void watchdog_device_reset(){
	AT91F_WDTRestart(AT91C_BASE_WDTC);
}

void watchdog_device_init(int timeoutMs){
	 int counter= AT91F_WDTGetPeriod(timeoutMs);
	 AT91F_WDTSetMode(AT91C_BASE_WDTC, AT91C_WDTC_WDRSTEN | AT91C_WDTC_WDRPROC | counter | (counter << 16));
	 AT91F_WDTC_CfgPMC();
}

static int get_reset_status(){
	int reset_status = AT91F_RSTGetStatus(AT91C_BASE_RSTC);
	reset_status &= AT91C_RSTC_RSTTYP;
	return reset_status;

}

int watchdog_device_is_watchdog_reset(){
	int reset_status = get_reset_status();
	return (reset_status == AT91C_RSTC_RSTTYP_WATCHDOG)? 1 : 0;
}

int watchdog_device_is_poweron_reset(){
	int reset_status = get_reset_status();
	return (reset_status == AT91C_RSTC_RSTTYP_POWERUP)? 1 : 0;
}
