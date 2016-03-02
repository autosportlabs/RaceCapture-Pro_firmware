#include "ADC_device.h"
#include <stdint.h>
#include "mod_string.h"
#include "stm32f30x.h"
#include "stm32f30x_adc.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_dma.h"

//on RC/T
//Battery voltage ADC3_IN1

#define ADC_SYSTEM_VOLTAGE_RANGE	20.0f

#define SCALING_BATTERYV	0.00465f

#define TOTAL_ADC_CHANNELS	1

volatile unsigned short ADC_Val[1];

//******************************************************************************
int ADC_device_init(void)
{
	   /* Configure the ADC clock */
	  RCC_ADCCLKConfig(RCC_ADC34PLLCLK_Div2);

	  /* Enable ADC3 clock */
	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC34, ENABLE);

	  /* DMA configuration */
	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

	    DMA_InitTypeDef DMA_InitStructure;
	    DMA_InitStructure.DMA_BufferSize = 1;
	    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_Val[0];
	    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC3->DR;
	    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	    DMA_InitStructure.DMA_Priority = DMA_Priority_High;

	    DMA_Init(DMA2_Channel5, &DMA_InitStructure);

	  /* ADC Channel configuration */
	  /* GPIOB Periph clock enable */
	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

      GPIO_InitTypeDef GPIO_InitStructure;
      GPIO_StructInit(&GPIO_InitStructure);
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
      GPIO_Init(GPIOB, &GPIO_InitStructure);

	  /* Calibration procedure
	   *  TODO see if this is necessary
	   *
	  ADC_VoltageRegulatorCmd(ADC1, ENABLE);

	  ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);
	  ADC_StartCalibration(ADC1);

	  while(ADC_GetCalibrationStatus(ADC1) != RESET );
	  calibration_value = ADC_GetCalibrationValue(ADC1);
      */

	  ADC_CommonInitTypeDef   ADC_CommonInitStructure;

	  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;
	  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_Circular;
	  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;
	  ADC_CommonInit(ADC3, &ADC_CommonInitStructure);

	  ADC_InitTypeDef         ADC_InitStructure;
	  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Enable;
	  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
	  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
	  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;
	  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
	  ADC_InitStructure.ADC_NbrOfRegChannel = 1;
	  ADC_Init(ADC3, &ADC_InitStructure);

	  /* ADC1 regular channel 6, 7 & 9 configuration */
	  ADC_RegularChannelConfig(ADC3, ADC_Channel_1, 1, ADC_SampleTime_19Cycles5); //PB1

	  /* Enables DMA channel */
	  DMA_Cmd(DMA2_Channel5, ENABLE);

	 /* Enable ADC3 DMA */
	 ADC_DMACmd(ADC3, ENABLE);

	/* Enable ADC3 */
	ADC_Cmd(ADC3, ENABLE);

	 /* wait for ADRDY */
	while(!ADC_GetFlagStatus(ADC3, ADC_FLAG_RDY));

	 /* Start ADC3 Conversion */
	 ADC_StartConversion(ADC3);

    return 1;
}

void ADC_device_sample_all(unsigned int *a0, unsigned int *a1, unsigned int *a2,
                           unsigned int *a3, unsigned int *a4, unsigned int *a5,
                           unsigned int *a6, unsigned int *a7)
{
    *a0 = ADC_Val[0];
    *a1 = 0;
    *a2 = 0;
    *a3 = 0;
    *a4 = 0;
    *a5 = 0;
    *a6 = 0;
    *a7 = 0;
}

unsigned int ADC_device_sample(unsigned int channel)
{
    return ADC_Val[channel];
}

float ADC_device_get_voltage_range(size_t channel)
{
	return ADC_SYSTEM_VOLTAGE_RANGE;
}

float ADC_device_get_channel_scaling(size_t channel)
{
	/* there is only battery voltage */
	return SCALING_BATTERYV;
}
