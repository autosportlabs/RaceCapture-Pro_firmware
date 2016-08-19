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

#ifndef _AT_H_
#define _AT_H_

#include "cpp_guard.h"
#include "dateTime.h"
#include "serial_buffer.h"

CPP_GUARD_BEGIN

/*
 * These setting define the various default parameters within the
 * AT state machine.
 */
/* Maximum # of commands queued at once */
#define AT_CMD_MAX_CMDS	3
/* Maximum String length of a command */
#define AT_CMD_MAX_LEN	64
/* Maximum # of chars in the device delimeter string (including NULL) */
#define AT_DEV_CVG_DELIM_MAX_LEN	3
/* Maximum number of message lines we can receive per command. */
#define AT_RSP_MAX_MSGS	8
/* Maximum length of a URC prefix */
#define AT_URC_MAX_LEN	16
/* Maximum number of URCs that can be registered. */
#define AT_URC_MAX_URCS	16
/* Maximum amount of time a URC message should take to complete */
#define AT_URC_TIMEOUT_MS	5

enum at_rx_state {
        AT_RX_STATE_READY,
        AT_RX_STATE_CMD,
        AT_RX_STATE_URC,
};

enum at_cmd_state {
        AT_CMD_STATE_READY,
        AT_CMD_STATE_IN_PROGRESS,
        AT_CMD_STATE_QUIET,
};

/**
 * All failure status values are < 0.  All successful command status values are > 0.
 * All successful URC status values are >= 0.
 */
enum at_rsp_status {
        AT_RSP_STATUS_SEND_FAIL = -5,
        AT_RSP_STATUS_TIMEOUT   = -4,
        AT_RSP_STATUS_FAILED    = -3,
        AT_RSP_STATUS_ABORTED   = -2,
        AT_RSP_STATUS_ERROR     = -1,
        AT_RSP_STATUS_NONE      =  0,
        AT_RSP_STATUS_OK        =  1,
        AT_RSP_STATUS_SEND_OK   =  2,
};

struct at_rsp {
        enum at_rsp_status status;
        tiny_millis_t run_time;
        size_t msg_count;
        char *msgs[AT_RSP_MAX_MSGS];
};

/* A command to send to our modem */
struct at_cmd {
        bool (*rsp_cb)(struct at_rsp *rsp, void *rsp_up);
        void *rsp_up;
        tiny_millis_t timeout_ms;
        char cmd[AT_CMD_MAX_LEN];
};

struct at_cmd_queue {
        size_t count;
        struct at_cmd *head;
        struct at_cmd cmds[AT_CMD_MAX_CMDS];
};

struct at_timing {
        tiny_millis_t cmd_start_ms;
        tiny_millis_t urc_start_ms;
        tiny_millis_t quiet_start_ms;
};

enum at_dev_cfg_flag {
	AT_DEV_CFG_FLAG_NONE = 0,
	/* Used for AT devices that will send URCS mid command response */
	AT_DEV_CFG_FLAG_RUDE = 1 << 0,
};

struct at_dev_cfg {
	char delim[AT_DEV_CVG_DELIM_MAX_LEN];
	tiny_millis_t quiet_period_ms;
	enum at_dev_cfg_flag flags;
};

enum at_urc_flags {
	AT_URC_FLAGS_NONE = 0, /* For init with no flags set */
	/* Indicates URC is only one msg with no status */
	AT_URC_FLAGS_NO_RSP_STATUS = 1 << 0,
	/* Indicates we should not strip the trailing msg whitespace chars. */
	AT_URC_FLAGS_NO_RSTRIP	   = 1 << 1,
};

struct at_urc {
        bool (*rsp_cb)(struct at_rsp *rsp, void *rsp_up);
        void *rsp_up;
        enum at_urc_flags flags;
        size_t pfx_len;
        char pfx[AT_URC_MAX_LEN];
};

struct at_urc_list {
        size_t count;
        struct at_urc urcs[AT_URC_MAX_URCS];
};

/**
 * Callback for sparse URCs.  This is designed to handle cases
 * where AT commands introduce sparse messages that would be difficult
 * to register a callback against.  An example would be `0,CONNECTED`;
 * this is hard to register since the 0 indicates the multiplexed
 * channel, and in this case there are 5 potential combinations.
 * Really this is a broken AT URC, so we use as sort of broken way
 * to handle it.  A proper URC would have something like
 * `+CONNINF: 0,"CONNECTED".  Then you would be able to handle this
 * with a URC handler since you could match the prefix.  Oh well,
 * imperfect solution for an imperfect world.
 * @param msg The message that was received.
 * @return true if the callback was able to parse the message,
 * false otherwise.
 */
typedef bool sparse_urc_cb_t(char* msg);

struct at_info {
        /*
         * All of this is really read only.  Don't alter directly.
         * In fact, you should probably never be accessing these directly
         * unless you are debugging.  Use the methods provided.  They will
         * keep the state in here sane.
         */
        enum at_rx_state rx_state;
        enum at_cmd_state cmd_state;
        struct serial_buffer *sb;
        struct at_cmd *cmd_ip;
        struct at_urc *urc_ip;
        struct at_dev_cfg dev_cfg;
        struct at_rsp rsp;
        struct at_timing timing;
        struct at_cmd_queue cmd_queue;
        struct at_urc_list urc_list;
        sparse_urc_cb_t *sparse_urc_cb;
};

void at_reset(struct at_info* ati);

bool at_info_init(struct at_info *ati, struct serial_buffer *sb);

void at_task(struct at_info *ati, const size_t ms_delay);

void at_set_sparse_urc_cb(struct at_info* ati, sparse_urc_cb_t* cb);

struct at_cmd* at_put_cmd(struct at_info *ati, const char *cmd,
                          const tiny_millis_t timeout,
                          bool (*rsp_cb)(struct at_rsp *rsp, void *rsp_up),
                          void *rsp_up);

struct at_urc* at_register_urc(struct at_info *ati, const char *pfx,
                               const enum at_urc_flags flags,
                               bool (*rsp_cb)(struct at_rsp *rsp, void *rsp_up),
                               void *rsp_up);

bool at_configure_device(struct at_info *ati, const tiny_millis_t qp_ms,
                         const char *delim, const enum at_dev_cfg_flag flags);

bool at_ok(struct at_rsp *rsp);

size_t at_parse_rsp_line(char *rsp, char *bkts[], const size_t num_bkts);

char* at_parse_rsp_str(char *rsp);


CPP_GUARD_END

#endif /* _AT_H_ */
