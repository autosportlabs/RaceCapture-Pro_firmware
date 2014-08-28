/* 12c.h
 * Interrupt + DMA i2c driver for the STM32F4xx
 * (c) 2014 AutosportLabs
 * Jeff Ciesielski <jeff@autosportlabs.com>
 */

#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>
#include <stdbool.h>

struct i2c_dev {
	/* This is just a convenient public handle for the opaque
	 * driver internals...Nothing to see here. Move along. */
	void *priv;
};

enum i2c_bus {
	I2C_1,
	I2C_2,
};

enum i2c_direction {
	I2C_DIR_RX,
	I2C_DIR_TX,
};

struct i2c_dev *i2c_get_device(enum i2c_bus device);
int i2c_init(struct i2c_dev *dev, uint32_t bus_speed);

int i2c_write(struct i2c_dev *dev, uint8_t addr, uint8_t *buf, int len);
int i2c_transact(struct i2c_dev *dev, uint8_t addr,
		 uint8_t *tx_buf, int tx_len,
		 uint8_t *rx_buf, int rx_len);
int i2c_read(struct i2c_dev *dev, uint8_t addr, uint8_t *buf, int len);

void i2c_lock(struct i2c_dev *dev);
void i2c_unlock (struct i2c_dev *dev);

#endif /* I2C_H */

