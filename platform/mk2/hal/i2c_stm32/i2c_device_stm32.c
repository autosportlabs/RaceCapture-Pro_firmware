/* 12c.c
 * Optimized DMA+Interrupt I2C Driver for the STM32F4
 * (c) 2014 AutosportLabs
 * Jeff Ciesielski <jeff@autosportlabs.com>
 */
#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <timers.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

#include <stm32f4xx.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_misc.h>
#include <stm32f4xx_dma.h>

#include <i2c_device_stm32.h>

struct rcc_params {
    void (*clock_cmd)(uint32_t, FunctionalState);
    uint32_t periph;
};

struct i2c_dma_stream {
    uint32_t channel;
    uint32_t irq_channel;
    DMA_Stream_TypeDef *stream;
    uint32_t all_flag_mask; /* required for clearing flags */
    uint32_t tc_flag;
};

struct i2c_pin {
    uint16_t pin_mask;
    uint8_t pin_source;
    GPIO_TypeDef *port;
    uint8_t pin_af;
    struct rcc_params rcc;
};

struct i2c_priv {
    bool initialized;
    I2C_TypeDef *ll_dev;
    xQueueHandle transact_q;
    xSemaphoreHandle transact_lock;
    xSemaphoreHandle event_lock;
    struct {
        struct i2c_pin sda;
        struct i2c_pin scl;
    } pins;
    struct {
        struct i2c_dma_stream rx;
        struct i2c_dma_stream tx;
        uint8_t *tx_buf;
        uint8_t *rx_buf;
    } dma;
    struct rcc_params rcc;
    uint8_t ev_irqn;
    uint8_t er_irqn;
    uint8_t trans_direction;
    uint8_t slave_addr;
    uint8_t tx_len;
    uint8_t rx_len;
    bool short_rx;
    bool error_flag;
};

