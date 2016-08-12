/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

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
#define IMU_DEVICE_COUNTS_PER_DEGREE_PER_SEC	131.0

#define ACCEL_MAX_RANGE 	ACCEL_COUNTS_PER_G * 4
#define IMU_TASK_PRIORITY	(tskIDLE_PRIORITY + 2)
#define IS_9150_ADDR            0x68

static struct is9150_all_sensor_data sensor_data[2];
static struct is9150_all_sensor_data *read_buf = &sensor_data[0];
static struct is9150_all_sensor_data *fill_buf = &sensor_data[1];
static enum imu_init_status init_status;

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

    res = is9150_init(i2c1, IS_9150_ADDR << 1);
    pr_info("IMU: init res=");
    pr_info_int(res);
    pr_info("\r\n");
    (void)res;

    /* Clear the sensor data structures */
    memset(sensor_data, 0x00, sizeof(struct is9150_all_sensor_data) * 2);
    init_status = 0 == res ? IMU_INIT_STATUS_SUCCESS : IMU_INIT_STATUS_FAILED;

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
        static const signed portCHAR task_name[] = "IMU Reader Task";
        xTaskCreate(imu_update_task, task_name, configMINIMAL_STACK_SIZE * 2,
                    NULL, IMU_TASK_PRIORITY, NULL);
}

enum imu_init_status imu_device_init_status()
{
        return init_status;
}

int imu_device_read(enum imu_channel channel)
{
        /*
         * Mapped to SAE J670E standard. Front of unit is RJ-45 and USB
         * plug facing front of vehicle when mounted on windshield.
         * Issue #425
         */
        switch(channel) {
        case IMU_CHANNEL_X:
                return read_buf->accel.accel_y;
        case IMU_CHANNEL_Y:
                return read_buf->accel.accel_x;
        case IMU_CHANNEL_Z:
                return read_buf->accel.accel_z;
        case IMU_CHANNEL_YAW:
                return -read_buf->gyro.gyro_z;
        case IMU_CHANNEL_PITCH:
                return read_buf->gyro.gyro_x;
        case IMU_CHANNEL_ROLL:
                return read_buf->gyro.gyro_y;
        default:
                return 0;
        }
}

float imu_device_counts_per_unit(enum imu_channel channel)
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
    default:
        ret = 0.0;
        break;
    }
    return ret;
}
