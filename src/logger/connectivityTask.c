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
#include "api_event.h"

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

#define TELEMETRY_BUFFER_FILE_SYNC_INTERVAL 100
#define TELEMETRY_STACK_SIZE 300
#define CELLULAR_TELEMETRY_STACK_SIZE 330
#define BAD_MESSAGE_THRESHOLD     10
#define API_EVENT_QUEUE_DEPTH 2
#define CELLULAR_TELEMETRY_BUFFER_QUEUE_DEPTH 1
#define METADATA_SAMPLE_INTERVAL    100
#define HARD_INIT_RETRY_THRESHOLD 5

#define TELEMETRY_BUFFER_FILENAME "tele.buf"
#define TELEMETRY_BUFFER_FILE_RETRY_MS 1000

#define BUFFERED_CHUNK_SIZE 7000
#define BUFFERED_CHUNK_WAIT 1000
#define BUFFERED_MAX_SIZE 1024 * 1000

static xQueueHandle g_sampleQueue[CONNECTIVITY_CHANNELS] = CONNECTIVITY_TASK_INIT;

#if BLUETOOTH_SUPPORT
static char bluetooth_buffer[BUFFER_SIZE];
#endif

#if CELLULAR_SUPPORT
static CellularState cellular_state = {
                .buffer_queue = NULL,
                .buffer_file = NULL,
                .buffer_buffer = {},
                .cell_buffer = {},
                .read_index = 0,
                .buffer_file_open = false,
                .should_reconnect = false,
                .server_tick_echo = 0,
                .server_tick_echo_changed_at = 0,
                .sample_offset_map = {0},
                .sample_offset_map_index = 0
};

bool cellular_telemetry_buffering_enabled(void)
{
        return cellular_state.buffer_file_open;
}

void cellular_telemetry_reconnect()
{
        cellular_state.should_reconnect = true;
}

void cellular_update_last_server_tick_echo(uint32_t server_tick_echo)
{
        if (server_tick_echo != cellular_state.server_tick_echo)
                cellular_state.server_tick_echo_changed_at = getCurrentTicks();
        cellular_state.server_tick_echo = server_tick_echo;
}

/* reset the sample offset map circular buffer */
static void cellular_reset_buffer_offset_map(void){
        memset(cellular_state.sample_offset_map, 0, SAMPLE_TRACKING_WINDOW * sizeof(SampleOffsetMap));
}

/* get an offset by the specified tick */
static bool cellular_get_buffer_offset_by_tick(uint32_t tick, uint32_t *offset)
{
        SampleOffsetMap *map = cellular_state.sample_offset_map;
        for (size_t i = 0; i < SAMPLE_TRACKING_WINDOW; i++){
                if (map->tick == tick){
                        *offset = map->buffer_file_index;
                        return true;
                }
                map++;
        }
        return false;
}

/* store an offset into the circular buffer using the tick as a 'key' */
static void cellular_add_buffer_offset_tick(uint32_t tick, uint32_t offset)
{
        int32_t index = cellular_state.sample_offset_map_index;
        index++;
        index = index >= SAMPLE_TRACKING_WINDOW ? 0 : index;
        cellular_state.sample_offset_map[index].buffer_file_index = offset;
        cellular_state.sample_offset_map[index].tick = tick;
        cellular_state.sample_offset_map_index = index;
}

/* A cheap way to extract a value from the json sample string */
static bool get_tick_from_sample_string(char * sample_string, uint32_t *tick)
{
        char * tick_string = strstr(sample_string,"{\"s\":{\"t\":");
        if (tick_string) {
                *tick = atoi(tick_string + 10);
                return true;
        }
        return false;
}
#endif

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

int process_rx_buffer(struct Serial *serial, char *buffer, size_t *rxCount)
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

#if BLUETOOTH_SUPPORT

static void create_bluetooth_connection_task(int16_t priority,
                xQueueHandle sampleQueue,
                enum led activity_led)
{
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
        xTaskCreate(bluetooth_connectivity_task, task_name, TELEMETRY_STACK_SIZE,
                    params, priority, NULL );
}
#endif

#if CELLULAR_SUPPORT
static void create_cellular_connection_tasks(int16_t priority,
                xQueueHandle sampleQueue,
                enum led activity_led)
{
        cellular_state.buffer_file = pvPortMalloc(sizeof(FIL));
        cellular_state.buffer_queue = xQueueCreate(CELLULAR_TELEMETRY_BUFFER_QUEUE_DEPTH, sizeof(BufferedLoggerMessage));

