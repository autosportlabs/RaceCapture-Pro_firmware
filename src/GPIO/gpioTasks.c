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


#include "gpioTasks.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerTaskEx.h"
#include "logger.h"
#include "taskUtil.h"
#include "printk.h"
#include "GPIO.h"

#define DEBOUNCE_DELAY_PERIOD		30
#define GPIO_TASK_STACK_SIZE		64

xSemaphoreHandle xOnPushbutton;

void startGPIOTasks(int priority)
{
        vSemaphoreCreateBinary( xOnPushbutton );

        /* Make all task names 16 chars including NULL char */
        static const signed portCHAR task_name[] = "Button Task    ";
        xTaskCreate(onPushbuttonTask, task_name, GPIO_TASK_STACK_SIZE,
                    NULL, priority, NULL);
}


void onPushbuttonTask(void *pvParameters)
{

    while(1) {
        if ( xSemaphoreTake(xOnPushbutton, portMAX_DELAY) == pdTRUE) {
            delayMs(DEBOUNCE_DELAY_PERIOD);

            if (GPIO_is_button_pressed()) {
                if (logging_is_active()) {
                    stopLogging();
                } else {
                    startLogging();
                }
            }
        }
    }
}
