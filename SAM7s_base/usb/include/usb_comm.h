#ifndef USB_COMM_H_
#define USB_COMM_H_
#include <stddef.h>

void startUSBCommTask(int priority);

void onUSBCommTask(void *);

void usb_init(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void usb_flush(void);

int usb_readLine(char *s, int len);

int usb_readLineWait(char *s, int len, size_t delay);

char usb_getchar(void);

char usb_getchar_wait(size_t delay);

void usb_putchar(char c);

void usb_puts(const char* s );

#endif /*USB_COMM_H_*/
