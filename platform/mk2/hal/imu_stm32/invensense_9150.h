#ifndef _INVENSENSE_9150_H_
#define _INVENSENSE_9150_H_
#include <i2c_device_stm32.h>

/* RETURN CODES */
#define IS_9150_ERR_INIT -1

/* REGISTER DEFINITIONS */
#define IS_REG_PWR_MGMT_1	0x6B
#define IS_REG_WHOAMI		0x75

/* Gyro Registers */
#define IS_REG_GYRO_CONFIG      0x1B
#define IS_GYRO_MEAS_START      0x43
#define IS_GYRO_MEAS_COUNT      6
#define IS_GYRO_MEAS_X_HI       0x43
#define IS_GYRO_MEAS_X_LO       0x44
#define IS_GYRO_MEAS_Y_HI       0x45
#define IS_GYRO_MEAS_Y_LO       0x46
#define IS_GYRO_MEAS_Z_HI       0x47
#define IS_GYRO_MEAS_Z_LO       0x48

/* Accel registers */
#define IS_REG_ACCEL_CONFIG     0x1C
#define IS_ACCEL_MEAS_START     0x3B
#define IS_ACCEL_MEAS_COUNT     6
#define IS_ACCEL_MEAS_X_HI      0x3B
#define IS_ACCEL_MEAS_X_LO      0x3C
#define IS_ACCEL_MEAS_Y_HI      0x3D
#define IS_ACCEL_MEAS_Y_LO      0x3E
#define IS_ACCEL_MEAS_Z_HI      0x3F
#define IS_ACCEL_MEAS_Z_LO      0x40

/* Temperature registers */
#define IS_TEMP_MEAS_START	0x41
#define IS_TEMP_MEAS_COUNT	2
#define IS_TEMP_MEAS_HI		0x41
#define IS_TEMP_MEAS_LO		0x42


/* Power management related */
#define IS_POWER_SLEEP_POS	6

/* Accel related */
#define IS_ACCEL_SCALE_POS      3
#define IS_ACCEL_NUM_BITS       2
#define IS_ACCEL_SCALE_2G	0x00
#define IS_ACCEL_SCALE_4G	0x01
#define IS_ACCEL_SCALE_8G	0x02
#define IS_ACCEL_SCALE_16G	0x03

/* Gyro related */
#define IS_GYRO_SCALE_POS       3
#define IS_GYRO_NUM_BITS        2
#define IS_GYRO_SCALE_250	0x00
#define IS_GYRO_SCALE_500	0x01
#define IS_GYRO_SCALE_1000	0x02
#define IS_GYRO_SCALE_2000	0x03

/*  Clock settings */
#define IS_CLOCK_POS            0
#define IS_CLOCK_NUM_BITS       3
#define IS_CLOCK_PLL		0x00
#define IS_CLOCK_GYRO_X		0x01
#define IS_CLOCK_GYRO_Y		0x02
#define IS_CLOCK_GYRO_Z		0x03
#define IS_CLOCK_32768_REF	0x04
#define IS_CLOCK_192_REF	0x05
#define IS_CLOCK_STOPPED	0x07

/* Chip Defaults */
#define IS_WHOAMI_9150	0x68
#define IS_WHOAMI_9250	0x71

struct is9150_gyro_data {
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
};

struct is9150_accel_data {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
};

struct is9150_all_sensor_data {
    struct is9150_gyro_data gyro;
    struct is9150_accel_data accel;
    int16_t temp;
};


/* Public Functions */
int is9150_init(struct i2c_dev *dev, uint8_t addr);
int is9150_read_gyro(struct is9150_gyro_data *data);
int is9150_read_accel(struct is9150_accel_data *data);
int is9150_read_temp(uint16_t *temp);
int is9150_read_all_sensors(struct is9150_all_sensor_data *data);

#endif
