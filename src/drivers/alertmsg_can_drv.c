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

#include "alertmsg_can_drv.h"
#include "CAN.h"
#include "printk.h"
#include <string.h>
#include "stdutil.h"

#define _LOG_PFX "[AlertMsgCAN] "

#define DEFAULT_CAN_TIMEOUT   100
#define ALERTMSG_CAN_BUS 1
#define ALERTMSG_CAN_BASE_ADDRESS 0xA100
#define ALERTMSG_MAX_PRIORITY 2

void alertmsg_can_send_message(const struct alertmessage *alertmsg)
{
        /* Filter the message  and zero pad */
        MIN(8, strlen(alertmsg->message));
        char message_text[17] = {0};
        strncpy(message_text, alertmsg->message, 16);

        /* Calculate offset using base address and priority */
        uint32_t address = ALERTMSG_CAN_BASE_ADDRESS + (MAX(ALERTMSG_MAX_PRIORITY, alertmsg->priority) * 2);

        pr_info(_LOG_PFX "Broadcasting alertmessage on CAN ");
        pr_info_int(address);
        pr_info_str_msg("; msg ", message_text);

        /* Send the two messages, each with 1/2 of message (16 characters max) */
        CAN_msg msg;
        msg.isExtendedAddress = true;
        msg.dataLength = 8;

        /* Send the first 1/2 of the alert message */
        for (size_t i = 0; i < 8; i++) {
                msg.data[i] = message_text[i];
        }
        msg.addressValue = address;
        if (!CAN_tx_msg(ALERTMSG_CAN_BUS, &msg, DEFAULT_CAN_TIMEOUT)) {
                pr_warning_str_msg(_LOG_PFX "Failed sending CAN alertmsg segment 1 ", message_text);
        }

        /* Send the second 1/2 of the alert message */
        for (size_t i = 0; i < 8; i++) {
                msg.data[i] = message_text[i + 8];
        }
        msg.addressValue = address + 1;
        if (!CAN_tx_msg(ALERTMSG_CAN_BUS, &msg, DEFAULT_CAN_TIMEOUT)) {
                pr_warning_str_msg(_LOG_PFX "Failed sending CAN alertmsg segment 2", message_text);
        }
}