static struct i2c_priv priv_drivers[] = {
#ifdef USE_DMA1
    {
        .initialized = false,
        .ll_dev = I2C1,
        .pins = {
            .sda = {
                .pin_mask = GPIO_Pin_7,
                .pin_source = GPIO_PinSource7,
                .port = GPIOB,
                .pin_af = GPIO_AF_I2C1,
                .rcc = {
                    .clock_cmd = RCC_AHB1PeriphClockCmd,
                    .periph = RCC_AHB1Periph_GPIOB,
                },
            },
            .scl = {
                .pin_mask = GPIO_Pin_6,
                .pin_source = GPIO_PinSource6,
                .port = GPIOB,
                .pin_af = GPIO_AF_I2C1,
                .rcc = {
                    .clock_cmd = RCC_AHB1PeriphClockCmd,
                    .periph = RCC_AHB1Periph_GPIOB,
                },
            }
        },
        .dma = {
            .rx = {
                .channel = DMA_Channel_1,
                .stream = DMA1_Stream0,
                .irq_channel = DMA1_Stream0_IRQn,
                .all_flag_mask = DMA_FLAG_FEIF0 | DMA_FLAG_DMEIF0 | DMA_FLAG_TEIF0 |
                DMA_FLAG_HTIF0 | DMA_FLAG_TCIF0,
                .tc_flag = DMA_IT_TCIF0,
            },
            .tx = {
                .channel = DMA_Channel_1,
                .stream = DMA1_Stream6,
                .irq_channel = DMA1_Stream6_IRQn,
                .all_flag_mask = DMA_FLAG_FEIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 |
                DMA_FLAG_HTIF6 | DMA_FLAG_TCIF6,
                .tc_flag = DMA_IT_TCIF6,
            },
        },
        .rcc = {
            .clock_cmd = RCC_APB1PeriphClockCmd,
            .periph = RCC_APB1Periph_I2C1,
        },
        .ev_irqn = I2C1_EV_IRQn,
        .er_irqn = I2C1_ER_IRQn,
        .tx_len = 0,
        .rx_len = 0,
        .error_flag = false,
    },
#endif	/* USE_DMA1 */
#ifdef USE_I2C2
    {
        .initialized = false,
        .ll_dev = I2C2,
        .pins = {
            .sda = {
                .pin_mask = GPIO_Pin_11,
                .pin_source = GPIO_PinSource11,
                .port = GPIOB,
                .pin_af = GPIO_AF_I2C2,
                .rcc = {
                    .clock_cmd = RCC_AHB1PeriphClockCmd,
                    .periph = RCC_AHB1Periph_GPIOB,
                },
            },
            .scl = {
                .pin_mask = GPIO_Pin_10,
                .pin_source = GPIO_PinSource10,
                .port = GPIOB,
                .pin_af = GPIO_AF_I2C2,
                .rcc = {
                    .clock_cmd = RCC_AHB1PeriphClockCmd,
                    .periph = RCC_AHB1Periph_GPIOB,
                },
            },
        },
        .dma = {
            .rx = {
                .channel = DMA_Channel_7,
                .stream = DMA1_Stream2,
                .irq_channel = DMA1_Stream2_IRQn,
                .all_flag_mask = DMA_FLAG_FEIF2 | DMA_FLAG_DMEIF2 | DMA_FLAG_TEIF2 |
                DMA_FLAG_HTIF2 | DMA_FLAG_TCIF2,
                .tc_flag = DMA_IT_TCIF2,

            },
            .tx = {
                .channel = DMA_Channel_7,
                .stream = DMA1_Stream7,
                .irq_channel = DMA1_Stream7_IRQn,
                .all_flag_mask = DMA_FLAG_FEIF7 | DMA_FLAG_DMEIF7 | DMA_FLAG_TEIF7 |
                DMA_FLAG_HTIF7 | DMA_FLAG_TCIF7,
                .tc_flag = DMA_IT_TCIF7,
            },
        },
        .rcc = {
            .clock_cmd = RCC_APB1PeriphClockCmd,
            .periph = RCC_APB1Periph_I2C2,
        },
        .ev_irqn = I2C2_EV_IRQn,
        .er_irqn = I2C2_ER_IRQn,
        .tx_len = 0,
        .rx_len = 0,
        .error_flag = false,
    }
#endif	/* USE_DMA2 */
};

static struct i2c_dev pub_drivers[] = {
#ifdef USE_DMA1
    {
        .priv = &priv_drivers[0],
    },
#endif	/* USE_DMA1 */
#ifdef USE_DMA2
    {
        .priv = &priv_drivers[1],
    }
#endif	/* USE_DMA2 */
};

struct i2c_dev *i2c_get_device(enum i2c_bus device)
{
    switch (device) {
#ifdef USE_DMA1
    case I2C_1:
            return &pub_drivers[0];
#endif	/* USE_DMA1 */
#ifdef USE_DMA2
    case I2C_2:
        return &pub_drivers[1];
#endif	/* USE_DMA2 */
    default:
        return NULL;
    }
}

int i2c_deinit(struct i2c_dev *dev)
{
    struct i2c_priv *p = (struct i2c_priv*)dev->priv;

    /* disable I2C device */
    I2C_Cmd(p->ll_dev, DISABLE);

    p->initialized = false;

    /* Disable interrupts for the device */
    I2C_ITConfig(p->ll_dev, I2C_IT_EVT, DISABLE);
    I2C_ITConfig(p->ll_dev, I2C_IT_BUF, DISABLE);
    I2C_ITConfig(p->ll_dev, I2C_IT_ERR, DISABLE);

    return 0;
}

static void i2c_dma_enable(struct i2c_priv *p)
{
    struct i2c_dma_stream *st;

    /* Get a pointer to the stream we'll be using */
    if (p->trans_direction == I2C_Direction_Receiver) {
        st = &p->dma.rx;
    } else {
        st = &p->dma.tx;
    }

    I2C_DMALastTransferCmd(p->ll_dev, ENABLE);

    /* Enable the DMA request */
    I2C_DMACmd(p->ll_dev, ENABLE);

    /* Enable the DMA RX Stream */
    DMA_Cmd(st->stream, ENABLE);
}

