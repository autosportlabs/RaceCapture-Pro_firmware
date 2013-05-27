/*
 * Race Capture printk
 */

#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <serial.h>
#include <stddef.h>

enum log_level {
        EMERG = 0,
        ALERT = 1,
        CRIT = 2,
        ERR = 3,
        WARNING = 4,
        NOTICE = 5,
        INFO = 6,
        DEBUG = 7,
};

#define pr_info(arg1) \
        printk(INFO, arg1)

size_t write_log_serial(Serial *s);
int printk(enum log_level level, const char *msg);

#endif /* __PRINTK_H__ */
