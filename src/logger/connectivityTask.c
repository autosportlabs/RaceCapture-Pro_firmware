/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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
#include "led.h"
#include "api.h"
#include "bluetooth.h"
#include "capabilities.h"
#include "connectivityTask.h"
#include "devices_common.h"
#include "loggerApi.h"
#include "loggerConfig.h"
#include "loggerHardware.h"
#include "mem_mang.h"
#include <string.h>
#include "modp_numtoa.h"
#include "null_device.h"
#include "printk.h"
#include "queue.h"
#include "sampleRecord.h"
#include "serial.h"
#include "cellular.h"
#include "stdint.h"
#include "task.h"
#include "taskUtil.h"
#include "usart.h"
#include "gps_device.h"


#if (CONNECTIVITY_CHANNELS == 1)
#define CONNECTIVITY_TASK_INIT {NULL}
#elif (CONNECTIVITY_CHANNELS == 2)
#define CONNECTIVITY_TASK_INIT {NULL, NULL}
#else
#error "invalid connectivity task count"
#endif

/*wait time for sample queue. can be portMAX_DELAY to wait forever, or zero to not wait at all */
#define TELEMETRY_QUEUE_WAIT_TIME					0

#define IDLE_TIMEOUT							configTICK_RATE_HZ / 10
#define INIT_DELAY	 							600
#define BUFFER_SIZE 							1025
#define TELEMETRY_DISCONNECT_TIMEOUT            60000

#define TELEMETRY_STACK_SIZE	300
#define BAD_MESSAGE_THRESHOLD					10

#define METADATA_SAMPLE_INTERVAL				100

static xQueueHandle g_sampleQueue[CONNECTIVITY_CHANNELS] = CONNECTIVITY_TASK_INIT;


static size_t trimBuffer(char *buffer, size_t count)
{

    char *c = (buffer + count - 1);
    while (count > 0 && (*c == '\r' || *c == '\n')) {
        *c = '\0';
        c--;
        count--;
    }
    return count;
}

static int processRxBuffer(struct Serial *serial, char *buffer, size_t *rxCount)
{
        const int count = serial_read_line_wait(serial, buffer + *rxCount,
						BUFFER_SIZE - *rxCount, 0);

	if (count < 0) {
		pr_error("[connectivityTask] Serial device closed\r\n");
		return 0;
	}

    *rxCount += count;
    int processMsg = 0;

    if (*rxCount >= BUFFER_SIZE - 1) {
        buffer[BUFFER_SIZE - 1] = '\0';
        *rxCount = BUFFER_SIZE - 1;
        processMsg = 1;
        pr_error_str_msg("Rx Buffer overflow:", buffer);
    }
    if (*rxCount > 0) {
        char lastChar = buffer[*rxCount - 1];
        if ('\r' == lastChar || '\n' == lastChar) {
            *rxCount = trimBuffer(buffer, *rxCount);
            processMsg = 1;
        }
    }
    return processMsg;
}

void queueTelemetryRecord(const LoggerMessage *msg)
{
    for (size_t i = 0; i < CONNECTIVITY_CHANNELS; i++)
            send_logger_message(g_sampleQueue[i], msg);
}

/*combined telemetry - for when there's only one telemetry / wireless port available on system
//e.g. "Y-adapter" scenario */
static void createCombinedTelemetryTask(int16_t priority,
                                        xQueueHandle sampleQueue)
{
        ConnectivityConfig *connConfig =
                &getWorkingLoggerConfig()->ConnectivityConfigs;
        size_t btEnabled = connConfig->bluetoothConfig.btEnabled;
        size_t cellEnabled = connConfig->cellularConfig.cellEnabled;

        if (btEnabled || cellEnabled) {
                ConnParams * params = (ConnParams *)portMalloc(sizeof(ConnParams));

                params->periodicMeta = btEnabled && cellEnabled;

                /*defaults*/
                params->check_connection_status = &null_device_check_connection_status;
                params->init_connection = &null_device_init_connection;
                params->serial = SERIAL_TELEMETRY;
                params->sampleQueue = sampleQueue;
                params->connection_timeout = 0;
                params->always_streaming = false;

                if (btEnabled) {
                        params->check_connection_status = &bt_check_connection_status;
                        params->init_connection = &bt_init_connection;
                        params->disconnect = &bt_disconnect;
                        params->always_streaming = true;
                        params->max_sample_rate = SAMPLE_50Hz;
                }

#if CELLULAR_SUPPORT
                /*cell overrides wireless*/
                if (cellEnabled) {
                        params->check_connection_status = &cellular_check_connection_status;
                        params->init_connection = &cellular_init_connection;
                        params->disconnect = &cellular_disconnect;
                        params->always_streaming = false;
                        params->max_sample_rate = SAMPLE_10Hz;
                }
#endif
                /* Make all task names 16 chars including NULL char */
                static const signed portCHAR task_name[] = "Multi Conn Task";
                xTaskCreate(connectivityTask, task_name, TELEMETRY_STACK_SIZE,
                            params, priority, NULL );
        }
}

