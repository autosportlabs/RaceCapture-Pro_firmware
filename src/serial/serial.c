#include "serial.h"
#include "usart.h"
#include "usb_comm.h"
#include "modp_numtoa.h"

static Serial serial_ports[SERIAL_COUNT];

void init_serial(void){
	usart_init_serial(&serial_ports[SERIAL_GPS], UART_GPS);
	usart_init_serial(&serial_ports[SERIAL_TELEMETRY], UART_TELEMETRY);
	usart_init_serial(&serial_ports[SERIAL_WIRELESS], UART_WIRELESS);
	usart_init_serial(&serial_ports[SERIAL_AUX], UART_AUX);
	usb_init_serial(&serial_ports[SERIAL_USB]);
}

void configure_serial(serial_id_t port, uint8_t bits, uint8_t parity, uint8_t stopBits, uint32_t baud){
	size_t configurablePort = 1;
	uart_id_t uartPort;

	switch(port){
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
		configurablePort = 0;
		break;
	}

	if (configurablePort){
		usart_config(uartPort, bits, parity, stopBits, baud);
	}

}

Serial * get_serial(serial_id_t port){
	return port < SERIAL_COUNT ? &serial_ports[port] : NULL;
}

size_t serial_read_byte(Serial *serial, uint8_t *b, size_t delay){
	return serial->get_c_wait((char *)b, delay);
}

void put_int(Serial *serial, int n){
	char buf[12];
	modp_itoa10(n,buf);
	serial->put_s(buf);
}

void put_ll(Serial *serial, long long l) {
   char buf[21];
   modp_ltoa10(l, buf);
   serial->put_s(buf);
}

void put_float(Serial *serial, float f,int precision){
	char buf[20];
	modp_ftoa(f,buf,precision);
	serial->put_s(buf);
}

void put_double(Serial *serial, double f, int precision){
	char buf[30];
	modp_dtoa(f,buf,precision);
	serial->put_s(buf);
}

void put_hex(Serial *serial, int n){
	char buf[30];
	modp_itoaX(n,buf,16);
	serial->put_s(buf);
}

void put_uint(Serial *serial, unsigned int n){
	char buf[20];
	modp_uitoa10(n,buf);
	serial->put_s(buf);
}

void put_nameIndexUint(Serial *serial, const char *s, int i, unsigned int n){
	serial->put_s(s);
	serial->put_s("_");
	put_uint(serial, i);
	serial->put_s("=");
	put_uint(serial, n);
	serial->put_s(";");
}

void put_nameSuffixUint(Serial *serial, const char *s, const char *suf, unsigned int n){
	serial->put_s(s);
	serial->put_s("_");
	serial->put_s(suf);
	serial->put_s("=");
	put_uint(serial, n);
	serial->put_s(";");
}

void put_nameUint(Serial *serial, const char *s, unsigned int n){
	serial->put_s(s);
	serial->put_s("=");
	put_uint(serial, n);
	serial->put_s(";");
}

void put_nameIndexInt(Serial *serial, const char *s, int i, int n){
	serial->put_s(s);
	serial->put_s("_");
	put_uint(serial, i);
	serial->put_s("=");
	put_int(serial, n);
	serial->put_s(";");
}

void put_nameSuffixInt(Serial *serial, const char *s, const char *suf, int n){
	serial->put_s(s);
	serial->put_s("_");
	serial->put_s(suf);
	serial->put_s("=");
	put_int(serial, n);
	serial->put_s(";");
}

void put_nameInt(Serial *serial, const char *s, int n){
	serial->put_s(s);
	serial->put_s("=");
	put_int(serial, n);
	serial->put_s(";");
}

void put_nameIndexDouble(Serial *serial, const char *s, int i, double n, int precision){
	serial->put_s(s);
	serial->put_s("_");
	put_uint(serial, i);
	serial->put_s("=");
	put_double(serial, n,precision);
	serial->put_s(";");
}

