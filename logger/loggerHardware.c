#include "loggerHardware.h"
#include "loggerConfig.h"
#include "board.h"
#include "accelerometer.h"
#include "sdcard.h"
#include "constants.h"
#include "memory.h"
#include "spi.h"
#include "CAN.h"
#include "printk.h"

/* ADC field definition for the Mode Register: Reminder
                       TRGEN    => Selection bewteen Software or hardware start of conversion
                       TRGSEL   => Relevant if the previous field set a Hardware Triggering Mode
                       LOWRES   => 10-bit result if ths bit is cleared 0
                       SLEEP    => normal mode if ths is cleared
                       PRESCAL  => ADCclock = MCK / [(PRESCAL + 1)*2]
                       STARTUP  => Startup Time = [(STARTUP + 1)*8] / ADCclock
                       SHTIM    => Tracking time = (SHTIM + 1) / ADCclock
 */
#define   TRGEN    (0x0)    // Software triggering
#define   TRGSEL   (0x0)    // Without effect in Software triggering
#define   LOWRES   (0x0)    // 10-bit result output format
#define   SLEEP    (0x0)    // Normal Mode (instead of SLEEP Mode)
#define   PRESCAL  (0x4)    // Max value
#define   STARTUP  (0xc)    // This time period must be higher than 20 �s
#define   SHTIM    (0x3)    // Must be higher than 3 ADC clock cycles but depends on output
                            // impedance of the analog driver to the ADC input
/* Channel selection */
#define   CHANNEL  (0)      // Write the targeted channel (Notation: the first channel is 0
                            // and the last is 7)

//Timer/Counter declarations
#define TIMER0_INTERRUPT_LEVEL 	5
#define TIMER1_INTERRUPT_LEVEL 	5
#define TIMER2_INTERRUPT_LEVEL 	5

#define MAX_TIMER_VALUE			0xFFFF

extern void ( timer0_irq_handler )( void );
extern void ( timer1_irq_handler )( void );
extern void ( timer2_irq_handler )( void );
extern void ( timer0_slow_irq_handler )( void );
extern void ( timer1_slow_irq_handler )( void );
extern void ( timer2_slow_irq_handler )( void );

unsigned int g_timer0_overflow;
unsigned int g_timer1_overflow;
unsigned int g_timer2_overflow;
unsigned int g_timer_counts[CONFIG_TIMER_CHANNELS];

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
	InitGPIO(loggerConfig);
	InitADC();
	InitPWM(loggerConfig);
	initTimerChannels(loggerConfig);
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

void SetFREQ_ANALOG(unsigned int freqAnalogPort){
	AT91F_PIO_SetOutput( AT91C_BASE_PIOA, freqAnalogPort );	
}

void ClearFREQ_ANALOG(unsigned int freqAnalogPort){
	AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, freqAnalogPort );
}

///////////////////PWM Functions

static unsigned int MapPwmHardwareChannel(unsigned int softwareChannel){
	switch(softwareChannel){
	case 0:
		return 3;
		break;
	case 1:
		return 0;
		break;
	case 2:
		return 1;
		break;
	case 3:
	default:
		return 2;
		break;
	}
}

//------------------------------------------------------------------------------
/// Finds a prescaler/divisor couple to generate the desired frequency from
/// MCK.
/// Returns the value to enter in PWMC_MR or 0 if the configuration cannot be
/// met.
/// \param frequency  Desired frequency in Hz.
/// \param mck  Master clock frequency in Hz.
//------------------------------------------------------------------------------
static unsigned short PWM_GetClockConfiguration(
    unsigned int frequency,
    unsigned int mck)
{
    const unsigned int divisors[11] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
    unsigned char divisor = 0;
    unsigned int prescaler;

    // Find prescaler and divisor values
    prescaler = (mck / divisors[divisor]) / frequency;
    while ((prescaler > 255) && (divisor < 11)) {

        divisor++;
        prescaler = (mck / divisors[divisor]) / frequency;
    }

    // Return result
    if (divisor < 11) {
        return prescaler | (divisor << 8);
    }
    else {
        return 0;
    }
}

//------------------------------------------------------------------------------
/// Configures PWM clocks A & B to run at the given frequencies. This function
/// finds the best MCK divisor and prescaler values automatically.
/// \param clka  Desired clock A frequency (0 if not used).
/// \param clkb  Desired clock B frequency (0 if not used).
/// \param mck  Master clock frequency.
//------------------------------------------------------------------------------
static void PWM_ConfigureClocks(unsigned int clka, unsigned int clkb, unsigned int mck){

    unsigned int mode = 0;
    unsigned int result;

    // Clock A
    if (clka != 0) {
        result = PWM_GetClockConfiguration(clka, mck);
        mode |= result;
    }

    // Clock B
    if (clkb != 0) {
        result = PWM_GetClockConfiguration(clkb, mck);
        mode |= (result << 16);
    }

    // Configure clocks
    AT91C_BASE_PWMC->PWMC_MR = mode;
}

