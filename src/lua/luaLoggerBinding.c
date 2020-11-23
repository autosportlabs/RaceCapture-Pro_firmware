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

#include "ADC.h"
#include "CAN.h"
#include "CAN_aux_queue.h"
#include "FreeRTOS.h"
#include "GPIO.h"
#include "OBD2.h"
#include "PWM.h"
#include "channel_config.h"
#include "dateTime.h"
#include "gps.h"
#include "imu.h"
#include "lap_stats.h"
#include "lauxlib.h"
#include "led.h"
#include "logger.h"
#include "loggerConfig.h"
#include "loggerData.h"
#include "loggerSampleData.h"
#include "loggerTaskEx.h"
#include "luaBaseBinding.h"
#include "luaLoggerBinding.h"
#include "luaScript.h"
#include "luaTask.h"
#include <string.h>
#include "modp_numtoa.h"
#include "printk.h"
#include "queue.h"
#include "semphr.h"
#include "serial.h"
#include "task.h"
#include "timer.h"
#include "virtual_channel.h"
#include "predictive_timer_2.h"
#include "shiftx_drv.h"
#include "api_event.h"
#include "math.h"
#include "taskUtil.h"
#include "connectivityTask.h"

#define TEMP_BUFFER_LEN 		256
#define DEFAULT_CAN_TIMEOUT 		100
#define DEFAULT_SERIAL_TIMEOUT		100
#define LUA_DEFAULT_SERIAL_PORT 	SERIAL_AUX
#define LUA_DEFAULT_SERIAL_BAUD 	115200
#define LUA_DEFAULT_SERIAL_BITS 	8
#define LUA_DEFAULT_SERIAL_PARITY	0
#define LUA_DEFAULT_SERIAL_STOP_BITS	1

static int lua_get_virtual_channel(lua_State *ls);
static int lua_set_led(lua_State *ls);

static int lua_get_uptime(lua_State *L)
{
        lua_pushinteger(L, (int) getUptime());
        return 1;
}

static int lua_get_date_time(lua_State *L)
{
        const GpsSample sample = getGpsSample();

        /*
         * Have to use DateTime because LUA only uses floats and ints.
         * Time is a uint64t value so can't just use it directly.  Hence
         * this.  If folks want a monotonically increasing value, they
         * need to use getUptime call.
         */
        DateTime dt;
        getDateTimeFromEpochMillis(&dt, sample.time);

        lua_pushinteger(L, (int) dt.year);
        lua_pushinteger(L, (int) dt.month);
        lua_pushinteger(L, (int) dt.day);
        lua_pushinteger(L, (int) dt.hour);
        lua_pushinteger(L, (int) dt.minute);
        lua_pushinteger(L, (int) dt.second);
        lua_pushinteger(L, (int) dt.millisecond);

        return 7;
}

static int lua_get_gps_altitude(lua_State *L)
{
        lua_pushnumber(L, getAltitude());
        return 1;
}


static int lua_set_bg_streaming(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_boolean(L, 1);

        const bool val = lua_toboolean(L, 1);
        getWorkingLoggerConfig()->ConnectivityConfigs.
        telemetryConfig.backgroundStreaming = (char) val;

        return 0;
}

static int lua_get_bg_streaming(lua_State *L)
{
        lua_pushboolean(L, getWorkingLoggerConfig()->ConnectivityConfigs.
                        telemetryConfig.backgroundStreaming);
        return 1;
}

static int lua_get_gps_at_start_finish(lua_State *L)
{
        lua_pushboolean(L, getAtStartFinish());
        return 1;
}

static int lua_reset_lap_stats(lua_State *L)
{
        lapstats_reset(true);
        return 0;
}

#if PWM_CHANNELS > 0

static int lua_set_pwm_clk_freq(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);
        const int val = lua_tointeger(L,1);

        LoggerConfig *loggerConfig = getWorkingLoggerConfig();
        uint16_t clock_frequency = filterPwmClockFrequency(val);
        loggerConfig->PWMClockFrequency = clock_frequency;
        PWM_set_clock_frequency(clock_frequency);

        return 0;
}

static int lua_get_pwm_clk_freq(lua_State *L)
{
        lua_pushinteger(L, getWorkingLoggerConfig()->PWMClockFrequency);
        return 1;
}

static void validate_pwm_channel(lua_State *l, const size_t channel)
{
        if (channel >= CONFIG_PWM_CHANNELS)
                luaL_error(l, "Invalid PWM channel index");
}

static int lua_set_pwm_duty_cycle(lua_State *L)
{
        lua_validate_args_count(L, 2, 2);
        lua_validate_arg_number(L, 1);
        lua_validate_arg_number(L, 2);

        const size_t channel = lua_tointeger(L, 1);
        validate_pwm_channel(L, channel);

        const size_t dutyCycleRaw = lua_tointeger(L, 2);
        PWM_set_duty_cycle(channel, (unsigned short) dutyCycleRaw);
        return 0;
}

static int lua_set_pwm_period(lua_State *L)
{
        lua_validate_args_count(L, 2, 2);
        lua_validate_arg_number(L, 1);
        lua_validate_arg_number(L, 2);

        const size_t channel = lua_tointeger(L, 1);
        validate_pwm_channel(L, channel);

        const size_t periodRaw = lua_tointeger(L, 2);
        PWM_channel_set_period(channel, (unsigned short) periodRaw);
        return 0;
}

static int lua_set_analog_out(lua_State *L)
{
        lua_validate_args_count(L, 2, 2);
        lua_validate_arg_number(L, 1);
        lua_validate_arg_number(L, 2);

        const size_t channel = lua_tointeger(L,1);
        validate_pwm_channel(L, channel);

        const float dutyCycle = (float) lua_tonumber(L, 2) /
                                PWM_VOLTAGE_SCALING;
        PWM_set_duty_cycle(channel, (unsigned short) dutyCycle);
        return 0;
}
#endif

