/*
 * Race Capture printk
 */

#ifndef __PRINTK_H__
#define __PRINTK_H__

#include "serial.h"
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

#define pr_info(arg1) printk(INFO, arg1)
#define pr_info_int(arg1) printk_int(INFO, arg1);

#define pr_debug(arg1) printk(DEBUG, arg1);
#define pr_debug_int(arg1) printk_int(DEBUG, arg1);

#define pr_warning(arg1) printk(WARNING, arg1);
#define pr_warning_int(arg1) printk_int(WARNING, arg1);

#define pr_error(arg1) printk(ERR, arg1);
#define pr_error_int(arg1) printk_int(ERR, arg1);

size_t read_log_to_serial(Serial *s);
int printk(enum log_level level, const char *msg);
int printk_int(enum log_level level, int value);
enum log_level set_log_level(enum log_level level);

#endif /* __PRINTK_H__ */