static void i2c_dma_disable(struct i2c_priv *p, struct i2c_dma_stream *st)
{
    /* DIsable dma transfers (and the last transfer bit) */
    I2C_DMALastTransferCmd(p->ll_dev, DISABLE);
    I2C_DMACmd(p->ll_dev, DISABLE);

    DMA_Cmd(st->stream, DISABLE);

    /* Clear any pending flags in the DMA controller, might be
     * redundant, but better than a lingering interrupt... */
    DMA_ClearFlag(st->stream, st->all_flag_mask);
}

static void i2c_dma_init(struct i2c_priv *p)
{
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    struct i2c_dma_stream *st;

    /* Configure the Priority Group to 2 bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* Get a pointer to the stream we'll be using */
    if (p->trans_direction == I2C_Direction_Receiver) {
        st = &p->dma.rx;
    } else {
        st = &p->dma.tx;
    }

    /* Clear any pending DMA Flags */
    DMA_ClearFlag(st->stream, st->all_flag_mask);

    /* Enable the I2Cx Tx DMA Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = st->irq_channel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    /* Reset the DMA stream */
    DMA_DeInit(st->stream);
    DMA_InitStructure.DMA_Channel = st->channel;

    /* DMA Direction, length, and buffer depend on whether we're receiving or transmitting */
    if (p->trans_direction == I2C_Direction_Receiver) {
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
        DMA_InitStructure.DMA_BufferSize = p->rx_len;
        DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)p->dma.rx_buf;
    } else {
        DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
        DMA_InitStructure.DMA_BufferSize = p->tx_len;
        DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)p->dma.tx_buf;
    }

    /* The rest of the DMA Hardware is set up the same way every time */
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &p->ll_dev->DR;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(st->stream, &DMA_InitStructure);

    /* Enable DMA Stream Transfer Complete interrupt */
    DMA_ITConfig(st->stream, DMA_IT_TC, ENABLE);

}

static void i2c_nvic_setup(struct i2c_priv *p)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* Enable the I2Cx Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = p->ev_irqn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable error interrupts */
    NVIC_InitStructure.NVIC_IRQChannel = p->er_irqn;
    NVIC_Init(&NVIC_InitStructure);

}

