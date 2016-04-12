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
#include "esp8266_drv.h"
#include "macros.h"
#include "messaging.h"
#include "panic.h"
#include "printk.h"
#include "rx_buff.h"
#include "serial.h"
#include "serial_device.h"
#include "semphr.h"
#include "task.h"
#include "wifi.h"
#include <stdbool.h>

#define _THREAD_NAME	"WiFi Task"
#define _CONN_WAIT_MS	100
#define _STACK_SIZE	256
#define _RX_BUFF_SIZE	1024
#define _LOG_PFX	"[wifi] "

static struct {
        struct Serial *incoming_conn;
        struct rx_buff rxb;
        xSemaphoreHandle semaphor;
} state;

static void _new_conn_cb(struct Serial *s)
{
        /* Set the incoming connection serial pointer and release the task */
        state.incoming_conn = s;
        xSemaphoreGive(state.semaphor);
}

/**
 * Handles all of our incoming messages and what we do with them.
 */
static void process_rx_msgs()
{
        while(state.incoming_conn) {
                rx_buff_clear(&state.rxb);
                char *data_in = rx_buff_read(&state.rxb, state.incoming_conn,
                                             _CONN_WAIT_MS);

                if (!data_in) {
                        /* Message was not received in time */
                        serial_flush(state.incoming_conn);
                        state.incoming_conn = NULL;
                        break;
                }

                /* If here, we have a message to handle */
                pr_info_str_msg(_LOG_PFX "Received CMD: ", data_in);
                const size_t len_in = strlen(data_in);
                process_read_msg(state.incoming_conn, data_in, len_in);
        }
}


static void _task_loop()
{
        xSemaphoreTake(state.semaphor, portMAX_DELAY);
        process_rx_msgs();
}

static void _task(void *params)
{
        for(;;)
                _task_loop();

        panic(PANIC_CAUSE_UNREACHABLE);
}

bool wifi_init_task(const int wifi_task_priority,
                    const int wifi_drv_priority)
{
        /* Get our serial port setup */
        struct Serial *s = serial_device_get(SERIAL_AUX);
        if (!s)
                return false;

        state.semaphor = xSemaphoreCreateBinary();
        if (!state.semaphor)
                return false;

        /* Allocate our RX buffer for incomming data */
        if (!rx_buff_init(&state.rxb, _RX_BUFF_SIZE, NULL))
                return false;

        if (!esp8266_drv_init(s, wifi_drv_priority, _new_conn_cb))
                return false;

        const signed char * const task_name =
                (const signed char *) _THREAD_NAME;
        const size_t stack_size = _STACK_SIZE;
        xTaskCreate(_task, task_name, stack_size, NULL,
                            wifi_task_priority, NULL);

        return true;
}

void wifi_reset_config(struct wifi_cfg *cfg)
{
        /* For now simply zero this out */
        memset(cfg, 0, sizeof(struct wifi_cfg));

        /* Inform the Wifi device that settings may have changed */
        wifi_update_client_config(&cfg->client);
}

/**
 * Wrapper for the driver to update the client wifi config
 * settings.  This is here because it makes sense for these
 * calls to enter the Wifi subsystem here instead of through
 * the driver directly.
 */
bool wifi_update_client_config(struct wifi_client_cfg *wcc)
{
        return esp8266_drv_update_client_cfg(wcc);
}