static int lua_get_analog(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        float analogValue = -1;

#if ANALOG_CHANNELS > 0
        const int val = lua_tointeger(L, 1);
        unsigned int channel = (unsigned int) val;
        ADCConfig *ac = getADCConfigChannel(val);

        /* TODO: Move this into ADC code where it should be */
        if (NULL != ac) {
                float adcRaw = ADC_read(channel);
                switch(ac->scalingMode) {
                case SCALING_MODE_RAW:
                        analogValue = adcRaw;
                        break;
                case SCALING_MODE_LINEAR:
                        analogValue = (ac->linearScaling * (float) adcRaw) + ac->linearOffset;
                        break;
                case SCALING_MODE_MAP:
                        analogValue = get_mapped_value((float) adcRaw,
                                                       &(ac->scalingMap));
                        break;
                }
        }
#endif

        lua_pushnumber(L, analogValue);
        return 1;
}

#if TIMER_CHANNELS > 0
static float get_ppr(const size_t chan_id)
{
        TimerConfig *c = get_timer_config(chan_id);
        return NULL == c ? 0 : c->pulsePerRevolution;
}

static int lua_get_rpm(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const size_t channel = (size_t) lua_tointeger(L, 1);
        const float ppr = get_ppr(channel);
        if (0 == ppr)
                return luaL_error(L, "Invalid channel index given");

        lua_pushinteger(L, (int)((float)timer_get_rpm(channel) / ppr));
        return 1;
}


static int lua_get_period_ms(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const size_t channel = (size_t) lua_tointeger(L, 1);
        const float ppr = get_ppr(channel);
        if (0 == ppr)
                return luaL_error(L, "Invalid channel index given");

        lua_pushinteger(L, (int)(ppr * (float)timer_get_ms(channel)));
        return 1;
}

static int lua_get_freq(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const size_t channel = (size_t) lua_tointeger(L, 1);
        const float ppr = get_ppr(channel);
        if (0 == ppr)
                return luaL_error(L, "Invalid channel index given");

        lua_pushinteger(L, (int)((float)timer_get_hz(channel) / ppr));
        return 1;
}

static int lua_get_timer_raw(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const size_t channel = (size_t) lua_tointeger(L, 1);
        lua_pushinteger(L, timer_get_raw(channel));
        return 1;
}

static int lua_reset_timer_count(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        timer_reset_count(lua_tointeger(L, 1));
        return 0;
}

static int lua_get_timer_count(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const int channel = lua_tointeger(L, 1);
        lua_pushinteger(L, timer_get_count(channel));
        return 1;
}
#endif

#if GPIO_CHANNELS > 0
static int lua_get_button(lua_State *L)
{
        lua_pushboolean(L, GPIO_is_button_pressed());
        return 1;
}
#endif

static struct Serial* lua_get_serial(lua_State *L, const serial_id_t pid)
{
        struct Serial *serial = serial_device_get(pid);
        if (!serial)
                /* This will never return. */
                luaL_error(L, "Serial port not found");

        return serial;
}

/**
 * Initializes the specified serial port
 * Lua Params:
 * port - the serial port to initialize (defaults to SERIAL_AUX)
 * baud - Baud Rate (defaults to 115200)
 * bits - Number of bit in the message (8 or 7) (defaults to 8)
 * parity - (1 = Even Parity, 2 = Odd Parity, 0 = No Parity)
 * (defaults to No Parity)
 * stopBits - number of stop bits (1 or 2) (defaults to 1)
 */
static int lua_init_serial(lua_State *L)
{
        lua_validate_args_count(L, 0, 5);

        serial_id_t port = LUA_DEFAULT_SERIAL_PORT;
        uint32_t baud = LUA_DEFAULT_SERIAL_BAUD;
        uint8_t bits  = LUA_DEFAULT_SERIAL_BITS;
        uint8_t parity = LUA_DEFAULT_SERIAL_PARITY;
        uint8_t stop_bits = LUA_DEFAULT_SERIAL_STOP_BITS;

        switch(lua_gettop(L)) {
        case 5:
                lua_validate_arg_number(L, 5);
                stop_bits = lua_tointeger(L, 5);
        case 4:
                lua_validate_arg_number(L, 4);
                parity = lua_tointeger(L, 4);
        case 3:
                lua_validate_arg_number(L, 3);
                bits = lua_tointeger(L, 3);
        case 2:
                lua_validate_arg_number(L, 2);
                baud = lua_tointeger(L, 2);
        case 1:
                lua_validate_arg_number(L, 1);
                port = lua_tointeger(L, 1);
        }

        struct Serial* serial = lua_get_serial(L, port);
        serial_config(serial, bits, parity, stop_bits, baud);
        lua_pushboolean(L, true);

        return 1;
}

/**
 * Read a raw character from the specified serial port
 * Lua Params:
 * port - the serial port to initialize
 * timeout - the read timeout, in ms.
 *
 * Lua Returns:
 * The raw character read, or nil if no characters received (receive timeout)
 *
 */
static int lua_serial_read_char(lua_State *L)
{
        lua_validate_args_count(L, 0, 2);

        serial_id_t port = LUA_DEFAULT_SERIAL_PORT;
        size_t timeout = DEFAULT_SERIAL_TIMEOUT;

        switch(lua_gettop(L)) {
        case 2:
                lua_validate_arg_number(L, 2);
                timeout = lua_tointeger(L, 2);
        case 1:
                lua_validate_arg_number(L, 1);
                port = (serial_id_t) lua_tointeger(L, 1);
        }

        char c;
        struct Serial *serial = lua_get_serial(L, port);
        if (0 < serial_read_c_wait(serial, &c, timeout)) {
                lua_pushinteger(L, (int) c);
        } else {
                lua_pushnil(L);
        }

        return 1;
}