void put_nameSuffixDouble(Serial *serial, const char *s, const char *suf, double n, int precision){
	serial->put_s(s);
	serial->put_s("_");
	serial->put_s(suf);
	serial->put_s("=");
	put_double(serial, n,precision);
	serial->put_s(";");
}

void put_nameDouble(Serial *serial, const char *s, double n, int precision){
	serial->put_s(s);
	serial->put_s("=");
	put_double(serial, n, precision);
	serial->put_s(";");
}

void put_nameIndexFloat(Serial *serial, const char *s, int i, float n, int precision){
	serial->put_s(s);
	serial->put_s("_");
	put_uint(serial, i);
	serial->put_s("=");
	put_float(serial, n, precision);
	serial->put_s(";");
}

void put_nameSuffixFloat(Serial *serial, const char *s, const char *suf, float n, int precision){
	serial->put_s(s);
	serial->put_s("_");
	serial->put_s(suf);
	serial->put_s("=");
	put_float(serial, n, precision);
	serial->put_s(";");
}

void put_nameFloat(Serial *serial, const char *s, float n, int precision){
	serial->put_s(s);
	serial->put_s("=");
	put_float(serial, n, precision);
	serial->put_s(";");
}

void put_nameString(Serial *serial, const char *s, const char *v){
	serial->put_s(s);
	serial->put_s("=\"");
	serial->put_s(v);
	serial->put_s("\";");
}

void put_nameSuffixString(Serial *serial, const char *s, const char *suf, const char *v){
	serial->put_s(s);
	serial->put_s("_");
	serial->put_s(suf);
	serial->put_s("=\"");
	serial->put_s(v);
	serial->put_s("\";");
}

void put_nameIndexString(Serial *serial, const char *s, int i, const char *v){
	serial->put_s(s);
	serial->put_s("_");
	put_uint(serial, i);
	serial->put_s("=\"");
	serial->put_s(v);
	serial->put_s("\";");
}

void put_escapedString(Serial * serial, const char *v, int length){
	const char *value = v;
	while (value - v < length){
		switch(*value){
			case '\n':
				serial->put_s("\\n");
				break;
			case '\r':
				serial->put_s("\\r");
				break;
			case '"':
				serial->put_s("\\\"");
				break;
			default:
				serial->put_c(*value);
				break;
		}
		value++;
	}
}

void put_nameEscapedString(Serial *serial, const char *s, const char *v, int length){
	serial->put_s(s);
	serial->put_s("=\"");
	const char *value = v;
	while (value - v < length){
		switch(*value){
			case ' ':
				serial->put_s("\\_");
				break;
			case '\n':
				serial->put_s("\\n");
				break;
			case '\r':
				serial->put_s("\\r");
				break;
			case '"':
				serial->put_s("\\\"");
				break;
			default:
				serial->put_c(*value);
				break;
		}
		value++;
	}
	serial->put_s("\";");
}


void put_bytes(Serial *serial, char *data, unsigned int length){
	while (length > 0){
    	serial->put_c(*data);
    	data++;
    	length--;
	}
}

void put_crlf(Serial *serial){
	serial->put_s("\r\n");
}

void read_line(Serial *serial, char *buffer, size_t bufferSize){
	size_t bufIndex = 0;
    char c;
	while(bufIndex < bufferSize - 1){
		c = serial->get_c();
		if (c) {
			if ('\r' == c){
				break;
			}
			else {
				buffer[bufIndex++] = c;
			}
		}
	}
	buffer[bufIndex]='\0';
}

void interactive_read_line(Serial *serial, char * buffer, size_t bufferSize){
	size_t bufIndex = 0;
    char c;
	while(bufIndex < bufferSize - 1){
		c = serial->get_c();
		if (c) {
			if ('\r' == c){
				break;
			}
			else if ('\b' == c){
				if (bufIndex > 0){
					bufIndex--;
					serial->put_c(c);
				}
			}
			else {
				serial->put_c(c);
				buffer[bufIndex++] = c;
			}
		}
	}
	serial->put_s("\r\n");
	buffer[bufIndex]='\0';
}
