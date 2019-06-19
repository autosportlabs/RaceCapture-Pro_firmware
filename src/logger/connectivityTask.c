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
#include "semphr.h"
#include "sampleRecord.h"
#include "serial.h"
#include "cellular.h"
#include "stdint.h"
#include "task.h"
#include "taskUtil.h"
#include "usart.h"
#include "gps_device.h"
#include "api_event.h"
#include "sdcard.h"

#if (CONNECTIVITY_CHANNELS == 1)
#define CONNECTIVITY_TASK_INIT {NULL}
#elif (CONNECTIVITY_CHANNELS == 2)
#define CONNECTIVITY_TASK_INIT {NULL, NULL}
#else
#error "invalid connectivity task count"
#endif

#define _LOG_PFX "[Conn task] "

/*wait time for sample queue. can be portMAX_DELAY to wait forever, or zero to not wait at all */
#define TELEMETRY_QUEUE_WAIT_TIME     0

#define IDLE_TIMEOUT       configTICK_RATE_HZ / 10
#define INIT_DELAY         600
#define BUFFER_SIZE        1025
#define TELEMETRY_DISCONNECT_TIMEOUT            60000

#define TELEMETRY_STACK_SIZE 300
#define CELLULAR_TELEMETRY_STACK_SIZE 330
#define BAD_MESSAGE_THRESHOLD     10
#define API_EVENT_QUEUE_DEPTH 2
#define CELLULAR_TELEMETRY_BUFFER_QUEUE_DEPTH 1
#define METADATA_SAMPLE_INTERVAL    100
#define HARD_INIT_RETRY_THRESHOLD 5

#define TELEMETRY_BUFFER_FILENAME "tele.buf"
#define TELEMETRY_BUFFER_FILE_RETRY_MS 1000
#define BUFFER_BUFFER_SIZE 2000

static xQueueHandle g_sampleQueue[CONNECTIVITY_CHANNELS] = CONNECTIVITY_TASK_INIT;

