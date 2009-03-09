#include "loggerHardware.h"
#include "board.h"
#include "lib_AT91SAM7S256.h"


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
#define   STARTUP  (0xc)    // This time period must be higher than 20 µs
#define   SHTIM    (0x3)    // Must be higher than 3 ADC clock cycles but depends on output
                            // impedance of the analog driver to the ADC input
/* Channel selection */
#define   CHANNEL  (0)      // Write the targeted channel (Notation: the first channel is 0
                            // and the last is 7)


//Timer/Counter declarations
#define TIMER0_INTERRUPT_LEVEL 	5
#define TIMER1_INTERRUPT_LEVEL 	5
#define TIMER2_INTERRUPT_LEVEL 	5

#define TC_CLKS                 0x7
#define TC_CLKS_MCK2            0x0
#define TC_CLKS_MCK8            0x1
#define TC_CLKS_MCK32           0x2
#define TC_CLKS_MCK128          0x3
#define TC_CLKS_MCK1024         0x4

#define MAX_TIMER_VALUE			0xFFFF

extern void ( timer0_irq_handler )( void );
extern void ( timer1_irq_handler )( void );
extern void ( timer2_irq_handler )( void );

unsigned int g_timer0_overflow;
unsigned int g_timer1_overflow;
unsigned int g_timer2_overflow;

void InitGPIO(){
    AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, GPIO_MASK ) ;
}

unsigned int GetGPIOBits(){
	return AT91F_PIO_GetStatus(AT91C_BASE_PIOA);
		
}

void SetGPIOBits(unsigned int portBits){
	AT91F_PIO_SetOutput( AT91C_BASE_PIOA, portBits );
}

void ClearGPIOBits(unsigned int portBits){
	AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, portBits );
}

void SetFREQ_ANALOG(unsigned int freqAnalogPort){
	AT91F_PIO_SetOutput( AT91C_BASE_PIOA, freqAnalogPort );	
}
void ClearFREQ_ANALOG(unsigned int freqAnalogPort){
	AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, freqAnalogPort );
}



void StopPWM(unsigned int pwmChannel){
	if (pwmChannel <= 3){
		AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,1 << 0);	
	}
}

void StopAllPWM(){
	AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,(1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
}

void StartAllPWM(){
	AT91F_PWMC_StartChannel(AT91C_BASE_PWMC,(1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
}

void StartPWM(unsigned int pwmChannel){
	if (pwmChannel <= 3){
		AT91F_PWMC_StartChannel(AT91C_BASE_PWMC,1 << pwmChannel);
	}
}

void EnableAllPWM(){
	EnablePWM0();
	EnablePWM1();
	EnablePWM2();
	EnablePWM3();	
}

void EnablePWM0(){
	/////////////////////////////////////////
	//PWM0
	///////////////////////////////////////// 
 	//Configure Peripherials for PWM outputs
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			0, 				// mux function A
			AT91C_PIO_PA23); // mux funtion B

	//Configure PWM Clock
    PWM_ConfigureClocks(PWM_FREQUENCY * MAX_DUTY_CYCLE, 0, BOARD_MCK);

    // Configure PWMC channel (left-aligned)
    PWM_ConfigureChannel(0, AT91C_PWMC_CPRE_MCKA, 0, AT91C_PWMC_CPOL);
    PWM_SetPeriod(0, MAX_DUTY_CYCLE);
    PWM_SetDutyCycle(0, MIN_DUTY_CYCLE);
    PWM_EnableChannel(0);	
}

void EnablePWM1(){
	
	/////////////////////////////////////////
	//PWM1
	///////////////////////////////////////// 
 	//Configure Peripherials for PWM outputs
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			0, 				// mux function A
			AT91C_PIO_PA24); // mux funtion B

	//Configure PWM Clock
    PWM_ConfigureClocks(PWM_FREQUENCY * MAX_DUTY_CYCLE, 0, BOARD_MCK);

    // Configure PWMC channel (left-aligned)
    PWM_ConfigureChannel(1, AT91C_PWMC_CPRE_MCKA, 0, AT91C_PWMC_CPOL);
    PWM_SetPeriod(1, MAX_DUTY_CYCLE);
    PWM_SetDutyCycle(1, MIN_DUTY_CYCLE);
    PWM_EnableChannel(1);	
}

void EnablePWM2(){
	/////////////////////////////////////////
	//PWM2
	///////////////////////////////////////// 
 	//Configure Peripherials for PWM outputs
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			0, 				// mux function A
			AT91C_PIO_PA25); // mux funtion B

	//Configure PWM Clock
    PWM_ConfigureClocks(PWM_FREQUENCY * MAX_DUTY_CYCLE, 0, BOARD_MCK);

    // Configure PWMC channel (left-aligned)
    PWM_ConfigureChannel(2, AT91C_PWMC_CPRE_MCKA, 0, AT91C_PWMC_CPOL);
    PWM_SetPeriod(2, MAX_DUTY_CYCLE);
    PWM_SetDutyCycle(2, MIN_DUTY_CYCLE);
    PWM_EnableChannel(2);	
}

