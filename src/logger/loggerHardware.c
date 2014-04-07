#include "loggerHardware.h"
#include "loggerConfig.h"
#include "accelerometer.h"
#include "ADC.h"
#include "timer.h"
#include "CAN.h"
#include "PWM.h"
#include "LED.h"
#include "GPIO.h"
#include "watchdog.h"
#include "sdcard.h"
#include "memory.h"
#include "spi.h"
#include "memory.h"
#include "constants.h"
#include "virtual_channel.h"

void InitLoggerHardware(){
	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	init_spi_lock();
	LED_init();
	imu_init();
	ADC_init();
	timer_init(loggerConfig);
	PWM_init(loggerConfig);
	GPIO_init(loggerConfig);
	InitFSHardware();
	CAN_init(CAN_BAUD_500K);
}