static void createWirelessConnectionTask(int16_t priority,
                                         xQueueHandle sampleQueue,
                                         enum led activity_led)
{
#if BLUETOOTH_SUPPORT
        ConnParams *params = portMalloc(sizeof(ConnParams));
        params->connectionName = "Wireless";
        params->periodicMeta = 0;
        params->connection_timeout = 0;
        params->check_connection_status = &bt_check_connection_status;
        params->disconnect = &bt_disconnect;
        params->init_connection = &bt_init_connection;
        params->serial = SERIAL_BLUETOOTH;
        params->sampleQueue = sampleQueue;
        params->always_streaming = true;
        params->max_sample_rate = SAMPLE_50Hz;
        params->activity_led = activity_led;

        /* Make all task names 16 chars including NULL char */
        static const signed portCHAR task_name[] = "Bluetooth Task ";
        xTaskCreate(connectivityTask, task_name, TELEMETRY_STACK_SIZE,
                    params, priority, NULL );
#endif
}

static void createTelemetryConnectionTask(int16_t priority,
                                          xQueueHandle sampleQueue,
                                          enum led activity_led)
{
#if CELLULAR_SUPPORT
        ConnParams * params = (ConnParams *)portMalloc(sizeof(ConnParams));
        params->connectionName = "Telemetry";
        params->periodicMeta = 0;
        params->connection_timeout = TELEMETRY_DISCONNECT_TIMEOUT;
        params->disconnect = &cellular_disconnect;
        params->check_connection_status = &cellular_check_connection_status;
        params->init_connection = &cellular_init_connection;
        params->serial = SERIAL_TELEMETRY;
        params->sampleQueue = sampleQueue;
        params->always_streaming = false;
        params->max_sample_rate = SAMPLE_10Hz;
        params->activity_led = activity_led;

        /* Make all task names 16 chars including NULL char */
        static const signed portCHAR task_name[] = "Cell Telem Task";

        xTaskCreate(connectivityTask, task_name, TELEMETRY_STACK_SIZE,
                    params, priority, NULL );
#endif
}

void startConnectivityTask(int16_t priority)
{
        for (size_t i = 0; i < CONNECTIVITY_CHANNELS; i++) {
                g_sampleQueue[i] = create_logger_message_queue();

                if (NULL == g_sampleQueue[i]) {
                        pr_error("conn: err sample queue\r\n");
                        return;
                }
        }

        switch (CONNECTIVITY_CHANNELS) {
        case 1:
                createCombinedTelemetryTask(priority, g_sampleQueue[0]);
                break;
        case 2: {
                ConnectivityConfig *connConfig =
                        &getWorkingLoggerConfig()->ConnectivityConfigs;
                /*
                 * Logic to control which connection is considered 'primary',
                 * which is used later to determine which task has control
                 * over LED flashing
                 */
                const uint8_t cellEnabled =
                        connConfig->cellularConfig.cellEnabled;

                if (cellEnabled)
                        createTelemetryConnectionTask(priority,
                                                      g_sampleQueue[1], LED_TELEMETRY);

                if (connConfig->bluetoothConfig.btEnabled) {
                        /* Pick the bluetooth LED if available */
                        enum led activity_led = led_available(LED_BLUETOOTH) ? LED_BLUETOOTH : LED_TELEMETRY;
                        activity_led = cellEnabled && activity_led == LED_TELEMETRY ? LED_UNKNOWN : activity_led;

                    createWirelessConnectionTask(priority,
                                                 g_sampleQueue[0],
                                                 activity_led);

                }
        }
                break;
        default:
                pr_error("conn: err init\r\n");
                break;
        }
}

static void toggle_connectivity_indicator(const enum led indicator)
{
        led_toggle(indicator);
}

static void clear_connectivity_indicator(const enum led indicator)
{
        led_disable(indicator);
}

