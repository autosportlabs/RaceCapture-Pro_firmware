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

#include "loggerConfig.h"
#include "printk.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_misc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "timer_config.h"
#include "timer_device.h"

#include <stdbool.h>
#include <string.h>

#define INPUT_CAPTURE_FILTER 	0X0
#define MK2_TIMER_CHANNELS	3
#define LOG_PFX			"[timer_device] "
#define PRESCALER_FAST		110
#define PRESCALER_MEDIUM	1101
#define PRESCALER_SLOW		11010
#define TIMER_CLK_FREQ_FAST_HZ	168000000 /* Logical Timer 1 */
#define TIMER_CLK_FREQ_SLOW_HZ	84000000  /* Logical Timer 0 & 2 */
#define TIMER_IRQ_PRIORITY 	5
#define TIMER_IRQ_SUB_PRIORITY 	0
#define TIMER_PERIOD		0xFFFF

static struct state {
        uint16_t period;
        uint16_t duty_cycle;
        uint16_t q_period_ticks;
} g_state[MK2_TIMER_CHANNELS];

static struct config {
        uint16_t prescaler;
        uint32_t q_period_us;
        enum timer_edge edge;
} g_config[MK2_TIMER_CHANNELS];

static uint16_t get_polarity(const enum timer_edge edge)
{
        switch(edge) {
        case TIMER_EDGE_RISING:
                return TIM_ICPolarity_Rising;
        case TIMER_EDGE_FALLING:
        default:
                return TIM_ICPolarity_Falling;
        }
}

/*
 * Logical to hardware mappings for RCP MK2
 *
 * TIMER 0 = PA6 / TIM13_CH1 / **TIM3_CH1** /
 * TIMER 1 = PA2 / TIM2_CH3(32bit) / **TIM9_CH1** / TIM5_CH3(32bit)
 * TIMER 2 = PA3 / TIM5_CH4(32bit) / TIM9_CH2 / **TIM2_CH4(32bit)**
 */

static void set_local_uri_source(TIM_TypeDef *tim)
{
        /*
         * Set update request interrupt source to only occur during overflow.
         * Otherwise it will be set to interrupt at a global scope, which
         * means during overflow/underflow, setting of UG, or update
         * generation through the slave controller (which happens every pulse).
         */
        TIM_UpdateRequestConfig(tim, TIM_UpdateSource_Regular);
}

static void init_timer_0(struct config *cfg)
{
    //enable and configure GPIO for alternate function
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
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
    TIM_TimeBaseInitStructure.TIM_Prescaler = cfg->prescaler - 1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = TIMER_PERIOD;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    /* Fix the Update Request Interrupt Source */
    set_local_uri_source(TIM3);

    TIM_Cmd(TIM3, ENABLE);

    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = get_polarity(cfg->edge);
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = INPUT_CAPTURE_FILTER;
    TIM_PWMIConfig(TIM3, &TIM_ICInitStructure);

    /* Select the slave Mode: Reset Mode */
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable);

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
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}

static void init_timer_1(struct config *cfg)
{
    //enable and configure GPIO for alternate function
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
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
    TIM_TimeBaseInitStructure.TIM_Prescaler = cfg->prescaler - 1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = TIMER_PERIOD;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM9, &TIM_TimeBaseInitStructure);

    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = get_polarity(cfg->edge);
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = INPUT_CAPTURE_FILTER;
    TIM_PWMIConfig(TIM9, &TIM_ICInitStructure);

    /* Select the slave Mode: Reset Mode */
    TIM_SelectSlaveMode(TIM9, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM9, TIM_MasterSlaveMode_Enable);

    TIM_SelectInputTrigger(TIM9, TIM_TS_TI1FP1);

    /* Fix the Update Request Interrupt Source */
    set_local_uri_source(TIM9);

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
    TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);
}

