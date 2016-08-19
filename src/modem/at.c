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

#include "at.h"
#include "dateTime.h"
#include "macros.h"
#include "printk.h"
#include "serial_buffer.h"
#include "str_util.h"
#include <ctype.h>
#include <string.h>

#define AT_DEFAULT_QP_MS	250
#define AT_DEFAULT_DELIMETER	"\r\n"

static const enum log_level dbg_lvl = INFO;

#define AT_STATUS_MSG(a, b)	{(a), ARRAY_LEN(a), (b)}
static const struct at_rsp_status_msgs {
        const char* str;
        size_t str_len;
        enum at_rsp_status status;
} at_status_msgs[] = {
        AT_STATUS_MSG("OK", AT_RSP_STATUS_OK),
        AT_STATUS_MSG("SEND OK", AT_RSP_STATUS_SEND_OK),
        AT_STATUS_MSG("SEND FAIL", AT_RSP_STATUS_SEND_FAIL),
        AT_STATUS_MSG("FAIL", AT_RSP_STATUS_FAILED),
        AT_STATUS_MSG("ERROR", AT_RSP_STATUS_ERROR),
        AT_STATUS_MSG("ABORTED", AT_RSP_STATUS_ABORTED),
};

static bool is_timed_out(const tiny_millis_t t_start,
                         const tiny_millis_t t_len)
{
        return t_start + t_len <= getUptime();
}

static void _complete_msg_cleanup(struct at_info *ati)
{
        serial_buffer_clear(ati->sb);
        memset(&ati->rsp, 0, sizeof(ati->rsp));
        ati->rx_state = AT_RX_STATE_READY;
}

static void complete_urc(struct at_info *ati, const enum at_rsp_status status)
{
        ati->rsp.status = status;
        ati->rsp.run_time = getUptime() - ati->timing.urc_start_ms;

        bool more = false;
        if (ati->urc_ip->rsp_cb)
                more = ati->urc_ip->rsp_cb(&ati->rsp, ati->urc_ip->rsp_up);

        if (more)
                /* Then we have more to process after this status message */
                return;

        /* Post URC, cleanup is a wee bit different than of cmd */
        _complete_msg_cleanup(ati);
        ati->urc_ip = NULL;
}

static void complete_cmd(struct at_info *ati, const enum at_rsp_status status)
{
        ati->rsp.status = status;
        ati->rsp.run_time = getUptime() - ati->timing.cmd_start_ms;

        bool more = false;
        if (ati->cmd_ip->rsp_cb)
                more = ati->cmd_ip->rsp_cb(&ati->rsp, ati->cmd_ip->rsp_up);

        if (more)
                /* Then we have more to process after this status message */
                return;

        /* Do all post command cleanup here */
        _complete_msg_cleanup(ati);
        ati->cmd_ip = NULL;

        /* Begin the quiet period post command */
        ati->cmd_state = AT_CMD_STATE_QUIET;
        ati->timing.quiet_start_ms = getUptime();
}


static void at_task_run_no_bytes(struct at_info *ati)
{
        if (AT_RX_STATE_URC == ati->rx_state) {
                /*
                 * Then we are waiting for the rest of a URC response.
                 * Ensure that we have not passed the URC timeout period.
                 * Honestly, we should NEVER really get here, or if we do,
                 * we should never timeout on a URC.  Doing so likely
                 * indicates a bug in usage or architecture.
                 */
                if (is_timed_out(ati->timing.urc_start_ms,
                                 AT_URC_TIMEOUT_MS)) {
                        pr_warning("[at] Timed out in a URC!?!. "
                                   "Should never happen. Likely a bug.\r\n");
                        return complete_urc(ati, AT_RSP_STATUS_TIMEOUT);
                }
        } else if (AT_CMD_STATE_IN_PROGRESS == ati->cmd_state) {
                /*
                 * If here, then we received no data, but a command is in
                 * progress.  In this case, ensure that the command hasn't
                 * reached its timeout period.
                 */
                if (is_timed_out(ati->timing.cmd_start_ms,
                                 ati->cmd_ip->timeout_ms)) {
                        printk(dbg_lvl, "[at] Command timed out\r\n");
                        return complete_cmd(ati, AT_RSP_STATUS_TIMEOUT);
                }
        }

        /*
         * If here, then either we are not in the correct state or nothing
         * has timed out.  Move along.
         */
}