void EnablePWM3(){
	/////////////////////////////////////////
	//PWM3
	///////////////////////////////////////// 
 	//Configure Peripherials for PWM outputs
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			0, 				// mux function A
			AT91C_PIO_PA7); // mux funtion B

	//Configure PWM Clock
    PWM_ConfigureClocks(PWM_FREQUENCY * MAX_DUTY_CYCLE, 0, BOARD_MCK);

    // Configure PWMC channel (left-aligned)
    PWM_ConfigureChannel(3, AT91C_PWMC_CPRE_MCKA, 0, AT91C_PWMC_CPOL);
    PWM_SetPeriod(3, MAX_DUTY_CYCLE);
    PWM_SetDutyCycle(3, MIN_DUTY_CYCLE);
    PWM_EnableChannel(3);	
}


//------------------------------------------------------------------------------
/// Configures PWM clocks A & B to run at the given frequencies. This function
/// finds the best MCK divisor and prescaler values automatically.
/// \param clka  Desired clock A frequency (0 if not used).
/// \param clkb  Desired clock B frequency (0 if not used).
/// \param mck  Master clock frequency.
//------------------------------------------------------------------------------
void PWM_ConfigureClocks(unsigned int clka, unsigned int clkb, unsigned int mck)
{
    unsigned int mode = 0;
    unsigned int result;

    // Clock A
    if (clka != 0) {

        result = GetClockConfiguration(clka, mck);
        mode |= result;
    }

    // Clock B
    if (clkb != 0) {

        result = GetClockConfiguration(clkb, mck);
        mode |= (result << 16);
    }

    // Configure clocks
    AT91C_BASE_PWMC->PWMC_MR = mode;
}


//------------------------------------------------------------------------------
/// Finds a prescaler/divisor couple to generate the desired frequency from
/// MCK.
/// Returns the value to enter in PWMC_MR or 0 if the configuration cannot be
/// met.
/// \param frequency  Desired frequency in Hz.
/// \param mck  Master clock frequency in Hz.
//------------------------------------------------------------------------------
unsigned short GetClockConfiguration(
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
/// Configures PWM a channel with the given parameters.
/// The PWM controller must have been clocked in the PMC prior to calling this
/// function.
/// \param channel  Channel number.
/// \param prescaler  Channel prescaler.
/// \param alignment  Channel alignment.
/// \param polarity  Channel polarity.
//------------------------------------------------------------------------------
void PWM_ConfigureChannel(
    unsigned char channel,
    unsigned int prescaler,
    unsigned int alignment,
    unsigned int polarity)
{

    // Disable channel
    AT91C_BASE_PWMC->PWMC_DIS = 1 << channel;

    // Configure channel
    AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CMR = prescaler | alignment | polarity;
}


//------------------------------------------------------------------------------
/// Sets the period value used by a PWM channel. This function writes directly
/// to the CPRD register if the channel is disabled; otherwise, it uses the
/// update register CUPD.
/// \param channel  Channel number.
/// \param period  Period value.
//------------------------------------------------------------------------------
void PWM_SetPeriod(unsigned char channel, unsigned short period)
{
    // If channel is disabled, write to CPRD
    if ((AT91C_BASE_PWMC->PWMC_SR & (1 << channel)) == 0) {

        AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CPRDR = period;
    }
    // Otherwise use update register
    else {

        AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CMR |= AT91C_PWMC_CPD;
        AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CUPDR = period;
    }
}

unsigned short PWM_GetPeriod(unsigned char channel){
	return AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CPRDR;
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
void PWM_SetDutyCycle(unsigned char channel, unsigned short duty)
{

    // SAM7S errata
#if defined(at91sam7s16) || defined(at91sam7s161) || defined(at91sam7s32) \
    || defined(at91sam7s321) || defined(at91sam7s64) || defined(at91sam7s128) \
    || defined(at91sam7s256) || defined(at91sam7s512)
#endif

    // If channel is disabled, write to CDTY
    if ((AT91C_BASE_PWMC->PWMC_SR & (1 << channel)) == 0) {

        AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CDTYR = duty;
    }
    // Otherwise use update register
    else {

        AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CMR &= ~AT91C_PWMC_CPD;
        AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CUPDR = duty;
    }
}


unsigned short PWM_GetDutyCycle(unsigned char channel){
	
	return AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CDTYR;
	
}

//------------------------------------------------------------------------------
/// Enables the given PWM channel. This does NOT enable the corresponding pin;
/// this must be done in the user code.
/// \param channel  Channel number.
//------------------------------------------------------------------------------
void PWM_EnableChannel(unsigned char channel)
{
    AT91C_BASE_PWMC->PWMC_ENA = 1 << channel;
}


void InitADC(){

       /* Clear all previous setting and result */
       AT91F_ADC_SoftReset (AT91C_BASE_ADC);
       
       /* First Step: Set up by using ADC Mode register */
       AT91F_ADC_CfgModeReg (AT91C_BASE_ADC,
                           (SHTIM << 24) | (STARTUP << 16) | (PRESCAL << 8) | 
                            (SLEEP << 5) | (LOWRES <<4) | (TRGSEL << 1) | (TRGEN )) ;

        /* Second Step: Select the active channel */
        AT91F_ADC_EnableChannel (AT91C_BASE_ADC, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7)); 
}


void ReadAllADC(	unsigned int *a0, 
						unsigned int *a1, 
						unsigned int *a2,
						unsigned int *a3,
						unsigned int *a4,
						unsigned int *a5,
						unsigned int *a6,
						unsigned int *a7 ){
	
        /* Third Step: Start the conversion */
        AT91F_ADC_StartConversion (AT91C_BASE_ADC);
        
        /* Fourth Step: Waiting Stop Of conversion by pulling */
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

unsigned int ReadADC(unsigned int channel){
	
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
        
        /* Fifth Step: Read the ADC result output */
       	unsigned int result = AT91F_ADC_GetLastConvertedData(AT91C_BASE_ADC);
//        unsigned int result = AT91F_ADC_GetConvertedDataCH0 (AT91C_BASE_ADC);

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

void InitLEDs(){
	
    AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, LED_MASK ) ;
   //* Clear the LED's.
    AT91F_PIO_SetOutput( AT91C_BASE_PIOA, LED_MASK ) ;
}

void EnableLED(unsigned int Led){
        AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, Led );
}

void DisableLED(unsigned int Led){
        AT91F_PIO_SetOutput( AT91C_BASE_PIOA, Led );
}

void ToggleLED (unsigned int Led){
    if ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & Led ) == Led )
    {
        AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, Led );
    }
    else
    {
        AT91F_PIO_SetOutput( AT91C_BASE_PIOA, Led );
    }
}

