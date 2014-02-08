#include "loggerHardware.h"
#include "loggerConfig.h"
#include "board.h"
#include "accelerometer.h"
#include "ADC.h"
#include "timer.h"
#include "CAN.h"
#include "PWM.h"
#include "LED.h"
#include "GPIO.h"
#include "sdcard.h"
#include "constants.h"
#include "memory.h"
#include "spi.h"
#include "printk.h"

void InitLoggerHardware(){
	init_spi_lock();
	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	InitWatchdog(WATCHDOG_TIMEOUT_MS);
	LED_init();
	accelerometer_init();
	ADC_init();
	timer_init(loggerConfig);
	PWM_init(loggerConfig);
	GPIO_init(loggerConfig);
	InitFSHardware();
	CAN_init(CAN_BAUD_500K);
}

void ResetWatchdog(){
	AT91F_WDTRestart(AT91C_BASE_WDTC);
}

void InitWatchdog(int timeoutMs){
	 int counter= AT91F_WDTGetPeriod(timeoutMs);
	 AT91F_WDTSetMode(AT91C_BASE_WDTC, AT91C_WDTC_WDRSTEN | AT91C_WDTC_WDRPROC | counter | (counter << 16));
	 AT91F_WDTC_CfgPMC();
}

int flashLoggerConfig(){
	void * savedLoggerConfig = getSavedLoggerConfig();
	void * workingLoggerConfig = getWorkingLoggerConfig();

	return flashWriteRegion(savedLoggerConfig, workingLoggerConfig, sizeof (LoggerConfig));
}
