/*
 * Printk for Race capture
 */

#include <race_capture/printk.h>
#include <serial.h>

static enum log_level curr_level = NOTICE;

int printk(enum log_level level, const char *msg) {
        if (level > curr_level)
                return 0;

        Serial *s = get_serial_usb();
        s->put_s(msg);
        put_crlf(s);

        return 1;
}
