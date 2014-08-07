#include "ADC_device.h"
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_dma.h"

#define ADCx_DR_ADDRESS          ((uint32_t)0x4001224C)
#define ADCx                     ADC3
#define ADC_CHANNEL              ADC_Channel_7
#define ADCx_CLK                 RCC_APB2Periph_ADC3
#define ADCx_CHANNEL_GPIO_CLK    RCC_AHB1Periph_GPIOF
#define GPIO_PIN                 GPIO_Pin_9
#define GPIO_PORT                GPIOF
#define DMA_CHANNELx             DMA_Channel_2
#define DMA_STREAMx              DMA2_Stream0

static uint16_t uhADCxConvertedValue = 0;

int ADC_device_init(void){

	  ADC_InitTypeDef       ADC_InitStructure;
	  ADC_CommonInitTypeDef ADC_CommonInitStructure;
	  DMA_InitTypeDef       DMA_InitStructure;
	  GPIO_InitTypeDef      GPIO_InitStructure;

	  /* Enable ADCx, DMA and GPIO clocks ****************************************/
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	  RCC_AHB1PeriphClockCmd(ADCx_CHANNEL_GPIO_CLK, ENABLE);
	  RCC_APB2PeriphClockCmd(ADCx_CLK, ENABLE);


	  /* DMA2 Stream0 channel2 configuration **************************************/
	  DMA_InitStructure.DMA_Channel = DMA_CHANNELx;
	  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADCx_DR_ADDRESS;
	  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&uhADCxConvertedValue;
	  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	  DMA_InitStructure.DMA_BufferSize = 1;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	  DMA_Init(DMA_STREAMx, &DMA_InitStructure);
	  DMA_Cmd(DMA_STREAMx, ENABLE);

	  /* Configure ADC3 Channel7 pin as analog input ******************************/
	  GPIO_InitStructure.GPIO_Pin = GPIO_PIN;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	  GPIO_Init(GPIO_PORT, &GPIO_InitStructure);

	  /* ADC Common Init **********************************************************/
	  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	  ADC_CommonInit(&ADC_CommonInitStructure);

	  /* ADC3 Init ****************************************************************/
	  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	  ADC_InitStructure.ADC_NbrOfConversion = 1;
	  ADC_Init(ADCx, &ADC_InitStructure);

	  /* ADC3 regular channel7 configuration *************************************/
	  ADC_RegularChannelConfig(ADCx, ADC_CHANNEL, 1, ADC_SampleTime_3Cycles);

	 /* Enable DMA request after last transfer (Single-ADC mode) */
	  ADC_DMARequestAfterLastTransferCmd(ADCx, ENABLE);

	  /* Enable ADC3 DMA */
	  ADC_DMACmd(ADCx, ENABLE);

	  /* Enable ADC3 */
	  ADC_Cmd(ADCx, ENABLE);

	return 1;
}

void ADC_device_sample_all(	unsigned int *a0,
						unsigned int *a1,
						unsigned int *a2,
						unsigned int *a3,
						unsigned int *a4,
						unsigned int *a5,
						unsigned int *a6,
						unsigned int *a7 ){

}

unsigned int ADC_device_sample(unsigned int channel){
	return 0;
}

