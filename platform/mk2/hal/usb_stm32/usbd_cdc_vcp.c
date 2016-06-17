/**
  ******************************************************************************
  * @file    usbd_cdc_vcp.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   Generic media access Layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#pragma     data_alignment = 4
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "USB-CDC_device.h"
#include "usb_conf.h"
#include "usbd_cdc_vcp.h"
#include <portmacro.h>
#include <queue.h>
#include <semphr.h>
#include <stdbool.h>
#include <stm32f4xx_exti.h>
#include <task.h>
#include <timers.h>
#include <usb_core.h>
#include <usb_dcd.h>

/* TODO: Ditch this weird circular buffer and instead make transmit
 * driven from an interrupt */
/* These are external variables imported from CDC core to be used for IN
   transfer management. */
extern uint8_t  APP_Rx_Buffer []; /* Write CDC received data in this buffer.
                                     These data will be sent over USB IN endpoint
                                     in the CDC core functions. */
extern uint32_t APP_Rx_ptr_in;    /* Increment this pointer or roll it back to
                                     start address when writing received data
                                     in the buffer APP_Rx_Buffer. */
extern uint32_t APP_Rx_ptr_out;

extern USB_OTG_CORE_HANDLE           USB_OTG_dev;
extern uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);

/* Private function prototypes -----------------------------------------------*/
static uint16_t VCP_Init     (void);
static uint16_t VCP_DeInit   (void);
static uint16_t VCP_Ctrl     (uint32_t Cmd, uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataTx   (uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataRx   (uint8_t* Buf, uint32_t Len);

/* Locks */
static volatile bool vcp_configured = false;
/* Locks */
static xSemaphoreHandle _lock;

static xQueueHandle rx_queue;
static usb_device_data_rx_isr_cb_t* rx_isr_cb;

static volatile bool connected = false;
CDC_IF_Prop_TypeDef VCP_fops = {
    VCP_Init,
    VCP_DeInit,
    VCP_Ctrl,
    VCP_DataTx,
    VCP_DataRx
};

/* Public Functions */
void vcp_tx(uint8_t *buf, uint32_t len)
{
    /* If we aren't connected, just drop the data on the floor */
    if (!connected)
        return;

    xSemaphoreTake(_lock, portMAX_DELAY);
    VCP_DataTx(buf, len);
    xSemaphoreGive(_lock);
}

void vcp_setup(struct Serial* s, usb_device_data_rx_isr_cb_t* cb)
{
    vSemaphoreCreateBinary(_lock);
    xSemaphoreTake(_lock, portMAX_DELAY);
    rx_queue = serial_get_rx_queue(s);
    rx_isr_cb = cb;
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  VCP_Init
  *         Initializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Init(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(_lock, &xHigherPriorityTaskWoken);

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    connected = true;
    return USBD_OK;
}

/**
  * @brief  VCP_DeInit
  *         DeInitializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_DeInit(void)
{
    xSemaphoreTake(_lock, portMAX_DELAY);

    connected = false;
    return USBD_OK;
}


/**
  * @brief  VCP_Ctrl
  *         Manage the CDC class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Ctrl (uint32_t Cmd, uint8_t* Buf, uint32_t Len)
{
    /* This is a NOP since we aren't tying it in with a physical
     * serial port. Just return OK */

    return USBD_OK;
}

static bool check_tx_overrun(void)
{
    if (APP_Rx_ptr_in == APP_Rx_ptr_out - 1)
        return true;

    if ((APP_Rx_ptr_in == APP_RX_DATA_SIZE) && (APP_Rx_ptr_out == 0))
        return true;

    return false;
}

/**
 * @return true if the device is in suspended mode, false otherwise.
 */
static bool is_usb_suspended(void)
{
        /* Checks to see if the USB bus is suspended */
        return (bool) (USB_OTG_dev.regs.DREGS->DSTS & 0x1);
}

/**
 * De-inits our USB device if the device is suspended.  This prevents an
 * IRQ storm according to jeff_c, whom I trust on the matter.
 */
static void deinit_if_needed(void)
{
        if (connected && is_usb_suspended())
                VCP_DeInit();
}

/**
 * Re-inits our USB device if the device is returning from suspended state.
 * Normally it would be nice to get an interrupt to know this, but I have
 * been unable to locate a suitable one.  So receiving a character should
 * be sufficient to know we are potentially coming out of suspend.
 */
static void reinit_if_needed(void)
{
        if (!connected && !is_usb_suspended())
                VCP_Init();
}

/**
  * @brief  VCP_DataTx
  *         CDC received data to be send over USB IN endpoint are managed in
  *         this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
static uint16_t VCP_DataTx (uint8_t* Buf, uint32_t Len)
{
    bool overrun;

    /* If USB Is disconnected, drop the data on the floor */
    if (is_usb_suspended())
        return USBD_FAIL;

    while (Len--) {
        overrun = check_tx_overrun();

        while(overrun) {
            vTaskDelay(1);
            overrun = check_tx_overrun();
        }

        APP_Rx_Buffer[APP_Rx_ptr_in++] = *Buf++;

        /* Avoid running off the end of the buffer */
        if(APP_Rx_ptr_in >= APP_RX_DATA_SIZE) {
            APP_Rx_ptr_in = 0;
        }
    }

    return USBD_OK;
}

/**
  * @brief  VCP_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
static uint16_t VCP_DataRx (uint8_t* Buf, uint32_t Len)
{
        portBASE_TYPE hptw = false;

        reinit_if_needed();
        while(Len--)
                xQueueSendFromISR(rx_queue, Buf++, &hptw);

        if (rx_isr_cb)
                hptw |= rx_isr_cb();

        portEND_SWITCHING_ISR(hptw);
        return USBD_OK;
}

#ifdef USE_USB_OTG_FS
void OTG_FS_WKUP_IRQHandler(void)
{
    if(USB_OTG_dev.cfg.low_power) {
        *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
        SystemInit();
        USB_OTG_UngateClock(&USB_OTG_dev);
    }
    EXTI_ClearITPendingBit(EXTI_Line18);
}
#endif

/**
  * @brief  This function handles EXTI15_10_IRQ Handler.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_HS
void OTG_HS_WKUP_IRQHandler(void)
{
    if(USB_OTG_dev.cfg.low_power) {
        *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
        SystemInit();
        USB_OTG_UngateClock(&USB_OTG_dev);
    }
    EXTI_ClearITPendingBit(EXTI_Line20);
}
#endif

/**
  * @brief  This function handles OTG_HS Handler.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_HS
void OTG_HS_IRQHandler(void)
#else
void OTG_FS_IRQHandler(void)
#endif
{
    USBD_OTG_ISR_Handler (&USB_OTG_dev);
    deinit_if_needed();
}

#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
/**
  * @brief  This function handles EP1_IN Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_IN_IRQHandler(void)
{
    USBD_OTG_EP1IN_ISR_Handler (&USB_OTG_dev);
}

/**
  * @brief  This function handles EP1_OUT Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_OUT_IRQHandler(void)
{
    USBD_OTG_EP1OUT_ISR_Handler (&USB_OTG_dev);
}
#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
