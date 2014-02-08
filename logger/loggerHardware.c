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
#include "watchdog.h"
#include "sdcard.h"
#include "constants.h"
#include "memory.h"
#include "spi.h"
#include "printk.h"

void InitLoggerHardware(){
	init_spi_lock();
	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	watchdog_init(WATCHDOG_TIMEOUT_MS);
	LED_init();
	accelerometer_init();
	ADC_init();
	timer_init(loggerConfig);
	PWM_init(loggerConfig);
	GPIO_init(loggerConfig);
	InitFSHardware();
	CAN_init(CAN_BAUD_500K);
}

int flashLoggerConfig(){
	void * savedLoggerConfig = getSavedLoggerConfig();
	void * workingLoggerConfig = getWorkingLoggerConfig();

	return flashWriteRegion(savedLoggerConfig, workingLoggerConfig, sizeof (LoggerConfig));
}