static void init_timer_2(struct config *cfg)
{
    //enable and configure GPIO for alternate function
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
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
    TIM_TimeBaseInitStructure.TIM_Prescaler = cfg->prescaler - 1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = TIMER_PERIOD;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
    TIM_ICInitStructure.TIM_ICPolarity = get_polarity(cfg->edge);
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = INPUT_CAPTURE_FILTER;
    TIM_ICInit(TIM2, &TIM_ICInitStructure);

    set_local_uri_source(TIM2);
    TIM_Cmd(TIM2, ENABLE);

    /* Enable the TIM1 global Interrupt */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIMER_IRQ_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMER_IRQ_SUB_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable the CC2 Interrupt Request
    TIM_ITConfig(TIM2, TIM_IT_CC4, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update , ENABLE);
}

/**
 * @return the clock speed of the channel in Hz
 */
static uint32_t get_clk_speed(const size_t chan)
{
        switch (chan) {
        case 1:
                return TIMER_CLK_FREQ_FAST_HZ;
        case 0:
        case 2:
                return TIMER_CLK_FREQ_SLOW_HZ;
        default:
                return 0; /* Hell if we know the speed... */
        }
}

static uint16_t speed_to_prescaler(const size_t chan, const size_t speed)
{
        uint16_t prescalar;
        switch (speed) {
        case TIMER_SLOW:
                prescalar = PRESCALER_SLOW;
                break;
        case TIMER_FAST:
                prescalar = PRESCALER_FAST;
                break;
        case TIMER_MEDIUM:
        default:
                prescalar = PRESCALER_MEDIUM;
                break;
        }

        /*
         * For consistency among our timers, we set them all to increment
         * at the same rate.  Unfortunately, not all timers share the same
         * clock.  To compensate for this, adjust the prescalar as needed
         * knowing that our FAST clock is a whole number multiple of our
         * SLOW clock.
         */
        const uint16_t timer_mult = get_clk_speed(chan) / TIMER_CLK_FREQ_SLOW_HZ;
        const uint16_t ps = prescalar * timer_mult;

        pr_debug_int_msg(LOG_PFX "Prescalar: ", ps);
        return ps;
}

void reset_device_state(const size_t chan)
{
        if (chan >= MK2_TIMER_CHANNELS)
                return;

        struct state *s = g_state + chan;
        memset(s, 0, sizeof(struct state));
}

bool timer_device_init(const size_t chan, const uint32_t speed,
                       const uint32_t quiet_period_us,
                       const enum timer_edge edge)
{
        if (chan >= MK2_TIMER_CHANNELS)
                return false;

        pr_debug_int_msg(LOG_PFX "Initializing timer: ", chan);
        struct config *c = &g_config[chan];
        c->prescaler = speed_to_prescaler(chan, speed);
        c->q_period_us = quiet_period_us;
        c->edge = edge;

        reset_device_state(chan);

        switch (chan) {
        case 0:
                init_timer_0(c);
                return true;
        case 1:
                init_timer_1(c);
                return true;
        case 2:
                init_timer_2(c);
                return true;
        }

        /* Should never get here */
        return false;
}

void timer_device_reset_count(size_t chan) {}

uint32_t timer_device_get_count(size_t chan)
{
        return 0;
}

static uint32_t ticks_to_us(const size_t chan, const uint16_t ticks)
{
        /* Our pre-scalars are setup to never have less than 1 tick per us */
        const uint32_t us_per_tick = get_clk_speed(chan) / 1000000;
        return us_per_tick ? g_config[chan].prescaler * ticks / us_per_tick : 0;
}

uint32_t timer_device_get_usec(size_t chan)
{
        return ticks_to_us(chan, timer_device_get_period(chan));
}

uint32_t timer_device_get_period(size_t chan)
{
        return chan < MK2_TIMER_CHANNELS ? g_state[chan].period : 0;
}


/*
 * = = = IRQ methods below this point = = =
 */

