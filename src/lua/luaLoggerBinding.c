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

static int lua_calibrate_imu_zero(lua_State *L)
{
        imu_calibrate_zero();
        return 0;
}

static int lua_get_gps_at_start_finish(lua_State *L)
{
        lua_pushboolean(L, getAtStartFinish());
        return 1;
}

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

static int lua_get_analog(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const int val = lua_tointeger(L, 1);
        float analogValue = -1;
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
                        analogValue = (ac->linearScaling * (float) adcRaw);
                        break;
                case SCALING_MODE_MAP:
                        analogValue = get_mapped_value((float) adcRaw,
                                                       &(ac->scalingMap));
                        break;
                }
        }

        lua_pushnumber(L, analogValue);
        return 1;
}

static uint8_t get_ppr(const size_t chan_id)
{
        TimerConfig *c = get_timer_config(chan_id);
        return NULL == c ? 0 : c->pulsePerRevolution;
}

static int lua_get_rpm(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const size_t channel = (size_t) lua_tointeger(L, 1);
        const uint8_t ppr = get_ppr(channel);
        if (0 == ppr)
                return luaL_error(L, "Invalid channel index given");

        lua_pushinteger(L, timer_get_rpm(channel) / ppr);
        return 1;
}


static int lua_get_period_ms(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const size_t channel = (size_t) lua_tointeger(L, 1);
        const uint8_t ppr = get_ppr(channel);
        if (0 == ppr)
                return luaL_error(L, "Invalid channel index given");

        lua_pushinteger(L, timer_get_ms(channel) * ppr);
        return 1;
}

