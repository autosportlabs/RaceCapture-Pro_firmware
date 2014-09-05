/* 12c.c
 * A simple interrupt driven I2C driver for the stm32f4
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

#include <i2c_device_stm32.h>

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
	struct {
		struct i2c_pin sda;
		struct i2c_pin scl;
	} pins;
	struct rcc_params rcc;
	uint8_t ev_irqn;
	uint8_t er_irqn;
	uint8_t trans_direction;
	uint8_t slave_addr;
	uint8_t tx_len;
	uint8_t rx_len;
	bool gen_stop;
	bool error_flag;
};

static struct i2c_priv priv_drivers[] = {
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
			}
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
};

static struct i2c_dev pub_drivers[] = {
	{
		.priv = &priv_drivers[0],
	},
	{
		.priv = &priv_drivers[1],
	}
};

struct i2c_dev *i2c_get_device(enum i2c_bus device)
{
	switch (device) {
	case I2C_1:
		return &pub_drivers[0];
	case I2C_2:
		return &pub_drivers[1];
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

	if (p->initialized)
		return -1;

	/* First things first, make sure we're handed a sane bus speed */
	if (bus_speed < 100000 || bus_speed > 400000)
		return -2;

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
	I2C_DeInit(p->ll_dev);	I2C_StructInit(&I2C_InitStruct);

	I2C_InitStruct.I2C_ClockSpeed = bus_speed;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(p->ll_dev, &I2C_InitStruct);

	/* Now enable the freertos constructs */

	/* The transaction lock prevents multiple threads from attempting to utilize
	 * the peripheral at the same time */
	vSemaphoreCreateBinary(p->transact_lock);

	/* The Event lock is used to block in between i2c events */
	vSemaphoreCreateBinary(p->event_lock);
	xSemaphoreTake(p->event_lock, portMAX_DELAY);

	p->transact_q = xQueueCreate(128, sizeof(uint8_t));
	if (!p->transact_q)
		return -3;

	p->initialized = true;

	/* enable I2C device */
	I2C_Cmd(p->ll_dev, ENABLE);

	/* Set up NVIC parameters for this device (to enable interrupts)*/
	i2c_nvic_setup(p);

	/* Enable the event and buffer interrupts */
	I2C_ITConfig(p->ll_dev, I2C_IT_EVT, ENABLE);
	I2C_ITConfig(p->ll_dev, I2C_IT_BUF, ENABLE);
	I2C_ITConfig(p->ll_dev, I2C_IT_ERR, ENABLE);

	return 0;
}

static void i2c_start(struct i2c_priv *p)
{
	/* No reason to block on a lock here.  This can only happen if
	 * a device is misbehaving, but still good to check for it */
	while(I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_BUSY))
		vTaskDelay(1);

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

	if (tx_len > 128 || rx_len > 128)
		return -1;

	/* Lock this transaction window */
	xSemaphoreTake(p->transact_lock, portMAX_DELAY);

	p->slave_addr = addr;
	p->tx_len = tx_len;
	p->rx_len = rx_len;

	/* Clear the transaction queue */
	xQueueReset(p->transact_q);

	/* Set our transaction direction, if we're a transmitter, put
	 * all data to be transmitted into the transaction queue */
	if (tx_len == 0) {
		p->trans_direction = I2C_Direction_Receiver;
	} else {
		/* Move all of the tx data into the device's queue */
		for (int i = 0; i < tx_len; i++)
			xQueueSend(p->transact_q, tx_buf++, portMAX_DELAY);
		p->trans_direction = I2C_Direction_Transmitter;
	}

	/* Kick off the transaction */
	i2c_start(p);

	/* Wait until the transmission completes, give it 1 solid
	 * second and then bail */
	xSemaphoreTake(p->event_lock, 1000);

	/* Make sure that the transfer was successful */
	if (p->error_flag) {
		xQueueReset(p->transact_q);
		ret = -2;
		goto unlock_and_return;
	}

	/* Move any Rx data into the rx buffer */
	while (rx_len--)
		xQueueReceive(p->transact_q, rx_buf++, portMAX_DELAY);

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
	if (bit_pos + num_bits > 8)
		return -1;

	res = i2c_read_reg8(dev, dev_addr, reg_addr, &reg_val);
	if (res)
		return res;

	/* Build the register mask */
	for (int i = 0; i < num_bits; i++)
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
	if (bit_pos + num_bits > 8)
		return -1;

	res = i2c_read_reg8(dev, dev_addr, reg_addr, &reg_val);
	if (res)
		return res;

	/* Build the register mask */
	for (int i = 0; i < num_bits; i++)
		reg_mask |= 1 << (i + bit_pos);

	/* Clear the bits we're going to be writing */
	reg_val &= ~(reg_mask);

	/* now, write the new bit values into their appropriate place
	 * in the register */
	reg_val |= (bit_val << bit_pos) & reg_mask;

	/* Write the register back to the device */
	res = i2c_write_reg8(dev, dev_addr, reg_addr, reg_val);
	if (res)
		return res;

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