void initTimerChannels(){
	initTimer0();
	initTimer1();
	initTimer2();
}

void initTimer0(){

	g_timer0_overflow = 1;
	// Set PIO pins for Timer Counter 0
	//TIOA0 connected to PA0
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA,0,AT91C_PA0_TIOA0);
	
	/// Enable TC0's clock in the PMC controller
	AT91F_TC0_CfgPMC();
	
	AT91F_TC_Open ( 
	AT91C_BASE_TC0,
	
	TC_CLKS_MCK32 | 
	AT91C_TC_ETRGEDG_FALLING |	
	AT91C_TC_ABETRG |
	AT91C_TC_LDRA_RISING|
	AT91C_TC_LDRB_FALLING
	
	,AT91C_ID_TC0
	);
	
	AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC0, TIMER0_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, timer0_irq_handler);
	AT91C_BASE_TC0->TC_IER = AT91C_TC_LDRBS;  //  IRQ enable RB loading
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC0);	
}

void initTimer1(){

	g_timer1_overflow = 1;
	
	// Set PIO pins for Timer Counter 1
	// TIOA1 mapped to PA15
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA,0,AT91C_PA15_TIOA1);
	 
	// Enable TC0's clock in the PMC controller
	AT91F_TC1_CfgPMC();
	
	AT91F_TC_Open ( 
	AT91C_BASE_TC1,
	
	TC_CLKS_MCK32 | 
	AT91C_TC_ETRGEDG_FALLING |	
	AT91C_TC_ABETRG |
	AT91C_TC_LDRA_RISING|
	AT91C_TC_LDRB_FALLING
	
	,AT91C_ID_TC1
	);
	
	AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC1, TIMER1_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, timer1_irq_handler);
	AT91C_BASE_TC1->TC_IER = AT91C_TC_LDRBS;  //  IRQ enable RB loading
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC1);	
}

void initTimer2(){

	g_timer2_overflow = 1;
	// Set PIO pins for Timer Counter 2
	// TIOA2 mapped to PA26
	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA,0,AT91C_PA26_TIOA2);

	// Enable TC0's clock in the PMC controller
	AT91F_TC2_CfgPMC();
	
	AT91F_TC_Open ( 
	AT91C_BASE_TC2,
	
	TC_CLKS_MCK32 | 
	AT91C_TC_ETRGEDG_FALLING |	
	AT91C_TC_ABETRG |
	AT91C_TC_LDRA_RISING|
	AT91C_TC_LDRB_FALLING
	
	,AT91C_ID_TC2
	);
	
	AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC2, TIMER2_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, timer2_irq_handler);
	AT91C_BASE_TC2->TC_IER =AT91C_TC_LDRBS;  //  IRQ enable RB loading
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC2);	
}

void getAllTimerPeriods(unsigned int *t0, unsigned int *t1, unsigned int *t2){
	*t0 = getTimer0Period();
	*t1 = getTimer1Period();
	*t2 = getTimer2Period();	
}

unsigned int getTimer0Period(){
	return g_timer0_overflow ? MAX_TIMER_VALUE : AT91C_BASE_TC0->TC_RB;
}

unsigned int getTimer1Period(){
	return g_timer1_overflow ? MAX_TIMER_VALUE : AT91C_BASE_TC1->TC_RB;
}

unsigned int getTimer2Period(){
	return g_timer2_overflow ? MAX_TIMER_VALUE : AT91C_BASE_TC2->TC_RB;
}