static int lua_get_freq(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const size_t channel = (size_t) lua_tointeger(L, 1);
        const uint8_t ppr = get_ppr(channel);
        if (0 == ppr)
                return luaL_error(L, "Invalid channel index given");

        lua_pushinteger(L, timer_get_hz(channel) / ppr);
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

static int lua_get_button(lua_State *L)
{
        lua_pushboolean(L, GPIO_is_button_pressed());
        return 1;
}

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

static int lua_get_gps_distance(lua_State *L)
{
        lua_pushnumber(L, getLapDistanceInMiles());
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

static int lua_init_can(lua_State *L)
{
        lua_validate_args_count(L, 2, 2);
        lua_validate_arg_number(L, 1);
        lua_validate_arg_number(L, 2);

        const size_t port = lua_tointeger(L, 1);
        const size_t baud = lua_tointeger(L, 2);
        lua_pushinteger(L, CAN_init_port(port, baud));
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

        size_t channel;
        size_t timeout = DEFAULT_CAN_TIMEOUT;

        switch(lua_gettop(L)) {
        default:
                return lua_panic(L);
        case 2:
                lua_validate_arg_number(L, 2);
                timeout = lua_tointeger(L, 2);
        case 1:
                lua_validate_arg_number(L, 1);
                channel = lua_tointeger(L, 1);
        }

        CAN_msg msg;
        if (!CAN_rx_msg(channel, &msg, timeout))
                return 0;

        lua_pushinteger(L, msg.addressValue);
        lua_pushinteger(L, msg.isExtendedAddress);

        lua_newtable(L);
        for (int i = 1; i <= msg.dataLength; i++) {
                lua_pushnumber(L, i);
                lua_pushnumber(L, msg.data[i - 1]);
                lua_rawset(L, -3);
        }

        return 3;
}

static int lua_obd2_read(lua_State *L)
{
        lua_validate_args_count(L, 1, 2);

        size_t pid;
        size_t timeout = OBD2_PID_DEFAULT_TIMEOUT_MS;

        switch(lua_gettop(L)) {
        default:
                return lua_panic(L);
        case 2:
                lua_validate_arg_number(L, 2);
                timeout = lua_tointeger(L, 2);
        case 1:
                lua_validate_arg_number(L, 1);
                pid = lua_tointeger(L, 1);
        }

        int value;
        if (!OBD2_request_PID(pid, &value, timeout))
                return 0;

        lua_pushnumber(L, value);
        return 1;
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

        lua_pushinteger(L, chan_id);
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
        VirtualChannel *vc;
        if (lua_isnumber(ls, 1)) {
                vc = get_virtual_channel(lua_tointeger(ls, 1));
        } else {
                const int idx = find_virtual_channel(lua_tostring(ls, 1));
                vc = get_virtual_channel(idx);
        }

        if (!vc)
                return luaL_error(ls, "Virtual channel not found!");

        lua_pushnumber(ls, vc->currentValue);
        return 1;
}

static int lua_set_virt_channel_value(lua_State *L)
{
        lua_validate_args_count(L, 2, 2);

        const int id = lua_tointeger(L, 1);
        const float value = lua_tonumber(L, 2);
        set_virtual_channel_value(id, value);

        return 0;
}

void registerLuaLoggerBindings(lua_State *L)
{
        lua_registerlight(L,"getGpio", lua_get_gpio);
        lua_registerlight(L,"setGpio", lua_set_gpio);
        lua_registerlight(L,"getButton", lua_get_button);

        lua_registerlight(L, "setPwmDutyCycle", lua_set_pwm_duty_cycle);
        lua_registerlight(L, "setPwmPeriod", lua_set_pwm_period);
        lua_registerlight(L, "setAnalogOut", lua_set_analog_out);
        lua_registerlight(L, "setPwmClockFreq", lua_set_pwm_clk_freq);
        lua_registerlight(L, "getPwmClockFreq", lua_get_pwm_clk_freq);

        lua_registerlight(L, "getTimerRpm", lua_get_rpm);
        lua_registerlight(L, "getTimerPeriodMs", lua_get_period_ms);
        lua_registerlight(L, "getTimerFreq", lua_get_freq);
        lua_registerlight(L, "getTimerRaw", lua_get_timer_raw);
        lua_registerlight(L, "resetTimerCount", lua_reset_timer_count);
        lua_registerlight(L, "getTimerCount", lua_get_timer_count);

        lua_registerlight(L, "getAnalog", lua_get_analog);

        lua_registerlight(L, "getImu", lua_imu_read);
        lua_registerlight(L, "getImuRaw", lua_imu_read_raw);

        lua_registerlight(L, "getGpsSats", lua_get_gps_sat_count);
        lua_registerlight(L, "getGpsPos", lua_get_gps_position);
        lua_registerlight(L, "getGpsSpeed", lua_get_gps_speed);
        lua_registerlight(L, "getGpsQuality", lua_get_gps_quality);
        lua_registerlight(L, "getGpsDist", lua_get_gps_distance);
        lua_registerlight(L, "getGpsAltitude", lua_get_gps_altitude);

        lua_registerlight(L, "getLapCount", lua_get_lap_count);
        lua_registerlight(L, "getLapTime", lua_get_lap_time);
        lua_registerlight(L, "getGpsSec", lua_get_seconds_since_first_fix);
        lua_registerlight(L, "getAtStartFinish", lua_get_gps_at_start_finish);

        lua_registerlight(L, "getTickCount", lua_get_tick_count);
        lua_registerlight(L, "getTicksPerSecond", lua_get_ticks_per_second);

        lua_registerlight(L, "initCAN", lua_init_can);
        lua_registerlight(L, "txCAN", lua_send_can_msg);
        lua_registerlight(L, "rxCAN", lua_rx_can_msg);
        lua_registerlight(L, "setCANfilter", lua_set_can_filter);
        lua_registerlight(L, "readOBD2", lua_obd2_read);

        lua_registerlight(L, "startLogging", lua_logging_start);
        lua_registerlight(L, "stopLogging", lua_logging_stop);
        lua_registerlight(L, "isLogging" , lua_logging_is_active);

        lua_registerlight(L, "setLed", lua_set_led);

        lua_registerlight(L, "initSer", lua_init_serial);
        lua_registerlight(L, "readCSer", lua_serial_read_char);
        lua_registerlight(L, "readSer", lua_serial_read_line);
        lua_registerlight(L, "writeCSer", lua_serial_write_char);
        lua_registerlight(L, "writeSer", lua_serial_write_line);

        lua_registerlight(L, "flashLoggerCfg", lua_flash_config);

        lua_registerlight(L, "calibrateImuZero", lua_calibrate_imu_zero);

        lua_registerlight(L, "setBgStream", lua_set_bg_streaming);
        lua_registerlight(L, "getBgStream", lua_get_bg_streaming);

        lua_registerlight(L, "addChannel", lua_add_virt_channel);
        lua_registerlight(L, "getChannel", lua_get_virtual_channel);
        lua_registerlight(L, "setChannel", lua_set_virt_channel_value);

        lua_registerlight(L, "getUptime", lua_get_uptime);
        lua_registerlight(L, "getDateTime", lua_get_date_time);
}