int i2c_init(struct i2c_dev *dev, uint32_t bus_speed)
{
    struct i2c_priv *p = (struct i2c_priv*)dev->priv;

    GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef I2C_InitStruct;

    if (p->initialized) {
        return -1;
    }

    /* First things first, make sure we're handed a sane bus speed */
    if (bus_speed < 100000 || bus_speed > 400000) {
        return -2;
    }

    /* initialize the gpio clocks */
    p->pins.sda.rcc.clock_cmd(p->pins.sda.rcc.periph, ENABLE);
    p->pins.scl.rcc.clock_cmd(p->pins.scl.rcc.periph, ENABLE);

    /* setup SCL and SDA pins
    * You can connect I2C1 to two different
     * pairs of pins:
     * 1. SCL on PB6 and SDA on PB7
     * 2. SCL on PB8 and SDA on PB9
     */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;

    /* It's necessary to keep SCL ahead of SDA, otherwise the bus
     * randomly thinks it's been told to start... Even when disabled. */
    GPIO_InitStruct.GPIO_Pin = p->pins.scl.pin_mask;
    GPIO_Init(p->pins.scl.port, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = p->pins.sda.pin_mask;
    GPIO_Init(p->pins.sda.port, &GPIO_InitStruct);

    /* Connect I2C device pins to AF   */
    GPIO_PinAFConfig(p->pins.sda.port,
                     p->pins.sda.pin_source,
                     p->pins.sda.pin_af);
    GPIO_PinAFConfig(p->pins.scl.port,
                     p->pins.scl.pin_source,
                     p->pins.scl.pin_af);

    /* configure device */
    p->rcc.clock_cmd(p->rcc.periph, ENABLE);

    I2C_Cmd(p->ll_dev, DISABLE);
    I2C_DeInit(p->ll_dev);
    I2C_StructInit(&I2C_InitStruct);

    I2C_InitStruct.I2C_ClockSpeed = bus_speed;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(p->ll_dev, &I2C_InitStruct);

    /* Now enable the freertos constructs */

    /* The transaction lock prevents multiple threads from attempting to utilize
     * the peripheral at the same time */
    vSemaphoreCreateBinary(p->transact_lock);

    /* The Event lock is used to block in between i2c events */
    vSemaphoreCreateBinary(p->event_lock);
    xSemaphoreTake(p->event_lock, portMAX_DELAY);

    /* We need a very small queue for reception <= 2 bytes */
    p->transact_q = xQueueCreate(2, sizeof(uint8_t));
    if (!p->transact_q) {
        return -3;
    }

    p->initialized = true;

    /* enable I2C device */
    I2C_Cmd(p->ll_dev, ENABLE);

    /* Set up NVIC parameters for this device (to enable interrupts)*/
    i2c_nvic_setup(p);

    /* Enable event and buffer interrupts */
    I2C_ITConfig(p->ll_dev, I2C_IT_EVT, ENABLE);
    I2C_ITConfig(p->ll_dev, I2C_IT_BUF, ENABLE);

    /* Enable the error interrupts */
    I2C_ITConfig(p->ll_dev, I2C_IT_ERR, ENABLE);

    /* Reset the bus */
    I2C_GenerateSTOP(p->ll_dev, ENABLE);

    return 0;
}

static void i2c_start(struct i2c_priv *p)
{

    /* No reason to block on a lock here.  This can only happen if
     * a device is misbehaving, but still good to check for it */
    while(I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_BUSY)) {
        vTaskDelay(1);
    }

    /* Make sure we'll ack the bytes we receive */
    I2C_AcknowledgeConfig(p->ll_dev, ENABLE);

    /* Clear the last event */
    I2C_GetLastEvent(p->ll_dev);

    /* Clear out the data register */
    I2C_ReceiveData(p->ll_dev);

    /* Clear the device error flag */
    p->error_flag = false;

    /* Clear the stop bit */
    I2C_GenerateSTOP(p->ll_dev, DISABLE);

    I2C_GenerateSTART(p->ll_dev, ENABLE);

    /* Once the start is generated, sending of the slave's
     * address is handled by the interrupt handler */

    /* Bus is now live, carry on */
}


int i2c_transact(struct i2c_dev *dev, uint8_t addr,
                 uint8_t *tx_buf, size_t tx_len,
                 uint8_t *rx_buf, size_t rx_len)
{
    int ret = 0;
    struct i2c_priv *p = (struct i2c_priv*)dev->priv;

    if (tx_len > 128 || rx_len > 128) {
        return -1;
    }

    /* Lock this transaction window */
    xSemaphoreTake(p->transact_lock, portMAX_DELAY);

    p->slave_addr = addr;
    p->tx_len = tx_len;
    p->rx_len = rx_len;

    p->dma.tx_buf = tx_buf;
    p->dma.rx_buf = rx_buf;

    /* For DMA Reception <= 2 bytes, we have to use interrupts
     * because of ST's poor I2C Hardware design, everything else
     * uses DMA */
    if (p->rx_len <= 2) {
        p->short_rx = true;
    } else {
        p->short_rx = false;
    }

    /* Set our transaction direction, if we're a transmitter, put
     * all data to be transmitted into the transaction queue */
    if (tx_len == 0) {
        p->trans_direction = I2C_Direction_Receiver;
    } else {
        p->trans_direction = I2C_Direction_Transmitter;
    }

    /* Kick off the transaction */
    i2c_start(p);

    /* Wait until the transmission completes, give it 1 solid
     * second and then bail */
    if (xSemaphoreTake(p->event_lock, 1000) == pdFALSE) {
        ret = -2;
        I2C_GenerateSTOP(p->ll_dev, ENABLE);
        goto unlock_and_return;
    }

    /* Make sure that the transfer was successful */
    if (p->error_flag) {
        ret = -2;
    }

    /* Move any Rx data into the rx buffer (Only for transactions
     * <= 2 bytes) */
    if (p->rx_len <= 2) {
        while (rx_len--)
            xQueueReceive(p->transact_q, rx_buf++, portMAX_DELAY);
    }

unlock_and_return:
    xSemaphoreGive(p->transact_lock);

    /* Return success */
    return ret;
}