//------------------------------------------------------------------------------
/// Configures PWM a channel with the given parameters.
/// The PWM controller must have been clocked in the PMC prior to calling this
/// function.
/// \param channel  Channel number.
/// \param prescaler  Channel prescaler.
/// \param alignment  Channel alignment.
/// \param polarity  Channel polarity.
//------------------------------------------------------------------------------
static void PWM_ConfigureChannel(
    unsigned int channel,
    unsigned int prescaler,
    unsigned int alignment,
    unsigned int polarity)
{
	unsigned int hardwareChannel = MapPwmHardwareChannel(channel);
    // Disable channel
    AT91C_BASE_PWMC->PWMC_DIS = 1 << hardwareChannel;
    // Configure channel
    AT91C_BASE_PWMC->PWMC_CH[hardwareChannel].PWMC_CMR = prescaler | alignment | polarity;
}

static void InitPWMChannel(unsigned int channel, PWMConfig *config){
    // Configure PWMC channel (left-aligned)
    PWM_ConfigureChannel(channel, AT91C_PWMC_CPRE_MCKA, 0, AT91C_PWMC_CPOL);
    PWM_SetPeriod(channel, config->startupPeriod);
    PWM_SetDutyCycle(channel, config->startupDutyCycle);
    StartPWM(channel);
}

void InitPWM(LoggerConfig *loggerConfig){
	
	//Configure PWM Clock
	PWM_ConfigureClocks(loggerConfig->PWMClockFrequency * MAX_PWM_DUTY_CYCLE, 0, BOARD_MCK);

	//Configure PWM ports
	/////////////////////////////////////////
	//PWM0
	///////////////////////////////////////// 
 	//Configure Peripherials for PWM outputs
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			0, 				// mux function A
			AT91C_PIO_PA23); // mux funtion B

	/////////////////////////////////////////
	//PWM1
	///////////////////////////////////////// 
 	//Configure Peripherials for PWM outputs
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			0, 				// mux function A
			AT91C_PIO_PA24); // mux funtion B
	
	/////////////////////////////////////////
	//PWM2
	///////////////////////////////////////// 
 	//Configure Peripherials for PWM outputs
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			0, 				// mux function A
			AT91C_PIO_PA25); // mux funtion B

	/////////////////////////////////////////
	//PWM3
	///////////////////////////////////////// 
 	//Configure Peripherials for PWM outputs
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			0, 				// mux function A
			AT91C_PIO_PA7); // mux funtion B

	InitPWMChannel(0,&(loggerConfig->PWMConfigs[0]));
	InitPWMChannel(1,&(loggerConfig->PWMConfigs[1]));
	InitPWMChannel(2,&(loggerConfig->PWMConfigs[2]));
	InitPWMChannel(3,&(loggerConfig->PWMConfigs[3]));
	StartAllPWM();
}

