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
#include "at.h"
#include "esp8266.h"
#include "printk.h"
#include "serial.h"
#include "serial_buffer.h"
#include "task.h"
#include "wifi.h"

#define WIFI_TASK_THREAD_NAME	"WiFi Task"
#define WIFI_TASK_STACK_SIZE	512
#define WIFI_SERIAL_PORT	SERIAL_AUX
#define WIFI_SERIAL_BAUD	115200
#define WIFI_SERIAL_BITS	8
#define WIFI_SERIAL_STOP_BITS	1
#define WIFI_SERIAL_PARITY	0
#define WIFI_SERIAL_BUFF_SIZE	512
#define WIFI_AT_DEFAULT_QP_MS	250
#define WIFI_AT_DEFAULT_DELIM	"\r\n"
#define WIFI_AT_TASK_TIMEOUT_MS	5

static struct {
        Serial *serial;
        struct serial_buffer serial_buff;
        struct at_info ati;
} state;

static bool init_task_state()
{
        /* Get our serial port setup */
        state.serial = get_serial(WIFI_SERIAL_PORT);
        state.serial->init(WIFI_SERIAL_BITS, WIFI_SERIAL_PARITY,
                           WIFI_SERIAL_STOP_BITS, WIFI_SERIAL_BAUD);
        if (!state.serial)
                return false;

        /*
         * Initialize the serial buffer.  I put the s_buff here because
         * this way it is not accessible outside of this method.  And
         * since it is not on the stack (because static), it is safe to
         * do.
         */
        static char s_buff[WIFI_SERIAL_BUFF_SIZE];
        serial_buffer_create(&state.serial_buff, state.serial,
                             WIFI_SERIAL_BUFF_SIZE, s_buff);

        /* Init our AT engine here */
        init_at_info(&state.ati, &state.serial_buff,
                     WIFI_AT_DEFAULT_QP_MS, WIFI_AT_DEFAULT_DELIM);
        return true;
}

static void init_status_complete(const char *msg)
{
        pr_info_str_msg("[wifi] Device init status: ", msg);
        vTaskDelete(NULL);
        return; /* Panic */
}

static void wifi_task_loop()
{
        at_task(&state.ati, WIFI_AT_TASK_TIMEOUT_MS);

        switch(esp1866_get_dev_init_state()) {
        case DEV_INIT_STATE_NOT_READY:
                /* Then lets get it going */
                esp8266_begin_init(&state.ati);
                break;
        case DEV_INIT_INITIALIZING:
                /* Its in progress.  Nothing to do */
                break;
        case DEV_INIT_STATE_READY:
                /* W00t.  Success */
                return init_status_complete("Success!");
        case DEV_INIT_STATE_FAILED:
                return init_status_complete("Failure :(");
        }
}

static void wifi_task(void *params)
{
        if (!init_task_state()) {
                pr_error("[wifi] Failed to init\r\n");
                vTaskDelete(NULL);
                return; /* Panic! */
        }

        for(;;)
                wifi_task_loop();

        /* Panic! */
}

void start_wifi_task(const int priority)
{
        const signed char * const task_name =
                (const signed char *) WIFI_TASK_THREAD_NAME;
        const size_t stack_size = WIFI_TASK_STACK_SIZE;
        xTaskCreate(wifi_task, task_name, stack_size, NULL, priority, NULL);
}
