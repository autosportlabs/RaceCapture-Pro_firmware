#include <i2c_device_stm32.h>
#include <invensense_9150.h>
#include <stdint.h>
#include <stddef.h>
#include "taskUtil.h"
#include "printk.h"
#include <math.h>
static struct  {
    struct i2c_dev *i2c;
    uint8_t addr;
    uint8_t mag_addr;
} is9150_dev;

static float mag_sens_adj[3];

static int is9150_readreg(uint8_t reg_addr, uint8_t *reg_val)
{
    int res;
    res = i2c_read_reg8(is9150_dev.i2c, is9150_dev.addr,
                        reg_addr, reg_val);
    return res;
}

static int is9150_write_reg(uint8_t reg_addr, uint8_t reg_val)
{
    int res;
    res = i2c_write_reg8(is9150_dev.i2c, is9150_dev.addr,
                         reg_addr, reg_val);
    return res;
}

static int is9150_write_reg_bits(uint8_t reg_addr, size_t bit_pos,
                                 size_t num_bits, uint8_t bit_val)
{
    int res;
    res = i2c_write_reg_bits(is9150_dev.i2c, is9150_dev.addr,
                             reg_addr, bit_pos, num_bits, bit_val);
    return res;
}


static int is9150_write_mag_reg_bits(uint8_t reg_addr, size_t bit_pos,
                                     size_t num_bits, uint8_t bit_val)
{
    int res;
    res = i2c_write_reg_bits(is9150_dev.i2c, is9150_dev.mag_addr,
                             reg_addr, bit_pos, num_bits, bit_val);
    return res;
}

static int is9150_read_reg_block(uint8_t start_addr, size_t len,
                                 uint8_t *rx_buf)
{
    int res;
    res = i2c_read_mem_block(is9150_dev.i2c, is9150_dev.addr,
                             start_addr, rx_buf,
                             len);
    return res;
}


static int is9150_read_mag_reg_block(uint8_t start_addr, size_t len,
                                     uint8_t *rx_buf)
{
    int res;
    res = i2c_read_mem_block(is9150_dev.i2c, is9150_dev.mag_addr,
                             start_addr, rx_buf,
                             len);
    return res;
}

/* Bypass IMU to talk to magnetometer directly */
static int bypass_on(void)
{
    int res = is9150_write_reg_bits(IS_RA_USER_CTRL, IS_MST_EN_POS, 1, 0);
    if (res) {
        return res;
    }

    res = is9150_write_reg_bits(IS_RA_INT_PIN_CFG, 0, 8, 0x82);
    return res;
}

/* Disable IMU bypass */
static int bypass_off(void)
{
    int res = is9150_write_reg_bits(IS_RA_USER_CTRL, IS_MST_EN_POS, 1, 1);
    if (res) {
        return res;
    }

    res = is9150_write_reg_bits(IS_RA_INT_PIN_CFG, 0, 8, 0x80);
    return res;
}

/* Initialize the compass so the IMU reads it automatically on our behalf */
static int is9150_init_compass(void)
{
    int res = bypass_on();
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* read magnetometer factory calibration */
    /* first, power down */
    res = is9150_write_mag_reg_bits(IS_REG_MAG_CONTROL, 0, 8, IS_MAG_CTRL_POWER_DOWN);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* set magnetometer to read control bits */
    res = is9150_write_mag_reg_bits(IS_REG_MAG_CONTROL, 0, 8, IS_MAG_CTRL_FUSE_ACCESS);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* read the sensitivity data */
    uint8_t sens_data_raw[3];
    res = is9150_read_mag_reg_block(IS_REG_MAG_SENS_START, IS_REG_MAG_SENS_COUNT,
                                    sens_data_raw);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /*set compass factory calibration values */
    mag_sens_adj[0] = ((float)sens_data_raw[0] - 128.0) / 256.0 + 1.0f;
    mag_sens_adj[1] = ((float)sens_data_raw[1] - 128.0) / 256.0 + 1.0f;
    mag_sens_adj[2] = ((float)sens_data_raw[2] - 128.0) / 256.0 + 1.0f;

    /* bypass off */
    res = bypass_off();
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set I2C master mode */
    res = is9150_write_reg(IS_RA_MASTER_CTRL, 0x40);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set Slave 0 address */
    res = is9150_write_reg(IS_REG_SLV0_ADDR, 0x80 | 0x0C);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set Slave 0 register */
    res = is9150_write_reg(IS_REG_SLV0_REG, 0x02);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set Slave 0 control */
    res = is9150_write_reg(IS_REG_SLV0_CTRL, 0x88);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set Slave 1 address */
    res = is9150_write_reg(IS_REG_SLV1_ADDR, 0x0C);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set Slave 1 register */
    res = is9150_write_reg(IS_REG_SLV1_REG, 0x0a);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set Slave 1 control */
    res = is9150_write_reg(IS_REG_SLV1_CTRL, 0x81);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set Slave 1 DO */
    res = is9150_write_reg(IS_REG_SLV1_DO, 0x01);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set Mst delay */
    res = is9150_write_reg(IS_REG_MST_DELAY_CTRL, 0x03);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set yg offs TC */
    res = is9150_write_reg(IS_REG_YG_OFFS_TC, 0x80);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* set slowest slave sample rate */
    res = is9150_write_reg_bits(IS_REG_SLV4_CONTROL, IS_REG_SLV4_RATE_POS, IS_REG_SLV4_RATE_BITS, 31);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    return 0;
}