void StartAllPWM(){
	AT91F_PWMC_StartChannel(AT91C_BASE_PWMC,(1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
}

void StopAllPWM(){
	AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,(1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
}

void StartPWM(unsigned int channel){
	AT91F_PWMC_StartChannel(AT91C_BASE_PWMC,1 << channel);
}

void StopPWM(unsigned int channel){
	if (channel <= 3) AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,1 << channel);
}

//------------------------------------------------------------------------------
/// Sets the period value used by a PWM channel. This function writes directly
/// to the CPRD register if the channel is disabled; otherwise, it uses the
/// update register CUPD.
/// \param channel  Channel number.
/// \param period  Period value.
//------------------------------------------------------------------------------
void PWM_SetPeriod(unsigned int channel, unsigned short period)
{
	unsigned int hardwareChannel = MapPwmHardwareChannel(channel);
    // If channel is disabled, write to CPRD
    if ((AT91C_BASE_PWMC->PWMC_SR & (1 << hardwareChannel)) == 0) {
        AT91C_BASE_PWMC->PWMC_CH[hardwareChannel].PWMC_CPRDR = period;
    }
    // Otherwise use update register
    else {
        AT91C_BASE_PWMC->PWMC_CH[hardwareChannel].PWMC_CMR |= AT91C_PWMC_CPD;
        AT91C_BASE_PWMC->PWMC_CH[hardwareChannel].PWMC_CUPDR = period;
    }
}

unsigned short PWM_GetPeriod(unsigned int channel){
	unsigned int hardwareChannel = MapPwmHardwareChannel(channel);
	return AT91C_BASE_PWMC->PWMC_CH[hardwareChannel].PWMC_CPRDR;
}

//------------------------------------------------------------------------------
/// Sets the duty cycle used by a PWM channel. This function writes directly to
/// the CDTY register if the channel is disabled; otherwise it uses the
/// update register CUPD.
/// Note that the duty cycle must always be inferior or equal to the channel
/// period.
/// \param channel  Channel number.
/// \param duty  Duty cycle value.
//------------------------------------------------------------------------------
void PWM_SetDutyCycle(unsigned int channel, unsigned short duty){

	unsigned int hardwareChannel = MapPwmHardwareChannel(channel);
	//duty cycle of zero freaks out the PWM controller
	//seems to invert the polarity. fix this up until we understand better.
	if (duty < 1 ) duty = 1;
	if (duty > 100) duty = 100;

    // If channel is disabled, write to CDTY
    if ((AT91C_BASE_PWMC->PWMC_SR & (1 << hardwareChannel)) == 0) {
        AT91C_BASE_PWMC->PWMC_CH[hardwareChannel].PWMC_CDTYR = duty;
    }
    // Otherwise use update register
    else {
        AT91C_BASE_PWMC->PWMC_CH[hardwareChannel].PWMC_CMR &= ~AT91C_PWMC_CPD;
        AT91C_BASE_PWMC->PWMC_CH[hardwareChannel].PWMC_CUPDR = duty;
    }
}

unsigned short PWM_GetDutyCycle(unsigned int channel){
	unsigned int hardwareChannel = MapPwmHardwareChannel(channel);
	return AT91C_BASE_PWMC->PWMC_CH[hardwareChannel].PWMC_CDTYR;
}


void InitADC(void){

       /* Clear all previous setting and result */
       AT91F_ADC_SoftReset (AT91C_BASE_ADC);
       
       /* First Step: Set up by using ADC Mode register */
       AT91F_ADC_CfgModeReg (AT91C_BASE_ADC,
                           (SHTIM << 24) | (STARTUP << 16) | (PRESCAL << 8) | 
                            (SLEEP << 5) | (LOWRES <<4) | (TRGSEL << 1) | (TRGEN )) ;

        /* Second Step: Select the active channel */
        AT91F_ADC_EnableChannel (AT91C_BASE_ADC, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7)); 
}


void readAllADC(	unsigned int *a0, 
						unsigned int *a1, 
						unsigned int *a2,
						unsigned int *a3,
						unsigned int *a4,
						unsigned int *a5,
						unsigned int *a6,
						unsigned int *a7 ){
	
        /* Third Step: Start the conversion */
        AT91F_ADC_StartConversion (AT91C_BASE_ADC);
        
        /* Fourth Step: Waiting Stop Of conversion by polling */
        while (!((AT91F_ADC_GetStatus (AT91C_BASE_ADC)) & (1<<7)));
        
        *a0 = AT91F_ADC_GetConvertedDataCH0 (AT91C_BASE_ADC);
        *a1 = AT91F_ADC_GetConvertedDataCH1 (AT91C_BASE_ADC);
        *a2 = AT91F_ADC_GetConvertedDataCH2 (AT91C_BASE_ADC);
        *a3 = AT91F_ADC_GetConvertedDataCH3 (AT91C_BASE_ADC);
        *a4 = AT91F_ADC_GetConvertedDataCH4 (AT91C_BASE_ADC);
        *a5 = AT91F_ADC_GetConvertedDataCH5 (AT91C_BASE_ADC);
        *a6 = AT91F_ADC_GetConvertedDataCH6 (AT91C_BASE_ADC);
        *a7 = AT91F_ADC_GetConvertedDataCH7 (AT91C_BASE_ADC);
                                                
}

unsigned int readADC(unsigned int channel){
	
       /* Clear all previous setting and result */
    //   AT91F_ADC_SoftReset (AT91C_BASE_ADC);
       
       /* First Step: Set up by using ADC Mode register */
     //  AT91F_ADC_CfgModeReg (AT91C_BASE_ADC,
       //                     (SHTIM << 24) | (STARTUP << 16) | (PRESCAL << 8) | 
         //                   (SLEEP << 5) | (LOWRES <<4) | (TRGSEL << 1) | (TRGEN )) ;

        /* Second Step: Select the active channel */
 //       AT91F_ADC_EnableChannel (AT91C_BASE_ADC, (1<<channel)); 
        
        /* Third Step: Start the conversion */
        AT91F_ADC_StartConversion (AT91C_BASE_ADC);
        
        /* Fourth Step: Waiting Stop Of conversion by pulling */
        while (!((AT91F_ADC_GetStatus (AT91C_BASE_ADC)) & (1<<channel)));

        unsigned int result = 0;
        
        switch (channel){
        case 0:
        	result = AT91F_ADC_GetConvertedDataCH0 (AT91C_BASE_ADC);
        	break;
        case 1:
        	result = AT91F_ADC_GetConvertedDataCH1 (AT91C_BASE_ADC);
        	break;
        case 2:
        	result = AT91F_ADC_GetConvertedDataCH2 (AT91C_BASE_ADC);
        	break;
        case 3:
        	result = AT91F_ADC_GetConvertedDataCH3 (AT91C_BASE_ADC);
        	break;
        case 4:
        	result = AT91F_ADC_GetConvertedDataCH4 (AT91C_BASE_ADC);
        	break;
        case 5:
        	result = AT91F_ADC_GetConvertedDataCH5 (AT91C_BASE_ADC);
        	break;
        case 6:
        	result = AT91F_ADC_GetConvertedDataCH6 (AT91C_BASE_ADC);
        	break;
        case 7:
        	result = AT91F_ADC_GetConvertedDataCH7 (AT91C_BASE_ADC);
        	break;
        default:
        	break;
        }
	return result;
}

unsigned int ReadADCx(unsigned int channel){
	
    unsigned int result ;

    AT91F_ADC_EnableChannel (AT91C_BASE_ADC, (1<<channel)); 
    /* Third Step: Start the conversion */
    AT91F_ADC_StartConversion (AT91C_BASE_ADC);
    
    //Poll for result
    while (!((AT91F_ADC_GetStatus (AT91C_BASE_ADC)) & (1<<channel)));
    
   	result = AT91F_ADC_GetLastConvertedData(AT91C_BASE_ADC);
   	AT91F_ADC_DisableChannel(AT91C_BASE_ADC, (1<<channel));
       	
	return result;	
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

void initTimerChannels(LoggerConfig *loggerConfig){
	initTimer0(&(loggerConfig->TimerConfigs[0]));
	initTimer1(&(loggerConfig->TimerConfigs[1]));
	initTimer2(&(loggerConfig->TimerConfigs[2]));
}

unsigned int timerClockFromDivider(unsigned short divider){

	switch (divider){
		case 2:
			return AT91C_TC_CLKS_TIMER_DIV1_CLOCK;
		case 8:
			return AT91C_TC_CLKS_TIMER_DIV2_CLOCK;
		case 32:
			return AT91C_TC_CLKS_TIMER_DIV3_CLOCK;
		case 128:
			return AT91C_TC_CLKS_TIMER_DIV4_CLOCK;
		case 1024:
			return AT91C_TC_CLKS_TIMER_DIV5_CLOCK;
		default:
			return AT91C_TC_CLKS_TIMER_DIV5_CLOCK;
	}	
}

void initTimer0(TimerConfig *timerConfig){

	g_timer0_overflow = 1;
	g_timer_counts[0] = 0;

	// Set PIO pins for Timer Counter 0
	//TIOA0 connected to PA0
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA,0,AT91C_PA0_TIOA0);
	
	 //enable pullup (optoisolator input)
	AT91C_BASE_PIOA->PIO_PPUDR = AT91C_PA0_TIOA0; //disable pullup
	AT91C_BASE_PIOA->PIO_IFER = AT91C_PA0_TIOA0; //enable input filter

	/// Enable TC0's clock in the PMC controller
	AT91F_TC0_CfgPMC();
	
	AT91F_TC_Open ( 
	AT91C_BASE_TC0,
	
	timerClockFromDivider(timerConfig->timerDivider) | 
	AT91C_TC_ETRGEDG_FALLING |	
	AT91C_TC_ABETRG |
	AT91C_TC_LDRA_RISING|
	AT91C_TC_LDRB_FALLING
	
	,AT91C_ID_TC0
	);
	
	if (timerConfig->slowTimerEnabled){
		AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC0, TIMER0_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, timer0_slow_irq_handler);
		AT91C_BASE_TC0->TC_IER = AT91C_TC_LDRBS | AT91C_TC_COVFS;  // IRQ enable RB loading and overflow
	}
	else{
		AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC0, TIMER0_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, timer0_irq_handler);
		AT91C_BASE_TC0->TC_IER = AT91C_TC_COVFS;  // IRQ enable overflow
	}
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC0);	
}