/**
 * Read a single newline terminated line from the specified serial port
 * Lua Params:
 * port - the serial port to initialize
 * timeout - the read timeout, in ms.
 *
 * Lua Returns:
 * The character read, terminating with a nil character always.
 */
static int lua_serial_read_line(lua_State *L)
{
        lua_validate_args_count(L, 0, 2);

        serial_id_t port = LUA_DEFAULT_SERIAL_PORT;
        size_t timeout = DEFAULT_SERIAL_TIMEOUT;

        switch(lua_gettop(L)) {
        case 2:
                lua_validate_arg_number(L, 2);
                timeout = lua_tointeger(L, 2);
        case 1:
                lua_validate_arg_number(L, 1);
                port = (serial_id_t) lua_tointeger(L, 1);
        }

        /* STIEG: Would be nice to be rid of this tempBuffer */
        static char g_tempBuffer[TEMP_BUFFER_LEN];
        struct Serial *serial = lua_get_serial(L, port);
        int len = serial_read_line_wait(serial, g_tempBuffer,
                                        TEMP_BUFFER_LEN - 1, timeout);
        if (len < 0)
                len = 0;

        g_tempBuffer[len] = 0;
        lua_pushstring(L, g_tempBuffer);

        return 1;
}

/**
 * Writes the specified line to the serial port.
 * The call will block until all characters are written.
 *
 * Lua Params:
 * port - the serial port to write
 * line - the string to write. A newline will automatically be added at the end.
 *
 * Lua Returns:
 * no return values
 */
static int lua_serial_write_line(lua_State *L)
{
        lua_validate_args_count(L, 2, 2);
        lua_validate_arg_number(L, 1);
        lua_validate_arg_string(L, 2);

        const serial_id_t port = (serial_id_t) lua_tointeger(L, 1);
        const char *data = lua_tostring(L, 2);

        struct Serial *serial = lua_get_serial(L, port);
        serial_write_s(serial, data);
        serial_write_c(serial, '\n');

        return 0;
}

/**
 * Writes the specified raw character to the serial port.
 * The call will block until the character is written.
 *
 * Lua Params:
 * port - the serial port to write
 * char - the character (in raw digit format) to write.
 *
 * Lua Returns:
 * no return values (nil)
 *
 */
static int lua_serial_write_char(lua_State *L)
{
        lua_validate_args_count(L, 2, 2);
        lua_validate_arg_number(L, 1);
        lua_validate_arg_number(L, 2);

        const serial_id_t port = (serial_id_t) lua_tointeger(L, 1);
        const int data = lua_tointeger(L, 2);

        struct Serial *serial = lua_get_serial(L, port);
        serial_write_c(serial, (char) data);

        return 0;
}

#if GPIO_CHANNELS > 1
static int lua_get_gpio(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        lua_pushinteger(L, GPIO_get(lua_tointeger(L, 1)));
        return 1;
}

static int lua_set_gpio(lua_State *L)
{
        lua_validate_args_count(L, 2, 2);
        lua_validate_arg_number(L, 1);
        lua_validate_arg_boolean_flex(L, 2);

        const unsigned int channel = (unsigned int) lua_tointeger(L, 1);
        const unsigned int state = lua_toboolean_flex(L, 2);
        GPIO_set(channel, state);
        return 0;
}
#endif

static int lua_get_gps_sat_count(lua_State *L)
{
        lua_pushnumber(L, GPS_getSatellitesUsedForPosition());
        return 1;
}

static int lua_get_gps_position(lua_State *L)
{
        lua_pushnumber(L, GPS_getLatitude());
        lua_pushnumber(L, GPS_getLongitude());
        return 2;
}

static int lua_get_gps_speed(lua_State *L)
{
        lua_pushnumber(L,getGpsSpeedInMph());
        return 1;
}

static int lua_get_gps_quality(lua_State *L)
{
        lua_pushnumber(L,GPS_getQuality());
        return 1;
}

static int lua_get_gps_dop(lua_State *L)
{
        lua_pushnumber(L,GPS_getDOP());
        return 1;
}

static int lua_get_gps_vel(lua_State *L)
{
        lua_pushnumber(L,GPS_getVelocityX());
        lua_pushnumber(L,GPS_getVelocityY());
        lua_pushnumber(L,GPS_getVelocityZ());
        return 3;
}

static int lua_get_gps_distance(lua_State *L)
{
        lua_pushnumber(L, getLapDistanceInMiles());
        return 1;
}

static int lua_get_predicted_lap_time(lua_State *L)
{
        lua_pushnumber(L, getPredictedTimeInMinutes());
        return 1;
}

static int lua_get_lap_time(lua_State *L)
{
        lua_pushnumber(L, tinyMillisToMinutes(getLastLapTime()));
        return 1;
}

static int lua_get_lap_count(lua_State *L)
{
        lua_pushinteger(L, getLapCount());
        return 1;
}

static int lua_get_seconds_since_first_fix(lua_State *L)
{
        lua_pushnumber(L, tinyMillisToSeconds(getMillisSinceFirstFix()));
        return 1;
}

static int lua_get_tick_count(lua_State *L)
{
        lua_pushinteger(L, xTaskGetTickCount());
        return 1;
}

static int lua_get_ticks_per_second(lua_State *L)
{
        lua_pushinteger(L, configTICK_RATE_HZ);
        return 1;
}

#if IMU_CHANNELS > 0
static int lua_calibrate_imu_zero(lua_State *L)
{
        imu_calibrate_zero();
        return 0;
}

