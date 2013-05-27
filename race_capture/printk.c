/*
 * Printk for Race capture
 */

#include <race_capture/printk.h>
#include <race_capture/ring_buffer.h>
#include <serial.h>

static enum log_level curr_level = NOTICE;

#define LOG_BUFFER_SIZE 1024
char _log_buffer[LOG_BUFFER_SIZE];

const struct ring_buff log_buff = {
	.buf = _log_buffer,
        .size = sizeof(_log_buffer),
        .head = _log_buffer,
        .tail = _log_buffer
};

size_t write_log_serial(Serial *s)
{

}

int printk(enum log_level level, const char *msg) {
        if (level > curr_level)
                return 0;

        return 1;
}