typedef struct _BufferedLoggerMessage {
        enum LoggerMessageType type;
        size_t ticks;
        struct sample *sample;
} BufferedLoggerMessage;

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
                pr_error(_LOG_PFX "Serial device closed\r\n");
                return 0;
        }

        *rxCount += count;
        int processMsg = 0;

        if (*rxCount >= BUFFER_SIZE - 1) {
                buffer[BUFFER_SIZE - 1] = '\0';
                *rxCount = BUFFER_SIZE - 1;
                processMsg = 1;
                pr_error_str_msg(_LOG_PFX "Rx Buffer overflow:", buffer);
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

static xQueueHandle buffer_queue;
static FIL *buffer_file = NULL;
static xSemaphoreHandle fs_mutex = NULL;
static char buffer_buffer[BUFFER_BUFFER_SIZE + 1];
static int32_t read_index = 0;
static bool file_open = false;
static struct Serial *string_serial = NULL;

static void createTelemetryConnectionTask(int16_t priority,
                xQueueHandle sampleQueue,
                enum led activity_led)
{
#if CELLULAR_SUPPORT
        buffer_file = pvPortMalloc(sizeof(FIL));

        string_serial = serial_create_direct_buffer("str_ser", buffer_buffer, BUFFER_BUFFER_SIZE);
        {
                fs_mutex = xSemaphoreCreateMutex();
                buffer_queue = xQueueCreate(CELLULAR_TELEMETRY_BUFFER_QUEUE_DEPTH, sizeof(BufferedLoggerMessage));

                BufferingTaskParams * params = (BufferingTaskParams *)portMalloc(sizeof(BufferingTaskParams));
                params->connectionName = "TelemBuffer";
                params->periodicMeta = 0;
                params->sampleQueue = sampleQueue;
                params->buffer_queue = buffer_queue;
                params->always_streaming = false;
                params->max_sample_rate = SAMPLE_10Hz;

                /* Make all task names 16 chars including NULL char */
                static const signed portCHAR task_name[] = "Telem Buffer";

                xTaskCreate(cellular_buffering_task, task_name, CELLULAR_TELEMETRY_STACK_SIZE,
                            params, priority, NULL );
        }

        {
                TelemetryConnParams * params = (TelemetryConnParams *)portMalloc(sizeof(TelemetryConnParams));
                params->connectionName = "CellTelem";
                params->connection_timeout = TELEMETRY_DISCONNECT_TIMEOUT;
                params->disconnect = &cellular_disconnect;
                params->check_connection_status = &cellular_check_connection_status;
                params->tx_socket = &cellular_tx_socket;
                params->init_connection = &cellular_init_connection;
                params->serial = SERIAL_TELEMETRY;
                params->sampleQueue = buffer_queue;
                params->always_streaming = false;
                params->max_sample_rate = SAMPLE_10Hz;
                params->activity_led = activity_led;

                /* Make all task names 16 chars including NULL char */
                static const signed portCHAR task_name[] = "Cell Telemetry";

                xTaskCreate(cellular_connectivity_task, task_name, CELLULAR_TELEMETRY_STACK_SIZE,
                            params, priority, NULL );
        }

#endif
}

void startConnectivityTask(int16_t priority)
{
        for (size_t i = 0; i < CONNECTIVITY_CHANNELS; i++) {
                g_sampleQueue[i] = create_logger_message_queue();

                if (NULL == g_sampleQueue[i]) {
                        pr_error(_LOG_PFX "err sample queue\r\n");
                        return;
                }
        }

        switch (CONNECTIVITY_CHANNELS) {
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
                pr_error_int_msg(_LOG_PFX "Error creating connectivity task incorrect number of channels ", CONNECTIVITY_CHANNELS);
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

static void queue_api_event(const struct api_event * api_event, void * data)
{
        xQueueHandle queue = data;
        if (xQueueSendToBack(queue, api_event, 0)) {
                pr_trace(_LOG_PFX "queued api event\r\n");
        } else {
                pr_warning(_LOG_PFX "api event queue overflow\r\n");
        }
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

        xQueueHandle api_event_queue = xQueueCreate(API_EVENT_QUEUE_DEPTH, sizeof(struct api_event));
        api_event_create_callback(queue_api_event, api_event_queue);

        bool hard_init = true;
        while (1) {
                size_t connect_retries = 0;
                millis_t connected_at = 0;
                bool should_stream = logging_enabled ||
                                     logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming ||
                                     connParams->always_streaming;

                while (should_stream && connParams->init_connection(&deviceConfig, &connected_at, hard_init) != DEVICE_INIT_SUCCESS) {
                        pr_info(_LOG_PFX "not connected. retrying\r\n");
                        vTaskDelay(INIT_DELAY);
                        connect_retries++;
                        if (connect_retries > HARD_INIT_RETRY_THRESHOLD) {
                                pr_info(_LOG_PFX " Too many connection attempts\r\n");
                                hard_init = true;
                                connect_retries = 0;
                        }
                }
                if (connected_at > 0)
                        GPS_set_UTC_time(connected_at);

                serial_flush(serial);
                rxCount = 0;
                size_t badMsgCount = 0;
                size_t tick = 0;
                size_t last_message_time = getUptimeAsInt();
                bool should_reconnect = false;
                hard_init = false;
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
                                        pr_info_int_msg(_LOG_PFX "Unknown logger message type ", msg.type);
                                        break;
                                }
                        }
                        /*//////////////////////////////////////////////////////////
                        // Process any pending API events
                        ////////////////////////////////////////////////////////////*/
                        struct api_event api_event;
                        if (xQueueReceive(api_event_queue, &api_event, 0)) {
                                process_api_event(&api_event, serial);
                        }

                        /*//////////////////////////////////////////////////////////
                        // Process incoming message, if available
                        ////////////////////////////////////////////////////////////
                        //read in available characters, process message as necessary*/
                        int msgReceived = processRxBuffer(serial, buffer, &rxCount);
                        /*check the latest contents of the buffer for something that might indicate an error condition*/
                        if (connParams->check_connection_status(&deviceConfig) != DEVICE_STATUS_NO_ERROR) {
                                pr_info(_LOG_PFX "Disconnected\r\n");
                                break;
                        }

                        /*now process a complete message if available*/
                        if (msgReceived) {
                                last_message_time = getUptimeAsInt();

                                pr_info_str_msg("received msg ", buffer);

                                const int msgRes = process_api(serial, buffer, BUFFER_SIZE);
                                const int msgError = (msgRes == API_ERROR_MALFORMED);
                                if (msgError) {
                                        pr_debug(_LOG_PFX " (failed)\r\n");
                                }
                                if (msgError) {
                                        badMsgCount++;
                                } else {
                                        badMsgCount = 0;
                                }
                                if (badMsgCount >= BAD_MESSAGE_THRESHOLD) {
                                        pr_warning_int_msg(_LOG_PFX "re-connecting- empty/bad msgs :", badMsgCount );
                                        break;
                                }
                                rxCount = 0;
                        }

                        /*disconnect if a timeout is configured and
                        // we haven't heard from the other side for a while */
                        const size_t timeout = getUptimeAsInt() - last_message_time;
                        if (connection_timeout && timeout > connection_timeout ) {
                                pr_info_str_msg(_LOG_PFX " Timeout ", connParams->connectionName);
                                should_reconnect = true;
                        }
                }

                clear_connectivity_indicator(connParams->activity_led);
                connParams->disconnect(&deviceConfig);
        }
}