static void validate_imu_channel(lua_State *l, const size_t channel)
{
        if (channel >= CONFIG_IMU_CHANNELS)
                luaL_error(l, "Invalid IMU channel index");
}

static int lua_imu_read(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const size_t channel = lua_tointeger(L, 1);
        validate_imu_channel(L, channel);

        ImuConfig *ac = &getWorkingLoggerConfig()->ImuConfigs[channel];
        lua_pushnumber(L, imu_read_value(channel, ac));
        return 1;
}

static int lua_imu_read_raw(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const size_t channel = lua_tointeger(L, 1);
        validate_imu_channel(L, channel);

        lua_pushinteger(L, imu_read(channel));
        return 1;
}
#endif

static int lua_init_can(lua_State *L)
{
        lua_validate_args_count(L, 2, 3);

        bool termination_enabled = true;
        switch(lua_gettop(L)) {
        default:
                return lua_panic(L);
        case 3:
                lua_validate_arg_number(L, 3);
                termination_enabled = (bool)lua_tonumber(L, 3);
        case 2:
                lua_validate_arg_number(L, 2);
                lua_validate_arg_number(L, 1);
        }

        const size_t port = lua_tointeger(L, 1);
        const size_t baud = lua_tointeger(L, 2);
        lua_pushinteger(L, CAN_init_port(port, baud, termination_enabled));
        return 1;
}

static int lua_set_can_filter(lua_State *L)
{
        lua_validate_args_count(L, 5, 6);

        bool enable = true;
        switch(lua_gettop(L)) {
        default:
                return lua_panic(L);
        case 6:
                lua_validate_arg_boolean(L, 6);
                enable = lua_toboolean(L, 6);
        case 5:
                break;
        }

        for (int i = 1; i <= 5; ++i)
                lua_validate_arg_number(L, i);

        const uint8_t channel = lua_tointeger(L, 1);
        const uint8_t id = lua_tointeger(L, 2);
        const uint8_t extended = lua_tointeger(L, 3);
        const uint32_t filter = lua_tointeger(L, 4);
        const uint32_t mask = lua_tointeger(L, 5);

        const int result = CAN_set_filter(channel, id, extended, filter,
                                          mask, enable);
        lua_pushinteger(L, result);

        return 1;
}

static int lua_send_can_msg(lua_State *L)
{
        lua_validate_args_count(L, 4, 5);

        CAN_msg msg;
        uint8_t channel;
        size_t size;
        size_t timeout = DEFAULT_CAN_TIMEOUT;
        switch(lua_gettop(L)) {
        default:
                return lua_panic(L);
        case 5:
                lua_validate_arg_number(L, 5);
                timeout = lua_tointeger(L, 5);
        case 4:
                lua_validate_arg_number(L, 1);
                lua_validate_arg_number(L, 2);
                lua_validate_arg_number(L, 3);
                lua_validate_arg_table(L, 4);

                channel = (uint8_t) lua_tointeger(L, 1);
                msg.addressValue = (unsigned int) lua_tointeger(L, 2);
                msg.isExtendedAddress = lua_tointeger(L, 3);
                size = luaL_getn(L, 4);
        }

        if (size > CAN_MSG_SIZE)
                return luaL_error(L, "Table size it too large");

        for (int i = 0; i < size; i++) {
                lua_pushnumber(L, i + 1);
                lua_gettable(L, 4);
                msg.data[i] = lua_tonumber(L, -1);
                lua_pop(L, 1);
        }

        msg.dataLength = size;
        lua_pushinteger(L, CAN_tx_msg(channel, &msg, timeout));
        return 1;
}

static int lua_rx_can_msg(lua_State *L)
{
        lua_validate_args_count(L, 1, 2);

        size_t can_bus;
        size_t timeout = DEFAULT_CAN_TIMEOUT;

        switch(lua_gettop(L)) {
        default:
                return lua_panic(L);
        case 2:
                lua_validate_arg_number(L, 2);
                timeout = lua_tointeger(L, 2);
        case 1:
                lua_validate_arg_number(L, 1);
                can_bus = lua_tointeger(L, 1);
        }

        CAN_msg can_msg;
        if (!CAN_aux_queue_get_msg(can_bus, &can_msg, timeout))
                return 0;

        lua_pushinteger(L, can_msg.addressValue);
        lua_pushinteger(L, can_msg.isExtendedAddress);

        lua_newtable(L);
        for (int i = 1; i <= can_msg.dataLength; i++) {
                lua_pushnumber(L, i);
                lua_pushnumber(L, can_msg.data[i - 1]);
                lua_rawset(L, -3);
        }
        return 3;
}

static int lua_obd2_read(lua_State *L)
{
        lua_validate_args_count(L, 1, 2);

        uint8_t pid;

        switch(lua_gettop(L)) {
        default:
                return lua_panic(L);
        case 2:
        case 1:
                lua_validate_arg_number(L, 1);
                pid = lua_tointeger(L, 1);
        }

        float value;
        if (OBD2_get_value_for_pid(pid, &value)) {
                lua_pushnumber(L, value);
                return 1;
        } else
                return 0;
}

static int lua_obd2_set_delay(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);
        OBD2_set_pid_delay(lua_tonumber(L, 1));
        return 0;
}

static int lua_logging_start(lua_State *L)
{
        startLogging();
        return 0;
}

static int lua_logging_stop(lua_State *L)
{
        stopLogging();
        return 0;
}

static int lua_logging_is_active(lua_State *L)
{
        lua_pushinteger(L, logging_is_active());
        return 1;
}

#if CELLULAR_SUPPORT
static int lua_reconnect_cell(lua_State *L)
{
        cellular_telemetry_reconnect();
        return 0;
}
#endif

