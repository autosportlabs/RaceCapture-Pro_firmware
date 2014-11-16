#include "timer_device.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_misc.h"

#define TIMER_CHANNELS 3

unsigned int g_timer0_overflow;
unsigned int g_timer1_overflow;
unsigned int g_timer2_overflow;
unsigned int g_timer_counts[TIMER_CHANNELS];

#define TIMER_IRQ_PRIORITY 		4
#define TIMER_IRQ_SUB_PRIORITY 	0

#define INPUT_CAPTURE_FILTER 	0X0

static uint16_t timer0_cc2 = 0;
static uint16_t timer0_duty_cycle = 0;
static uint32_t timer0_frequency = 0;

static uint16_t timer1_cc2 = 0;
static uint16_t timer1_duty_cycle = 0;
static uint32_t timer1_frequency = 0;

static uint16_t timer2_cc2 = 0;
static uint16_t timer2_duty_cycle = 0;
static uint32_t timer2_frequency = 0;

//////////////////////////////////////////////////////////////////////
//logical to hardware mappings for RCP MK2
//TIMER 0 = PA6 / TIM13_CH1 / **TIM3_CH1** /
//TIMER 1 = PA2 / TIM2_CH3(32bit) / **TIM9_CH1** / TIM5_CH3(32bit)
//TIMER 2 = PA3 / **TIM5_CH4(32bit)** / TIM9_CH2 / TIM2_CH4(32bit)
//TIMER 3 = PA7 / TIM8_CH1N / **TIM14_CH1** / TIM3_CH2 / TIM1_CH1N
//////////////////////////////////////////////////////////////////////

static void init_timer_0(size_t divider, unsigned int slowTimerMode){
	//enable and configure GPIO for alternate function
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Connect TIM pins to Alternate Function
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);

	//initialize timer
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	uint16_t prescaler = 8400 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = prescaler;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	TIM_Cmd(TIM3, ENABLE);

	TIM_ICInitTypeDef  TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = INPUT_CAPTURE_FILTER;
	TIM_PWMIConfig(TIM3, &TIM_ICInitStructure);

	/* Select the slave Mode: Reset Mode */
	TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
	TIM_SelectMasterSlaveMode(TIM3,TIM_MasterSlaveMode_Enable);

	TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);

	/* Enable the TIM1 global Interrupt */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIMER_IRQ_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMER_IRQ_SUB_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Enable the CC2 Interrupt Request
	TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);
}

static void init_timer_1(size_t divider, unsigned int slowTimerMode){
	//enable and configure GPIO for alternate function
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Connect TIM pins to Alternate Function
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM9);

	//initialize timer
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	uint16_t prescaler = 16800 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = prescaler;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM9, &TIM_TimeBaseInitStructure);


	TIM_ICInitTypeDef  TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = INPUT_CAPTURE_FILTER;
	TIM_PWMIConfig(TIM9, &TIM_ICInitStructure);

	/* Select the slave Mode: Reset Mode */
	TIM_SelectSlaveMode(TIM9, TIM_SlaveMode_Reset);
	TIM_SelectMasterSlaveMode(TIM9,TIM_MasterSlaveMode_Enable);

	TIM_SelectInputTrigger(TIM9, TIM_TS_TI1FP1);
	TIM_Cmd(TIM9, ENABLE);

	/* Enable the TIM1 global Interrupt */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_TIM9_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIMER_IRQ_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMER_IRQ_SUB_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Enable the CC2 Interrupt Request
	TIM_ITConfig(TIM9, TIM_IT_CC2, ENABLE);

}

static void init_timer_2(size_t divider, unsigned int slowTimerMode){
	//enable and configure GPIO for alternate function
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Connect TIM pins to Alternate Function
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_TIM2);

	//initialize timer
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_DeInit(TIM2);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	uint16_t prescaler = 16800 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = prescaler;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 0xFFFFFFFF;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

	TIM_ICInitTypeDef  TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = INPUT_CAPTURE_FILTER;
	TIM_ICInit(TIM2, &TIM_ICInitStructure);
	TIM_Cmd(TIM2, ENABLE);

	/* Enable the TIM1 global Interrupt */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIMER_IRQ_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMER_IRQ_SUB_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Enable the CC2 Interrupt Request
	TIM_ITConfig(TIM2, TIM_IT_CC4 , ENABLE);
}

static unsigned int getTimer0Period(){
	return timer0_frequency;
}

static unsigned int getTimer1Period(){
	return timer1_frequency;
}

static unsigned int getTimer2Period(){
	return timer2_frequency;
}

int timer_device_init(size_t channel, unsigned int divider, unsigned int slowTimerMode){
	switch(channel){
		case 0:
			init_timer_0(divider, slowTimerMode);
			return 1;
		case 1:
			init_timer_1(divider, slowTimerMode);
			return 1;
		case 2:
			init_timer_2(divider, slowTimerMode);
			return 1;
		default:
			return 0;
	}
}

void timer_device_get_all_periods(unsigned int *t0, unsigned int *t1, unsigned int *t2){
	*t0 = getTimer0Period();
	*t1 = getTimer1Period();
	*t2 = getTimer2Period();
}

void timer_device_reset_count(unsigned int channel){
	if (channel >= 0 && channel < TIMER_CHANNELS){
		g_timer_counts[channel] = 0;
	}
}

unsigned int timer_device_get_count(unsigned int channel){
	if (channel >= 0 && channel < TIMER_CHANNELS){
		return g_timer_counts[channel];
	}
	else{
		return 0;
	}
}

unsigned int timer_device_get_period(unsigned int channel){
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

//logical timer 0
void TIM3_IRQHandler(void)
{
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	/* Clear Capture compare interrupt pending bit */
	TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

	/* Get the Input Capture value */
	timer0_cc2 = TIM_GetCapture2(TIM3);

	if (timer0_cc2 != 0)
	{
		/* Duty cycle computation */
		uint16_t IC1Value = TIM_GetCapture1(TIM3);
		timer0_duty_cycle = (IC1Value * 100) / timer0_cc2;
		timer0_frequency = (RCC_Clocks.HCLK_Frequency) / 16800  / IC1Value;
	}
	else
	{
		timer0_duty_cycle = 0;
		timer0_frequency = 0;
	}
}

//logical timer 1
void TIM1_BRK_TIM9_IRQHandler(void)
{
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	/* Clear Capture compare interrupt pending bit */
	TIM_ClearITPendingBit(TIM9, TIM_IT_CC2);

	/* Get the Input Capture value */
	timer1_cc2 = TIM_GetCapture2(TIM9);

	if (timer1_cc2 != 0)
	{
		/* Duty cycle computation */
		uint16_t IC1Value = TIM_GetCapture1(TIM9);
		timer1_duty_cycle = (IC1Value * 100) / timer1_cc2;
		timer1_frequency = (RCC_Clocks.HCLK_Frequency) / 16800  / IC1Value;
	}
	else
	{
		timer1_duty_cycle = 0;
		timer1_frequency = 0;
	}
}



void TIM2_IRQHandler(void)
{
	static uint32_t last = 0;
	//THIS IS A HORRIBLE HACK.
	//TIM2 CH4 does not seem to reset the counter upon input capture. this is doing it manually.
	//need to fix the configuration of this timer, if possible.
	uint32_t current = TIM_GetCapture4(TIM2);
	if (last < current){
		uint32_t delta = current - last;
		timer2_frequency = 10000  / delta;
	}
	last = current;
	TIM_ClearITPendingBit(TIM2, TIM_IT_CC4);
}

