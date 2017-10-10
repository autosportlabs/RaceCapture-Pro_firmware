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

#include "api.h"
#include "auto_logger.h"
#include "cpp_guard.h"
#include "dateTime.h"
#include "loggerTaskEx.h"
#include "printk.h"
#include <stdbool.h>
#include "loggerSampleData.h"
#include <string.h>

#define LOG_PFX   "[auto_logger] "

#define DEFAULT_AUTO_LOGGER_CHANNEL "Speed"

/*
 * NOTE:
 * We use the logging bool here because it helps us prevent contention
 * with manual user directives.  In example it will allow a user to start
 * logging using the manual button and not have the auto logger fight them
 * by shutting logging off right away.  Vice versa applies when coming off
 * the track.  It runs on the assumption that changing logging to a state
 * that it is already in is a no-op.
 */

static struct {
        struct auto_logger_config *cfg;
        struct auto_control_state control_state;
} auto_logger_state;

void auto_logger_reset_config(struct auto_logger_config* cfg)
{
        cfg->enabled = false;
        strcpy(cfg->channel, DEFAULT_AUTO_LOGGER_CHANNEL);
        auto_control_reset_trigger(&cfg->start, &cfg->stop);
}

void auto_logger_get_config(struct auto_logger_config* cfg,
                            struct Serial* serial,
                            const bool more)
{
        json_objStartString(serial, "autoLoggerCfg");
        json_bool(serial, "en", cfg->enabled, true);
        json_string(serial, "channel", cfg->channel, true);
        get_auto_control_trigger(serial, &cfg->start, "start", true);
        get_auto_control_trigger(serial, &cfg->stop, "stop", false);
        json_objEnd(serial, more);
}

bool auto_logger_set_config(struct auto_logger_config* cfg,
                            const jsmntok_t *json)
{
        jsmn_exists_set_val_bool(json, "en", &cfg->enabled);
        jsmn_exists_set_val_string(json, "channel", cfg->channel, DEFAULT_LABEL_LENGTH, true);
        set_auto_control_trigger(&cfg->start, "start", json);
        set_auto_control_trigger(&cfg->stop, "stop", json);
        return true;
}

static void auto_logger_sample_cb(const struct sample* sample,
                           const int tick, void* data)
{
        if (!auto_logger_state.cfg || !auto_logger_state.cfg->enabled)
                return;

        double value;
        if (!get_sample_value_by_name(sample, auto_logger_state.cfg->channel, &value))
                return;

        enum auto_control_trigger_result res = auto_control_check_trigger(value,
                                                                          &auto_logger_state.cfg->start,
                                                                          &auto_logger_state.cfg->stop,
                                                                          &auto_logger_state.control_state);
        switch(res){
                case AUTO_CONTROL_TRIGGERED:
                        startLogging();
                        pr_info(LOG_PFX "Auto-starting logging\r\n");
                        break;
                case AUTO_CONTROL_UNTRIGGERED:
                        stopLogging();
                        pr_info(LOG_PFX "Auto-stopping logging\r\n");
                        break;
                default:
                        break;
        }
}

bool auto_logger_init(struct auto_logger_config* cfg)
{
        if (!cfg)
                return false;

        auto_logger_state.cfg = cfg;
        auto_control_init_state(&auto_logger_state.control_state);

        logger_sample_create_callback(auto_logger_sample_cb, 10, NULL);
        return true;
}