static struct at_urc* is_urc_msg(struct at_info *ati, char *msg)
{
        /*
         * To figure this out, lets see if we have a URC call that
         * matches it.
         */
        for (size_t i = 0; i < ati->urc_list.count; ++i) {
                struct at_urc *urc = ati->urc_list.urcs + i;
                if (0 == strncmp(msg, urc->pfx, urc->pfx_len))
                        return urc;
        }

        return NULL;
}

static bool is_rsp_status(enum at_rsp_status *status, const char *msg)
{
        for (size_t i = 0; i < ARRAY_LEN(at_status_msgs); ++i) {
                const struct at_rsp_status_msgs *sms = at_status_msgs + i;
                if (0 == strcmp(msg, sms->str)) {
                        *status = sms->status;
                        return true;
                }
        }

        return false;
}

static bool _process_msg_generic(struct at_info *ati,
                                 const enum at_rx_state state,
                                 char *msg)
{
        if (AT_RSP_MAX_MSGS <= ati->rsp.msg_count) {
                pr_error("[at] BUG: Received more messages than can "
                         "handle. Dropping.\r\n");
                return false;
        }

        ati->rx_state = state;
        ati->rsp.msgs[ati->rsp.msg_count] = msg;
        ++ati->rsp.msg_count;

        return true;
}

static void process_urc_msg(struct at_info *ati, char *msg)
{
	const bool no_strip = ati->urc_ip->flags & AT_URC_FLAGS_NO_RSTRIP;
	if (!no_strip) {
		const size_t msg_len = serial_msg_strlen(msg);
		msg[msg_len] = '\0';
	}

	_process_msg_generic(ati, AT_RX_STATE_URC, msg);

	const bool no_status = !!(ati->urc_ip->flags & AT_URC_FLAGS_NO_RSP_STATUS);
	enum at_rsp_status status = AT_RSP_STATUS_NONE;
	if (no_status || is_rsp_status(&status, msg))
		complete_urc(ati, status);
}

static void process_cmd_msg(struct at_info *ati, char *msg)
{
	/* We always strip trialing message characters on cmd messages */
	const size_t msg_len = serial_msg_strlen(msg);
	msg[msg_len] = '\0';

	_process_msg_generic(ati, AT_RX_STATE_CMD, msg);

	enum at_rsp_status status;
	if (is_rsp_status(&status, msg))
		complete_cmd(ati, status);
}

static void begin_urc_msg(struct at_info *ati, struct at_urc *urc)
{
        ati->urc_ip = urc;
        ati->timing.urc_start_ms = getUptime();
}

static void process_cmd_or_urc_msg(struct at_info *ati, char *msg)
{
	/*
	 * We are starting a new message series or handling a device
	 * where URCs come in at any time (including mid message).
	 * Gotta figure out what type of message this is before we
	 * process it.
	 */
	if (ati->sparse_urc_cb && ati->sparse_urc_cb(msg)) {
		/* It was a sparse URC that was handled. */
		return;
	}

	/* Not a sparse URC. Let's see if it is a registered URC */
	struct at_urc* const urc = is_urc_msg(ati, msg);
	if (urc) {
		begin_urc_msg(ati, urc);
		return process_urc_msg(ati, msg);
	}

	/*
	 * Check if there is a command in progress.
	 * If so, then cmd_ip will be set and we will treat this
	 * message as a command response.
	 */
	if (ati->cmd_ip)
		return process_cmd_msg(ati, msg);

	/*
	 * If we end up here we have data but have no URC that handles
	 * it nor is there any command in progress. This means that we
	 * have an unhandled message (and these should not happen).
	 * Log it and move on with life.
	 */
	pr_warning_str_msg("[at] Unhandled msg received: ", msg);

	/* Need clean the buffer for new msgs */
	serial_buffer_clear(ati->sb);
}

