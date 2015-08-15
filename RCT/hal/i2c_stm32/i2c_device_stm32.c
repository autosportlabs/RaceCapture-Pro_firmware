/* 12c.c
 * Optimized DMA+Interrupt I2C Driver for the STM32F4
 * (c) 2014 AutosportLabs
 * Jeff Ciesielski <jeff@autosportlabs.com>
 */

#include <i2c_device_stm32.h>
struct i2c_dev *i2c_get_device(enum i2c_bus device)
{
    switch (device) {
#ifdef USE_DMA1
    case I2C_1:
            return &pub_drivers[0];
#endif  /* USE_DMA1 */
#ifdef USE_DMA2
    case I2C_2:
        return &pub_drivers[1];
#endif  /* USE_DMA2 */
    default:
        return NULL;
    }
}

int i2c_init(struct i2c_dev *dev, uint32_t bus_speed){
    return 0;
}

int i2c_transact(struct i2c_dev *dev, uint8_t addr,
                 uint8_t *tx_buf, size_t tx_len,
                 uint8_t *rx_buf, size_t rx_len){
    return 0;
}

int i2c_read_reg8(struct i2c_dev *dev, uint8_t dev_addr,
                  uint8_t reg_addr, uint8_t *reg_val){
    return 0;
}

int i2c_write_reg8(struct i2c_dev *dev, uint8_t dev_addr,
                   uint8_t reg_addr, uint8_t reg_val){
    return 0;
}

int i2c_write_reg_bits(struct i2c_dev *dev, uint8_t dev_addr,
                       uint8_t reg_addr, size_t bit_pos,
                       size_t num_bits, uint8_t bit_val){
    return 0;
}

int i2c_read_reg_bits(struct i2c_dev *dev, uint8_t dev_addr,
                      uint8_t reg_addr, size_t bit_pos,
                      size_t num_bits, uint8_t *bit_val){
    return 0;
}

int i2c_read_mem_block(struct i2c_dev *dev, uint8_t dev_addr,
                       uint8_t mem_addr, uint8_t *mem_buf,
                       size_t mem_len){
    return 0;
}

/* Deprecated */
int i2c_write_raw(struct i2c_dev *dev, uint8_t addr, uint8_t *buf, size_t len){
    return 0;
}

int i2c_read_raw(struct i2c_dev *dev, uint8_t addr, uint8_t *buf, size_t len){
    return 0;
}
