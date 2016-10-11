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
#include "stm32f30x.h"
#include "stm32f30x_adc.h"
#include "stm32f30x_dma.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "taskUtil.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * RC/T
 * Battery voltage ADC3_IN1
 */

#define ADC_SYSTEM_VOLTAGE_RANGE	20.0f
#define SCALING_BATTERYV		0.00465f
#define TOTAL_ADC_CHANNELS		1

volatile unsigned short *ADC_Val;

int ADC_device_init(void)
{
	ADC_Val = calloc(TOTAL_ADC_CHANNELS, sizeof(*ADC_Val));

        /* Configure the ADC clock */
        RCC_ADCCLKConfig(RCC_ADC34PLLCLK_Div2);

        /* Enable ADC3 clock */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC34, ENABLE);

        /* DMA configuration */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

        DMA_Cmd(DMA2_Channel5, DISABLE);
        DMA_DeInit(DMA2_Channel5);

        DMA_InitTypeDef DMA_InitStructure;
	DMA_StructInit(&DMA_InitStructure);
        DMA_InitStructure.DMA_BufferSize = TOTAL_ADC_CHANNELS;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &ADC_Val[0];
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &ADC3->DR;
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

        /* Calibration procedure */
        ADC_VoltageRegulatorCmd(ADC3, ENABLE);

	/* Disable ADC3 and its associated DMA before we calibrate if enabled. */
	if (ADC3->CR & ADC_CR_ADEN) {
		ADC_StopConversion(ADC3);
		while(SET == ADC_GetStartConversionStatus(ADC3));
		ADC_Cmd(ADC3, DISABLE);
		while(SET == ADC_GetDisableCmdStatus(ADC3));
	}

        /*
         * Wait at least 10us before starting calibration or enabling.
         * Since there is no interrupt to tell me this is ready we just
         * delay one scheduler tick which should be more than 10us.  This
         * assumes that the scheduler is always active during init.
         */
        delayTicks(1);

        /* We compare against Vref only */
        ADC_SelectDifferentialMode(ADC3, ADC_Channel_1, DISABLE);

        /* Now calibrate our ADC */
        ADC_SelectCalibrationMode(ADC3, ADC_CalibrationMode_Single);
        ADC_StartCalibration(ADC3);
        while(SET == ADC_GetCalibrationStatus(ADC3));

        ADC_InitTypeDef ADC_InitStructure;
        ADC_StructInit(&ADC_InitStructure);
        ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Enable;
        ADC_InitStructure.ADC_NbrOfRegChannel = 1;
        ADC_Init(ADC3, &ADC_InitStructure);

        /* Register the channel(s) to read/convert */
        ADC_RegularChannelConfig(ADC3, ADC_Channel_1, 1, ADC_SampleTime_19Cycles5);

        /* Enables DMA channel */
        ADC_DMACmd(ADC3, ENABLE);
        ADC_DMAConfig(ADC3, ADC_DMAMode_Circular);
        DMA_Cmd(DMA2_Channel5, ENABLE);

	/* Enable ADC3 */
	ADC_Cmd(ADC3, ENABLE);

        /* wait for ADRDY */
	while(!ADC_GetFlagStatus(ADC3, ADC_FLAG_RDY));

        /* Start ADC3 Conversion. Must be called after ADC enabled */
        ADC_StartConversion(ADC3);

        return 1;
}

static bool channel_in_bounds(const size_t channel)
{
        return channel < TOTAL_ADC_CHANNELS;
}

int ADC_device_sample(const size_t channel)
{
        return channel_in_bounds(channel) ? ADC_Val[channel] : -1;
}

float ADC_device_get_voltage_range(const size_t channel)
{
	return ADC_SYSTEM_VOLTAGE_RANGE;
}

float ADC_device_get_channel_scaling(const size_t channel)
{
	return SCALING_BATTERYV;
}
