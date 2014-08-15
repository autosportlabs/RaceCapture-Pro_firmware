#include "PWM_device.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "system_stm32f4xx.h"

#include <stdint.h>
#include <stddef.h>

#define MAX_DUTY_CYCLE 100
#define PWM_CHANNEL_COUNT 4

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

typedef struct _gpio {
	uint32_t rcc_ahb1;
	GPIO_TypeDef *port;
	uint16_t mask;
} gpio;

static gpio analogCtrlGpios[] = {
	{ RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_3},
	{ RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_4},
	{ RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_0},
	{ RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_1}
};

static void setAnalogControlGpio(size_t port, uint8_t state){
	if (port < PWM_CHANNEL_COUNT){
		if (state){
			GPIO_SetBits(analogCtrlGpios[port].port, analogCtrlGpios[port].mask);
		}
		else{
			GPIO_ResetBits(analogCtrlGpios[port].port, analogCtrlGpios[port].mask);
		}
	}
}

static void initAnalogControlGpios(){
	GPIO_InitTypeDef gpio_conf;

	for (size_t i = 0; i < PWM_CHANNEL_COUNT; ++i){
		gpio *gpioCfg = (analogCtrlGpios + i);
		GPIO_StructInit(&gpio_conf);
		gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
		RCC_AHB1PeriphClockCmd(gpioCfg->rcc_ahb1, ENABLE);
		gpio_conf.GPIO_Pin = gpioCfg->mask;
		gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
		gpio_conf.GPIO_PuPd = GPIO_PuPd_NOPULL;
		gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
		gpio_conf.GPIO_OType = GPIO_OType_PP;
		GPIO_Init(gpioCfg->port, &gpio_conf);
		setAnalogControlGpio(i, 0);
	}
}

int PWM_device_init(){

	initAnalogControlGpios();

	uint32_t period = 500;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	uint16_t prescaler = (uint16_t) ((SystemCoreClock /2) / 21000000) - 1;

	TIM_TimeBaseInitTypeDef timerInitStructure;
	timerInitStructure.TIM_Prescaler = prescaler;
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
void PWM_device_channel_enable_analog(size_t channel, uint8_t enabled){
	setAnalogControlGpio(channel, enabled);
}

