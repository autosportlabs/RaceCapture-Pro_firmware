#include <CAN_device_stm32.h>
#include <usb_it.h>
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	CAN_device_isr();
#if defined(USB_SERIAL_SUPPORT)
	USB_device_isr();
#endif
}
