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

#include "array_utils.h"
#include "capabilities.h"
#include "loggerConfig.h"
#include "modp_numtoa.h"
#include "printk.h"
#include "serial.h"
#include "usart.h"
#include "usb_comm.h"

static Serial serial_ports[SERIAL_COUNT];

bool _init_serial(serial_id_t serial_id, uart_id_t uart_id, const char *name)
{
        Serial *s = &serial_ports[serial_id];

        s->serial_id = serial_id;
        serial_set_name(s, name);

        return usart_init_serial(s, uart_id) != 0;
}

void init_serial(void)
{
        const bool init =
                _init_serial(SERIAL_GPS, UART_GPS, "GPS") &&
                _init_serial(SERIAL_TELEMETRY, UART_TELEMETRY, "Cellular") &&
                _init_serial(SERIAL_WIRELESS, UART_WIRELESS, "BlueTooth") &&
                _init_serial(SERIAL_AUX, UART_AUX, "Aux");

        if (!init)
                pr_error("[serial] Failed to initialize one or more ports\r\n");

#if defined(USB_SERIAL_SUPPORT)
        usb_init_serial(&serial_ports[SERIAL_USB]);
#endif /* USB_SERIAL_SUPPORT */
}

void configure_serial(serial_id_t port, uint8_t bits, uint8_t parity, uint8_t stopBits, uint32_t baud)
{
        uart_id_t uartPort;

        switch(port) {
        case SERIAL_GPS:
                uartPort = UART_GPS;
                break;
        case SERIAL_TELEMETRY:
                uartPort = UART_TELEMETRY;
                break;
        case SERIAL_WIRELESS:
                uartPort = UART_WIRELESS;
                break;
        case SERIAL_AUX:
                uartPort = UART_AUX;
                break;
        default:
                return;
        }

        /* If here, we have a real USART port to configure */
        usart_config(uartPort, bits, parity, stopBits, baud);
}

Serial* get_serial(serial_id_t port)
{
        return port < SERIAL_COUNT ? &serial_ports[port] : NULL;
}

static void _serial_log(const Serial *s, const char *action, bool *pfx,
                        const char *data)
{
        LoggerConfig *lc = getWorkingLoggerConfig();
        const bool enabled = lc->logging_cfg.serial[s->serial_id];

        if (!enabled || NULL == data)
                return;

        for(; *data; ++data) {
                if (*pfx) {
                        pr_info("[serial]");
                        if (s->sl.name) {
                                pr_info("[");
                                pr_info(s->sl.name);
                                pr_info("]");
                        }
                        pr_info(action);
                        *pfx = false;
                }

                switch(*data) {
                case('\r'):
                        pr_info("\\r");
                        break;
                case('\n'):
                        pr_info("\\n\r\n");
                        *pfx = true;
                        break;
                default:
                        pr_info_char(*data);
                        break;
                }
        }
}

static void do_rx_callback(Serial *s, const char *data)
{
        _serial_log(s, " RX: ", &s->sl.rx_pfx, data);
}

static void do_tx_callback(Serial *s, const char *data)
{
        _serial_log(s, " TX: ", &s->sl.tx_pfx, data);
}

static void do_rx_callback_char(Serial *s, const char data)
{
        const char str[] = { data, '\0' };
        do_rx_callback(s, str);
}

static void do_tx_callback_char(Serial *s, const char data)
{
        const char str[] = { data, '\0' };
        do_tx_callback(s, str);
}

bool serial_logging(Serial *s, const bool enable)
{
        LoggerConfig *lc = getWorkingLoggerConfig();

        const bool prev = lc->logging_cfg.serial[s->serial_id];
        lc->logging_cfg.serial[s->serial_id] = enable;

        return prev;
}

void serial_set_name(Serial *s, const char *name)
{
        s->sl.name = name;
}

void serial_flush(Serial *s)
{
        s->flush();
}

void serial_init(Serial *s, unsigned int bits, unsigned int parity,
                 unsigned int stopBits, unsigned int baud)
{
        s->init(bits, parity, stopBits, baud);
}

char serial_get_c(Serial *s)
{
        const char c = s->get_c();
        do_rx_callback_char(s, c);
        return c;
}

int serial_get_c_wait(Serial *s, char *c, const size_t delay)
{
        const int rv = s->get_c_wait(c, delay);
        do_rx_callback_char(s, *c);
        return rv;
}

int serial_get_line(Serial *s, char *l, const int len)
{
        const int rv = s->get_line(l, len);
        do_rx_callback(s, l);
        return rv;
}

int serial_get_line_wait(Serial *s, char *l, const int len,
                         const size_t delay)
{
        const int rv = s->get_line_wait(l, len, delay);
        do_rx_callback(s, l);
        return rv;
}

void serial_put_c(Serial *s, const char c)
{
        do_tx_callback_char(s, c);
        s->put_c(c);
}

void serial_put_s(Serial *s, const char *l)
{
        do_tx_callback(s, l);
        s->put_s(l);
}

size_t serial_read_byte(Serial *serial, uint8_t *b, size_t delay)
{
        return serial_get_c_wait(serial, (char*) b, delay);
}

void put_int(Serial *serial, int n)
{
        char buf[12];
        modp_itoa10(n, buf);
        serial_put_s(serial, buf);
}

void put_ll(Serial *serial, long long l)
{
        char buf[22];
        modp_ltoa10(l, buf);
        serial_put_s(serial, buf);
}

void put_float(Serial *serial, float f,int precision)
{
        char buf[20];
        modp_ftoa(f, buf, precision);
        serial_put_s(serial, buf);
}

void put_double(Serial *serial, double f, int precision)
{
        char buf[30];
        modp_dtoa(f,buf,precision);
        serial_put_s(serial, buf);
}