int i2c_write_reg8(struct i2c_dev *dev, uint8_t dev_addr,
                   uint8_t reg_addr, uint8_t reg_val)
{
    int res;
    uint8_t payload[2];
    payload[0] = reg_addr;
    payload[1] = reg_val;
    res = i2c_transact(dev, dev_addr, payload, 2, NULL, 0);
    return res;
}

int i2c_read_reg8(struct i2c_dev *dev, uint8_t dev_addr,
                  uint8_t reg_addr, uint8_t *reg_val)
{
    int res;
    res = i2c_transact(dev, dev_addr,
                       &reg_addr, 1,
                       reg_val, 1);
    return res;
}

int i2c_read_mem_block(struct i2c_dev *dev, uint8_t dev_addr,
                       uint8_t mem_addr, uint8_t *mem_buf,
                       size_t mem_len)
{
    int res;
    res = i2c_transact(dev, dev_addr,
                       &mem_addr, 1,
                       mem_buf, mem_len);
    return res;
}

int i2c_read_reg_bits(struct i2c_dev *dev, uint8_t dev_addr,
                      uint8_t reg_addr, size_t bit_pos,
                      size_t num_bits, uint8_t *bit_val)
{
    int res;
    uint8_t reg_val;
    uint8_t reg_mask = 0x00;

    /* We need to make sure that the mask won't run off the edge
     * of the register */
    if (bit_pos + num_bits > 8) {
        return -1;
    }

    res = i2c_read_reg8(dev, dev_addr, reg_addr, &reg_val);
    if (res) {
        return res;
    }

    /* Build the register mask */
    for (size_t i = 0; i < num_bits; i++)
        reg_mask |= 1 << (i + bit_pos);

    /* Mask off just the part we care about */
    reg_val &= reg_mask;

    /* Return the newly masked value shifted back to 0 */
    *bit_val = reg_val >> bit_pos;

    return 0;
}

int i2c_write_reg_bits(struct i2c_dev *dev, uint8_t dev_addr,
                       uint8_t reg_addr, size_t bit_pos,
                       size_t num_bits, uint8_t bit_val)
{
    int res;
    uint8_t reg_val;
    uint8_t reg_mask = 0x00;

    /* We need to make sure that the mask won't run off the edge
     * of the register */
    if (bit_pos + num_bits > 8) {
        return -1;
    }

    res = i2c_read_reg8(dev, dev_addr, reg_addr, &reg_val);
    if (res) {
        return res;
    }

    /* Build the register mask */
    for (size_t i = 0; i < num_bits; i++)
        reg_mask |= 1 << (i + bit_pos);

    /* Clear the bits we're going to be writing */
    reg_val &= ~(reg_mask);

    /* now, write the new bit values into their appropriate place
     * in the register */
    reg_val |= (bit_val << bit_pos) & reg_mask;

    /* Write the register back to the device */
    res = i2c_write_reg8(dev, dev_addr, reg_addr, reg_val);
    if (res) {
        return res;
    }

    return 0;
}

int i2c_write_raw(struct i2c_dev *dev, uint8_t dev_addr, uint8_t *buf, size_t len)
{
    int res = i2c_transact(dev, dev_addr, buf, len, NULL, 0);

    return res;
}

int i2c_read_raw(struct i2c_dev *dev, uint8_t dev_addr, uint8_t *buf, size_t len)
{
    int res = i2c_transact(dev, dev_addr, NULL, 0, buf, len);

    return res;
}