void cellular_buffering_task(void *params)
{

        BufferingTaskParams *connParams = (BufferingTaskParams*)params;
        LoggerMessage msg;

        xQueueHandle sampleQueue = connParams->sampleQueue;
        xQueueHandle buffer_queue = connParams->buffer_queue;

        const size_t max_telem_rate = connParams->max_sample_rate;

        size_t tick = 0;

        const LoggerConfig *logger_config = getWorkingLoggerConfig();

        bool logging_enabled = false;

        uint32_t re_open_buffer_file_timeout = 0;
        uint32_t last_open_buffer_attempt = getCurrentTicks();

        while (1) {
                bool should_stream = logging_enabled ||
                                     logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming ||
                                     connParams->always_streaming;

                while (1) {
                        if (!file_open && isTimeoutMs(last_open_buffer_attempt, re_open_buffer_file_timeout)) {
                                last_open_buffer_attempt = getCurrentTicks();
                                read_index = 0;
                                xSemaphoreTake(fs_mutex, portMAX_DELAY);
                                FRESULT initfs_rc = InitFS();
                                FRESULT fopen_rc = f_open(buffer_file, TELEMETRY_BUFFER_FILENAME, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);

                                if (FR_OK == initfs_rc && FR_OK == fopen_rc) {
                                        file_open = true;
                                        /* try to connect immediately on the first re-attempt*/
                                        re_open_buffer_file_timeout = 0;
                                }
                                else {
                                        if (FR_OK != initfs_rc) {
                                                pr_debug_int_msg(_LOG_PFX "Error Initializing filesystem: ", initfs_rc);
                                        }
                                        if (FR_OK != fopen_rc) {
                                                pr_debug_int_msg(_LOG_PFX "Error opening telemetry buffer file: ", fopen_rc);
                                        }
                                        /* re-connect a bit later */
                                        re_open_buffer_file_timeout = TELEMETRY_BUFFER_FILE_RETRY_MS;
                                }
                                pr_debug_str_msg(_LOG_PFX "Creating telemetry buffer file: ", file_open ? "win" : "fail");
                                xSemaphoreGive(fs_mutex);
                        }

                        should_stream =
                                logging_enabled ||
                                connParams->always_streaming ||
                                logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming;

                        const char res = receive_logger_message(sampleQueue, &msg, IDLE_TIMEOUT);

                        /*///////////////////////////////////////////////////////////
                        // Process a pending message from logger task, if exists
                        ////////////////////////////////////////////////////////////*/
                        if (pdFALSE != res) {
                                switch(msg.type) {
                                case LoggerMessageType_Start: {
                                        tick = 0;
                                        logging_enabled = true;
                                        break;
                                }
                                case LoggerMessageType_Stop: {
                                        logging_enabled = false;
                                        break;
                                }
                                case LoggerMessageType_Sample: {
                                        if (!should_stream ||
                                            !should_sample(msg.ticks, max_telem_rate))
                                                break;

                                        const int send_meta = tick == 0 ||
                                                              (connParams->periodicMeta &&
                                                               (tick % METADATA_SAMPLE_INTERVAL == 0));

                                        bool fs_failed = false;
                                        xSemaphoreTake(fs_mutex, portMAX_DELAY);
                                        if (!file_open) {
                                                goto BUFFER_DONE;
                                        }

                                        FRESULT fseek_res = f_lseek(buffer_file, f_size(buffer_file));
                                        if (FR_OK != fseek_res) {
                                                pr_error_int_msg(_LOG_PFX "Failed to seek to end of buffer: ", fseek_res);
                                                fs_failed = true;
                                                goto BUFFER_DONE;
                                        }

                                        fs_write_sample_record(buffer_file, msg.sample, tick, send_meta);

                                        FRESULT fsync_res = f_sync(buffer_file);
                                        if (FR_OK != fsync_res) {
                                                pr_error_int_msg(_LOG_PFX "Failed to sync buffer file: ", fsync_res);
                                                fs_failed = true;
                                                goto BUFFER_DONE;
                                        }
                                        BUFFER_DONE:
                                        if (fs_failed) {
                                                f_close(buffer_file);
                                                file_open = false;
                                        }
                                        xSemaphoreGive(fs_mutex);

                                        BufferedLoggerMessage buffer_msg;
                                        buffer_msg.sample = msg.sample;
                                        buffer_msg.ticks = msg.ticks;
                                        buffer_msg.type = msg.type;
                                        xQueueSend(buffer_queue, &buffer_msg, 0);

                                        tick++;
                                        break;
                                }
                                default:
                                        pr_info_int_msg(_LOG_PFX "Unknown logger message type ", msg.type);
                                        break;
                                }
                        }

                }
        }
}