static void i2c_stage_tx_byte(struct i2c_priv *p)
{
	portBASE_TYPE q_ret, task_woken;

	/* If we have data remaining and are supposed to be
	 * here, transmit the next byte */
	if (p->tx_len && !p->error_flag) {
		uint8_t tx_byte;

		/* Grab the next item out of the transaction queue */
		q_ret = xQueueReceiveFromISR(p->transact_q, &tx_byte, &task_woken);

		/* If there was nothing there, something went
		 * horribly wrong */
		if (q_ret != pdTRUE) {
			p->error_flag = true;
			return;
		}


		/* Send the next byte */
		I2C_SendData(p->ll_dev, tx_byte);

		/* Decrement the amount of data we have left
		 * to transmit */
		p->tx_len--;

		/* Don't give up our lock until all data has
		 * been transmitted */
		return;
	}

	/* If there is nothing to receive, generate a stop,
	 * otherwise, generate another start and move along */
	if (!p->rx_len) {
		I2C_GenerateSTOP(p->ll_dev, ENABLE);
		xSemaphoreGiveFromISR(p->event_lock, &task_woken);
		portEND_SWITCHING_ISR(task_woken);
	} else {
		p->trans_direction = I2C_Direction_Receiver;
		I2C_AcknowledgeConfig(p->ll_dev, ENABLE);
		I2C_GenerateSTART(p->ll_dev, ENABLE);
	}


}

static void i2c_rx_data(struct i2c_priv *p)
{
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
		if (p->rx_len)
			return;
	}

	xSemaphoreGiveFromISR(p->event_lock, &task_woken);
	portEND_SWITCHING_ISR(task_woken);

}

static void i2c_clear_addr(struct i2c_priv *p)
{
	if (p->rx_len == 1 && p->trans_direction == I2C_Direction_Receiver) {
		I2C_AcknowledgeConfig(p->ll_dev, DISABLE);
		I2C_NACKPositionConfig(p->ll_dev, I2C_NACKPosition_Current);
	}

	/* This clears the ADDR Bit */
	I2C_GetLastEvent(p->ll_dev);

	if (p->rx_len == 1 && p->trans_direction == I2C_Direction_Receiver) {
		I2C_GenerateSTOP(p->ll_dev, ENABLE);
	} else if (p->trans_direction == I2C_Direction_Transmitter) {
		i2c_stage_tx_byte(p);
	}
}

static void i2c_common_event_handler(struct i2c_priv *p)
{
	if (I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_SB)) {
		I2C_GetLastEvent(p->ll_dev);
		I2C_Send7bitAddress(p->ll_dev,
				    p->slave_addr,
				    p->trans_direction);
	}

	if (I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_ADDR)) {
		i2c_clear_addr(p);
	}
	if (I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_RXNE)) {
		i2c_rx_data(p);
	}
	if (I2C_GetFlagStatus(p->ll_dev, I2C_FLAG_TXE)) {
			i2c_stage_tx_byte(p);
	}

	/* Generate a stop if there is an error detected */
	if (p->error_flag)
		I2C_GenerateSTOP(p->ll_dev, ENABLE);
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

	p->error_flag = true;

	xSemaphoreGiveFromISR(p->event_lock, &task_woken);
	portEND_SWITCHING_ISR(task_woken);
}

/* TODO: error Interrupt handlers */
void I2C1_EV_IRQHandler(void)
{
	i2c_common_event_handler(&priv_drivers[0]);
}

void I2C2_EV_IRQHandler(void)
{
	i2c_common_event_handler(&priv_drivers[1]);
}

void I2C1_ER_IRQHandler(void)
{
	i2c_common_error_handler(&priv_drivers[0]);
}

void I2C2_ER_IRQHandler(void)
{
	i2c_common_error_handler(&priv_drivers[1]);
}