/**
 * Updates the device state based on the period ticks (p_ticks) and the
 * duty cycle based on both period ticks and high-level ticks (h_ticks).
 * If in the quiet time, it saves the period ticks value to be used later.
 * otherwise it updates the period and duty_cycle values and clears the
 * buffer used for ticks read during the quiet period.
 * @param chan The timer channel index.
 * @param p_ticks The tick count for the entire period.
 * @param h_ticks The tick count for the length of the high signal level.
 */
static void update_device_state(const size_t chan, const uint16_t p_ticks,
                                const uint16_t h_ticks)
{
        /* Ensure that we have some minimum sane number of ticks */
        if (chan >= MK2_TIMER_CHANNELS || p_ticks <= 3)
                return;

        struct state *s = &g_state[chan];

        const uint16_t total_ticks = p_ticks + s->q_period_ticks;
        const uint32_t us = ticks_to_us(chan, total_ticks);
        if (us < g_config[chan].q_period_us) {
                /*
                 * Then this is a reset in the quiet window.  Only need to adjust
                 * the state to reflect that we saw this event.
                 */
                s->q_period_ticks = total_ticks;
                return;
        }

        /* If here, then past quiet period.  Update complete state */
        s->period = total_ticks;
        s->duty_cycle = 100 * (uint32_t) (h_ticks / total_ticks);
        s->q_period_ticks = 0;
}

/* Logical Timer 0 IRQ Handler */
void TIM3_IRQHandler(void)
{
        /* Edge detected interrupt */
        if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET) {
                TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

                const uint16_t p_ticks = TIM_GetCapture1(TIM3);
                const uint16_t p_h_ticks = TIM_GetCapture2(TIM3);
                update_device_state(0, p_ticks, p_h_ticks);
        }

        /* Overflow interrupt */
        if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
                TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

                reset_device_state(0);
        }

}

/* Logical Timer 1 IRQ Handler */
void TIM1_BRK_TIM9_IRQHandler(void)
{
        /* Edge detected interrupt */
        if (TIM_GetITStatus(TIM9, TIM_IT_CC2) != RESET) {
                TIM_ClearITPendingBit(TIM9, TIM_IT_CC2);

                const uint16_t p_ticks = TIM_GetCapture1(TIM9);
                const uint16_t p_h_ticks = TIM_GetCapture2(TIM9);
                update_device_state(1, p_ticks, p_h_ticks);
        }

        /* Overflow interrupt */
        if (TIM_GetITStatus(TIM9, TIM_IT_Update) != RESET) {
                TIM_ClearITPendingBit(TIM9, TIM_IT_Update);

                reset_device_state(1);
        }
}

/*
 * Logical Timer 2 IRQ Handler with a HACK
 *
 * Unfortunately on MK2 there is a hardware bug where the GPIO input
 * for TIM2 can't trigger the interrupt on the slave controller for
 * the timer (because no connection link exists in the chip).  This means
 * that we can't reset the timer every time there is an interrupt.  It
 * also means that we don't get the neat logic used to calculate duty
 * cycle (serious voodoo in the configuration of CCR 1 and 2 on this chip).
 * We make TIM2 functional by emulating the effect of the reset as best we
 * can here and handle overflow as well.
 */
void TIM2_IRQHandler(void)
{
        static uint32_t last;
        static uint32_t cc4_irqs;

        if (TIM_GetITStatus(TIM2, TIM_IT_CC4) != RESET) {
                TIM_ClearITPendingBit(TIM2, TIM_IT_CC4);

                const uint32_t current = TIM_GetCapture4(TIM2);
                const uint16_t p_ticks = last < current ? current - last :
                        TIMER_PERIOD - last + current;

                /* No duty cycle on timer 2 because of hardware bug */
                update_device_state(2, p_ticks, 0);
                last = current;
                ++cc4_irqs;
        }

        if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
                TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

                if (0 == cc4_irqs)
                        reset_device_state(2);

                cc4_irqs = 0;
        }

}