void put_hex(Serial *serial, int n)
{
        char buf[30];
        modp_itoaX(n,buf,16);
        serial_put_s(serial, buf);
}

void put_uint(Serial *serial, unsigned int n)
{
        char buf[20];
        modp_uitoa10(n,buf);
        serial_put_s(serial, buf);
}

void put_nameIndexUint(Serial *serial, const char *s, int i, unsigned int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        put_uint(serial, i);
        serial_put_s(serial, "=");
        put_uint(serial, n);
        serial_put_s(serial, ";");
}

void put_nameSuffixUint(Serial *serial, const char *s, const char *suf, unsigned int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        serial_put_s(serial, suf);
        serial_put_s(serial, "=");
        put_uint(serial, n);
        serial_put_s(serial, ";");
}

void put_nameUint(Serial *serial, const char *s, unsigned int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=");
        put_uint(serial, n);
        serial_put_s(serial, ";");
}

void put_nameIndexInt(Serial *serial, const char *s, int i, int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        put_uint(serial, i);
        serial_put_s(serial, "=");
        put_int(serial, n);
        serial_put_s(serial, ";");
}

void put_nameSuffixInt(Serial *serial, const char *s, const char *suf, int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        serial_put_s(serial, suf);
        serial_put_s(serial, "=");
        put_int(serial, n);
        serial_put_s(serial, ";");
}

void put_nameInt(Serial *serial, const char *s, int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=");
        put_int(serial, n);
        serial_put_s(serial, ";");
}

void put_nameIndexDouble(Serial *serial, const char *s, int i, double n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        put_uint(serial, i);
        serial_put_s(serial, "=");
        put_double(serial, n,precision);
        serial_put_s(serial, ";");
}

void put_nameSuffixDouble(Serial *serial, const char *s, const char *suf, double n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        serial_put_s(serial, suf);
        serial_put_s(serial, "=");
        put_double(serial, n,precision);
        serial_put_s(serial, ";");
}

void put_nameDouble(Serial *serial, const char *s, double n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=");
        put_double(serial, n, precision);
        serial_put_s(serial, ";");
}

void put_nameIndexFloat(Serial *serial, const char *s, int i, float n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        put_uint(serial, i);
        serial_put_s(serial, "=");
        put_float(serial, n, precision);
        serial_put_s(serial, ";");
}

void put_nameSuffixFloat(Serial *serial, const char *s, const char *suf, float n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        serial_put_s(serial, suf);
        serial_put_s(serial, "=");
        put_float(serial, n, precision);
        serial_put_s(serial, ";");
}

void put_nameFloat(Serial *serial, const char *s, float n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=");
        put_float(serial, n, precision);
        serial_put_s(serial, ";");
}

void put_nameString(Serial *serial, const char *s, const char *v)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=\"");
        serial_put_s(serial, v);
        serial_put_s(serial, "\";");
}

void put_nameSuffixString(Serial *serial, const char *s, const char *suf, const char *v)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        serial_put_s(serial, suf);
        serial_put_s(serial, "=\"");
        serial_put_s(serial, v);
        serial_put_s(serial, "\";");
}

void put_nameIndexString(Serial *serial, const char *s, int i, const char *v)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        put_uint(serial, i);
        serial_put_s(serial, "=\"");
        serial_put_s(serial, v);
        serial_put_s(serial, "\";");
}

void put_escapedString(Serial * serial, const char *v, int length)
{
        const char *value = v;
        while (value - v < length) {
                switch(*value) {
                case '\n':
                        serial_put_s(serial, "\\n");
                        break;
                case '\r':
                        serial_put_s(serial, "\\r");
                        break;
                case '"':
                        serial_put_s(serial, "\\\"");
                        break;
                default:
                        serial_put_c(serial, *value);
                        break;
                }
                value++;
        }
}

void put_nameEscapedString(Serial *serial, const char *s, const char *v, int length)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=\"");
        const char *value = v;
        while (value - v < length) {
                switch(*value) {
                case ' ':
                        serial_put_s(serial, "\\_");
                        break;
                case '\n':
                        serial_put_s(serial, "\\n");
                        break;
                case '\r':
                        serial_put_s(serial, "\\r");
                        break;
                case '"':
                        serial_put_s(serial, "\\\"");
                        break;
                default:
                        serial_put_c(serial, *value);
                        break;
                }
                value++;
        }
        serial_put_s(serial, "\";");
}


void put_bytes(Serial *serial, char *data, unsigned int length)
{
        while (length > 0) {
                serial_put_c(serial, *data);
                data++;
                length--;
        }
}

void put_crlf(Serial *serial)
{
        serial_put_s(serial, "\r\n");
}

void read_line(Serial *serial, char *buffer, size_t bufferSize)
{
        size_t bufIndex = 0;
        char c;
        while(bufIndex < bufferSize - 1) {
                c = serial_get_c(serial);
                if (c) {
                        if ('\r' == c) {
                                break;
                        } else {
                                buffer[bufIndex++] = c;
                        }
                }
        }
        buffer[bufIndex]='\0';
}

void interactive_read_line(Serial *serial, char * buffer, size_t bufferSize)
{
        size_t bufIndex = 0;
        char c;
        while(bufIndex < bufferSize - 1) {
                c = serial_get_c(serial);
                if (c) {
                        if ('\r' == c) {
                                break;
                        } else if ('\b' == c) {
                                if (bufIndex > 0) {
                                        bufIndex--;
                                        serial_put_c(serial, c);
                                }
                        } else {
                                serial_put_c(serial, c);
                                buffer[bufIndex++] = c;
                        }
                }
        }
        serial_put_s(serial, "\r\n");
        buffer[bufIndex]='\0';
}
