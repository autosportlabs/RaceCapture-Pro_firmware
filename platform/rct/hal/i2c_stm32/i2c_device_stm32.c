/*********************************************************************************/
/* The MIT License (MIT)							 */
/* 										 */
/* Copyright (c)  2015  Jeff Ciesielski <jeffciesielski@gmail.com>		 */
/* 										 */
/* Permission is hereby granted, free of charge, to any person obtaining a copy	 */
/* of this software and associated documentation files (the "Software"), to deal */
/* in the Software without restriction, including without limitation the rights	 */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell	 */
/* copies of the Software, and to permit persons to whom the Software is	 */
/* furnished to do so, subject to the following conditions:			 */
/* 										 */
/* The above copyright notice and this permission notice shall be included in	 */
/* all copies or substantial portions of the Software.				 */
/* 										 */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR	 */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,	 */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE	 */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER	 */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN	 */
/* THE SOFTWARE.								 */
/*********************************************************************************/
/* 12c.c
 * Interrupt I2C Driver for the STM32F3
 */
#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <timers.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

#include <stm32f30x.h>
#include <stm32f30x_i2c.h>
#include <stm32f30x_gpio.h>
#include <stm32f30x_rcc.h>
#include <stm32f30x_misc.h>

#include <i2c_device_stm32.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define I2C_IRQ_PRIORITY 5

struct rcc_params {
	void (*clock_cmd)(uint32_t, FunctionalState);
	uint32_t periph;
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
	uint8_t *tx_buf;
	uint8_t *rx_buf;
	struct {
		struct i2c_pin sda;
		struct i2c_pin scl;
	} pins;
	struct rcc_params rcc;
	uint8_t ev_irqn;
	uint8_t er_irqn;
	uint16_t trans_direction;
	uint8_t slave_addr;
	uint8_t tx_len;
	uint8_t tx_total;
	uint8_t rx_len;
	uint8_t rx_total;
	bool error_flag;
};

