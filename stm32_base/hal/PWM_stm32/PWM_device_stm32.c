#include "PWM_device.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"

#define MAX_DUTY_CYCLE 100


typedef struct _pwm {
	uint32_t pin;
	uint16_t pinSource;
} pwm;

static pwm pwms[] = {
	{ GPIO_Pin_12, GPIO_PinSource12 },
	{ GPIO_Pin_13, GPIO_PinSource13 },
	{ GPIO_Pin_14, GPIO_PinSource14 },
	{ GPIO_Pin_15, GPIO_PinSource15 }
};


int PWM_device_init(){

	uint32_t period = 500;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	TIM_TimeBaseInitTypeDef timerInitStructure;
	timerInitStructure.TIM_Prescaler = 40000;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = period;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &timerInitStructure);
	TIM_ARRPreloadConfig(TIM4, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
	return 1;
}

void PWM_device_configure_clock(unsigned short clockFrequency){

}


void PWM_device_channel_init(unsigned int channel, unsigned short period, unsigned short dutyCycle){

	GPIO_InitTypeDef gpioStructure;
	gpioStructure.GPIO_Pin = pwms[channel].pin;
	gpioStructure.GPIO_Mode = GPIO_Mode_AF;
	gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
	gpioStructure.GPIO_OType = GPIO_OType_PP;
	gpioStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOD, &gpioStructure);

	TIM_OCInitTypeDef outputChannelInit = {0,};
	//TIM_OCStructInit(&outputChannelInit);
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = 400;
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;
	switch(channel){
		case 0:
			TIM_OC1Init(TIM4, &outputChannelInit);
			TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
			break;
		case 1:
			TIM_OC2Init(TIM4, &outputChannelInit);
			TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
			break;
		case 2:
			TIM_OC3Init(TIM4, &outputChannelInit);
			TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
			break;
		case 3:
			TIM_OC4Init(TIM4, &outputChannelInit);
			TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
			break;
		default:
			break;
	}
	GPIO_PinAFConfig(GPIOD, pwms[channel].pinSource, GPIO_AF_TIM4);
}

void PWM_device_channel_start_all(){
}

void PWM_device_channel_stop_all(){
}

void PWM_device_channel_start(unsigned int channel){
}

void PWM_device_channel_stop(unsigned int channel){
}

void PWM_device_channel_set_period(unsigned int channel, unsigned short period)
{
}

unsigned short PWM_device_channel_get_period(unsigned int channel){
	return 0;
}

void PWM_device_set_duty_cycle(unsigned int channel, unsigned short duty){
}

unsigned short PWM_device_get_duty_cycle(unsigned int channel){
	return 0;
}


