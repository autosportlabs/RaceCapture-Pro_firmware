/*
 * Race Capture Firmware.
 */

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>

#include <race_capture/printk.h>
#include <tasks/heartbeat.h>

#define TASK_NAME (signed portCHAR *) "heartbeat_task"
#define TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define TASK_STACK_SIZE 100

static void heartbeat_task( void )
{
        portTickType xLastWakeTime = xTaskGetTickCount();

        while(1) {
                pr_info("Ka-Thump");

                // Wait 1 second before beating again.
                vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ);
        }
}

void start_heartbeat_task( void )
{
        xTaskCreate(heartbeat_task, TASK_NAME, TASK_STACK_SIZE, NULL,
                    TASK_PRIORITY, NULL );
        return;
}