static void at_task_run_bytes_read(struct at_info *ati, char *msg)
{
        /*
         * If the device is a rude device, then we need to treat every message
         * as a new message. Sane AT devices will buffer all URCs until after a
         * command is complete.  Some however do not and thus we must acomodate
         * them here.  This has potential drawbacks if a URC and a command
         * response conflict, but that is the price of a rude device.
         */
        if (ati->dev_cfg.flags & AT_DEV_CFG_FLAG_RUDE)
                return process_cmd_or_urc_msg(ati, msg);

        switch (ati->rx_state) {
        case AT_RX_STATE_CMD:
                /* We are in the middle of receiving a command message. */
                return process_cmd_msg(ati, msg);
        case AT_RX_STATE_URC:
                /* We are in the middle of receiving a unexpected message. */
                return process_urc_msg(ati, msg);
        case AT_RX_STATE_READY:
                /* We are starting a new message series. */
                return process_cmd_or_urc_msg(ati, msg);
        }
}

/**
 * Keeps an eye on the at command state.  If we are in the quiet period, this
 * method will keep an eye on the time so that we will change state back to
 * the READY state once the timeout has expired.
 */
void at_task_quiet_period_handler(struct at_info *ati)
{
        if (AT_CMD_STATE_QUIET == ati->cmd_state &&
            is_timed_out(ati->timing.quiet_start_ms,
                         ati->dev_cfg.quiet_period_ms))
                ati->cmd_state = AT_CMD_STATE_READY;
}

/**
 * @return The pointer to the next command if available, NULL otherwise.
 */
static struct at_cmd* at_task_get_next_cmd(struct at_info *ati)
{
        if (ati->cmd_queue.count == 0)
                return NULL; /* No commands to queue up */

        struct at_cmd *cmd = ati->cmd_queue.head;

        /* Increment and wrap if needed */
        if (++ati->cmd_queue.head >= ati->cmd_queue.cmds + AT_CMD_MAX_CMDS)
                ati->cmd_queue.head = ati->cmd_queue.cmds;

        --ati->cmd_queue.count;
        return cmd;
}

static bool at_task_cmd_handler(struct at_info *ati)
{
        if (AT_CMD_STATE_READY != ati->cmd_state)
                return false; /* Not in proper state for a new command */

        struct at_cmd* next_cmd = at_task_get_next_cmd(ati);
        if (NULL == next_cmd)
                return false; /* No command to queue. */

        /* If here, then we get a command rolling */
        ati->cmd_ip = next_cmd;
        ati->cmd_state = AT_CMD_STATE_IN_PROGRESS;

        serial_buffer_clear(ati->sb);
        serial_buffer_append(ati->sb, ati->cmd_ip->cmd);
        serial_buffer_append(ati->sb, ati->dev_cfg.delim);
        serial_buffer_tx(ati->sb);
        serial_buffer_clear(ati->sb);

        ati->timing.cmd_start_ms = getUptime();
        return true;
}

/**
 * Runs the at_task loop.  This loop listens for incomming messages and
 * handles them appropriately, checks for timeouts and handles them if
 * needed, handles quiet periods between AT commands and submits AT
 * commands that have been queued up.
 */
void at_task(struct at_info *ati, const size_t ms_delay)
{
        char *msg = serial_buffer_rx(ati->sb, ms_delay);

        if (msg)
                at_task_run_bytes_read(ati, lstrip_inline(msg));
        else
                at_task_run_no_bytes(ati);

        at_task_quiet_period_handler(ati);
        at_task_cmd_handler(ati);
}

/**
 * Utility to reset the AT state machine.  Useful after device reset.
 */
void at_reset(struct at_info *ati)
{
        if (!ati->sb) {
                pr_error("[at] AT machine not yet initialized!\r\n");
                return;
        }

        ati->rx_state = AT_RX_STATE_READY;
        ati->cmd_state = AT_CMD_STATE_READY;
        ati->cmd_queue.head = ati->cmd_queue.cmds;
        ati->cmd_queue.count = 0;

        serial_buffer_reset(ati->sb);
}

/**
 * Initializes an at_info struct for use with the AT system.
 * @param ati The at_info structure to initialize.
 * @param sb The serial_buffer to use for tx/rx.  Data is kept in the buffer
 *           and is referenced until the command completes.  Then its gone.
 * @return true if the parameters were acceptable, false otherwise.
 */