static struct i2c_priv priv_drivers[] = {
#ifdef USE_I2C1
	{
		.initialized = false,
		.ll_dev = I2C1,
		.pins = {
			.sda = {
				.pin_mask = GPIO_Pin_7,
				.pin_source = GPIO_PinSource7,
				.port = GPIOB,
				.pin_af = GPIO_AF_4,
				.rcc = {
					.clock_cmd = RCC_AHBPeriphClockCmd,
					.periph = RCC_AHBPeriph_GPIOB,
				},
			},
			.scl = {
				.pin_mask = GPIO_Pin_6,
				.pin_source = GPIO_PinSource6,
				.port = GPIOB,
				.pin_af = GPIO_AF_4,
				.rcc = {
					.clock_cmd = RCC_AHBPeriphClockCmd,
					.periph = RCC_AHBPeriph_GPIOB,
				},
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
#endif	/* USE_I2C1 */
#ifdef USE_I2C2
	{
		.initialized = false,
		.ll_dev = I2C2,
		.pins = {
			.sda = {
				.pin_mask = GPIO_Pin_11,
				.pin_source = GPIO_PinSource11,
				.port = GPIOB,
				.pin_af = GPIO_AF_4,
				.rcc = {
					.clock_cmd = RCC_AHB1PeriphClockCmd,
					.periph = RCC_AHB1Periph_GPIOB,
				},
			},
			.scl = {
				.pin_mask = GPIO_Pin_10,
				.pin_source = GPIO_PinSource10,
				.port = GPIOB,
				.pin_af = GPIO_AF_4,
				.rcc = {
					.clock_cmd = RCC_AHB1PeriphClockCmd,
					.periph = RCC_AHB1Periph_GPIOB,
				},
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
#ifdef USE_I2C1
	{
		.priv = &priv_drivers[0],
	},
#endif	/* USE_I2C1 */
#ifdef USE_I2C2
	{
		.priv = &priv_drivers[1],
	}
#endif	/* USE_I2C2 */
};

struct i2c_timingr_setting {
	uint32_t bus_speed;
	uint32_t periph_clk;
	uint32_t timingr_val;
};

#define TIMINGR_FROM_DS_TABLE(presc, scll, sclh, sdadel, scldel)	\
	(((presc << 28) & I2C_TIMINGR_PRESC) | (scll & I2C_TIMINGR_SCLL) | \
	 ((sclh << 8) & I2C_TIMINGR_SCLH) | ((sdadel << 16) & I2C_TIMINGR_SDADEL) | \
	 ((scldel << 20) & I2C_TIMINGR_SCLDEL))

/*
 * These values are pulled from section 28.4.9 of the STM32F3xx
 * reference manual
 */
static struct i2c_timingr_setting timingr_map[] = {
	{
		.bus_speed = 100000,
		.periph_clk = 8000000,
		.timingr_val = TIMINGR_FROM_DS_TABLE(1, 0x13, 0xf, 0x2, 0x4),
	},
	{
		.bus_speed = 400000,
		.periph_clk = 8000000,
		.timingr_val = TIMINGR_FROM_DS_TABLE(0, 0x09, 0x3, 0x1, 0x3),
	},
	{
		.bus_speed = 100000,
		.periph_clk = 48000000,
		.timingr_val = TIMINGR_FROM_DS_TABLE(0xb, 0x13, 0xf, 0x2, 0x4),
	},
	{
		.bus_speed = 400000,
		.periph_clk = 48000000,
		.timingr_val = TIMINGR_FROM_DS_TABLE(5, 0x9, 0x3, 0x3, 0x3),
	},
};

struct i2c_dev *i2c_get_device(enum i2c_bus device)
{
	switch (device) {
#ifdef USE_I2C1
	case I2C_1:
		return &pub_drivers[0];
#endif	/* USE_I2C1 */
#ifdef USE_I2C2
	case I2C_2:
		return &pub_drivers[1];
#endif	/* USE_I2C2 */
	default:
		return NULL;
	}
}

static void i2c_irq_set_state(struct i2c_priv *p, FunctionalState state)
{
	I2C_ITConfig(p->ll_dev, I2C_IT_ERRI, state);
	I2C_ITConfig(p->ll_dev, I2C_IT_TCI, state);
	I2C_ITConfig(p->ll_dev, I2C_IT_STOPF, state);
	I2C_ITConfig(p->ll_dev, I2C_IT_NACKI, state);
	I2C_ITConfig(p->ll_dev, I2C_IT_ADDRI, state);
	I2C_ITConfig(p->ll_dev, I2C_IT_RXI, state);
	I2C_ITConfig(p->ll_dev, I2C_IT_TXI, state);
}

int i2c_deinit(struct i2c_dev *dev)
{
	struct i2c_priv *p = (struct i2c_priv*)dev->priv;

	/* Disable interrupts for the device */
	i2c_irq_set_state(p, DISABLE);

	/* disable I2C device */
	I2C_Cmd(p->ll_dev, DISABLE);

	p->initialized = false;

	return 0;
}

static void i2c_nvic_setup(struct i2c_priv *p)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* Enable the I2Cx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = p->ev_irqn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = I2C_IRQ_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable error interrupts */
	NVIC_InitStructure.NVIC_IRQChannel = p->er_irqn;
	NVIC_Init(&NVIC_InitStructure);
}

static int i2c_get_timing(I2C_TypeDef *periph, uint32_t bus_speed,
			  uint32_t *timingr)
{
	RCC_ClocksTypeDef clocks;
	uint32_t periph_clk;
	int res = -1;

	RCC_GetClocksFreq(&clocks);

	if (periph == I2C1) {
		periph_clk = clocks.I2C1CLK_Frequency;
	} else if (periph == I2C2) {
		periph_clk = clocks.I2C2CLK_Frequency;
	} else {
                return -1;
        }

	for (int i = 0; i < ARRAY_SIZE(timingr_map); i++) {
		if ((bus_speed == timingr_map[i].bus_speed) &&
		    (periph_clk == timingr_map[i].periph_clk)) {
			*timingr = timingr_map[i].timingr_val;
			res = 0;
			break;
		}
	}
	return res;
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
	if (bus_speed != 100000 && bus_speed != 400000) {
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

	uint32_t timingr;

	if (i2c_get_timing(p->ll_dev, bus_speed, &timingr)) {
		return -3;
	}

	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_Timing = timingr;
	I2C_Init(p->ll_dev, &I2C_InitStruct);

	/* Now enable the freertos constructs */

	/* The transaction lock prevents multiple threads from attempting to utilize
	 * the peripheral at the same time */
	vSemaphoreCreateBinary(p->transact_lock);

	/* The Event lock is used to block in between i2c events */
	vSemaphoreCreateBinary(p->event_lock);
	xSemaphoreTake(p->event_lock, portMAX_DELAY);

	p->initialized = true;

	/* enable I2C device */
	I2C_Cmd(p->ll_dev, ENABLE);

	/* Set up NVIC parameters for this device (to enable interrupts)*/
	i2c_nvic_setup(p);

	/* Enable the interrupts */
	i2c_irq_set_state(p, ENABLE);

	return 0;
}

static void i2c_start(struct i2c_priv *p)
{

	uint8_t num_bytes;
	uint32_t endmode;
	uint32_t startmode;

	/* No reason to block on a lock here.  This can only happen if
	 * a device is misbehaving, but still good to check for it */
	while(I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_BUSY)) {
		vTaskDelay(1);
	}

	/* Clear out Stop Signal */
	I2C_GenerateSTOP(p->ll_dev, DISABLE);

	/* Clear the device error flag */
	p->error_flag = false;

	/* Set up the automated end mode of this transmission.  If
	 * we're just transmitting (or receiving), we'll let the
	 * controller set the stop bit.  If we have to do a complete
	 * transaction (i.e. writing an address then reading it's
	 * contents) we'll want a soft stop so that we can re-start
	 */
	if (p->trans_direction == I2C_Direction_Transmitter) {
		num_bytes = p->tx_len;
		if (p->rx_len > 0) {
			endmode = I2C_SoftEnd_Mode;
		} else {
			endmode = I2C_AutoEnd_Mode;
		}
		startmode = I2C_Generate_Start_Write;
	} else {
		num_bytes = p->rx_len;
		endmode = I2C_AutoEnd_Mode;
		startmode = I2C_Generate_Start_Read;
	}

	I2C_TransferHandling(p->ll_dev, p->slave_addr,
			     num_bytes, endmode, startmode);

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

	if (tx_len > 256 || rx_len > 256) {
		return -1;
	}

	/* Lock this transaction window */
	xSemaphoreTake(p->transact_lock, portMAX_DELAY);

	p->slave_addr = addr;
	p->tx_len = tx_len;
	p->rx_len = rx_len;

	p->tx_buf = tx_buf;
	p->rx_buf = rx_buf;

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

unlock_and_return:
	xSemaphoreGive(p->transact_lock);

	/* Return */
	return ret;
}

/* The following are helper functions to wrap common functionality
 * such as reading and writing registers */
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
	for (size_t i = 0; i < num_bits; i++) {
		reg_mask |= 1 << (i + bit_pos);
	}

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
static void i2c_common_event_handler(struct i2c_priv *p)
{
	portBASE_TYPE task_woken = pdFALSE;

	/* We sent a slave address and were approved for the transmission */
	if (I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_ADDR)) {
		I2C_ClearFlag(p->ll_dev, I2C_FLAG_ADDR);
	} else if (I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_NACKF)) {
		/* If we got here, we got NACK'd, meaning there is no
		 * device with the target address on the bus, this is
		 * an error */
		I2C_ClearFlag(p->ll_dev, I2C_FLAG_NACKF);
		I2C_GenerateSTOP(p->ll_dev, ENABLE);
		p->error_flag = true;
	}

	/* TX Buffer is empty */
	if (I2C_GetITStatus(p->ll_dev, I2C_IT_TXIS) && p->tx_len) {
		I2C_SendData(p->ll_dev, *(p->tx_buf++));
		p->tx_len--;
	}

	/* RX Buffer is Full and needs drained */
	if (I2C_GetITStatus(p->ll_dev, I2C_IT_RXNE)) {
		*(p->rx_buf++) = I2C_ReceiveData(p->ll_dev);
		p->rx_len--;
	}

	/* This is a transaction complete event, meaning the Byte
	 * counter in the controller has hit its limit.
	 * If there is something to receive, set up the RX
	 * transaction
	 */
	if (I2C_GetITStatus(p->ll_dev, I2C_IT_TC)) {
		if (p->trans_direction == I2C_Direction_Transmitter && p->rx_len) {
			p->trans_direction = I2C_Direction_Receiver;
			I2C_TransferHandling(p->ll_dev, p->slave_addr,
					     p->rx_len, I2C_AutoEnd_Mode,
					     I2C_Generate_Start_Read);
		}
	}

	/* A stop bit was detected on the bus, this is the end of our transaction */
	if (I2C_GetITStatus(p->ll_dev, I2C_IT_STOPF)) {
		I2C_ClearITPendingBit(p->ll_dev, I2C_IT_STOPF);
		xSemaphoreGiveFromISR(p->event_lock, &task_woken);
		portEND_SWITCHING_ISR(task_woken);
	}
}

static void i2c_common_error_handler(struct i2c_priv *p)
{
	portBASE_TYPE task_woken = pdFALSE;

	/* Send stop */
	I2C_GenerateSTOP(p->ll_dev, ENABLE);

	/* Disable any additional ISRs */
	i2c_irq_set_state(p, DISABLE);

	p->error_flag = true;

	xSemaphoreGiveFromISR(p->event_lock, &task_woken);
	portEND_SWITCHING_ISR(task_woken);
}

/* EVENT/ERROR HANDLERS */
#ifdef USE_I2C1
void I2C1_EV_IRQHandler(void)
{
	i2c_common_event_handler(&priv_drivers[0]);
}

void I2C1_ER_IRQHandler(void)
{
	i2c_common_error_handler(&priv_drivers[0]);
}
#endif	/* USE_I2C1 */
#ifdef USE_I2C2
void I2C2_EV_IRQHandler(void)
{
	i2c_common_event_handler(&priv_drivers[1]);
}


void I2C2_ER_IRQHandler(void)
{
	i2c_common_error_handler(&priv_drivers[1]);
}
#endif	/* USE_I2C2 */
