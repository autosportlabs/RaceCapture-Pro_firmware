/* 12c.h
 * Interrupt + DMA i2c driver for the STM32F4xx
 * (c) 2014 AutosportLabs
 * Jeff Ciesielski <jeff@autosportlabs.com>
 */

#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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
int i2c_transact(struct i2c_dev *dev, uint8_t addr,
                 uint8_t *tx_buf, size_t tx_len,
                 uint8_t *rx_buf, size_t rx_len);
int i2c_read_reg8(struct i2c_dev *dev, uint8_t dev_addr,
                  uint8_t reg_addr, uint8_t *reg_val);
int i2c_write_reg8(struct i2c_dev *dev, uint8_t dev_addr,
                   uint8_t reg_addr, uint8_t reg_val);
int i2c_write_reg_bits(struct i2c_dev *dev, uint8_t dev_addr,
                       uint8_t reg_addr, size_t bit_pos,
                       size_t num_bits, uint8_t bit_val);
int i2c_read_reg_bits(struct i2c_dev *dev, uint8_t dev_addr,
                      uint8_t reg_addr, size_t bit_pos,
                      size_t num_bits, uint8_t *bit_val);
int i2c_read_mem_block(struct i2c_dev *dev, uint8_t dev_addr,
                       uint8_t mem_addr, uint8_t *mem_buf,
                       size_t mem_len);

/* Deprecated */
int i2c_write_raw(struct i2c_dev *dev, uint8_t addr, uint8_t *buf, size_t len);
int i2c_read_raw(struct i2c_dev *dev, uint8_t addr, uint8_t *buf, size_t len);

#endif /* I2C_H */