/*** Interrupt handlers and helper functions ***/

static void i2c_rx_data(struct i2c_priv *p)
{
    /* This function is ONLY used if the I2C Device is receiving
     * <= 2 bytes. */
    portBASE_TYPE q_ret, task_woken;
    if (p->rx_len && !p->error_flag) {
        uint8_t rx_byte;

        /* Grab the byte out of the buffer and place
         * it in the transaction queue */
        rx_byte = I2C_ReceiveData(p->ll_dev);
        q_ret = xQueueSendFromISR(p->transact_q, &rx_byte, &task_woken);
        /* Decrement the amount of data we have left
         * to receive */
        p->rx_len--;

        /* If there was nothing there, something went
         * horribly wrong */
        if (q_ret != pdTRUE) {
            p->error_flag = true;
            return;
        }

        /* If this is the next to last byte, we need
         * to tell the hardware to NAK the next byte
         * and generate a stop condition */
        if (p->rx_len == 1) {
            I2C_AcknowledgeConfig(p->ll_dev, DISABLE);
            I2C_NACKPositionConfig(p->ll_dev, I2C_NACKPosition_Current);
            I2C_GenerateSTOP(p->ll_dev, ENABLE);
        }


        /* Don't give up our lock until all data has
         * been received */
        if (p->rx_len) {
            return;
        }
    }

    xSemaphoreGiveFromISR(p->event_lock, &task_woken);
    portEND_SWITCHING_ISR(task_woken);

}

static void i2c_initiate_transfer(struct i2c_priv *p)
{
    /* Clear out any pending I2C Flags */
    I2C_GetLastEvent(p->ll_dev);

    /* If this is a RX <= 2 bytes, we will use interrupts.
     * Otherwise, we'll need to initialize the DMA Controller */
    if (!(p->trans_direction == I2C_Direction_Receiver && p->rx_len <=2))
        i2c_dma_init(p);

    /* Send the slave address to initialize the transaction */
    I2C_Send7bitAddress(p->ll_dev,
                        p->slave_addr,
                        p->trans_direction);
}

static void i2c_clear_addr(struct i2c_priv *p)
{
    /* Another workaround for ST's wonky hardware.  We need to
     * explicitely set up the NACK if this is a single byte reception */
    if (p->rx_len == 1 && p->trans_direction == I2C_Direction_Receiver) {
        I2C_AcknowledgeConfig(p->ll_dev, DISABLE);
        I2C_NACKPositionConfig(p->ll_dev, I2C_NACKPosition_Current);
    }

    /* This clears the ADDR Bit */
    I2C_GetLastEvent(p->ll_dev);

    /* Yet ANOTHER workaround.  We have to attempt to generate
     * the stop bit early for 1 byte reception... If this is a RX
     * > 2 bytes OR a transmission, enable DMA */
    if (p->rx_len == 1 && p->trans_direction == I2C_Direction_Receiver) {
        I2C_GenerateSTOP(p->ll_dev, ENABLE);
    } else if (p->trans_direction == I2C_Direction_Transmitter || p->rx_len > 2) {
        i2c_dma_enable(p);
    }
}

static void i2c_setup_rx(struct i2c_priv *p)
{
    /* Disable TX DMA stream, change our direction, and generate
     * a start condition */
    i2c_dma_disable(p, &p->dma.tx);
    p->trans_direction = I2C_Direction_Receiver;
    I2C_GenerateSTART(p->ll_dev, ENABLE);
}

