#include "loggerHardware.h"
#include "loggerConfig.h"

#include "LED.h"
#include "watchdog.h"
#include "memory.h"
#include "constants.h"
#include "usart.h"

#if VIRTUAL_CHANNEL_SUPPORT == 1
#include "virtual_channel.h"
#endif

/* SD card support */
#if SDCARD_SUPPORT == 1
#include "sdcard.h"
#endif


#if USB_SERIAL_SUPPORT == 1
#include "usb_comm.h"
#endif

/* Sensor support */
#if IMU_CHANNELS > 0
#include "imu.h"
#endif
#if ANALOG_CHANNELS > 0
#include "ADC.h"
#endif
#if TIMER_CHANNELS > 0
#include "timer.h"
#endif
#if GPIO_CHANNELS > 0
#include "GPIO.h"
#endif
#if PWM_CHANNELS > 0
#include "PWM.h"
#endif
#if CAN_CHANNELS > 0
#include "CAN.h"
#endif
void InitLoggerHardware()
{

    LoggerConfig *loggerConfig = getWorkingLoggerConfig();
    usart_init();
    init_serial();
    LED_init();

#if IMU_CHANNELS > 0
    //imu_init(loggerConfig);
#endif
#if ANALOG_CHANNELS > 0
    ADC_init(loggerConfig);
#endif
#if PWM_CHANNELS > 0
    PWM_init(loggerConfig);
#endif
#if GPIO_CHANNELS > 0
    GPIO_init(loggerConfig);
#endif
#if TIMER_CHANNELS > 0
    timer_init(loggerConfig);
#endif
#if CAN_CHANNELS > 0
    CAN_init(loggerConfig);
#endif
#if SD_CARD_SUPPORT == 1
    InitFSHardware();
#endif

}
