#include "loggerHardware.h"
#include "loggerConfig.h"
#include "imu.h"
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
#include "usb_comm.h"
#include "usart.h"

void InitLoggerHardware(){

	init_spi_lock();
	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
//	usb_comm_init();
	usart_init();
	init_serial();
	LED_init();
//	imu_init(loggerConfig);
	ADC_init(loggerConfig);
//	timer_init(loggerConfig);
	PWM_init(loggerConfig);
	GPIO_init(loggerConfig);
	InitFSHardware();
//	CAN_init(CAN_BAUD_500K);
}
