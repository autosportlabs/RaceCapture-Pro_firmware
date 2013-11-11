#include "serial.h"
#include "usart.h"
#include "usb_comm.h"
#include "modp_numtoa.h"

static Serial Usart0;
static Serial Usart1;
static Serial Usb;

void init_serial(void){
	Usart0.init = &initUsart0;
	Usart0.flush = &usart0_flush;
	Usart0.get_c = &usart0_getchar;
	Usart0.get_c_wait = &usart0_getcharWait;
	Usart0.get_line = &usart0_readLine;
	Usart0.get_line_wait = &usart0_readLineWait;
	Usart0.put_c = &usart0_putchar;
	Usart0.put_s = &usart0_puts;

	Usart1.init = &initUsart1;
	Usart1.flush = &usart1_flush;
	Usart1.get_c = &usart1_getchar;
	Usart1.get_c_wait = &usart1_getcharWait;
	Usart1.get_line = &usart1_readLine;
	Usart1.get_line_wait = &usart1_readLineWait;
	Usart1.put_c = &usart1_putchar;
	Usart1.put_s = &usart1_puts;

	Usb.init = &usb_init;
	Usb.flush = &usb_flush;
	Usb.get_c = &usb_getchar;
	Usb.get_c_wait = &usb_getchar_wait;
	Usb.get_line = &usb_readLine;
	Usb.get_line_wait = &usb_readLineWait;
	Usb.put_c = &usb_putchar;
	Usb.put_s = &usb_puts;
}

Serial * get_serial_usart0(){
	return &Usart0;
}

Serial * get_serial_usart1(){
	return &Usart1;
}

Serial * get_serial_usb(){
	return &Usb;
}

void put_int(Serial *serial, int n){
	char buf[12];
	modp_itoa10(n,buf);
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

void interactive_read_line(Serial *serial, char * buffer, size_t bufferSize){
	int bufIndex = 0;
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
	serial->put_s("\n\r");
	buffer[bufIndex]='\0';
}