static int lua_set_led(lua_State *ls)
{
        lua_validate_args_count(ls, 2, 2);
        lua_validate_arg_number_or_string(ls, 1);
        lua_validate_arg_boolean_flex(ls, 2);

        /*
         * Numbers can be passed in as 123 or "123" in lua.  So handle
         * that case first
         */
        volatile const bool on = lua_toboolean_flex(ls, 2);
        bool res;
        if (lua_isnumber(ls, 1)) {
                res = led_set_index(lua_tointeger(ls, 1), on);
        } else {
                const enum led led = get_led_enum(lua_tostring(ls, 1));
                res = led_set(led, on);
        }

        lua_pushboolean(ls, res);
        return 1;
}


static int lua_flash_config(lua_State *L)
{
        lua_pushinteger(L, flashLoggerConfig());
        return 1;
}

static int lua_add_virt_channel(lua_State *L)
{
        lua_validate_args_count(L, 2, 6);

        ChannelConfig cc;
        channel_config_defaults(&cc);

        switch(lua_gettop(L)) {
        case 6:
                switch (validate_channel_config_units(lua_tostring(L, 6))) {
                case CHAN_CFG_STATUS_OK:
                        strcpy(cc.units, lua_tostring(L, 6));
                        break;
                case CHAN_CFG_STATUS_NO_UNITS:
                        return luaL_error(L, "Units is empty");
                case CHAN_CFG_STATUS_LONG_UNITS:
                        return luaL_error(L, "Units is too long");
                default:
                        return lua_panic(L);
                }
        case 5:
                cc.max = lua_tointeger(L, 5);
        case 4:
                cc.min = lua_tointeger(L, 4);
        case 3:
                cc.precision = lua_tointeger(L, 3);
        }

        cc.sampleRate = encodeSampleRate((unsigned short) lua_tointeger(L, 2));
        if (SAMPLE_DISABLED == cc.sampleRate)
                return luaL_error(L, "Unsupported sample rate");

        switch (validate_channel_config_label(lua_tostring(L, 1))) {
        case CHAN_CFG_STATUS_OK:
                strcpy(cc.label, lua_tostring(L, 1));
                break;
        case CHAN_CFG_STATUS_NO_LABEL:
                return luaL_error(L, "Label is empty");
        case CHAN_CFG_STATUS_LONG_LABEL:
                return luaL_error(L, "Label is too long");
        default:
                return lua_panic(L);
        }

        switch(validate_channel_config(&cc)) {
        case CHAN_CFG_STATUS_OK:
                break;
        case CHAN_CFG_STATUS_MAX_LT_MIN:
                return luaL_error(L, "Max is less than min");
        default:
                return lua_panic(L);
        }

        const int chan_id = create_virtual_channel(cc);
        if (INVALID_VIRTUAL_CHANNEL == chan_id)
                return luaL_error(L, "Unable to create channel. "
                                  "Maximum channels reached.");

        lua_pushinteger(L, chan_id + 1);
        return 1;
}

static int lua_get_virtual_channel(lua_State *ls)
{
        lua_validate_args_count(ls, 1, 1);
        lua_validate_arg_number_or_string(ls, 1);

        /*
         * Numbers can be passed in as 123 or "123" in lua.  So handle
         * that case first
         */
        if (lua_isnumber(ls, 1)) {
                VirtualChannel *vc;
                int channel_id = lua_tointeger(ls, 1);
                vc = get_virtual_channel(channel_id - 1);
                if (vc) {
                        lua_pushnumber(ls, vc->currentValue);
                        return 1;
                }
        } else {
                double value;
                char * units;
                if (get_channel_value_by_name(lua_tostring(ls, 1), &value, &units)) {
                        lua_pushnumber(ls, value);
                        return 1;
                }
        }
        /* Return error, channel not found */
        return luaL_error(ls, "getChannel: channel not found");
}

static int lua_set_virt_channel_value(lua_State *L)
{
        lua_validate_args_count(L, 2, 2);

        const int id = lua_tointeger(L, 1) - 1;
        const float value = lua_tonumber(L, 2);
        if ( id < 0 )
                return luaL_error(L, "setChannel: channel not found");

        set_virtual_channel_value(id, value);

        return 0;
}

static int lua_update_gps(lua_State *L)
/* Internal function to stimulate lap timer via lua script */
{
        lua_validate_args_count(L, 3, 8);

        GpsSample s;

        s.time = getUptime();
        s.altitude = 0;
        s.quality = GPS_QUALITY_3D_DGNSS;
        s.DOP = 0;
        s.satellites = 0;

        switch(lua_gettop(L)) {
        case 8:
                s.satellites = lua_tointeger(L, 8);
        case 7:
                s.DOP = lua_tonumber(L, 7);
        case 6:
                s.quality = lua_tointeger(L, 6);
        case 5:
                s.altitude = lua_tonumber(L, 5);
        case 4:
                s.time = lua_tointeger(L, 4);
        default:
                /**
                 * Minimum data needed to drive lap timer is
                 * Latitude, Longitude and Speed
                 */
                s.speed = lua_tonumber(L, 3);
                GeoPoint gp;
                gp.longitude = lua_tonumber(L, 2);
                gp.latitude = lua_tonumber(L, 1);
                s.point = gp;
        }
        lapstats_process_incremental(&s);
        GPS_sample_update(&s);
        if (isGpsSignalUsable(s.quality)) {
                GpsSnapshot snap = getGpsSnapshot();
                lapstats_processUpdate(&snap);
        }
        return 0;
}

