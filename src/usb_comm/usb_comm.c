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

#include "FreeRTOS.h"
#include "USB-CDC_device.h"
#include "loggerApi.h"
#include "loggerSampleData.h"
#include "messaging.h"
#include "panic.h"
#include "printk.h"
#include "rx_buff.h"
#include "serial.h"
#include "task.h"
#include "usb_comm.h"
#include <stdlib.h>
#include <string.h>

#define LOG_PFX			"[USB] "
#define USB_COMM_STACK_SIZE	320
#define USB_EVENT_QUEUE_DEPTH	8

static volatile struct {
        xQueueHandle event_queue;
        struct Serial* serial;
	int ls_handle;
        struct {
                struct rx_buff* buff;
        } rx;
} usb_state;


static void log_event_overflow(const char* event_name)
{
        pr_warning_str_msg(LOG_PFX "Event overflow: ", event_name);
}

/**
 * Structure used to house sample data for a wifi telemetry stream until
 * the usb_event handler picks it up.
 */
struct usb_sample_data {
        const struct sample* sample;
        size_t tick;
};

/**
 * Event struct used for all events that come into our wifi task
 */
struct usb_event {
        enum task {
                TASK_RX_DATA,
                TASK_SAMPLE,
        } task;
        union {
                struct usb_sample_data sample;
        } data ;
};

static bool data_rx_isr_cb()
{
        portBASE_TYPE hpta = false;
        /* Send usb_event message here to wake the task */
        struct usb_event event = {
                .task = TASK_RX_DATA,
        };

        xQueueSendFromISR(usb_state.event_queue, &event, &hpta);
        return !!hpta;
}

static void usb_sample_cb(const struct sample* sample,
                          const int tick, void* data)
{
        const struct usb_sample_data sample_data = {
                .sample = sample,
                .tick = tick,
        };

        struct usb_event event = {
                .task = TASK_SAMPLE,
                .data.sample = sample_data,
        };

        /* Send the message here to wake the timer */
        if (!xQueueSend(usb_state.event_queue, &event, 0))
                log_event_overflow("Sample CB");
}


/* *** USB Serial IOCTL Handler *** */

static int set_telemetry(int rate)
{
	const char* serial_name = serial_get_name(usb_state.serial);

	/* Stop the stream if the rate is <= 0 */
	if (rate <= 0 || usb_state.ls_handle >= 0) {
		pr_info_str_msg(LOG_PFX "Stopping telem stream on ",
				serial_name);

		if (!logger_sample_destroy_callback(usb_state.ls_handle))
			return SERIAL_IOCTL_STATUS_ERR;

		usb_state.ls_handle = -1;
	}

	if (rate > 0) {
		pr_info_str_msg(LOG_PFX "Starting telem stream on ",
				serial_name);
		usb_state.ls_handle =
			logger_sample_create_callback(usb_sample_cb,
						      rate, NULL);
		if (usb_state.ls_handle < 0)
			return SERIAL_IOCTL_STATUS_ERR;
	}

	return SERIAL_IOCTL_STATUS_OK;
}

static enum serial_ioctl_status usb_serial_ioctl(struct Serial* serial,
						 unsigned long req,
						 void* argp)
{
        switch(req) {
        case SERIAL_IOCTL_TELEMETRY:
		return set_telemetry((int) (long) argp);
        default:
                pr_warning_int_msg(LOG_PFX "Unhandled ioctl request: ",
                                   (int) req);
                return -1;
        }
}


/* *** Task Handlers *** */

/**
 *
 */
static void process_rx_msgs()
{
        struct Serial* const s = usb_state.serial;
        struct rx_buff* const rxb = usb_state.rx.buff;

        for(;;) {
                rx_buff_read(rxb, s, true);
                switch (rx_buff_get_status(rxb)) {
                case RX_BUFF_STATUS_EMPTY:
                case RX_BUFF_STATUS_PARTIAL:
                        /*
                         * Read and there was nothing or a
                         * partial message has been received. USB doesn't
                         * timeout since its an interactive console so we
                         * just return here and wait for more data to come.
                         */
                        return;
                case RX_BUFF_STATUS_READY:
                case RX_BUFF_STATUS_OVERFLOW:
                        /* A message awaits us */
                        put_crlf(s);
                        char *data_in = rx_buff_get_msg(rxb);
                        pr_trace_str_msg(LOG_PFX "Received CMD: ", data_in);
                        process_read_msg(s, data_in, strlen(data_in));
                        rx_buff_clear(rxb);
                }
        }
}

static void process_sample(struct usb_sample_data* data)
{
        struct Serial* serial = usb_state.serial;
        const struct sample* sample = data->sample;
        const size_t ticks = data->tick;
        const bool meta = ticks == 0;

        if (ticks != sample->ticks) {
                /* Then the sample has changed underneath us */
                pr_warning(LOG_PFX "Stale sample.  Dropping \r\n");
                return;
        }

        api_send_sample_record(serial, sample, ticks, meta);
        put_crlf(serial);
}


static void usb_comm_task(void *pvParameters)
{
        while (!USB_CDC_is_initialized());

        for(;;) {
                struct usb_event event;
                if (!xQueueReceive(usb_state.event_queue, &event,
                                   portMAX_DELAY))
                        continue;

                /* If here we have an event. Process it */
                switch (event.task) {
                case TASK_RX_DATA:
                        process_rx_msgs();
                        break;
                case TASK_SAMPLE:
                        process_sample(&event.data.sample);
                        break;
                default:
                        panic(PANIC_CAUSE_UNREACHABLE);
                }
        }

        panic(PANIC_CAUSE_UNREACHABLE);
}


/* *** Public Methods *** */

void startUSBCommTask(int priority)
{
        USB_CDC_device_init(priority, data_rx_isr_cb);

        /* Allocate our RX buffer for incomming data */
        usb_state.rx.buff = rx_buff_create(RX_MAX_MSG_LEN);
        if (!usb_state.rx.buff)
                goto init_fail;

        usb_state.event_queue = xQueueCreate(USB_EVENT_QUEUE_DEPTH,
                                             sizeof(struct usb_event));
        if (!usb_state.event_queue)
                goto init_fail;

        usb_state.serial = USB_CDC_get_serial();
        if (!usb_state.serial)
                goto init_fail;

        /* Set the Serial IOCTL handler here b/c this is managing task */
        serial_set_ioctl_cb(usb_state.serial, usb_serial_ioctl);
	usb_state.ls_handle = -1;

        /* Make all task names 16 chars including NULL char */
        static const signed portCHAR task_name[] = "USB Comm Task  ";
        xTaskCreate(usb_comm_task, task_name, USB_COMM_STACK_SIZE,
                    NULL, priority, NULL);
        return;

init_fail:
        pr_error(LOG_PFX "Init failed\r\n");
        return;
}
