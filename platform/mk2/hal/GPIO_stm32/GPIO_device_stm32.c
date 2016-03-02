#include "GPIO_device.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerConfig.h"
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_exti.h>
#include <stm32f4xx_misc.h>

#define GPIO_IRQ_PRIORITY 	5
#define GPIO_IRQ_SUB_PRIORITY 	0

extern xSemaphoreHandle xOnPushbutton;

typedef struct _gpio {
    uint32_t rcc_ahb1;
    GPIO_TypeDef *port;
    uint16_t mask;
    GPIOMode_TypeDef mode;
    uint8_t defaultOutputState;
} gpio;

static gpio gpios[] = {
    {RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_9, GPIO_Mode_IN, 0},
    {RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_8, GPIO_Mode_IN, 0},
    {RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_7, GPIO_Mode_IN, 0},
    {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_13, GPIO_Mode_OUT, 0},
    {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_14, GPIO_Mode_OUT, 0},
    {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_15, GPIO_Mode_OUT, 0},
    {RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_10, GPIO_Mode_OUT, 1},
    {RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_8, GPIO_Mode_IN, 0}
};

typedef enum {
    GPI0_ID = 0,
    GPI1_ID,
    GPI2_ID,
    GPO0_ID,
    GPO1_ID,
    GPO2_ID,
    EXT_CTRL_ID,
    PUSHBUTTON_ID,
} GPIO_IDs;

#define GPIO_COUNT 8

static void GPIO_set_port(size_t port, size_t state)
{
    if (state) {
        GPIO_SetBits(gpios[port].port, gpios[port].mask);
    } else {
        GPIO_ResetBits(gpios[port].port, gpios[port].mask);
    }
}

void GPIO_device_set(unsigned int port, unsigned int state)
{
    int gpioIndex = -1;

    switch (port) {
    case 0:
        gpioIndex = GPO0_ID;
        break;
    case 1:
        gpioIndex = GPO1_ID;
        break;
    case 2:
        gpioIndex = GPO2_ID;
        break;
    default:
        break;
    }

    if (gpioIndex >= 0) {
        GPIO_set_port(gpioIndex, state);
    }
}

int GPIO_device_is_button_pressed(void)
{
    return GPIO_ReadInputDataBit(gpios[PUSHBUTTON_ID].port,
                                 gpios[PUSHBUTTON_ID].mask) ? 0 : 1;
}

unsigned int GPIO_device_get(unsigned int port)
{
    int gpioIndex = -1;

    switch (port) {
    case 0:
        gpioIndex = GPI0_ID;
        break;
    case 1:
        gpioIndex = GPI1_ID;
        break;
    case 2:
        gpioIndex = GPI2_ID;
        break;
    default:
        break;
    }

    if (gpioIndex >= 0) {
        return GPIO_ReadInputDataBit(gpios[gpioIndex].port,
                                     gpios[gpioIndex].mask) ? 0 : 1;
    }

    return 0;
}

void init_pushbutton_irq(void)
{
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);

    /* Configure EXTI Line0 */
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Line0 Interrupt to the lowest priority */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = GPIO_IRQ_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = GPIO_IRQ_SUB_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

int GPIO_device_init(LoggerConfig * loggerConfig)
{
    GPIO_InitTypeDef gpio_conf;

    /* Clear the GPIO Structure */

    for (int i = 0; i < GPIO_COUNT; ++i) {
        gpio *gpioCfg = (gpios + i);
        GPIO_StructInit(&gpio_conf);
        gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
        RCC_AHB1PeriphClockCmd(gpioCfg->rcc_ahb1, ENABLE);
        gpio_conf.GPIO_Pin = gpioCfg->mask;
        gpio_conf.GPIO_Mode = gpioCfg->mode;
        if (gpio_conf.GPIO_Mode == GPIO_Mode_OUT) {
            gpio_conf.GPIO_PuPd = GPIO_PuPd_NOPULL;
            gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
            gpio_conf.GPIO_OType = GPIO_OType_PP;
            GPIO_set_port(i, gpioCfg->defaultOutputState);
        } else {
            gpio_conf.GPIO_Mode = GPIO_Mode_IN;
            gpio_conf.GPIO_PuPd = GPIO_PuPd_UP;
        }
        GPIO_Init(gpioCfg->port, &gpio_conf);
    }

    init_pushbutton_irq();
    return 1;
}

void EXTI9_5_IRQHandler(void)
{
    portBASE_TYPE xTaskWoken = pdFALSE;

    if (EXTI_GetITStatus(EXTI_Line8) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line8);
        xSemaphoreGiveFromISR(xOnPushbutton, &xTaskWoken);
        portEND_SWITCHING_ISR(xTaskWoken);
    }
}