static int lua_calc_gear(lua_State *L)
{
        /* Calculate gear helper function
         *
         * Function signatures:
         * calcGear(tireDiamCm, finalGearRatio, gear1Ratio, gear2Ratio, gear3Ratio, gear4Ratio, gear5Ratio, gear6Ratio)
         *
         * Default using the built-in GPS speed channel and "RPM" channel.
         *
         * calcGear(speedChannelName, rpmChannelName, tireDiamCm, finalGearRatio, gear1Ratio, gear2Ratio, gear3Ratio, gear4Ratio, gear5Ratio, gear6Ratio)
         *
         * Use specified Speed and RPM channel name.
         *
         */
        lua_validate_args_count(L, 3, 10);

        float speed = 0;
        float rpm = 0;

        double value;
        char * units;
        size_t params_start = 1;

        /* check if first 2 parameters are numbers; if so, use built in channels */
        if (lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
                lua_validate_args_count(L, 3, 8);

                /* internal speed is in kph */
                speed = getGPSSpeed();

                /* Ensure RPM is available in the current sample. If not, bail out*/
                if (!(get_channel_value_by_name("RPM", &value, &units)))
                        return 0;
                rpm = value;
        } else {
                /* For Speed and RPM, check if sample is currently available and if
                 * channel exists in current sample. If not, bail out
                 */
                if (!(get_channel_value_by_name(lua_tostring(L, 1), &value, &units)))
                        return 0;
                speed = value;
                if (strcasecmp("kph", units) != 0) {
                        /* if units are not kph, assume mph and convert */
                        speed *= 1.60934f;
                }

                if (!(get_channel_value_by_name(lua_tostring(L, 2), &value, &units)))
                        return 0;
                rpm = value;
                params_start += 2;
        }

        lua_validate_arg_number(L, params_start);
        float tire_diameter_cm = (float)lua_tonumber(L, params_start);
        params_start++;

        if (tire_diameter_cm == 0)
                return luaL_error(L, "Tire Diameter must be > 0");

        lua_validate_arg_number(L, params_start);
        float final_drive_ratio = (float)lua_tonumber(L, params_start);
        params_start++;

        if (final_drive_ratio == 0)
                return luaL_error(L, "Final Drive ratio must be > 0");

        /* cant calculate gear if speed below threshold */
        if (speed < 10)
                return 0;

        /* Calculate ratio based on cm per minute */
        float rpm_speed_ratio = (rpm / speed)/(final_drive_ratio * 1666.67f / (tire_diameter_cm * 3.14159f));

        float gear_error = 0.1;
        uint8_t gear_pos = 0;

        /* Cycle through the remaining parameters to see what ratio matches the specified gear */
        for (size_t i = params_start; i <= lua_gettop(L); i++) {
                lua_validate_arg_number(L, i);
                float gear_ratio = lua_tonumber(L, i);
                if (fabsf(gear_ratio - rpm_speed_ratio) < gear_error) {
                        gear_pos = i - (params_start - 1);
                        break;
                }
        }
        if (gear_pos > 0) {
                lua_pushinteger(L, gear_pos);
                return 1;
        } else {
                return 0;
        }
}

static int lua_sx_update_linear_graph(lua_State *L)
{
        lua_validate_args_count(L, 0, 1);
        if (lua_gettop(L) > 0) {
                lua_validate_arg_number(L, 1);
                uint16_t value = lua_tointeger(L, 1);
                lua_pushinteger(L, shiftx_update_linear_graph(value));
        } else {
                lua_pushinteger(L, 0);
        }
        return 1;
}

static int lua_sx_set_alert(lua_State *L)
{
        lua_validate_args_count(L, 5, 5);

        lua_validate_arg_number(L, 1);
        uint8_t alert_id = lua_tointeger(L, 1);

        lua_validate_arg_number(L, 2);
        uint8_t red = lua_tointeger(L, 2);

        lua_validate_arg_number(L, 3);
        uint8_t green = lua_tointeger(L, 3);

        lua_validate_arg_number(L, 4);
        uint8_t blue = lua_tointeger(L, 4);

        lua_validate_arg_number(L, 5);
        uint8_t flash = lua_tointeger(L, 5);

        struct shiftx_led_params led_params = {red, green, blue, flash};
        lua_pushinteger(L, shiftx_set_alert(alert_id, led_params));
        return 1;
}

static int lua_sx_update_alert(lua_State *L)
{
        lua_validate_args_count(L, 1, 2);

        lua_validate_arg_number(L, 1);
        uint8_t alert_id = lua_tointeger(L, 1);

        if (lua_gettop(L) > 1) {
                lua_validate_arg_number(L, 2);
                uint16_t value = lua_tointeger(L, 2);
                lua_pushinteger(L, shiftx_update_alert(alert_id, value));
        } else {
                lua_pushinteger(L, 0);
        }
        return 1;
}

static int lua_sx_set_display(lua_State *L)
{
        lua_validate_args_count(L, 1, 2);

        lua_validate_arg_number(L, 1);
        uint8_t digit_index = lua_tointeger(L, 1);

        if (lua_gettop(L) > 1 && lua_isnumber(L, 2)) {
                lua_validate_arg_number(L, 2);
                uint8_t value = lua_tointeger(L, 2);
                /* digit index offset by ascii 0 (48) */
                lua_pushinteger(L, shiftx_set_display(digit_index, 48 + value));
        } else {
                /* blank display if nil value supplied as arg #2, or if arg 2 is missing */
                shiftx_set_display(digit_index, 32); /* ascii 32 (space character) */
                lua_pushinteger(L, 0);
        }
        return 1;
}

static int lua_sx_set_led(lua_State *L)
{
        lua_validate_args_count(L, 6, 6);

        lua_validate_arg_number(L, 1);
        uint8_t led_index = lua_tointeger(L, 1);

        lua_validate_arg_number(L, 2);
        uint8_t leds_to_set = lua_tointeger(L, 2);

        lua_validate_arg_number(L, 3);
        uint8_t red = lua_tointeger(L, 3);

        lua_validate_arg_number(L, 4);
        uint8_t green = lua_tointeger(L, 4);

        lua_validate_arg_number(L, 5);
        uint8_t blue = lua_tointeger(L, 5);

        lua_validate_arg_number(L, 6);
        uint8_t flash = lua_tointeger(L, 6);

        struct shiftx_led_params led_params = {red, green, blue, flash};
        lua_pushinteger(L, shiftx_set_discrete_led(led_index, leds_to_set, led_params));
        return 1;
}

