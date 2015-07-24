#include "FreeRTOS.h"
#include "task.h"
#include <semphr.h>

#include <string.h>
#include <stdbool.h>
#include "imu_device.h"
#include "loggerConfig.h"
#include "printk.h"
#include "modp_numtoa.h"
#include <i2c_device_stm32.h>
#include <invensense_9150.h>

#define IMU_DEVICE_COUNTS_PER_G 		16384
#define IMU_DEVICE_COUNTS_PER_DEGREE_PER_SEC	32.8
#define IMU_DEVICE_COUNTS_PER_DEGREE			1

#define ACCEL_MAX_RANGE 	ACCEL_COUNTS_PER_G * 4
#define IMU_TASK_PRIORITY	(tskIDLE_PRIORITY + 2)

static struct is9150_all_sensor_data sensor_data[2];
static struct is9150_all_sensor_data *read_buf = &sensor_data[0];
static struct is9150_all_sensor_data *fill_buf = &sensor_data[1];
static void imu_update_buf_ptrs(void)
{
    static struct is9150_all_sensor_data *tmp;
    tmp = read_buf;
    read_buf = fill_buf;
    fill_buf = tmp;
}

static void imu_update_task(void *params)
{
    (void)params;
    int res;

    struct i2c_dev *i2c1 = i2c_get_device(I2C_1);

    i2c_init(i2c1, 400000);

    res = is9150_init(i2c1);
    pr_info_int_msg("IMU: init res=", res);

    (void)res;

    /* Clear the sensor data structures */
    memset(sensor_data, 0x00, sizeof(struct is9150_all_sensor_data) * 2);

    while(1) {
        res = is9150_read_all_sensors(fill_buf);
        if (!res)
            imu_update_buf_ptrs();
        vTaskDelay(1);
    }
}

void imu_device_init()
{
    /* Create a lock around the sensor buffers */

    xTaskCreate(imu_update_task,
                (signed portCHAR*)"IMU update",
                configMINIMAL_STACK_SIZE,
                NULL,
                IMU_TASK_PRIORITY,
                NULL);

}

int imu_device_read(unsigned int channel)
{
    unsigned int ret = 0;

    switch(channel) {
    case IMU_CHANNEL_X:
        ret = read_buf->accel.accel_x;
        break;
    case IMU_CHANNEL_Y:
        ret = read_buf->accel.accel_y;
        break;
    case IMU_CHANNEL_Z:
        ret = read_buf->accel.accel_z;
        break;
    case IMU_CHANNEL_YAW:
        ret = read_buf->gyro.gyro_z;
        break;
    case IMU_CHANNEL_PITCH:
        ret = read_buf->gyro.gyro_x;
        break;
    case IMU_CHANNEL_ROLL:
        ret = read_buf->gyro.gyro_y;
        break;
    case IMU_CHANNEL_COMPASS:
        ret = read_buf->mag.compass;
        break;
    default:
        break;
    }

    return ret;
}

float imu_device_counts_per_unit(unsigned int channel)
{
    float ret;
    switch(channel) {
    case IMU_CHANNEL_YAW:
    case IMU_CHANNEL_PITCH:
    case IMU_CHANNEL_ROLL:
        ret = IMU_DEVICE_COUNTS_PER_DEGREE_PER_SEC;
        break;
    case IMU_CHANNEL_X:
    case IMU_CHANNEL_Y:
    case IMU_CHANNEL_Z:
        ret = IMU_DEVICE_COUNTS_PER_G;
        break;
    case IMU_CHANNEL_COMPASS:
        ret = IMU_DEVICE_COUNTS_PER_DEGREE;
        break;
    default:
        ret = 0.0;
        break;
    }
    return ret;
}
