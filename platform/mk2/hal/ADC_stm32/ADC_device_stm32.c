/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ADC_device.h"
#include "mem_mang.h"
#include <string.h>
#include "stm32f4xx.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
/*
 * On RCP MK2
 * Analog1 - PC4 - ADC12_IN14
 * Analog2 - PC5 - ADC12_IN15
 * Analog3 - PB0 - ADC12_IN8
 * Analog4 - PB1 - ADC12_IN9
 * Analog5 - PC3 - ADC123_IN13
 * Analog6 - PC2 - ADC123_IN12
 * Analog7 - PC1 - ADC123_IN11
 * Analog8 - PC0 - ADC123_IN10
 * Analog9 - PA4 - ADC12_IN4
 */

#define ADC_PORT_VOLTAGE_RANGE 		5.0f
#define ADC_SYSTEM_VOLTAGE_RANGE	20.0f
#define SCALING_5V 		0.00125691302f
#define SCALING_BATTERYV	0.00465f
#define TOTAL_ADC_CHANNELS	9

static uint16_t *ADCConvertedValues;

static void ADC_GPIO_Configuration(void)
{
        {
                GPIO_InitTypeDef GPIO_InitStructure;
                GPIO_StructInit(&GPIO_InitStructure);
                RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                GPIO_Init(GPIOA, &GPIO_InitStructure);
        }
        {
                GPIO_InitTypeDef GPIO_InitStructure;
                GPIO_StructInit(&GPIO_InitStructure);
                RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                GPIO_Init(GPIOB, &GPIO_InitStructure);
        }
        {
                GPIO_InitTypeDef GPIO_InitStructure;
                GPIO_StructInit(&GPIO_InitStructure);
                RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
                GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_0 | GPIO_Pin_1 |
                                               GPIO_Pin_2 | GPIO_Pin_3 |
                                               GPIO_Pin_4 | GPIO_Pin_5);
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                GPIO_Init(GPIOC, &GPIO_InitStructure);
        }
}

//******************************************************************************
int ADC_device_init(void)
{
        /* TODO: Ditch malloc here, this can be done statically */
        size_t adcBufferSize = sizeof(uint16_t) * TOTAL_ADC_CHANNELS;
        ADCConvertedValues = portMalloc(adcBufferSize);
        memset(ADCConvertedValues, 0, adcBufferSize);

        ADC_InitTypeDef ADC_InitStructure;
        ADC_CommonInitTypeDef ADC_CommonInitStructure;
        DMA_InitTypeDef DMA_InitStructure;

        ADC_DeInit();
        ADC_GPIO_Configuration();

        /* Enable peripheral clocks */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

        /* DMA2_Stream0 channel0 configuration */
        DMA_DeInit(DMA2_Stream2);
        DMA_InitStructure.DMA_Channel = DMA_Channel_1;
        DMA_InitStructure.DMA_PeripheralBaseAddr =
                (uint32_t)&ADC2->DR;
        DMA_InitStructure.DMA_Memory0BaseAddr =
                (uint32_t)&ADCConvertedValues[0];
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
        DMA_InitStructure.DMA_BufferSize = 9;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize =
                DMA_PeripheralDataSize_HalfWord;
        DMA_InitStructure.DMA_MemoryDataSize =
                DMA_MemoryDataSize_HalfWord;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        DMA_InitStructure.DMA_Priority = DMA_Priority_High;
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
        DMA_Init(DMA2_Stream2, &DMA_InitStructure);
        /* DMA2_Stream0 enable */
        DMA_Cmd(DMA2_Stream2, ENABLE);

        /* ADC Common Init */
        ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
        ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
        ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
        ADC_CommonInitStructure.ADC_TwoSamplingDelay =
                ADC_TwoSamplingDelay_5Cycles;
        ADC_CommonInit(&ADC_CommonInitStructure);

        /* ADC2 Init */
        ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
        ADC_InitStructure.ADC_ScanConvMode = ENABLE;
        ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
        ADC_InitStructure.ADC_ExternalTrigConvEdge =
                ADC_ExternalTrigConvEdge_None;
        ADC_InitStructure.ADC_ExternalTrigConv =
                ADC_ExternalTrigConv_T1_CC1;
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
        ADC_InitStructure.ADC_NbrOfConversion = 9;
        ADC_Init(ADC2, &ADC_InitStructure);

        /* ADC2 regular channel configuration */
        ADC_RegularChannelConfig(ADC2, ADC_Channel_14, 1,
                                 ADC_SampleTime_480Cycles);
        ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 2,
                                 ADC_SampleTime_480Cycles);
        ADC_RegularChannelConfig(ADC2, ADC_Channel_8, 3,
                                 ADC_SampleTime_480Cycles);
        ADC_RegularChannelConfig(ADC2, ADC_Channel_9, 4,
                                 ADC_SampleTime_480Cycles);
        ADC_RegularChannelConfig(ADC2, ADC_Channel_13, 5,
                                 ADC_SampleTime_480Cycles);
        ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 6,
                                 ADC_SampleTime_480Cycles);
        ADC_RegularChannelConfig(ADC2, ADC_Channel_11, 7,
                                 ADC_SampleTime_480Cycles);
        ADC_RegularChannelConfig(ADC2, ADC_Channel_4, 8,
                                 ADC_SampleTime_480Cycles);
        ADC_RegularChannelConfig(ADC2, ADC_Channel_10, 9,
                                 ADC_SampleTime_480Cycles);

        /* Enable DMA request after last transfer (Single-ADC mode) */
        ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);

        /* Enable ADC2 DMA */
        ADC_DMACmd(ADC2, ENABLE);

        /* Enable ADC2 */
        ADC_Cmd(ADC2, ENABLE);

        /* Start ADC2 Software Conversion */
        ADC_SoftwareStartConv(ADC2);

        return 1;
}

static bool channel_in_bounds(const size_t channel)
{
        return channel < TOTAL_ADC_CHANNELS;
}

int ADC_device_sample(const size_t channel)
{
        return channel_in_bounds(channel) ?
                ADCConvertedValues[channel] : -1;
}

float ADC_device_get_voltage_range(const size_t channel)
{
        switch (channel) {
        case 7:
                return ADC_SYSTEM_VOLTAGE_RANGE;
        default:
                return ADC_PORT_VOLTAGE_RANGE;
        }
}

float ADC_device_get_channel_scaling(const size_t channel)
{
        switch (channel) {
        case 7:
                return SCALING_BATTERYV;
        default:
                return SCALING_5V;
        }
}
