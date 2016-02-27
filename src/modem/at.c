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

#include "array_utils.h"
#include "at.h"
#include "dateTime.h"
#include "mod_string.h"
#include "printk.h"
#include "serial_buffer.h"

#include <ctype.h>

static const enum log_level dbg_lvl = INFO;

#define AT_STATUS_MSG(a, b)	{(a), sizeof(a), (b)}
static const struct at_rsp_status_msgs {
        const char* str;
        size_t str_len;
        enum at_rsp_status status;
} at_status_msgs[] = {
        AT_STATUS_MSG("OK", AT_RSP_STATUS_OK),
        AT_STATUS_MSG("FAILED", AT_RSP_STATUS_FAILED),
        AT_STATUS_MSG("ABORTED", AT_RSP_STATUS_ABORTED),
        AT_STATUS_MSG("ERROR", AT_RSP_STATUS_ERROR),
};

static char* trim(char *str)
{
        /*
         * Shamelessly borrowed and modified from
         * http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
         */

        /* Trim leading space */
        while(isspace((int) *str))
                ++str;

        if(!*str)
                return str; /* All spaces */

        /* Trim trailing space */
        char *end = str + strlen(str) - 1;
        while(end > str && isspace((int) *end))
                --end;

        *++end = '\0';
        return str;
}


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

        if (ati->urc_ip->rsp_cb)
                ati->urc_ip->rsp_cb(&ati->rsp, ati->urc_ip->rsp_up);

        /* Post URC, cleanup is a wee bit different than of cmd */
        _complete_msg_cleanup(ati);
        ati->urc_ip = NULL;
}

static void complete_cmd(struct at_info *ati, const enum at_rsp_status status)
{
        ati->rsp.status = status;
        ati->rsp.run_time = getUptime() - ati->timing.cmd_start_ms;

        if (ati->cmd_ip->rsp_cb)
                ati->cmd_ip->rsp_cb(&ati->rsp, ati->cmd_ip->rsp_up);

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
                                 const char *msg)
{
        if (AT_RSP_MAX_MSGS <= ati->rsp.msg_count) {
                pr_error("[at] BUG: Received more messages than can "
                         "handle. Dropping.");
                return false;
        }

        ati->rx_state = state;
        ati->rsp.msgs[ati->rsp.msg_count] = msg;
        ++ati->rsp.msg_count;

        return true;
}

static void process_urc_msg(struct at_info *ati, char *msg)
{
        _process_msg_generic(ati, AT_RX_STATE_URC, msg);

        const bool no_status = ati->urc_ip->flags & AT_URC_FLAGS_NO_RSP_STATUS;
        enum at_rsp_status status = AT_RSP_STATUS_NONE;
        if (no_status || is_rsp_status(&status, msg))
                complete_urc(ati, status);
}

static void process_cmd_msg(struct at_info *ati, char *msg)
{
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

static void at_task_run_bytes_read(struct at_info *ati, char *msg)
{
        /*
         * If here, then we have read a message.  Now we have to process it.
         * Our rx state will dictate how we process this message beacuse that
         * allows us to know previous messages and what message type to expect.
         */
        switch (ati->rx_state) {
        case AT_RX_STATE_CMD:
                /* We are in the middle of receiving a command message. */
                return process_cmd_msg(ati, msg);
        case AT_RX_STATE_URC:
                /* We are in the middle of receiving a unexpected message. */
                return process_urc_msg(ati, msg);
        case AT_RX_STATE_READY:
                /*
                 * We are starting a new message series.  Gotta figure out what
                 * type of message this is before we process it.  Then we process
                 * it appropriately.
                 */
                ; /* C is stupid after labels.  Workaround :\ */
                struct at_urc *urc = is_urc_msg(ati, msg);
                if (urc) {
                        begin_urc_msg(ati, urc);
                        return process_urc_msg(ati, msg);
                } else if (ati->cmd_ip) { /* Is there a command in progress */
                        /* If so, then cmd_ip is already set */
                        return process_cmd_msg(ati, msg);
                } else {
                        /* We got data but have no URC or CMD for it */
                        pr_warning_str_msg("[at] Unhandled msg received: ",
                                           msg);
                        /* Need a clean buffer for expected msgs */
                        serial_buffer_clear(ati->sb);
                }
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
                at_task_run_bytes_read(ati, trim(msg));
        else
                at_task_run_no_bytes(ati);

        at_task_quiet_period_handler(ati);
        at_task_cmd_handler(ati);
}

/**
 * Initializes an at_info struct for use with the AT system.
 * @param ati The at_info structure to initialize.
 * @param sb The serial_buffer to use for tx/rx.  Data is kept in the buffer
 *           and is referenced until the command completes.  Then its gone.
 * @param quiet_period_ms The quiet period to wait between the end of a
 *        command and when to start the next.  Some modems need time to recover
 *        and send URCs.
 * @return true if the parameters were acceptable, false otherwise.
 */
bool init_at_info(struct at_info *ati, struct serial_buffer *sb,
                  const tiny_millis_t quiet_period_ms, const char *delim)
{
        if (!ati || !sb)
                return false;

        /* Clear everything.  We don't know where at_info has been */
        memset(ati, 0, sizeof(*ati));

        ati->rx_state = AT_RX_STATE_READY;
        ati->cmd_state = AT_CMD_STATE_READY;
        ati->sb = sb;
        if (!at_configure_device(ati, quiet_period_ms, delim))
                return false;

        /* Init the HEAD pointer of our queue... else segfault */
        ati->cmd_queue.head = ati->cmd_queue.cmds;

        serial_buffer_reset(ati->sb);

        return true;
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
                          void (*rsp_cb)(struct at_rsp *rsp, void *rsp_up),
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
                               void (*rsp_cb)(struct at_rsp *rsp, void *rsp_up),
                               void *rsp_up)
{
        if (ati->urc_list.count >= AT_URC_MAX_URCS) {
                 /* Full up */
                pr_warning("[at] URC list full");
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
 * @return true if the parameters are acceptable, false otherwise.
 */
bool at_configure_device(struct at_info *ati, const tiny_millis_t qp_ms,
                         const char *delim)
{
        if (!delim || strlen(delim) >= AT_DEV_CVG_DELIM_MAX_LEN)
                return false;

        ati->dev_cfg.quiet_period_ms = qp_ms;
        strcpy(ati->dev_cfg.delim, delim); /* Sane b/c strlen check above */
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