void initTimer1(TimerConfig *timerConfig){

	g_timer1_overflow = 1;
	g_timer_counts[1] = 0;
	
	// Set PIO pins for Timer Counter 1
	// TIOA1 mapped to PA15
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA,0,AT91C_PA15_TIOA1);
	 
	AT91C_BASE_PIOA->PIO_PPUDR = AT91C_PA15_TIOA1; //disable pullup
	AT91C_BASE_PIOA->PIO_IFER = AT91C_PA15_TIOA1; //enable input filter

	// Enable TC0's clock in the PMC controller
	AT91F_TC1_CfgPMC();
	
	AT91F_TC_Open ( 
	AT91C_BASE_TC1,
	
	timerClockFromDivider(timerConfig->timerDivider) | 
	AT91C_TC_ETRGEDG_FALLING |	
	AT91C_TC_ABETRG |
	AT91C_TC_LDRA_RISING|
	AT91C_TC_LDRB_FALLING
	
	,AT91C_ID_TC1
	);
	
	if (timerConfig->slowTimerEnabled){
		AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC1, TIMER1_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, timer1_slow_irq_handler);
		AT91C_BASE_TC1->TC_IER = AT91C_TC_LDRBS | AT91C_TC_COVFS;  // IRQ enable RB loading and overflow
	}
	else{
		AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC1, TIMER1_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, timer1_irq_handler);
		AT91C_BASE_TC1->TC_IER = AT91C_TC_COVFS;  // IRQ enable overflow
	}
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC1);	
}

