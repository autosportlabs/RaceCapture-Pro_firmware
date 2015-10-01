#include "usb_comm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mod_string.h"
#include "USB-CDC_device.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "memory.h"
#include "serial.h"
#include "messaging.h"

#define BUFFER_SIZE 1025

static char lineBuffer[BUFFER_SIZE];

#define mainUSB_COMM_STACK	( 1000 )

static int usb_comm_init(const int priority)
{
    return USB_CDC_device_init(priority);
}

void usb_init_serial(Serial *serial)
{
    serial->init = &usb_init;
    serial->flush = &usb_flush;
    serial->get_c = &usb_getchar;
    serial->get_c_wait = &usb_getcharWait;
    serial->get_line = &usb_readLine;
    serial->get_line_wait = &usb_readLineWait;
    serial->put_c = &usb_putchar;
    serial->put_s = &usb_puts;
}

void startUSBCommTask(int priority)
{
        usb_comm_init(priority);

        xTaskCreate(onUSBCommTask,( signed portCHAR * ) "OnUSBComm",
                    mainUSB_COMM_STACK, NULL, priority, NULL );
}

void onUSBCommTask(void *pvParameters)
{
        while (1) {
                if (USB_CDC_is_initialized())
                        process_msg(get_serial(SERIAL_USB), lineBuffer, BUFFER_SIZE);
                taskYIELD();
        }
}

void usb_init(unsigned int bits, unsigned int parity,
              unsigned int stopBits, unsigned int baud)
{
    //null function - does not apply to USB CDC
}

void usb_flush(void)
{
    char c;
    while(usb_getcharWait(&c, 0));
}

int usb_getcharWait(char *c, size_t delay)
{
    return USB_CDC_ReceiveByteDelay(c, delay);
}

char usb_getchar(void)
{
    char c;
    usb_getcharWait(&c, portMAX_DELAY);
    return c;
}

int usb_readLine(char *s, int len)
{
    return usb_readLineWait(s,len,portMAX_DELAY);
}

int usb_readLineWait(char *s, int len, size_t delay)
{
    int count = 0;
    while(count < len - 1) {
        char c = 0;
        if (!usb_getcharWait(&c, delay)) break;
        *s++ = c;
        count++;
        if (c == '\n') break;
    }
    *s = '\0';
    return count;
}

void usb_puts(const char *s)
{
    while ( *s ) {
        USB_CDC_SendByte(*s++ );
    }
}

void usb_putchar(char c)
{
    USB_CDC_SendByte(c);
}