static int lua_sx_config_linear_graph(lua_State *L)
{
        lua_validate_args_count(L, 3, 4);

        lua_validate_arg_number(L, 1);
        uint8_t rendering_style = lua_tointeger(L, 1);

        lua_validate_arg_number(L, 2);
        uint8_t linear_style = lua_tointeger(L, 2);

        lua_validate_arg_number(L, 3);
        uint16_t low_range = lua_tointeger(L, 3);

        uint16_t high_range = 0;
        if (lua_gettop(L) > 3) {
                high_range = lua_tointeger(L, 4);
        }

        lua_pushinteger(L, shiftx_config_linear_graph(rendering_style, linear_style, low_range, high_range));
        return 1;
}

static int lua_sx_set_linear_threshold(lua_State *L)
{
        lua_validate_args_count(L, 7, 7);

        lua_validate_arg_number(L, 1);
        uint8_t threshold_id = lua_tointeger(L, 1);

        lua_validate_arg_number(L, 2);
        uint8_t segment_length = lua_tointeger(L, 2);

        lua_validate_arg_number(L, 3);
        uint16_t threshold = lua_tointeger(L, 3);

        lua_validate_arg_number(L, 4);
        uint8_t red = lua_tointeger(L, 4);

        lua_validate_arg_number(L, 5);
        uint8_t green = lua_tointeger(L, 5);

        lua_validate_arg_number(L, 6);
        uint8_t blue = lua_tointeger(L, 6);

        lua_validate_arg_number(L, 7);
        uint8_t flash = lua_tointeger(L, 7);

        struct shiftx_led_params led_params = {red, green, blue, flash};
        lua_pushinteger(L, shiftx_set_linear_threshold(threshold_id, segment_length, threshold, led_params));
        return 1;
}

static int lua_sx_set_alert_threshold(lua_State *L)
{
        lua_validate_args_count(L, 7, 7);

        lua_validate_arg_number(L, 1);
        uint8_t alert_id = lua_tointeger(L, 1);

        lua_validate_arg_number(L, 2);
        uint8_t threshold_id = lua_tointeger(L, 2);

        lua_validate_arg_number(L, 3);
        uint16_t threshold = lua_tointeger(L, 3);

        lua_validate_arg_number(L, 4);
        uint8_t red = lua_tointeger(L, 4);

        lua_validate_arg_number(L, 5);
        uint8_t green = lua_tointeger(L, 5);

        lua_validate_arg_number(L, 6);
        uint8_t blue = lua_tointeger(L, 6);

        lua_validate_arg_number(L, 7);
        uint8_t flash = lua_tointeger(L, 7);

        struct shiftx_led_params led_params = {red, green, blue, flash};
        lua_pushinteger(L, shiftx_set_alert_threshold(alert_id, threshold_id, threshold, led_params));
        return 1;
}

static int lua_sx_set_config(lua_State *L)
{
        lua_validate_args_count(L, 0, 6);

        struct shiftx_configuration * shiftx_config = shiftx_get_config();

        switch(lua_gettop(L)) {
        default:
                return lua_panic(L);
        case 6:
                /* enable / disable button events */
                lua_validate_arg_boolean(L, 6);
                shiftx_config->button_events_enabled = lua_toboolean(L, 6);
        case 5:
                /* auto brightness scaling */
                lua_validate_arg_number(L, 5);
                shiftx_config->auto_brightness_scaling = lua_tointeger(L, 5);
        case 4:
                /* CAN Base Address */
                lua_validate_arg_number(L, 4);
                shiftx_config->base_address = lua_tointeger(L, 4);
        case 3:
                /* CAN bus */
                lua_validate_arg_number(L, 3);
                shiftx_config->can_bus = lua_tointeger(L, 3);
        case 2:
                /* ShiftX brightness (0 to 100; 0=automatic brightness)*/
                lua_validate_arg_number(L, 2);
                shiftx_config->brightness = lua_tointeger(L, 2);
        case 1:
                /* ShiftX orientation (0=normal, 1= inverted) */
                lua_validate_arg_number(L, 1);
                shiftx_config->orientation_inverted = lua_tointeger(L, 1);
        }
        delayMs(500);
        lua_pushinteger(L, shiftx_update_config());
        return 1;
}

static int lua_rx_button_press(lua_State *L)
{
        uint8_t button_id;
        uint8_t state;

        if (shiftx_rx_button_press(&button_id, &state)) {
                lua_pushinteger(L, button_id);
                lua_pushinteger(L, state);
                return 2;
        } else {
                return 0;
        }
}

static int lua_tx_button(lua_State *L)
{
        lua_validate_args_count(L, 2, 2);

        lua_validate_arg_number(L, 1);
        uint8_t button_id = lua_tointeger(L, 1);

        lua_validate_arg_number(L, 2);
        uint8_t state = lua_tointeger(L, 2);

        /* Send a button press to connected clients */
        struct api_event event;
        event.source = NULL; /* not coming from any serial source */
        event.type = ApiEventType_ButtonState;
        event.data.butt_state.button_id = button_id;
        event.data.butt_state.state = state;

        /* Broadcast to active connections */
        api_event_process_callbacks(&event);
        return 0;
}