void cellular_connectivity_task(void *params)
{

        char * buffer = (char *)portMalloc(BUFFER_SIZE);
        size_t rxCount = 0;

        TelemetryConnParams *connParams = (TelemetryConnParams*)params;
        BufferedLoggerMessage msg;

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

        xQueueHandle api_event_queue = xQueueCreate(API_EVENT_QUEUE_DEPTH, sizeof(struct api_event));
        api_event_create_callback(queue_api_event, api_event_queue);

        bool hard_init = true;
        bool buffering_enabled = false;

        while (1) {
                size_t connect_retries = 0;
                millis_t connected_at = 0;
                bool should_stream = logging_enabled ||
                                     logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming ||
                                     connParams->always_streaming;

                while (should_stream && connParams->init_connection(&deviceConfig, &connected_at, hard_init) != DEVICE_INIT_SUCCESS) {
                        pr_info(_LOG_PFX "not connected. retrying\r\n");
                        vTaskDelay(INIT_DELAY);
                        connect_retries++;
                        if (connect_retries > HARD_INIT_RETRY_THRESHOLD) {
                                pr_info(_LOG_PFX " Too many connection attempts\r\n");
                                hard_init = true;
                                connect_retries = 0;
                        }
                }
                if (connected_at > 0)
                        GPS_set_UTC_time(connected_at);

                serial_flush(serial);
                rxCount = 0;
                size_t badMsgCount = 0;
                size_t tick = 0;
                size_t last_message_time = getUptimeAsInt();
                bool should_reconnect = false;
                hard_init = false;

                int32_t backlog_size = f_size(buffer_file) - read_index;

                if ( backlog_size > 0) {
                        pr_info_int_msg(_LOG_PFX "Telemetry backlog: ", backlog_size);
                }

                while (1) {
                        if ( should_reconnect )
                                break; /*break out and trigger the re-connection if needed */

                        should_stream =
                                logging_enabled ||
                                connParams->always_streaming ||
                                logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming;

                        bool current_buffering_enabled = file_open;
                        if (buffering_enabled != current_buffering_enabled) {
                                pr_info(current_buffering_enabled ? _LOG_PFX "Telemetry buffering enabled\r\n" : _LOG_PFX "Telemetry buffering disabled\r\n");
                                buffering_enabled = current_buffering_enabled;
                        }

                        const char res = xQueueReceive(sampleQueue, &msg, IDLE_TIMEOUT);

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

                                        if (tick == 0) {
                                                /* send one metadata sample at the beginning of the connection */
                                                api_send_sample_record(serial, msg.sample, tick, true);
                                                put_crlf(serial);
                                        }
                                        if (!current_buffering_enabled) {
                                                /* Fall back to non-buffered sample streaming */
                                                api_send_sample_record(serial, msg.sample, tick, false);
                                                put_crlf(serial);
                                        }
                                        else {
                                                /* Stream buffered samples, catching up with the tail of the file as needed */
                                                while (true) {
                                                        char * read_string = NULL;
                                                        xSemaphoreTake(fs_mutex, portMAX_DELAY);
                                                        FRESULT fseek_res = f_lseek(buffer_file, read_index);
                                                        read_string = f_gets(buffer_buffer, BUFFER_BUFFER_SIZE, buffer_file);
                                                        read_index += strlen(read_string);
                                                        xSemaphoreGive(fs_mutex);

                                                        if (FR_OK != fseek_res) {
                                                                pr_error_int_msg("Error reading telemetry buffer, aborting ", fseek_res);
                                                                break;
                                                        }
                                                        if (strlen(read_string) == 0) {
                                                                break;
                                                        }
                                                        serial_write_s(serial, read_string);
                                                }
                                        }

                                        tick++;
                                        break;
                                }
                                default:
                                        pr_info_int_msg(_LOG_PFX "Unknown logger message type ", msg.type);
                                        break;
                                }
                        }

                        /*//////////////////////////////////////////////////////////
                        // Process any pending API events
                        ////////////////////////////////////////////////////////////*/
                        struct api_event api_event;
                        if (xQueueReceive(api_event_queue, &api_event, 0)) {
                                process_api_event(&api_event, serial);
                        }

                        /*//////////////////////////////////////////////////////////
                        // Process incoming message, if available
                        ////////////////////////////////////////////////////////////
                        //read in available characters, process message as necessary*/
                        int msgReceived = processRxBuffer(serial, buffer, &rxCount);
                        /*check the latest contents of the buffer for something that might indicate an error condition*/
                        if (connParams->check_connection_status(&deviceConfig) != DEVICE_STATUS_NO_ERROR) {
                                pr_info(_LOG_PFX "Disconnected\r\n");
                                break;
                        }

                        /*now process a complete message if available*/
                        if (msgReceived) {
                                last_message_time = getUptimeAsInt();

                                const int msgRes = process_api(serial, buffer, BUFFER_SIZE);
                                const int msgError = (msgRes == API_ERROR_MALFORMED);
                                if (msgError) {
                                        pr_debug(_LOG_PFX " (failed)\r\n");
                                }
                                if (msgError) {
                                        badMsgCount++;
                                } else {
                                        badMsgCount = 0;
                                }
                                if (badMsgCount >= BAD_MESSAGE_THRESHOLD) {
                                        pr_warning_int_msg(_LOG_PFX "re-connecting- empty/bad msgs :", badMsgCount );
                                        break;
                                }
                                rxCount = 0;
                        }

                        /*disconnect if a timeout is configured and
                        // we haven't heard from the other side for a while */
                        const size_t timeout = getUptimeAsInt() - last_message_time;
                        if (connection_timeout && timeout > connection_timeout ) {
                                pr_info_str_msg(_LOG_PFX " Timeout ", connParams->connectionName);
                                should_reconnect = true;
                        }
                }

                clear_connectivity_indicator(connParams->activity_led);
                connParams->disconnect(&deviceConfig);
        }
}


