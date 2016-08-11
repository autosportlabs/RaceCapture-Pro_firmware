/*
 * Race Capture Firmware
 *
 * Copyright (C) 2015 Autosport Labs
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
 *
 *
 * RaceCapture Pro main
 *
 * NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
 *        The processor MUST be in supervisor mode when vTaskStartScheduler is
 *        called.  The demo applications included in the FreeRTOS.org download switch
 *        to supervisor mode prior to main being called.  If you are not using one of
 *        these demo application projects then ensure Supervisor mode is used.
 */

#include "FreeRTOS.h"
#include "OBD2_task.h"
#include "capabilities.h"
#include "connectivityTask.h"
#include "constants.h"
#include "cpu.h"
#include "fileWriter.h"
#include "gpioTasks.h"
#include "gpsTask.h"
#include "led.h"
#include "loggerHardware.h"
#include "loggerTaskEx.h"
#include "luaScript.h"
#include "luaTask.h"
#include "messaging.h"
#include "panic.h"
#include "printk.h"
#include "task.h"
#include "usb_comm.h"
#include "wifi.h"
#include "watchdog.h"
#include <app_info.h>
#include <stdbool.h>

__attribute__((aligned (4)))
static const struct app_info_block info_block = {
    .magic_number = APP_INFO_MAGIC_NUMBER,
    .info_crc = 0xDEADBEEF,
};

/*
 * Define task priorities here.  Ensure that these values are
 * all below `configMAX_PRIORITIES - 1` in the FreeRTOSConfig.h.
 * See http://www.freertos.org/RTOS-task-priority.html for more
 * info on how to best set these.  0 is lowest priority.
 */
#define TASK_PRIORITY(v)	(tskIDLE_PRIORITY + v)
#define RCP_LOGGING_PRIORITY	TASK_PRIORITY(4)
#define RCP_INPUT_PRIORITY	TASK_PRIORITY(3)
#define RCP_OUTPUT_PRIORITY	TASK_PRIORITY(2)
#define RCP_LUA_PRIORITY	TASK_PRIORITY(1)

void setupTask(void *param)
{
        initialize_tracks();
        initialize_logger_config();

        InitLoggerHardware();
        initMessaging();

	startGPSTask(RCP_INPUT_PRIORITY);

#if USB_SERIAL_SUPPORT
        startUSBCommTask(RCP_INPUT_PRIORITY);
#endif

        startOBD2Task(RCP_INPUT_PRIORITY);
        startConnectivityTask(RCP_OUTPUT_PRIORITY);
        startLoggerTaskEx(RCP_LOGGING_PRIORITY);

#if GPIO_CHANNELS > 0
        startGPIOTasks(RCP_INPUT_PRIORITY);
#endif

#if SDCARD_SUPPORT
        startFileWriterTask(RCP_OUTPUT_PRIORITY);
#endif

#if LUA_SUPPORT
        lua_task_init(RCP_LUA_PRIORITY);
#endif

#if WIFI_SUPPORT
        wifi_init_task(RCP_OUTPUT_PRIORITY, RCP_INPUT_PRIORITY);
#endif

        /* Removes this setup task from the scheduler */
	pr_info("[main] Setup Task complete!\r\n");
        vTaskDelete(NULL);
}

int main( void )
{
        ALWAYS_KEEP(info_block);
        cpu_init();
        pr_info("*** Start! ***\r\n");

        /* Defined as part of our compilation process */
        if (true == ASL_WATCHDOG)
                watchdog_init(WATCHDOG_TIMEOUT_MS);

        const signed portCHAR task_name[] = "Hardware Init";
        xTaskCreate(setupTask, task_name, HARDWARE_INIT_STACK_SIZE,
                    NULL, RCP_LUA_PRIORITY, NULL);

        vTaskStartScheduler();

        /* SHOULD NEVER GET HERE */
        panic(PANIC_CAUSE_SCHEDULER);
        return 1;
}