void registerLuaLoggerBindings(lua_State *L)
{
#if GPIO_CHANNELS > 0
        lua_registerlight(L,"getButton", lua_get_button);
#endif
#if GPIO_CHANNELS > 1
        lua_registerlight(L,"getGpio", lua_get_gpio);
        lua_registerlight(L,"setGpio", lua_set_gpio);
#endif

#if PWM_CHANNELS > 0
        lua_registerlight(L, "setPwmDutyCycle", lua_set_pwm_duty_cycle);
        lua_registerlight(L, "setPwmPeriod", lua_set_pwm_period);
        lua_registerlight(L, "setAnalogOut", lua_set_analog_out);
        lua_registerlight(L, "setPwmClockFreq", lua_set_pwm_clk_freq);
        lua_registerlight(L, "getPwmClockFreq", lua_get_pwm_clk_freq);
#endif

#if TIMER_CHANNELS > 0
        lua_registerlight(L, "getTimerRpm", lua_get_rpm);
        lua_registerlight(L, "getTimerPeriodMs", lua_get_period_ms);
        lua_registerlight(L, "getTimerFreq", lua_get_freq);
        lua_registerlight(L, "getTimerRaw", lua_get_timer_raw);
        lua_registerlight(L, "resetTimerCount", lua_reset_timer_count);
        lua_registerlight(L, "getTimerCount", lua_get_timer_count);
#endif

        lua_registerlight(L, "getAnalog", lua_get_analog);

#if IMU_CHANNELS > 0
        lua_registerlight(L, "getImu", lua_imu_read);
        lua_registerlight(L, "getImuRaw", lua_imu_read_raw);
        lua_registerlight(L, "calibrateImuZero", lua_calibrate_imu_zero);
#endif

        lua_registerlight(L, "getGpsSats", lua_get_gps_sat_count);
        lua_registerlight(L, "getGpsPos", lua_get_gps_position);
        lua_registerlight(L, "getGpsSpeed", lua_get_gps_speed);
        lua_registerlight(L, "getGpsQuality", lua_get_gps_quality);
        lua_registerlight(L, "getGpsDOP", lua_get_gps_dop);
        lua_registerlight(L, "getGpsDist", lua_get_gps_distance);
        lua_registerlight(L, "getGpsAltitude", lua_get_gps_altitude);
        lua_registerlight(L, "getGpsVelocity", lua_get_gps_vel);

        lua_registerlight(L, "getPredTime", lua_get_predicted_lap_time);
        lua_registerlight(L, "getLapCount", lua_get_lap_count);
        lua_registerlight(L, "getLapTime", lua_get_lap_time);
        lua_registerlight(L, "getGpsSec", lua_get_seconds_since_first_fix);
        lua_registerlight(L, "getAtStartFinish", lua_get_gps_at_start_finish);
        lua_registerlight(L, "resetLapStats", lua_reset_lap_stats);

        lua_registerlight(L, "getTickCount", lua_get_tick_count);
        lua_registerlight(L, "getTicksPerSecond", lua_get_ticks_per_second);

        lua_registerlight(L, "initCAN", lua_init_can);
        lua_registerlight(L, "txCAN", lua_send_can_msg);
        lua_registerlight(L, "rxCAN", lua_rx_can_msg);
        lua_registerlight(L, "setCANfilter", lua_set_can_filter);
        lua_registerlight(L, "readOBD2", lua_obd2_read);
        lua_registerlight(L, "setOBD2Delay", lua_obd2_set_delay);

        lua_registerlight(L, "startLogging", lua_logging_start);
        lua_registerlight(L, "stopLogging", lua_logging_stop);
        lua_registerlight(L, "isLogging", lua_logging_is_active);
#if CELLULAR_SUPPORT
        lua_registerlight(L, "reconnectCell", lua_reconnect_cell);
#endif
        lua_registerlight(L, "setLed", lua_set_led);

        lua_registerlight(L, "initSer", lua_init_serial);
        lua_registerlight(L, "readCSer", lua_serial_read_char);
        lua_registerlight(L, "readSer", lua_serial_read_line);
        lua_registerlight(L, "writeCSer", lua_serial_write_char);
        lua_registerlight(L, "writeSer", lua_serial_write_line);

        lua_registerlight(L, "flashLoggerCfg", lua_flash_config);

        lua_registerlight(L, "setBgStream", lua_set_bg_streaming);
        lua_registerlight(L, "getBgStream", lua_get_bg_streaming);

        lua_registerlight(L, "addChannel", lua_add_virt_channel);
        lua_registerlight(L, "getChannel", lua_get_virtual_channel);
        lua_registerlight(L, "setChannel", lua_set_virt_channel_value);

        lua_registerlight(L, "getUptime", lua_get_uptime);
        lua_registerlight(L, "getDateTime", lua_get_date_time);

        lua_registerlight(L, "updateGps", lua_update_gps);

        lua_registerlight(L, "txButton", lua_tx_button);

        /* helper functions */
        lua_registerlight(L, "calcGear", lua_calc_gear);

        /* ShiftX2/3 support functions */
        lua_registerlight(L, "sxUpdateLinearGraph", lua_sx_update_linear_graph);
        lua_registerlight(L, "sxSetAlert", lua_sx_set_alert);
        lua_registerlight(L, "sxUpdateAlert", lua_sx_update_alert);
        lua_registerlight(L, "sxSetDisplay", lua_sx_set_display);
        lua_registerlight(L, "sxSetLed", lua_sx_set_led);
        lua_registerlight(L, "sxCfgLinearGraph", lua_sx_config_linear_graph);
        lua_registerlight(L, "sxSetLinearThresh", lua_sx_set_linear_threshold);
        lua_registerlight(L, "sxSetAlertThresh", lua_sx_set_alert_threshold);
        lua_registerlight(L, "sxSetConfig", lua_sx_set_config);
        lua_registerlight(L, "sxRxButton", lua_rx_button_press);
}