void initTimer2(TimerConfig *timerConfig){

	g_timer2_overflow = 1;
	g_timer_counts[2] = 0;
	// Set PIO pins for Timer Counter 2
	// TIOA2 mapped to PA26
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA,0,AT91C_PA26_TIOA2);

	AT91C_BASE_PIOA->PIO_PPUDR = AT91C_PA26_TIOA2; //disable pullup
	AT91C_BASE_PIOA->PIO_IFER = AT91C_PA26_TIOA2; //enable input filter

	// Enable TC0's clock in the PMC controller
	AT91F_TC2_CfgPMC();
	
	AT91F_TC_Open ( 
	AT91C_BASE_TC2,
	
	timerClockFromDivider(timerConfig->timerDivider) | 
	AT91C_TC_ETRGEDG_FALLING |	
	AT91C_TC_ABETRG |
	AT91C_TC_LDRA_RISING|
	AT91C_TC_LDRB_FALLING
	
	,AT91C_ID_TC2
	);
	
	if (timerConfig->slowTimerEnabled){
		AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC2, TIMER2_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, timer2_slow_irq_handler);
		AT91C_BASE_TC2->TC_IER =AT91C_TC_LDRBS | AT91C_TC_COVFS;  // IRQ enable RB loading and overflow
	}
	else{
		AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC2, TIMER2_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, timer2_irq_handler);
		AT91C_BASE_TC2->TC_IER = AT91C_TC_COVFS;  // IRQ enable overflow
	}	
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC2);
}

void getAllTimerPeriods(unsigned int *t0, unsigned int *t1, unsigned int *t2){
	*t0 = getTimer0Period();
	*t1 = getTimer1Period();
	*t2 = getTimer2Period();	
}

void resetTimerCount(unsigned int channel){
	if (channel >= 0 && channel < CONFIG_TIMER_CHANNELS){
		g_timer_counts[channel] = 0;			
	}
}

unsigned int getTimerCount(unsigned int channel){
	if (channel >= 0 && channel < CONFIG_TIMER_CHANNELS){
		return g_timer_counts[channel];			
	}
	else{
		return 0;
	}
}

unsigned int getTimerPeriod(unsigned int channel){
	switch (channel){
		case 0:
			return getTimer0Period();
		case 1:
			return getTimer1Period();
		case 2:
			return getTimer2Period();
	}
	return 0;
}

unsigned int getTimer0Period(){
	return g_timer0_overflow ? 0 : AT91C_BASE_TC0->TC_RB;
}

unsigned int getTimer1Period(){
	return g_timer1_overflow ? 0 : AT91C_BASE_TC1->TC_RB;
}

unsigned int getTimer2Period(){
	return g_timer2_overflow ? 0 : AT91C_BASE_TC2->TC_RB;
}

int flashLoggerConfig(){
	void * savedLoggerConfig = getSavedLoggerConfig();
	void * workingLoggerConfig = getWorkingLoggerConfig();

	return flashWriteRegion(savedLoggerConfig, workingLoggerConfig, sizeof (LoggerConfig));
}