static void i2c_common_event_handler(struct i2c_priv *p)
{
    portBASE_TYPE task_woken = pdFALSE;

    /* We successfully sent a start bit */
    if (I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_SB)) {
        i2c_initiate_transfer(p);
    }

    /* We sent a slave address and were approved for the transmission */
    if (I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_ADDR)) {
        i2c_clear_addr(p);
    }

    /* For RX <= 2 bytes, we need to manually pull the data out
     * of the controller */
    if (I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_RXNE) && p->short_rx) {
        i2c_rx_data(p);
    }

    /* This is a TX Complete event, meaning the DMA Controller
     * has finished it's work.  If there is nothing to receive,
     * clear the event lock, otherwise, set up the RX Transaction */
    if (I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_BTF)) {
        if (p->rx_len == 0) {
            I2C_GenerateSTOP(p->ll_dev, ENABLE);
            xSemaphoreGiveFromISR(p->event_lock, &task_woken);
            portEND_SWITCHING_ISR(task_woken);
        } else {
            i2c_setup_rx(p);
        }
    }

    /* Generate a stop if there is an error detected */
    if (p->error_flag) {
        I2C_GenerateSTOP(p->ll_dev, ENABLE);
    }
}

static void i2c_common_error_handler(struct i2c_priv *p)
{
    portBASE_TYPE task_woken = pdFALSE;

    /* Read SRs to clear them */
    I2C_GetLastEvent(p->ll_dev);

    /* Make sure that the SR1 register is clear */
    p->ll_dev->SR1 = 0;

    /* Send stop */
    I2C_GenerateSTOP(p->ll_dev, ENABLE);

    /* Kill both RX and TX DMA Streams */
    i2c_dma_disable(p, &p->dma.rx);
    i2c_dma_disable(p, &p->dma.tx);

    p->error_flag = true;

    xSemaphoreGiveFromISR(p->event_lock, &task_woken);
    portEND_SWITCHING_ISR(task_woken);
}

static void i2c_common_dma_handler(struct i2c_priv *p, struct i2c_dma_stream *st)
{
    portBASE_TYPE task_woken = pdFALSE;

    /* If the DMA Stream has finished it's work, we're in the
     * clear and can clear the interrupt flag */
    if (DMA_GetITStatus(st->stream, st->tc_flag)) {
        DMA_ClearITPendingBit(st->stream, st->tc_flag);

        /* For reception > 2 bytes, we need to generate a
         * stop and clear the event lock (Meaning that the
         * transaction is complete) */
        if (p->trans_direction == I2C_Direction_Receiver) {
            I2C_GenerateSTOP(p->ll_dev, ENABLE);
            xSemaphoreGiveFromISR(p->event_lock, &task_woken);
            portEND_SWITCHING_ISR(task_woken);
        }
        /* Turn off this DMA stream */
        i2c_dma_disable(p, st);
    }
}

/* EVENT/ERROR HANDLERS */
#ifdef USE_DMA1
void I2C1_EV_IRQHandler(void)
{
    i2c_common_event_handler(&priv_drivers[0]);
}

void I2C1_ER_IRQHandler(void)
{
    i2c_common_error_handler(&priv_drivers[0]);
}
#endif	/* USE_DMA1 */
#ifdef USE_DMA2
void I2C2_EV_IRQHandler(void)
{
    i2c_common_event_handler(&priv_drivers[1]);
}


void I2C2_ER_IRQHandler(void)
{
    i2c_common_error_handler(&priv_drivers[1]);
}
#endif	/* USE_DMA2 */

/* DMA HANDLERS */
#ifdef USE_DMA1
/* I2C1 TX DMA TC Interrupt */
void DMA1_Stream6_IRQHandler(void)
{
    i2c_common_dma_handler(&priv_drivers[0], &priv_drivers[0].dma.tx);
}

/* I2C1 RX DMA TC Interrupt */
void DMA1_Stream0_IRQHandler(void)
{
    i2c_common_dma_handler(&priv_drivers[0], &priv_drivers[0].dma.rx);
}
#endif	/* USE_DMA1 */
#ifdef USE_DMA2
void DMA1_Stream2_IRQHandler(void)
{
    i2c_common_dma_handler(&priv_drivers[1], &priv_drivers[1].dma.tx);
}

void DMA1_Stream7IRQHandler(void)
{
    i2c_common_dma_handler(&priv_drivers[1], &priv_drivers[1].dma.tx);
}
#endif	/* USE_DMA2 */
