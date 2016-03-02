#include "USB-CDC_device.h"
#include <string.h>

#include <usbd_cdc_core.h>
#include <usbd_usr.h>
#include <usbd_conf.h>
#include <usbd_desc.h>
#include <usbd_cdc_vcp.h>

static volatile int _init_flag = 0;
USB_OTG_CORE_HANDLE USB_OTG_dev __attribute__ ((aligned (4)));

int USB_CDC_device_init(const int priority)
{
    vcp_setup();

    /* Initialize the USB hardware */
    USBD_Init(&USB_OTG_dev,
              USB_OTG_FS_CORE_ID,
              &USR_desc,
              &USBD_CDC_cb,
              &USR_cb);
    _init_flag = 1;

    return 0;
}

void USB_CDC_send_debug(portCHAR *string)
{
    int len = strnlen(string, 2048);
    vcp_tx((uint8_t*)string, len);
}

void USB_CDC_SendByte( portCHAR cByte )
{
    vcp_tx((uint8_t*)&cByte, 1);
}

portBASE_TYPE USB_CDC_ReceiveByte(portCHAR *data)
{
    return vcp_rx((uint8_t*)data, 1, 0);
}

portBASE_TYPE USB_CDC_ReceiveByteDelay(portCHAR *data, portTickType delay )
{
    return vcp_rx((uint8_t*)data, 1, delay);
}

int USB_CDC_is_initialized()
{
    return _init_flag;
}