void connectivityTask(void *params)
{

    char * buffer = (char *)portMalloc(BUFFER_SIZE);
    size_t rxCount = 0;

    ConnParams *connParams = (ConnParams*)params;
    LoggerMessage msg;

    struct Serial *serial = serial_device_get(connParams->serial);

    xQueueHandle sampleQueue = connParams->sampleQueue;
    uint32_t connection_timeout = connParams->connection_timeout;
    const size_t max_telem_rate = connParams->max_sample_rate;

    DeviceConfig deviceConfig;
    deviceConfig.serial = serial;
    deviceConfig.buffer = buffer;
    deviceConfig.length = BUFFER_SIZE;

    const LoggerConfig *logger_config = getWorkingLoggerConfig();

    bool logging_enabled = false;

    while (1) {
        millis_t connected_at = 0;
        bool should_stream = logging_enabled ||
                             logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming ||
                             connParams->always_streaming;

        while (should_stream && connParams->init_connection(&deviceConfig, &connected_at) != DEVICE_INIT_SUCCESS) {
            pr_info("conn: not connected. retrying\r\n");
            vTaskDelay(INIT_DELAY);
        }
        if (connected_at > 0)
                GPS_set_UTC_time(connected_at);

        serial_flush(serial);
        rxCount = 0;
        size_t badMsgCount = 0;
        size_t tick = 0;
        size_t last_message_time = getUptimeAsInt();
        bool should_reconnect = false;

        while (1) {
            if ( should_reconnect )
                break; /*break out and trigger the re-connection if needed */

            should_stream =
                    logging_enabled ||
                    connParams->always_streaming ||
                    logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming;

            const char res = receive_logger_message(sampleQueue, &msg,
                                                    IDLE_TIMEOUT);

            /*///////////////////////////////////////////////////////////
            // Process a pending message from logger task, if exists
            ////////////////////////////////////////////////////////////*/
            if (pdFALSE != res) {
                switch(msg.type) {
                case LoggerMessageType_Start: {
                    api_sendLogStart(serial);
                    put_crlf(serial);
                    tick = 0;
                    logging_enabled = true;
                    /* If we're not already streaming trigger a re-connect */
                    if (!should_stream)
                        should_reconnect = true;
                    break;
                }
                case LoggerMessageType_Stop: {
                    api_sendLogEnd(serial);
                    put_crlf(serial);
                    if (! (logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming ||
                           connParams->always_streaming))
                        should_reconnect = true;
                    logging_enabled = false;
                    break;
                }
                case LoggerMessageType_Sample: {
                        if (!should_stream ||
                            !should_sample(msg.ticks, max_telem_rate))
                                break;

                        toggle_connectivity_indicator(connParams->activity_led);

                        const int send_meta = tick == 0 ||
                                (connParams->periodicMeta &&
                                 (tick % METADATA_SAMPLE_INTERVAL == 0));
                        api_send_sample_record(serial, msg.sample, tick, send_meta);

                        put_crlf(serial);
                        tick++;
                        break;
                }
                default:
                    break;
                }
            }

            /*//////////////////////////////////////////////////////////
            // Process incoming message, if available
            ////////////////////////////////////////////////////////////
            //read in available characters, process message as necessary*/
            int msgReceived = processRxBuffer(serial, buffer, &rxCount);
            /*check the latest contents of the buffer for something that might indicate an error condition*/
            if (connParams->check_connection_status(&deviceConfig) != DEVICE_STATUS_NO_ERROR) {
                pr_info("conn: disconnected\r\n");
                break;
            }

            /*now process a complete message if available*/
            if (msgReceived) {
                last_message_time = getUptimeAsInt();

                const int msgRes = process_api(serial, buffer, BUFFER_SIZE);
                const int msgError = (msgRes == API_ERROR_MALFORMED);
                if (msgError) {
                    pr_debug("(failed)\r\n");
                }
                if (msgError) {
                    badMsgCount++;
                } else {
                    badMsgCount = 0;
                }
                if (badMsgCount >= BAD_MESSAGE_THRESHOLD) {
                    pr_warning_int_msg("re-connecting- empty/bad msgs :", badMsgCount );
                    break;
                }
                rxCount = 0;
            }

            /*disconnect if a timeout is configured and
            // we haven't heard from the other side for a while */
            const size_t timeout = getUptimeAsInt() - last_message_time;
            if (connection_timeout && timeout > connection_timeout ) {
                pr_info_str_msg(connParams->connectionName, ": timeout");
                should_reconnect = true;
            }
        }

        clear_connectivity_indicator(connParams->activity_led);
        connParams->disconnect(&deviceConfig);
    }
}
