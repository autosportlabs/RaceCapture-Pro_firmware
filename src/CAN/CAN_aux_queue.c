/*
 * CAN_aux_queue.c
 *
 *  Created on: Feb 26, 2017
 *      Author: brent
 */
#include "CAN_aux_queue.h"
#include "capabilities.h"
#include "printk.h"
#include "taskUtil.h"


#define _LOG_PFX "[CAN AUX] "
#define CAN_AUX_QUEUE_LENGTH 5
static xQueueHandle can_aux_queue[CAN_CHANNELS] = {0};

bool CAN_aux_queue_init(void)
{
    for (size_t i = 0; i < CAN_CHANNELS; i++) {
            can_aux_queue[i] = xQueueCreate(CAN_AUX_QUEUE_LENGTH, sizeof(CAN_msg));
            if (! can_aux_queue[i]) {
                    pr_error_int_msg(_LOG_PFX "Failed to alloc CAN aux queue with size ", CAN_AUX_QUEUE_LENGTH);
                    return false;
            }
    }
    return true;
}

bool CAN_aux_queue_put_msg(CAN_msg * can_msg, size_t timeout_ms)
{
        uint8_t can_bus = can_msg->can_bus;
        if (can_bus >= CAN_CHANNELS)
                return false;

        if (xQueueSend(can_aux_queue[can_bus], can_msg, msToTicks(timeout_ms)) == pdTRUE)
                return true;
        else {
                pr_debug_int_msg(_LOG_PFX "timeout on put CAN aux msg for bus ", can_bus);
                return false;
        }
}

bool CAN_aux_queue_get_msg(uint8_t can_bus, CAN_msg * can_msg, size_t timeout_ms)
{
        if (xQueueReceive(can_aux_queue[can_bus], can_msg, msToTicks(timeout_ms)) == pdTRUE)
                return true;
        else {
            pr_debug_int_msg(_LOG_PFX "timeout on get CAN aux msg for bus ", can_bus);
            return 0;
        }
}
