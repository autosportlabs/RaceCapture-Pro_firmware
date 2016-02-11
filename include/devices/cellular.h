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

#ifndef _CELLULAR_H_
#define _CELLULAR_H_

#include "cpp_guard.h"
#include "dateTime.h"
#include "devices_common.h"
#include "loggerConfig.h"
#include "serial_buffer.h"
#include "stddef.h"

CPP_GUARD_BEGIN

typedef enum {
        TELEMETRY_STATUS_IDLE = 0,
        TELEMETRY_STATUS_CONNECTED,
        TELEMETRY_STATUS_CURRENT_CONNECTION_TERMINATED,
        TELEMETRY_STATUS_REJECTED_DEVICE_ID,
        TELEMETRY_STATUS_DATA_PLAN_NOT_AVAILABLE,
        TELEMETRY_STATUS_SERVER_CONNECTION_FAILED,
        TELEMETRY_STATUS_INTERNET_CONFIG_FAILED,
        TELEMETRY_STATUS_CELL_REGISTRATION_FAILED,
        TELEMETRY_STATUS_MODEM_INIT_FAILED,
} telemetry_status_t;

enum cellular_modem {
        CELLULAR_MODEM_UNKNOWN = 0,
        CELLULAR_MODEM_SIM900,
        CELLULAR_MODEM_UBLOX_SARA_U280,
};

enum cellular_net_status {
        CELLULAR_NETWORK_STATUS_UNKNOWN = 0,
        CELLULAR_NETWORK_NOT_SEARCHING,
        CELLULAR_NETWORK_SEARCHING,
        CELLULAR_NETWORK_DENIED,
        CELLULAR_NETWORK_REGISTERED,
};


/* E.164 standard.  15 digits + 1 null terminator */
#define CELLULAR_INFO_NUMBER_MAX_LENGTH 16

/* Up to 17 digits with IMEIST (15 with IMEI).  So 17 + 1 */
#define CELLULAR_INFO_IMEI_MAX_LENGTH 18

/* Don't know a good value.  So setting arbitrary one. */
#define CELLULAR_INFO_OPERATOR_MAX_LEN 18

struct at_config {
        unsigned int urc_delay_ms;
};

struct cellular_info {
        enum cellular_net_status net_status;
        int signal;
        char number[CELLULAR_INFO_NUMBER_MAX_LENGTH];
        char imei[CELLULAR_INFO_IMEI_MAX_LENGTH];
        char op[CELLULAR_INFO_OPERATOR_MAX_LEN];
};

struct telemetry_info {
        telemetry_status_t status;
        tiny_millis_t active_since;
        int socket;
};


struct cell_modem_methods {
        const struct at_config* (*get_at_config)();
        bool (*init_modem)(struct serial_buffer *sb,
                           struct cellular_info *ci);
        bool (*get_sim_info)(struct serial_buffer *sb,
                             struct cellular_info *ci);
        bool (*register_on_network)(struct serial_buffer *sb,
                                    struct cellular_info *ci);
        bool (*get_network_info)(struct serial_buffer *sb,
                                 struct cellular_info *ci);
        bool (*setup_pdp)(struct serial_buffer *sb,
                          struct cellular_info *ci,
                          const CellularConfig *cc);
        bool (*open_telem_connection)(struct serial_buffer *sb,
                                      struct cellular_info *ci,
                                      struct telemetry_info *ti,
                                      const TelemetryConfig *tc);
        bool (*close_telem_connection)(struct serial_buffer *sb,
                                       struct cellular_info *ci,
                                       struct telemetry_info *ti);
};

int cell_get_signal_strength();

bool is_rsp(const char **msgs, const size_t count, const char *ans);
bool is_rsp_ok(const char **msgs, const size_t count);

size_t cellular_exec_cmd(struct serial_buffer *sb, const size_t wait,
                         const char **rx_msgs, const size_t rx_msgs_size);

int cellular_exec_match(struct serial_buffer *sb, const size_t wait,
                        const char **rx_msgs, const size_t rx_msgs_size,
                        const char *answrs[], const size_t answrs_size,
                        const size_t rx_msg_idx);

const char* cell_get_subscriber_number();
const char* cell_get_IMEI();
telemetry_status_t cellular_get_connection_status();
enum cellular_net_status cellmodem_get_status(void);
int32_t cellular_active_time();
int cellular_disconnect(DeviceConfig *config);
int cellular_init_connection(DeviceConfig *config);
int cellular_check_connection_status(DeviceConfig *config);
const char* readsCell(struct serial_buffer *sb, size_t timeout);
void putsCell(struct serial_buffer *sb, const char *data);
const char* cellular_get_net_status_desc();

CPP_GUARD_END

#endif /* _CELLULAR_H_ */
