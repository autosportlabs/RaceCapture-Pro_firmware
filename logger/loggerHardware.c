#include "loggerHardware.h"
#include "loggerConfig.h"
#include "board.h"
#include "accelerometer.h"
#include "ADC.h"
#include "timer.h"
#include "CAN.h"
#include "PWM.h"
#include "sdcard.h"
#include "constants.h"
#include "memory.h"
#include "spi.h"
#include "printk.h"

static unsigned int GetGPIOBits(void){
	return AT91F_PIO_GetInput(AT91C_BASE_PIOA);
}

static void ClearGPIOBits(unsigned int portBits){
	AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, portBits );
}

static void SetGPIOBits(unsigned int portBits){
	AT91F_PIO_SetOutput( AT91C_BASE_PIOA, portBits );
}

void InitLoggerHardware(){
	init_spi_lock();
	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	InitWatchdog(WATCHDOG_TIMEOUT_MS);
	accelerometer_init();
	ADC_init();
	timer_init(loggerConfig);
	PWM_init(loggerConfig);
	InitGPIO(loggerConfig);
	InitLEDs();
	InitPushbutton();
	InitSDCard();
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

void InitGPIO(LoggerConfig *loggerConfig){


//	AT91F_PIO_CfgInput(AT91C_BASE_PIOA, GPIO_MASK);
//	AT91C_BASE_PIOA->PIO_PPUDR = GPIO_MASK; //disable pullup
//	AT91C_BASE_PIOA->PIO_IFER = GPIO_MASK; //enable input filter
//	AT91C_BASE_PIOA->PIO_MDER = GPIO_MASK; //enable multi drain

	GPIOConfig * gpios = loggerConfig->GPIOConfigs;
	if (gpios[0].mode == CONFIG_GPIO_IN){
		AT91F_PIO_CfgInput(AT91C_BASE_PIOA, GPIO_1);
		AT91C_BASE_PIOA->PIO_PPUDR = GPIO_1; //disable pullup
		AT91C_BASE_PIOA->PIO_IFER = GPIO_1; //enable input filter
	}
	else{
	    AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, GPIO_1 );
	    AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, GPIO_1 );
	}
	if (gpios[1].mode == CONFIG_GPIO_IN){
		AT91F_PIO_CfgInput(AT91C_BASE_PIOA, GPIO_2);
		AT91C_BASE_PIOA->PIO_PPUDR = GPIO_2; //disable pullup
		AT91C_BASE_PIOA->PIO_IFER = GPIO_2; //enable input filter
	}
	else{
	    AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, GPIO_2 );
	    AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, GPIO_2 );
	}
	if (gpios[2].mode == CONFIG_GPIO_IN){
		AT91F_PIO_CfgInput(AT91C_BASE_PIOA, GPIO_3);
		AT91C_BASE_PIOA->PIO_PPUDR = GPIO_3; //disable pullup
		AT91C_BASE_PIOA->PIO_IFER = GPIO_3; //enable input filter
	}
	else{
	    AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, GPIO_3 );
	    AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, GPIO_3 );
	}
}

void InitSDCard(void){
	AT91F_PIO_CfgInput(AT91C_BASE_PIOA, SD_CARD_DETECT | SD_WRITE_PROTECT);
	AT91C_BASE_PIOA->PIO_PPUER = SD_CARD_DETECT | SD_WRITE_PROTECT; //enable pullup
	AT91C_BASE_PIOA->PIO_IFER = SD_CARD_DETECT | SD_WRITE_PROTECT; //enable input filter
	InitFSHardware();
}

int isCardPresent(void){
	return (GetGPIOBits() & SD_CARD_DETECT) == 0;
}

int isCardWritable(void){
	return (GetGPIOBits() & SD_WRITE_PROTECT) == 0;
}

int isButtonPressed(void){
	return (GetGPIOBits() & PIO_PUSHBUTTON_SWITCH) != 0;
}

void setGpio(unsigned int channel, unsigned int state){
	unsigned int gpioBits = 0;
	switch (channel){
		case 0:
			gpioBits = GPIO_1;
			break;
		case 1:
			gpioBits = GPIO_2;
			break;
		case 2:
			gpioBits = GPIO_3;
			break;
	}
	if (state){
		SetGPIOBits(gpioBits);
	} else{
		ClearGPIOBits(gpioBits);
	}
}

int readGpio(unsigned int channel){
	unsigned int gpioStates = AT91F_PIO_GetInput(AT91C_BASE_PIOA);
	int value = 0;
	switch (channel){
		case 0:
			value = ((gpioStates & GPIO_1) != 0);
			break;
		case 1:
			value = ((gpioStates & GPIO_2) != 0);
			break;
		case 2:
			value = ((gpioStates & GPIO_3) != 0);
			break;
		default:
			break;
	}
	return value;
}

void readGpios(unsigned int *gpio1, unsigned int *gpio2, unsigned int *gpio3){
	unsigned int gpioStates = AT91F_PIO_GetInput(AT91C_BASE_PIOA);
	*gpio1 = ((gpioStates & GPIO_1) != 0);
	*gpio2 = ((gpioStates & GPIO_2) != 0);
	*gpio3 = ((gpioStates & GPIO_3) != 0);
}

void InitLEDs(void){
    AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, LED_MASK ) ;
   //* Clear the LED's.
    AT91F_PIO_SetOutput( AT91C_BASE_PIOA, LED_MASK ) ;
}

void InitPushbutton(void){
	AT91F_PIO_CfgInput(AT91C_BASE_PIOA, PIO_PUSHBUTTON_SWITCH);
	AT91C_BASE_PIOA->PIO_PPUER = PIO_PUSHBUTTON_SWITCH; //enable pullup
	AT91C_BASE_PIOA->PIO_IFER = PIO_PUSHBUTTON_SWITCH; //enable input filter
	AT91C_BASE_PIOA->PIO_MDER = PIO_PUSHBUTTON_SWITCH; //enable multi drain
}

void enableLED(unsigned int Led){
        AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, Led );
}

void disableLED(unsigned int Led){
        AT91F_PIO_SetOutput( AT91C_BASE_PIOA, Led );
}

void toggleLED (unsigned int Led){
    if ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & Led ) == Led )
    {
        AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, Led );
    }
    else
    {
        AT91F_PIO_SetOutput( AT91C_BASE_PIOA, Led );
    }
}


int flashLoggerConfig(){
	void * savedLoggerConfig = getSavedLoggerConfig();
	void * workingLoggerConfig = getWorkingLoggerConfig();

	return flashWriteRegion(savedLoggerConfig, workingLoggerConfig, sizeof (LoggerConfig));
}