bool at_info_init(struct at_info *ati, struct serial_buffer *sb)
{
        if (!ati || !sb) {
                pr_error("[at] Bad init parameter\r\n");
                return false;
        }

        /* Clear everything.  We don't know where at_info has been */
        memset(ati, 0, sizeof(*ati));
        ati->sb = sb;

	at_configure_device(ati, AT_DEFAULT_QP_MS, AT_DEFAULT_DELIMETER,
			    AT_DEV_CFG_FLAG_NONE);

        /* Reset the state machine, and now we are ready to run */
        at_reset(ati);
        return true;
}

/**
 * Allows the caller to set the sparse URC handler.  This handler should only
 * be used in cases where the URCs coming back from the device are so bad that
 * they can not be parsed by the normal URC parser.
 */
void at_set_sparse_urc_cb(struct at_info *ati, sparse_urc_cb_t* cb)
{
        ati->sparse_urc_cb = cb;
}

/**
 * Puts a new AT command in the execute queue.
 * @param ati The at_info state struct.
 * @param cmd The string command to execute.
 * @param timeout_ms The amount of time to wait for a response in ms.
 * @param rsp_cb The callback to execute when the command completes.  May
 *        be NULL if you want no callback (not advised).
 * @param rsp_up A void* parameter to pass to the call back when invoked.
 * @return The pointer to the location of the at_cmd struct in the queue,
 *         else NULL if submisison failed.
 */
struct at_cmd* at_put_cmd(struct at_info *ati, const char *cmd,
                          const tiny_millis_t timeout_ms,
                          bool (*rsp_cb)(struct at_rsp *rsp, void *rsp_up),
                          void *rsp_up)
{
        if (ati->cmd_queue.count >= AT_CMD_MAX_CMDS) {
                 /* Full up */
                pr_warning("[at] Command queue full");
                return NULL;
        }

        if (strlen(cmd) >= AT_CMD_MAX_LEN) {
                /* Command too long */
                pr_warning_str_msg("[at] Command too long: ", cmd);
                return NULL;
        }

        /* If here, we have space and its ok.  Add it */
        struct at_cmd *atcmd = ati->cmd_queue.head + ati->cmd_queue.count;
        if (atcmd >= ati->cmd_queue.cmds + AT_CMD_MAX_CMDS)
                atcmd -= AT_CMD_MAX_CMDS;

        ++ati->cmd_queue.count;
        atcmd->timeout_ms = timeout_ms;
        atcmd->rsp_cb = rsp_cb;
        atcmd->rsp_up = rsp_up;
        strcpy(atcmd->cmd, cmd); /* Sane b/c len check above */

        return atcmd;
}

/**
 * Registers a URC with the AT engine.  URC stands for unsolicited result
 * code.  These are asynchronous commands that can appear at any time,
 * except when a command is already in progress (defined as the point between
 * when the control character of the command is sent to when the response
 * message returns from the command).  The idea is that when you register
 * a command and it is seen, then you will get a call back to the handler.
 * This allows you to sanely handle state changes as they occur and avoid
 * unnecessary polling for status when URCs can provide interrupt based
 * updates for you.
 * @param ati An at_info structure.
 * @param pfx The URC prefix.  These often look like "+SOMECMD:"
 * @param flags Control flags that change behavior of AT engine.  These are
 *        needed because there are differences in how devices send URCs.
 * @param rsp_cb The callback to invoke when a URC is received.
 * @param rsp_up A void* parameter to pass to the call back when invoked.
 * @return The pointer to the URC entry in our list, or NULL if we failed
 *         to register the URC.
 */
struct at_urc* at_register_urc(struct at_info *ati, const char *pfx,
                               const enum at_urc_flags flags,
                               bool (*rsp_cb)(struct at_rsp *rsp, void *rsp_up),
                               void *rsp_up)
{
        if (ati->urc_list.count >= AT_URC_MAX_URCS) {
                 /* Full up */
                pr_warning("[at] URC list full\r\n");
                return NULL;
        }

        const size_t pfx_len = strlen(pfx);
        if (pfx_len >= AT_URC_MAX_LEN) {
                /* URC prefix is too long */
                pr_warning_str_msg("[at] URC prefix too long: ", pfx);
                return NULL;
        }

        /* If here, we have space and its ok.  Add it */
        struct at_urc *aturc = ati->urc_list.urcs + ati->urc_list.count;
        ++ati->urc_list.count;

        aturc->rsp_cb = rsp_cb;
        aturc->rsp_up = rsp_up;
        aturc->flags = flags;
        aturc->pfx_len = pfx_len;
        strcpy(aturc->pfx, pfx); /* Sane b/c len check above */

        return aturc;
}