        {
                BufferingTaskParams * params = (BufferingTaskParams *)portMalloc(sizeof(BufferingTaskParams));
                params->connectionName = "TelemBuffer";
                params->periodicMeta = 0;
                params->sampleQueue = sampleQueue;
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
                params->connection_timeout = TELEMETRY_DISCONNECT_TIMEOUT * 1000;
                params->disconnect = &cellular_disconnect;
                params->check_connection_status = &cellular_check_connection_status;
                params->init_connection = &cellular_init_connection;
                params->serial = SERIAL_TELEMETRY;
                params->sampleQueue = cellular_state.buffer_queue;
                params->always_streaming = false;
                params->max_sample_rate = SAMPLE_10Hz;
                params->activity_led = activity_led;

                /* Make all task names 16 chars including NULL char */
                static const signed portCHAR task_name[] = "Cell Telemetry";

                xTaskCreate(cellular_connectivity_task, task_name, CELLULAR_TELEMETRY_STACK_SIZE,
                            params, priority, NULL );
        }
}
#endif



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
                //ConnectivityConfig *connConfig =
                  //      &getWorkingLoggerConfig()->ConnectivityConfigs;
                /*
                 * Logic to control which connection is considered 'primary',
                 * which is used later to determine which task has control
                 * over LED flashing
                 */

#if CELLULAR_SUPPORT
                const uint8_t cellEnabled = getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig.cellEnabled;
                if (cellEnabled)
                        create_cellular_connection_tasks(priority,
                                                      g_sampleQueue[1], LED_TELEMETRY);
#else
#if BLUETOOTH_SUPPORT
                const uint8_t cellEnabled = false;
#endif
#endif
#if BLUETOOTH_SUPPORT
                if (getWorkingLoggerConfig()->ConnectivityConfigs.bluetoothConfig.btEnabled) {
                        /* Pick the bluetooth LED if available */
                        enum led activity_led = led_available(LED_BLUETOOTH) ? LED_BLUETOOTH : LED_TELEMETRY;
                        activity_led = cellEnabled && activity_led == LED_TELEMETRY ? LED_UNKNOWN : activity_led;

                        create_bluetooth_connection_task(priority,
                                                     g_sampleQueue[0],
                                                     activity_led);

                }
#endif
        }
        break;
        default:
                pr_error_int_msg(_LOG_PFX "Error creating connectivity task incorrect number of channels ", CONNECTIVITY_CHANNELS);
                break;
        }
}

#if BLUETOOTH_SUPPORT

static void queue_bluetooth_api_event(const struct api_event * api_event, void * data)
{
        xQueueHandle queue = data;
        if (xQueueSendToBack(queue, api_event, 0)) {
                pr_trace(_LOG_PFX "queued api event\r\n");
        } else {
                pr_warning(_LOG_PFX "bluetooth api event queue overflow\r\n");
        }
}
void bluetooth_connectivity_task(void *params)
{
        size_t rx_buffer_count = 0;

        ConnParams *connParams = (ConnParams*)params;
        LoggerMessage msg;

        struct Serial *serial = serial_device_get(connParams->serial);

        xQueueHandle sampleQueue = connParams->sampleQueue;
        uint32_t connection_timeout = connParams->connection_timeout;
        const size_t max_telem_rate = connParams->max_sample_rate;

        DeviceConfig deviceConfig;
        deviceConfig.serial = serial;
        deviceConfig.buffer = bluetooth_buffer;
        deviceConfig.length = BUFFER_SIZE;

        const LoggerConfig *logger_config = getWorkingLoggerConfig();

        bool logging_enabled = false;

        xQueueHandle api_event_queue = xQueueCreate(API_EVENT_QUEUE_DEPTH, sizeof(struct api_event));
        api_event_create_callback(queue_bluetooth_api_event, api_event_queue);

        bool hard_init = true;
        while (1) {
                size_t connect_retries = 0;
                millis_t connected_at = 0;
                uint32_t last_tick = 0;
                bool should_stream = logging_enabled ||
                                     logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming ||
                                     connParams->always_streaming;

                while (should_stream && connParams->init_connection(&deviceConfig, &connected_at, &last_tick, hard_init) != DEVICE_INIT_SUCCESS) {
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
                rx_buffer_count = 0;
                size_t bad_message_count = 0;
                uint32_t tick = 0;
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

                                        led_toggle(connParams->activity_led);

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
                        int msgReceived = process_rx_buffer(serial, bluetooth_buffer, &rx_buffer_count);
                        /*check the latest contents of the buffer for something that might indicate an error condition*/
                        if (connParams->check_connection_status(&deviceConfig) != DEVICE_STATUS_NO_ERROR) {
                                pr_info(_LOG_PFX "Disconnected\r\n");
                                break;
                        }

                        /*now process a complete message if available*/
                        if (msgReceived) {
                                last_message_time = getUptimeAsInt();

                                pr_info_str_msg("received msg ", bluetooth_buffer);

                                const int msgRes = process_api(serial, bluetooth_buffer, BUFFER_SIZE);
                                const int msgError = (msgRes == API_ERROR_MALFORMED);
                                if (msgError) {
                                        pr_debug(_LOG_PFX " (failed)\r\n");
                                }
                                if (msgError) {
                                        bad_message_count++;
                                } else {
                                        bad_message_count = 0;
                                }
                                if (bad_message_count >= BAD_MESSAGE_THRESHOLD) {
                                        pr_warning_int_msg(_LOG_PFX "re-connecting- empty/bad msgs :", bad_message_count );
                                        break;
                                }
                                rx_buffer_count = 0;
                        }

                        /*disconnect if a timeout is configured and
                        // we haven't heard from the other side for a while */
                        const size_t timeout = getUptimeAsInt() - last_message_time;
                        if (connection_timeout && timeout > connection_timeout ) {
                                pr_info_str_msg(_LOG_PFX " Timeout ", connParams->connectionName);
                                should_reconnect = true;
                        }
                }

                led_disable(connParams->activity_led);
                connParams->disconnect(&deviceConfig);
        }
}
#endif