int is9150_init(struct i2c_dev *dev)
{
    int res;
    uint8_t reg = 0x00;

    is9150_dev.i2c = dev;
    is9150_dev.addr = IS_9150_ADDR << 1; /* 7 bit addressing */
    is9150_dev.mag_addr = IS_9150_MAG_ADDR << 1; /* 7 bit addressing */

    /* Read the 'who am I' register to make sure that the device
     * is out there and alive */
    res = is9150_readreg(IS_REG_WHOAMI, &reg);
    if (res) {
        return IS_9150_ERR_INIT;
    } else if (reg != IS_WHOAMI_DEFAULT) {
        return IS_9150_ERR_INIT;
    }

    /* Set the sleep bit in the power management register, this
     * puts the device down for configuration */
    res = is9150_write_reg_bits(IS_REG_PWR_MGMT_1, IS_POWER_SLEEP_POS,
                                1, 1);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set the device clock source to internal PLL + GYRO_X sync */
    /* NOTE: This has shown to be more accurate than the PLL
     * alone according to datasheet */
    res = is9150_write_reg_bits(IS_REG_PWR_MGMT_1, IS_CLOCK_POS,
                                IS_CLOCK_NUM_BITS, IS_CLOCK_GYRO_X);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set the Gyro range to 1000* per second */
    res = is9150_write_reg_bits(IS_REG_GYRO_CONFIG, IS_GYRO_SCALE_POS,
                                IS_GYRO_NUM_BITS, IS_GYRO_SCALE_2000);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Set the accelerometer range to 4G per second */
    res = is9150_write_reg_bits(IS_REG_ACCEL_CONFIG, IS_ACCEL_SCALE_POS,
                                IS_ACCEL_NUM_BITS, IS_ACCEL_SCALE_4G);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    /* Clear the sleep bit in the power management register, this
     * beings sampling of the internal devices */
    res = is9150_write_reg_bits(IS_REG_PWR_MGMT_1, IS_POWER_SLEEP_POS,
                                1, 0);
    if (res) {
        return IS_9150_ERR_INIT;
    }

    res = is9150_init_compass();
    if (res) {
        return IS_9150_ERR_INIT;
    }

    return 0;
}

int is9150_read_gyro(struct is9150_gyro_data *data)
{
    int res;
    uint8_t reg_res[6] = {0};
    res = is9150_read_reg_block(IS_GYRO_MEAS_START, IS_GYRO_MEAS_COUNT,
                                reg_res);

    data->gyro_x = reg_res[0] << 8 | reg_res[1];
    data->gyro_y = reg_res[2] << 8 | reg_res[3];
    data->gyro_z = reg_res[4] << 8 | reg_res[5];
    return res;
}

int is9150_read_accel(struct is9150_accel_data *data)
{
    int res;
    uint8_t reg_res[6] = {0};
    res = is9150_read_reg_block(IS_ACCEL_MEAS_START, IS_ACCEL_MEAS_COUNT,
                                reg_res);

    data->accel_x = reg_res[0] << 8 | reg_res[1];
    data->accel_y = reg_res[2] << 8 | reg_res[3];
    data->accel_z = reg_res[4] << 8 | reg_res[5];
    return res;
}

int is9150_read_mag(struct is9150_mag_data *data)
{
    uint8_t reg_res[6] = {0};
    int res = is9150_read_reg_block(IS_REG_EXT_SENS_DATA_00, IS_MAG_MEAS_COUNT,
                                    reg_res);

    data->mag_x = (((int16_t)reg_res[1]) << 8) | reg_res[0];
    data->mag_y = (((int16_t)reg_res[3]) << 8) | reg_res[2];
    data->mag_z = (((int16_t)reg_res[5]) << 8) | reg_res[4];

    /* apply factory calibration */
    data->mag_x = ((int32_t)data->mag_x * mag_sens_adj[0]);
    data->mag_y = ((int32_t)data->mag_y * mag_sens_adj[1]);
    data->mag_z = ((int32_t)data->mag_z * mag_sens_adj[2]);

    float heading = atan2((double)data->mag_y, (double)data->mag_x) * 180.0 / 3.14159265 + 180.0;
    heading = heading < 0 ? heading + 360 : heading;
    data->compass = heading;
    return res;
}

int is9150_read_temp(uint16_t *temp)
{
    int res;
    uint8_t reg_res[2] = {0};
    res = is9150_read_reg_block(IS_TEMP_MEAS_START, IS_TEMP_MEAS_COUNT,
                                reg_res);

    *temp = reg_res[0] << 8 | reg_res[1];
    return res;
}

int is9150_read_all_sensors(struct is9150_all_sensor_data *data)
{
    int res;
    uint8_t reg_res[14] = {0};
    res = is9150_read_reg_block(IS_ACCEL_MEAS_START,
                                IS_ACCEL_MEAS_COUNT + IS_GYRO_MEAS_COUNT + IS_TEMP_MEAS_COUNT,
                                reg_res);

    if (res) {
        return res;
    }

    data->accel.accel_x = reg_res[0] << 8 | reg_res[1];
    data->accel.accel_y = reg_res[2] << 8 | reg_res[3];
    data->accel.accel_z = reg_res[4] << 8 | reg_res[5];

    data->temp = reg_res[6] << 8 | reg_res[7];

    data->gyro.gyro_x = reg_res[8] << 8 | reg_res[9];
    data->gyro.gyro_y = reg_res[10] << 8 | reg_res[11];
    data->gyro.gyro_z = reg_res[12] << 8 | reg_res[13];

    /* Magnetometer needs to be read separately */
    res = is9150_read_mag(&data->mag);
    return res;
}