/**
 * Sets device specific settings that are unique per device.
 * @param ati The pointer to the at_info struct.
 * @param qp_ms The quiet period in milliseconds.
 * @param delim The command delimeter characters.  Normally this is "\r\n".
 * @param flags One or more at_dev_cfg_flag items or'd together.
 * @return true if the parameters are acceptable, false otherwise.
 */
bool at_configure_device(struct at_info *ati, const tiny_millis_t qp_ms,
                         const char *delim, const enum at_dev_cfg_flag flags)
{
        if (!delim || strlen(delim) >= AT_DEV_CVG_DELIM_MAX_LEN) {
                pr_error("[at] Failed to set delimeter\r\n");
                return false;
        }

        ati->dev_cfg.quiet_period_ms = qp_ms;
        strcpy(ati->dev_cfg.delim, delim); /* Sane b/c strlen check above */
	ati->dev_cfg.flags = flags;
        return true;
}

/**
 * Tests if the given response has an OK status.
 * @return true if it exists and does, false otherwise.
 */
bool at_ok(struct at_rsp *rsp)
{
        return rsp && AT_RSP_STATUS_OK == rsp->status;
}

/**
 * Breaks up an AT response line into the individual components, including the
 * leading response tag.  This allows us to more easily parse and validate the
 * messages.  If there are more components than buckets, then what hasn't been
 * processed will go in the last bucket.
 * @param rsp The AT response line to be parsed.  Note it will get modified.
 * @param bkts An array of char pointers that act as the buckets for the AT msg
 * tokens.  We put the start of each token in a bucket.
 * @param num_bkts The number of buckets we have to work with.
 * @return The number of buckets used.
 */
size_t at_parse_rsp_line(char *rsp, char *bkts[], const size_t num_bkts)
{
        if (!rsp || !bkts || !num_bkts)
                return 0;

        size_t idx = 0;
        bool in_str = false;
        bool str_esc = false;
        bool is_new = true;

        for (; *rsp && idx < num_bkts; ++rsp) {
                if (str_esc) {
                        /*
                         * If here, we are in a string and last char was
                         * an escape char.  So ignore it and move along.
                         */
                        str_esc = false;
                        continue;
                }

                if (is_new) {
                        is_new = false;
                        bkts[idx++] = rsp;

                        if (idx == num_bkts)
                                break;
                }

                switch(*rsp) {
                case '\\':
                        /* Ignore the next character if in a string */
                        str_esc = in_str;
                        break;
                case '"':
                        in_str = !in_str;
                        break;
                case ':':
                case ',':
                        /*
                         * We have found a boundary (maybe).  If we are in a
                         * string, ignore it.  Else parse it and move along.
                         */
                        if (in_str)
                                break;

                        *rsp = '\0';
                        is_new = true;
                        break;
                default:
                        break;
                }
        }

        return idx;
}

/**
 * This command will "parse" out a string in an AT command response.  Put
 * another way, all strings seem to come back surrounded in quotes.  This
 * logic ensures that we extract the string and respect any escape
 * characters that might be in there.
 * @param rsp the parsed message which should contain an AT string.
 * @return A pointer to the string within the quotes, NULL if there was
 * an error during parsing.
 */
char* at_parse_rsp_str(char *rsp)
{
        bool esc = false;
        char *beg = NULL, *end = NULL;

        for (; *rsp; ++rsp) {
                if (esc) {
                        esc = false;
                        continue;
                }

                switch(*rsp) {
                case '"':
                        if (!beg) {
                                beg = rsp;
                        } else if (!end) {
                                end = rsp;
                        }
                        break;
                case '\\':
                        esc = beg != NULL && end == NULL;
                        break;
                }
        }

        /* At this point if it was successful, beg and end will be set */
        if (!beg || !end)
                return NULL;

        *end = '\0';
        return ++beg;
}