#if CELLULAR_SUPPORT
void cellular_buffering_task(void *params)
{

        BufferingTaskParams *connParams = (BufferingTaskParams*)params;
        LoggerMessage msg;

        xQueueHandle sampleQueue = connParams->sampleQueue;
        const size_t max_telem_rate = connParams->max_sample_rate;

        size_t tick = 0;

        const LoggerConfig *logger_config = getWorkingLoggerConfig();

        bool logging_enabled = false;

        uint32_t re_open_buffer_file_timeout = 0;
        uint32_t last_open_buffer_attempt = getCurrentTicks();
        uint32_t buffer_file_open_retries = 0;

        while (1) {
                bool should_stream = logging_enabled ||
                                     logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming ||
                                     connParams->always_streaming;

                while (1) {
                        if (!cellular_state.buffer_file_open && isTimeoutMs(last_open_buffer_attempt, re_open_buffer_file_timeout)) {
                                last_open_buffer_attempt = getCurrentTicks();
                                cellular_reset_buffer_offset_map();
                                cellular_state.read_index = 0;
                                fs_lock();
                                bool fs_good = sdcard_fs_mounted();
                                if (!fs_good) {
                                        FRESULT initfs_rc = InitFS();
                                        fs_good = (initfs_rc == FR_OK);
                                        if (FR_OK != initfs_rc) {
                                                if (!buffer_file_open_retries)
                                                        pr_info_int_msg(_LOG_PFX "Error Initializing filesystem: ", initfs_rc);
                                        }
                                }

                                if (fs_good) {
                                        bool sd_write_validated = test_sd(NULL, 1, 0, 1);
                                        if (sd_write_validated) {
                                                FRESULT fopen_rc = f_open(cellular_state.buffer_file, TELEMETRY_BUFFER_FILENAME, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
                                                if (FR_OK == fopen_rc) {
                                                        FRESULT truncate_rc = f_truncate(cellular_state.buffer_file);
                                                        if (FR_OK == truncate_rc){
                                                                cellular_state.buffer_file_open = true;
                                                                /* try to connect immediately on the first re-attempt*/
                                                                re_open_buffer_file_timeout = 0;
                                                                buffer_file_open_retries = 0;
                                                        }
                                                        else{
                                                                if (!buffer_file_open_retries)
                                                                        pr_info_int_msg(_LOG_PFX "Error truncating telemetry buffer file: ", truncate_rc);
                                                        }
                                                }
                                                else {
                                                        if (!buffer_file_open_retries)
                                                                pr_info_int_msg(_LOG_PFX "Error opening telemetry buffer file: ", fopen_rc);
                                                }
                                        }
                                        else {
                                                if (!buffer_file_open_retries)
                                                        pr_info(_LOG_PFX "SD write verification failed. Try re-formatting SD card\r\n");
                                        }
                                }
                                fs_unlock();
                                if (!cellular_state.buffer_file_open) {
                                        /* try re-opening a bit later */
                                        re_open_buffer_file_timeout = TELEMETRY_BUFFER_FILE_RETRY_MS;
                                        buffer_file_open_retries++;
                                }
                                pr_debug_str_msg(_LOG_PFX "Creating telemetry buffer file: ", cellular_state.buffer_file_open ? "win" : "fail");
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
                                        fs_lock();
                                        if (!cellular_state.buffer_file_open) {
                                                goto BUFFER_DONE;
                                        }

                                        DWORD file_size = f_size(cellular_state.buffer_file);

                                        if (file_size > BUFFERED_MAX_SIZE) {
                                                pr_info_int_msg(_LOG_PFX "Max buffer size exceeded, resetting: ", BUFFERED_MAX_SIZE);
                                                fs_failed = true;
                                                goto BUFFER_DONE;
                                        }

                                        FRESULT fseek_res = f_lseek(cellular_state.buffer_file, file_size);
                                        if (FR_OK != fseek_res) {
                                                pr_error_int_msg(_LOG_PFX "Failed to seek to end of buffer: ", fseek_res);
                                                fs_failed = true;
                                                goto BUFFER_DONE;
                                        }

                                        fs_write_sample_record(cellular_state.buffer_file, msg.sample, tick, send_meta);

                                        if (tick % TELEMETRY_BUFFER_FILE_SYNC_INTERVAL == 0) {
                                                pr_debug_int_msg(_LOG_PFX "Flushing buffer file: ", tick);
                                                FRESULT fsync_res = f_sync(cellular_state.buffer_file);
                                                if (FR_OK != fsync_res) {
                                                        pr_error_int_msg(_LOG_PFX "Failed to sync buffer file: ", fsync_res);
                                                        fs_failed = true;
                                                        goto BUFFER_DONE;
                                                }
                                        }

                                        BUFFER_DONE:
                                        if (fs_failed ) {
                                                f_close(cellular_state.buffer_file);
                                                cellular_state.buffer_file_open = false;
                                        }
                                        fs_unlock();

                                        BufferedLoggerMessage buffer_msg;
                                        buffer_msg.sample = msg.sample;
                                        buffer_msg.ticks = msg.ticks;
                                        buffer_msg.type = msg.type;
                                        xQueueSend(cellular_state.buffer_queue, &buffer_msg, 0);

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

static void queue_cellular_api_event(const struct api_event * api_event, void * data)
{
        xQueueHandle queue = data;
        if (xQueueSendToBack(queue, api_event, 0)) {
                pr_trace(_LOG_PFX "queued api event\r\n");
        } else {
                pr_warning(_LOG_PFX "cellular api event queue overflow\r\n");
        }
}

void cellular_connectivity_task(void *params)
{
        size_t rx_buffer_count = 0;

        TelemetryConnParams *connParams = (TelemetryConnParams*)params;
        BufferedLoggerMessage msg;

        struct Serial *serial = serial_device_get(connParams->serial);

        xQueueHandle sampleQueue = connParams->sampleQueue;
        uint32_t connection_timeout = connParams->connection_timeout;
        const size_t max_telem_rate = connParams->max_sample_rate;

        DeviceConfig deviceConfig;
        deviceConfig.serial = serial;
        deviceConfig.buffer = cellular_state.cell_buffer;
        deviceConfig.length = BUFFER_SIZE;

        const LoggerConfig *logger_config = getWorkingLoggerConfig();

        bool logging_enabled = false;

        xQueueHandle api_event_queue = xQueueCreate(API_EVENT_QUEUE_DEPTH, sizeof(struct api_event));
        api_event_create_callback(queue_cellular_api_event, api_event_queue);

        bool hard_init = true;
        bool buffering_enabled = false;

        while (1) {
                size_t connect_retries = 0;
                millis_t connected_at = 0;
                uint32_t last_tick = 0;
                bool should_stream = logging_enabled ||
                                     logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming ||
                                     connParams->always_streaming;

                while (should_stream && connParams->init_connection(&deviceConfig, &connected_at, &last_tick, hard_init) != DEVICE_INIT_SUCCESS) {
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
                rx_buffer_count = 0;
                size_t bad_api_msg_count = 0;
                cellular_state.should_reconnect = false;

                uint32_t file_offset = 0;
                if(cellular_get_buffer_offset_by_tick(last_tick, &file_offset)) {
                        cellular_state.read_index = file_offset;
                        cellular_state.server_tick_echo = last_tick;
                }
                else {
                        pr_info_int_msg(_LOG_PFX "could not find precise location in buffer file for tick: ", last_tick);
                }

                cellular_state.server_tick_echo = 0;
                cellular_state.server_tick_echo_changed_at = getCurrentTicks();

                hard_init = false;

                fs_lock();
                int32_t backlog_size = f_size(cellular_state.buffer_file) - cellular_state.read_index;
                fs_unlock();

                if ( backlog_size > 0) {
                        pr_info_int_msg(_LOG_PFX "Telemetry backlog: ", backlog_size);
                }

                while (1) {
                        if ( cellular_state.should_reconnect )
                                break; /*break out and trigger the re-connection if needed */

                        should_stream =
                                logging_enabled ||
                                connParams->always_streaming ||
                                logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming;

                        bool current_buffering_enabled = cellular_state.buffer_file_open;
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
                                        logging_enabled = true;
                                        /* If we're not already streaming trigger a re-connect */
                                        if (!should_stream)
                                                cellular_state.should_reconnect = true;
                                        break;
                                }
                                case LoggerMessageType_Stop: {
                                        api_sendLogEnd(serial);
                                        put_crlf(serial);
                                        if (! (logger_config->ConnectivityConfigs.telemetryConfig.backgroundStreaming ||
                                               connParams->always_streaming))
                                                cellular_state.should_reconnect = true;
                                        logging_enabled = false;
                                        break;
                                }
                                case LoggerMessageType_Sample: {
                                        if (!should_stream ||
                                            !should_sample(msg.ticks, max_telem_rate))
                                                break;

                                        led_toggle(connParams->activity_led);

                                        if (!current_buffering_enabled) {
                                                /* Fall back to non-buffered sample streaming */
                                                api_send_sample_record(serial, msg.sample, msg.ticks, false);
                                                put_crlf(serial);
                                        }
                                        else {
                                                /* Stream buffered samples, catching up with the tail of the file as needed */
                                                int32_t start_index = cellular_state.read_index;
                                                while (true) {
                                                        char * read_string = NULL;
                                                        fs_lock();
                                                        FRESULT fseek_res = f_lseek(cellular_state.buffer_file, cellular_state.read_index);
                                                        read_string = f_gets(cellular_state.buffer_buffer, BUFFER_BUFFER_SIZE, cellular_state.buffer_file);
                                                        fs_unlock();

                                                        if (FR_OK != fseek_res) {
                                                                pr_error_int_msg("Error reading telemetry buffer, aborting ", fseek_res);
                                                                break;
                                                        }

                                                        if (read_string == NULL || strlen(read_string) == 0)
                                                                break;

                                                        serial_write_s(serial, read_string);
                                                        cellular_state.read_index += strlen(read_string);

                                                        uint32_t tick = 0;
                                                        if(get_tick_from_sample_string(read_string, &tick)){
                                                                cellular_add_buffer_offset_tick(tick, cellular_state.read_index);
                                                        }

                                                        if (cellular_state.read_index - start_index > BUFFERED_CHUNK_SIZE){
                                                                delayMs(BUFFERED_CHUNK_WAIT);
                                                                start_index = cellular_state.read_index;
                                                        }
                                                }
                                        }
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
                        int msgReceived = process_rx_buffer(serial, cellular_state.cell_buffer, &rx_buffer_count);
                        /*check the latest contents of the buffer for something that might indicate an error condition*/
                        if (connParams->check_connection_status(&deviceConfig) != DEVICE_STATUS_NO_ERROR) {
                                pr_info(_LOG_PFX "Disconnected\r\n");
                                break;
                        }

                        /*now process a complete message if available*/
                        if (msgReceived) {
                                const int msgRes = process_api(serial, cellular_state.cell_buffer, BUFFER_SIZE);
                                const int msgError = (msgRes == API_ERROR_MALFORMED);
                                if (msgError) {
                                        pr_error_int_msg(_LOG_PFX " process_api_failed ", msgRes);
                                        pr_error_str_msg(_LOG_PFX " message: ", cellular_state.cell_buffer);
                                }
                                if (msgError) {
                                        bad_api_msg_count++;
                                } else {
                                        bad_api_msg_count = 0;
                                }
                                if (bad_api_msg_count >= BAD_MESSAGE_THRESHOLD) {
                                        pr_warning_int_msg(_LOG_PFX "re-connecting- empty/bad msgs :", bad_api_msg_count );
                                        break;
                                }
                                rx_buffer_count = 0;
                        }

                        /*disconnect if a timeout is configured and
                        // we haven't heard from the other side for a while */
                        if (isTimeoutMs(cellular_state.server_tick_echo_changed_at, connection_timeout)) {
                                pr_info_str_msg(_LOG_PFX " Heartbeat timeout ", connParams->connectionName);
                                cellular_state.should_reconnect = true;
                        }
                }

                led_disable(connParams->activity_led);
                connParams->disconnect(&deviceConfig);
        }
}
#endif

