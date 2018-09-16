/**
 ******************************************************************************
 * @file    stm324x9i_eval_sdio_sd.c
 * @author  MCD Application Team
 * @version V1.0.3
 * @date    13-November-2013
 * @brief   This file provides a set of functions needed to manage the SDIO SD
 *          Card memory mounted on STM324x9I-EVAL evaluation board.
 *
 *
 *  @verbatim
 *
 *          ===================================================================
 *                                   How to use this driver
 *          ===================================================================
 *          It implements a high level communication layer for read and write
 *          from/to this memory. The needed STM32 hardware resources (SDIO and
 *          GPIO) are defined in stm324x9i_eval.h file, and the initialization is
 *          performed in SD_LowLevel_Init() function declared in stm324x9i_eval.c
 *          file.
 *          You can easily tailor this driver to any other development board,
 *          by just adapting the defines for hardware resources and
 *          SD_LowLevel_Init() function.
 *
 *          A - SD Card Initialization and configuration
 *          ============================================
 *            - To initialize the SD Card, use the SD_Init() function.  It
 *              Initializes the SD Card and put it into StandBy State (Ready
 *              for data transfer). This function provides the following operations:
 *
 *              1 - Apply the SD Card initialization process at 400KHz and check
 *                  the SD Card type (Standard Capacity or High Capacity). You
 *                  can change or adapt this frequency by adjusting the
 *                  "SDIO_INIT_CLK_DIV" define inside the stm324x9i_eval.h file.
 *                  The SD Card frequency (SDIO_CK) is computed as follows:
 *
 *                     +---------------------------------------------+
 *                     | SDIO_CK = SDIOCLK / (SDIO_INIT_CLK_DIV + 2) |
 *                     +---------------------------------------------+
 *
 *                  In initialization mode and according to the SD Card standard,
 *                  make sure that the SDIO_CK frequency doesn't exceed 400KHz.
 *
 *              2 - Get the SD CID and CSD data. All these information are
 *                  managed by the SDCardInfo structure. This structure provides
 *                  also ready computed SD Card capacity and Block size.
 *
 *              3 - Configure the SD Card Data transfer frequency. By Default,
 *                  the card transfer frequency is set to 24MHz. You can change
 *                  or adapt this frequency by adjusting the "SDIO_TRANSFER_CLK_DIV"
 *                  define inside the stm324x9i_eval.h file.
 *                  The SD Card frequency (SDIO_CK) is computed as follows:
 *
 *                     +---------------------------------------------+
 *                     | SDIO_CK = SDIOCLK / (SDIO_INIT_CLK_DIV + 2) |
 *                     +---------------------------------------------+
 *
 *                  In transfer mode and according to the SD Card standard,
 *                  make sure that the SDIO_CK frequency doesn't exceed 25MHz
 *                  and 50MHz in High-speed mode switch.
 *                  To be able to use a frequency higher than 24MHz, you should
 *                  use the SDIO peripheral in bypass mode. Refer to the
 *                  corresponding reference manual for more details.
 *
 *              4 -  Select the corresponding SD Card according to the address
 *                   read with the step 2.
 *
 *              5 -  Configure the SD Card in wide bus mode: 4-bits data.
 *
 *          B - SD Card Read operation
 *          ==========================
 *           - You can read SD card by using two functions : SD_ReadBlock() and
 *             SD_ReadMultiBlocks() functions. These functions support only
 *             512-byte block length.
 *           - The SD_ReadBlock() function reads only one block (512-byte). This
 *             function can transfer the data using DMA controller or using
 *             polling mode. To select between DMA or polling mode refer to
 *             "SD_DMA_MODE" or "SD_POLLING_MODE" inside the stm324x9i_eval_sdio_sd.h
 *             file and uncomment the corresponding line. By default the SD DMA
 *             mode is selected
 *           - The SD_ReadMultiBlocks() function reads only mutli blocks (multiple
 *             of 512-byte).
 *           - Any read operation should be followed by two functions to check
 *             if the DMA Controller and SD Card status.
 *              - SD_ReadWaitOperation(): this function insure that the DMA
 *                controller has finished all data transfer.
 *              - SD_GetStatus(): to check that the SD Card has finished the
 *                data transfer and it is ready for data.
 *
 *           - The DMA transfer is finished by the SDIO Data End interrupt.
 *             User has to call the SD_ProcessIRQ() function inside the SDIO_IRQHandler()
 *             and SD_ProcessDMAIRQ() function inside the DMA2_Streamx_IRQHandler().
 *             Don't forget to enable the SDIO_IRQn and DMA2_Stream3_IRQn or
 *             DMA2_Stream6_IRQn interrupts using the NVIC controller.
 *
 *          C - SD Card Write operation
 *          ===========================
 *           - You can write SD card by using two function: SD_WriteBlock() and
 *             SD_WriteMultiBlocks() functions. These functions support only
 *             512-byte block length.
 *           - The SD_WriteBlock() function write only one block (512-byte). This
 *             function can transfer the data using DMA controller or using
 *             polling mode. To select between DMA or polling mode refer to
 *             "SD_DMA_MODE" or "SD_POLLING_MODE" inside the stm324x9i_eval_sdio_sd.h
 *             file and uncomment the corresponding line. By default the SD DMA
 *             mode is selected
 *           - The SD_WriteMultiBlocks() function write only mutli blocks (multiple
 *             of 512-byte).
 *           - Any write operation should be followed by two functions to check
 *             if the DMA Controller and SD Card status.
 *              - SD_ReadWaitOperation(): this function insure that the DMA
 *                controller has finished all data transfer.
 *              - SD_GetStatus(): to check that the SD Card has finished the
 *                data transfer and it is ready for data.
 *
 *           - The DMA transfer is finished by the SDIO Data End interrupt.
 *             User has to call the SD_ProcessIRQ() function inside the SDIO_IRQHandler()
 *             and SD_ProcessDMAIRQ() function inside the DMA2_Streamx_IRQHandler().
 *             Don't forget to enable the SDIO_IRQn and DMA2_Stream3_IRQn or
 *             DMA2_Stream6_IRQn interrupts using the NVIC controller.
 *
 *
 *          D - SD card status
 *          ==================
 *           - At any time, you can check the SD Card status and get the SD card
 *             state by using the SD_GetStatus() function. This function checks
 *             first if the SD card is still connected and then get the internal
 *             SD Card transfer state.
 *           - You can also get the SD card SD Status register by using the
 *             SD_SendSDStatus() function.
 *
 *          E - Programming Model (Selecting DMA for SDIO data Transfer)
 *          ============================================================
 *             Status = SD_Init(); // Initialization Step as described in section A
 *
 *             // SDIO Interrupt ENABLE
 *             NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
 *             NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
 *             NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
 *             NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
 *             NVIC_Init(&NVIC_InitStructure);
 *             // DMA2 STREAMx Interrupt ENABLE
 *             NVIC_InitStructure.NVIC_IRQChannel = SD_SDIO_DMA_IRQn;
 *             NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
 *             NVIC_Init(&NVIC_InitStructure);
 *
 *             // Write operation as described in Section C
 *             Status = SD_WriteBlock(buffer, address, 512);
 *             Status = SD_WaitWriteOperation();
 *             while(SD_GetStatus() != SD_TRANSFER_OK);
 *
 *             Status = SD_WriteMultiBlocks(buffer, address, 512, NUMBEROFBLOCKS);
 *             Status = SD_WaitWriteOperation();
 *             while(SD_GetStatus() != SD_TRANSFER_OK);
 *
 *             // Read operation as described in Section B
 *             Status = SD_ReadBlock(buffer, address, 512);
 *             Status = SD_WaitReadOperation();
 *             while(SD_GetStatus() != SD_TRANSFER_OK);
 *
 *             Status = SD_ReadMultiBlocks(buffer, address, 512, NUMBEROFBLOCKS);
 *             Status = SD_WaitReadOperation();
 *             while(SD_GetStatus() != SD_TRANSFER_OK);
 *
 *             - Add the SDIO and DMA2 StreamX (3 or 6) IRQ Handlers:
 *                 void SDIO_IRQHandler(void)
 *                 {
 *                   SD_ProcessIRQ();
 *                 }
 *                 void SD_SDIO_DMA_IRQHANDLER(void)
 *                 {
 *                   SD_ProcessDMAIRQ();
 *                 }
 *
 *          F - Programming Model (Selecting Polling for SDIO data Transfer)
 *          ================================================================
 *            //Only SD Card Single Block operation are managed.
 *            Status = SD_Init(); // Initialization Step as described in section
 *
 *            // Write operation as described in Section C
 *            Status = SD_WriteBlock(buffer, address, 512);
 *
 *            // Read operation as described in Section B
 *            Status = SD_ReadBlock(buffer, address, 512);
 *
 *          STM32 SDIO Pin assignment
 *          =========================
 *          +-----------------------------------------------------------+
 *          |                     Pin assignment                        |
 *          +-----------------------------+---------------+-------------+
 *          |  STM32 SDIO Pins            |     SD        |    Pin      |
 *          +-----------------------------+---------------+-------------+
 *          |      SDIO D2                |   D2          |    1        |
 *          |      SDIO D3                |   D3          |    2        |
 *          |      SDIO CMD               |   CMD         |    3        |
 *          |                             |   VCC         |    4 (3.3 V)|
 *          |      SDIO CLK               |   CLK         |    5        |
 *          |                             |   GND         |    6 (0 V)  |
 *          |      SDIO D0                |   D0          |    7        |
 *          |      SDIO D1                |   D1          |    8        |
 *          +-----------------------------+---------------+-------------+
 *
 *  @endverbatim
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
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

/* Includes ------------------------------------------------------------------*/
//#include "stm324x9i_eval_ioe16.h"
#include "stm32f4xx.h"
#include "stm32f4xx_misc.h"
#include "stm32f4xx_sdio.h"
#include "fatfs_sd_sdio.h"
#include <string.h>
/*
#include "tm_stm32f4_usart.h"
#define logf(x)	TM_USART_Puts(USART1, x); TM_USART_Puts(USART1, "\n");
*/
#define logf(x)


static uint32_t CardType = SDIO_STD_CAPACITY_SD_CARD_V1_1;
static uint32_t CSD_Tab[4], CID_Tab[4], RCA = 0;
static uint8_t SDSTATUS_Tab[16];
__IO uint32_t StopCondition = 0;
__IO SD_Error TransferError = SD_OK;
__IO uint32_t TransferEnd = 0, DMAEndOfTransfer = 0;
SD_CardInfo SDCardInfo;

SDIO_InitTypeDef SDIO_InitStructure;
SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
SDIO_DataInitTypeDef SDIO_DataInitStructure;
/**
 * @}
 */

/** @defgroup STM324x9I_EVAL_SDIO_SD_Private_Function_Prototypes
 * @{
 */
static SD_Error CmdError (void);
static SD_Error CmdResp1Error (uint8_t cmd);
static SD_Error CmdResp7Error (void);
static SD_Error CmdResp3Error (void);
static SD_Error CmdResp2Error (void);
static SD_Error CmdResp6Error (uint8_t cmd, uint16_t *prca);
static SD_Error SDEnWideBus (FunctionalState NewState);
static SD_Error IsCardProgramming (uint8_t *pstatus);
static SD_Error FindSCR (uint16_t rca, uint32_t *pscr);
uint8_t convert_from_bytes_to_power_of_two (uint16_t NumberOfBytes);

static volatile DSTATUS TM_FATFS_SD_SDIO_Stat = STA_NOINIT;	/* Physical drive status */

#define BLOCK_SIZE            512

uint8_t TM_FATFS_SDIO_WriteEnabled(void) {
#if FATFS_USE_WRITEPROTECT_PIN > 0
	return !TM_GPIO_GetInputPinValue(FATFS_USE_WRITEPROTECT_PIN_PORT, FATFS_USE_WRITEPROTECT_PIN_PIN);
#else
	return 1;
#endif
}

DSTATUS TM_FATFS_SD_SDIO_disk_initialize(void) {
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Detect pin */
#if FATFS_USE_DETECT_PIN > 0
	TM_GPIO_Init(FATFS_USE_DETECT_PIN_PORT, FATFS_USE_DETECT_PIN_PIN, TM_GPIO_Mode_IN, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
#endif

	/* Write protect pin */
#if FATFS_USE_WRITEPROTECT_PIN > 0
	TM_GPIO_Init(FATFS_USE_WRITEPROTECT_PIN_PORT, FATFS_USE_WRITEPROTECT_PIN_PIN, TM_GPIO_Mode_IN, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
#endif

	// Configure the NVIC Preemption Priority Bits
	NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init (&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = SD_SDIO_DMA_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_Init (&NVIC_InitStructure);

	SD_LowLevel_DeInit();
	SD_LowLevel_Init();

	//Check disk initialized
	if (SD_Init() == SD_OK) {
		TM_FATFS_SD_SDIO_Stat &= ~STA_NOINIT;	/* Clear STA_NOINIT flag */
	} else {
		TM_FATFS_SD_SDIO_Stat |= STA_NOINIT;
	}
	//Check write protected
	if (!TM_FATFS_SDIO_WriteEnabled()) {
		TM_FATFS_SD_SDIO_Stat |= STA_PROTECT;
	} else {
		TM_FATFS_SD_SDIO_Stat &= ~STA_PROTECT;
	}

	return TM_FATFS_SD_SDIO_Stat;
}

DSTATUS TM_FATFS_SD_SDIO_disk_status(void) {
	if (SD_Detect() != SD_PRESENT) {
		return STA_NOINIT;
	}

	if (!TM_FATFS_SDIO_WriteEnabled()) {
		TM_FATFS_SD_SDIO_Stat |= STA_PROTECT;
	} else {
		TM_FATFS_SD_SDIO_Stat &= ~STA_PROTECT;
	}

	return TM_FATFS_SD_SDIO_Stat;
}

DRESULT TM_FATFS_SD_SDIO_disk_read(BYTE *buff, DWORD sector, UINT count) {
	SD_Error Status = SD_OK;

	if ((TM_FATFS_SD_SDIO_Stat & STA_NOINIT)) {
		return RES_NOTRDY;
	}

	if ((DWORD)buff & 3) {
		DRESULT res = RES_OK;
		DWORD scratch[BLOCK_SIZE / 4];

		while (count--) {
			res = TM_FATFS_SD_SDIO_disk_read((void *)scratch, sector++, 1);

			if (res != RES_OK) {
				break;
			}

			memcpy(buff, scratch, BLOCK_SIZE);

			buff += BLOCK_SIZE;
		}

		return res;
	}

	Status = SD_ReadMultiBlocks(buff, sector << 9, BLOCK_SIZE, count);

	if (Status == SD_OK) {
		SDTransferState State;

		Status = SD_WaitReadOperation();

		while ((State = SD_GetStatus()) == SD_TRANSFER_BUSY);

		if ((State == SD_TRANSFER_ERROR) || (Status != SD_OK)) {
			return RES_ERROR;
		} else {
			return RES_OK;
		}
	} else {
		return RES_ERROR;
	}
}

DRESULT TM_FATFS_SD_SDIO_disk_write(const BYTE *buff, DWORD sector, UINT count) {
	SD_Error Status = SD_OK;

	if (!TM_FATFS_SDIO_WriteEnabled()) {
		return RES_WRPRT;
	}

	if (SD_Detect() != SD_PRESENT) {
		return RES_NOTRDY;
	}

	if ((DWORD)buff & 3) {
		DRESULT res = RES_OK;
		DWORD scratch[BLOCK_SIZE / 4];

		while (count--) {
			memcpy(scratch, buff, BLOCK_SIZE);
			res = TM_FATFS_SD_SDIO_disk_write((void *)scratch, sector++, 1);

			if (res != RES_OK) {
				break;
			}

			buff += BLOCK_SIZE;
		}

		return(res);
	}

	Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9, BLOCK_SIZE, count); // 4GB Compliant

	if (Status == SD_OK) {
		SDTransferState State;

		Status = SD_WaitWriteOperation(); // Check if the Transfer is finished

		while ((State = SD_GetStatus()) == SD_TRANSFER_BUSY); // BUSY, OK (DONE), ERROR (FAIL)

		if ((State == SD_TRANSFER_ERROR) || (Status != SD_OK)) {
			return RES_ERROR;
		} else {
			return RES_OK;
		}
	} else {
		return RES_ERROR;
	}
}

DRESULT TM_FATFS_SD_SDIO_disk_ioctl(BYTE cmd, void *buff) {
	switch (cmd) {
		case GET_SECTOR_SIZE :     // Get R/W sector size (WORD)
			*(WORD *) buff = 512;
		break;
		case GET_BLOCK_SIZE :      // Get erase block size in unit of sector (DWORD)
			*(DWORD *) buff = 32;
		break;
		case CTRL_SYNC :
		case CTRL_ERASE_SECTOR :
		break;
	}

	return RES_OK;
}

void SDIO_IRQHandler(void) {
	SD_ProcessIRQSrc();
}

#ifdef SD_SDIO_DMA_STREAM3
void DMA2_Stream3_IRQHandler(void) {
	SD_ProcessDMAIRQ();
}
#endif

#ifdef SD_SDIO_DMA_STREAM6
void DMA2_Stream6_IRQHandler(void) {
	SD_ProcessDMAIRQ();
}
#endif



/**
 * @}
 */

/** @defgroup STM324x9I_EVAL_SDIO_SD_Private_Functions
 * @{
 */

/**
 * @brief  DeInitializes the SDIO interface.
 * @param  None
 * @retval None
 */
void SD_DeInit (void) {
	SD_LowLevel_DeInit();
}

/**
 * @brief  Initializes the SD Card and put it into StandBy State (Ready for data
 *         transfer).
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_Init (void)
{
	__IO SD_Error errorstatus = SD_OK;

	/* SDIO Peripheral Low Level Init */
	//SD_LowLevel_Init();
	SDIO_DeInit ();
	errorstatus = SD_PowerON ();

	if (errorstatus != SD_OK) {
		logf ("SD_PowerON failed\r\n");
		/*!< CMD Response TimeOut (wait for CMDSENT flag) */
		return (errorstatus);
	}

	logf ("SD_PowerON OK\r\n");

	errorstatus = SD_InitializeCards ();

	if (errorstatus != SD_OK) {
		logf ("SD_InitializeCards failed\r\n");
		/*!< CMD Response TimeOut (wait for CMDSENT flag) */
		return (errorstatus);
	}

	logf ("SD_InitializeCards OK\r\n");

	/*!< Configure the SDIO peripheral */
	/*!< SDIO_CK = SDIOCLK / (SDIO_TRANSFER_CLK_DIV + 2) */
	/*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
	SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV;
	SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
	SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
	SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
	SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
	SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
	SDIO_Init (&SDIO_InitStructure);

	/*----------------- Read CSD/CID MSD registers ------------------*/
	errorstatus = SD_GetCardInfo (&SDCardInfo);

	if (errorstatus == SD_OK) {
		/*----------------- Select Card --------------------------------*/
		logf ("SD_GetCardInfo OK\r\n");
		errorstatus = SD_SelectDeselect ((uint32_t) (SDCardInfo.RCA << 16));
	}
	else {
		logf ("SD_SelectDeselect failed\r\n");
	}

	if (errorstatus == SD_OK) {
		logf ("SD_SelectDeselect OK\r\n");
#if FATFS_SDIO_4BIT == 1
		//4 bit mode
		errorstatus = SD_EnableWideBusOperation (SDIO_BusWide_4b);
#else
		//1 bit mode
		errorstatus = SD_EnableWideBusOperation (SDIO_BusWide_1b);
#endif
	}
	else {
		logf ("SD_EnableWideBusOperation failed\r\n");
	}

	if (errorstatus == SD_OK) {
		logf ("SD_EnableWideBusOperation OK\r\n");
	}

	return (errorstatus);
}

/**
 * @brief  Gets the cuurent sd card data transfer status.
 * @param  None
 * @retval SDTransferState: Data Transfer state.
 *   This value can be:
 *        - SD_TRANSFER_OK: No data transfer is acting
 *        - SD_TRANSFER_BUSY: Data transfer is acting
 */
SDTransferState SD_GetStatus (void)
{
	SDCardState cardstate = SD_CARD_TRANSFER;

	cardstate = SD_GetState ();

	if (cardstate == SD_CARD_TRANSFER) {
		return (SD_TRANSFER_OK);
	} else if (cardstate == SD_CARD_ERROR) {
		return (SD_TRANSFER_ERROR);
	} else {
		return (SD_TRANSFER_BUSY);
	}
}

/**
 * @brief  Returns the current card's state.
 * @param  None
 * @retval SDCardState: SD Card Error or SD Card Current State.
 */
SDCardState SD_GetState(void) {
	uint32_t resp1 = 0;

	if (SD_Detect () == SD_PRESENT ) {
		if (SD_SendStatus (&resp1) != SD_OK) {
			return SD_CARD_ERROR;
		} else {
			return (SDCardState) ((resp1 >> 9) & 0x0F);
		}
	}

	return SD_CARD_ERROR;
}

/**
 * @brief  Detect if SD card is correctly plugged in the memory slot.
 * @param  None
 * @retval Return if SD is detected or not
 */
uint8_t SD_Detect(void) {
	__IO uint8_t status = SD_PRESENT;

	/* Check status */
	if (!TM_FATFS_CheckCardDetectPin()) {
		status = SD_NOT_PRESENT;
	}

	/* Return status */
	return status;
}

/**
 * @brief  Enquires cards about their operating voltage and configures
 *   clock controls.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_PowerON (void)
{
	__IO SD_Error errorstatus = SD_OK;
	uint32_t response = 0, count = 0, validvoltage = 0;
	uint32_t SDType = SD_STD_CAPACITY;

	/*!< Power ON Sequence -----------------------------------------------------*/
	/*!< Configure the SDIO peripheral */
	/*!< SDIO_CK = SDIOCLK / (SDIO_INIT_CLK_DIV + 2) */
	/*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
	/*!< SDIO_CK for initialization should not exceed 400 KHz */
	SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV;
	SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
	SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
	SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
	SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
	SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
	SDIO_Init (&SDIO_InitStructure);

	/*!< Set Power State to ON */
	SDIO_SetPowerState (SDIO_PowerState_ON);

	/*!< Enable SDIO Clock */
	SDIO_ClockCmd (ENABLE);

	/*!< CMD0: GO_IDLE_STATE ---------------------------------------------------*/
	/*!< No CMD response required */
	SDIO_CmdInitStructure.SDIO_Argument = 0x0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdError ();

	if (errorstatus != SD_OK) {
		/*!< CMD Response TimeOut (wait for CMDSENT flag) */
		return (errorstatus);
	}

	/*!< CMD8: SEND_IF_COND ----------------------------------------------------*/
	/*!< Send CMD8 to verify SD card interface operating condition */
	/*!< Argument: - [31:12]: Reserved (shall be set to '0')
	- [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
	- [7:0]: Check Pattern (recommended 0xAA) */
	/*!< CMD Response: R7 */
	SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp7Error ();

	if (errorstatus == SD_OK) {
		CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0; /*!< SD Card 2.0 */
		SDType = SD_HIGH_CAPACITY;
	} else {
		/*!< CMD55 */
		SDIO_CmdInitStructure.SDIO_Argument = 0x00;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand (&SDIO_CmdInitStructure);
		errorstatus = CmdResp1Error (SD_CMD_APP_CMD );
	}

	/*!< CMD55 */
	SDIO_CmdInitStructure.SDIO_Argument = 0x00;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);
	errorstatus = CmdResp1Error (SD_CMD_APP_CMD );

	/*!< If errorstatus is Command TimeOut, it is a MMC card */
	/*!< If errorstatus is SD_OK it is a SD card: SD card 2.0 (voltage range mismatch)
	or SD card 1.x */
	if (errorstatus == SD_OK) {
		/*!< SD CARD */
		/*!< Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
		while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL )) {
			/*!< SEND CMD55 APP_CMD with RCA as 0 */
			SDIO_CmdInitStructure.SDIO_Argument = 0x00;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand (&SDIO_CmdInitStructure);

			errorstatus = CmdResp1Error (SD_CMD_APP_CMD );

			if (errorstatus != SD_OK) {
				return (errorstatus);
			}
			SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD | SDType;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand (&SDIO_CmdInitStructure);

			errorstatus = CmdResp3Error ();
			if (errorstatus != SD_OK) {
				return (errorstatus);
			}

			response = SDIO_GetResponse (SDIO_RESP1);
			validvoltage = (((response >> 31) == 1) ? 1 : 0);
			count++;
		}
		if (count >= SD_MAX_VOLT_TRIAL ) {
			errorstatus = SD_INVALID_VOLTRANGE;
			return (errorstatus);
		}

		if (response &= SD_HIGH_CAPACITY ) {
			CardType = SDIO_HIGH_CAPACITY_SD_CARD;
		}

	}/*!< else MMC Card */

	return (errorstatus);
}

/**
 * @brief  Turns the SDIO output signals off.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_PowerOFF (void)
{
	SD_Error errorstatus = SD_OK;

	/*!< Set Power State to OFF */
	SDIO_SetPowerState (SDIO_PowerState_OFF);

	return (errorstatus);
}

/**
 * @brief  Intialises all cards or single card as the case may be Card(s) come
 *         into standby state.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_InitializeCards (void)
{
	SD_Error errorstatus = SD_OK;
	uint16_t rca = 0x01;

	if (SDIO_GetPowerState () == SDIO_PowerState_OFF) {
		errorstatus = SD_REQUEST_NOT_APPLICABLE;
		return (errorstatus);
	}

	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType) {
		/*!< Send CMD2 ALL_SEND_CID */
		SDIO_CmdInitStructure.SDIO_Argument = 0x0;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ALL_SEND_CID;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand (&SDIO_CmdInitStructure);

		errorstatus = CmdResp2Error ();

		if (SD_OK != errorstatus) {
			return (errorstatus);
		}

		CID_Tab[0] = SDIO_GetResponse (SDIO_RESP1);
		CID_Tab[1] = SDIO_GetResponse (SDIO_RESP2);
		CID_Tab[2] = SDIO_GetResponse (SDIO_RESP3);
		CID_Tab[3] = SDIO_GetResponse (SDIO_RESP4);
	}

	if (
		(SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) ||
		(SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||
		(SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType) ||
		(SDIO_HIGH_CAPACITY_SD_CARD == CardType)
	) {
		/*!< Send CMD3 SET_REL_ADDR with argument 0 */
		/*!< SD Card publishes its RCA. */
		SDIO_CmdInitStructure.SDIO_Argument = 0x00;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand (&SDIO_CmdInitStructure);

		errorstatus = CmdResp6Error (SD_CMD_SET_REL_ADDR, &rca);

		if (SD_OK != errorstatus) {
			return (errorstatus);
		}
	}

	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType) {
		RCA = rca;

		/*!< Send CMD9 SEND_CSD with argument as card's RCA */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) (rca << 16);
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_CSD;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand (&SDIO_CmdInitStructure);

		errorstatus = CmdResp2Error ();

		if (SD_OK != errorstatus) {
			return (errorstatus);
		}

		CSD_Tab[0] = SDIO_GetResponse (SDIO_RESP1);
		CSD_Tab[1] = SDIO_GetResponse (SDIO_RESP2);
		CSD_Tab[2] = SDIO_GetResponse (SDIO_RESP3);
		CSD_Tab[3] = SDIO_GetResponse (SDIO_RESP4);
	}

	return SD_OK;
}

/**
 * @brief  Returns information about specific card.
 * @param  cardinfo: pointer to a SD_CardInfo structure that contains all SD card
 *         information.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_GetCardInfo (SD_CardInfo *cardinfo)
{
	SD_Error errorstatus = SD_OK;
	uint8_t tmp = 0;

	cardinfo->CardType = (uint8_t) CardType;
	cardinfo->RCA = (uint16_t) RCA;

	/*!< Byte 0 */
	tmp = (uint8_t) ((CSD_Tab[0] & 0xFF000000) >> 24);
	cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6;
	cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;
	cardinfo->SD_csd.Reserved1 = tmp & 0x03;

	/*!< Byte 1 */
	tmp = (uint8_t) ((CSD_Tab[0] & 0x00FF0000) >> 16);
	cardinfo->SD_csd.TAAC = tmp;

	/*!< Byte 2 */
	tmp = (uint8_t) ((CSD_Tab[0] & 0x0000FF00) >> 8);
	cardinfo->SD_csd.NSAC = tmp;

	/*!< Byte 3 */
	tmp = (uint8_t) (CSD_Tab[0] & 0x000000FF);
	cardinfo->SD_csd.MaxBusClkFrec = tmp;

	/*!< Byte 4 */
	tmp = (uint8_t) ((CSD_Tab[1] & 0xFF000000) >> 24);
	cardinfo->SD_csd.CardComdClasses = tmp << 4;

	/*!< Byte 5 */
	tmp = (uint8_t) ((CSD_Tab[1] & 0x00FF0000) >> 16);
	cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4;
	cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;

	/*!< Byte 6 */
	tmp = (uint8_t) ((CSD_Tab[1] & 0x0000FF00) >> 8);
	cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;
	cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;
	cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;
	cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
	cardinfo->SD_csd.Reserved2 = 0; /*!< Reserved */

	if ((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1 )|| (CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0)) {
		cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;

		/*!< Byte 7 */
		tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF);
		cardinfo->SD_csd.DeviceSize |= (tmp) << 2;

		/*!< Byte 8 */
		tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24);
		cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;

		cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
		cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);

		/*!< Byte 9 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16);
		cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
		cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
		cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;
		/*!< Byte 10 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8);
		cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;

		cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1);
		cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
		cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
		cardinfo->CardCapacity *= cardinfo->CardBlockSize;
	} else if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) {
		/*!< Byte 7 */
		tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF);
		cardinfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16;

		/*!< Byte 8 */
		tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24);

		cardinfo->SD_csd.DeviceSize |= (tmp << 8);

		/*!< Byte 9 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16);

		cardinfo->SD_csd.DeviceSize |= (tmp);

		/*!< Byte 10 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8);

		cardinfo->CardCapacity = ((uint64_t)cardinfo->SD_csd.DeviceSize + 1) * 512 * 1024;
		cardinfo->CardBlockSize = 512;
	}

	cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
	cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;

	/*!< Byte 11 */
	tmp = (uint8_t) (CSD_Tab[2] & 0x000000FF);
	cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
	cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);

	/*!< Byte 12 */
	tmp = (uint8_t) ((CSD_Tab[3] & 0xFF000000) >> 24);
	cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
	cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
	cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
	cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;

	/*!< Byte 13 */
	tmp = (uint8_t) ((CSD_Tab[3] & 0x00FF0000) >> 16);
	cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
	cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
	cardinfo->SD_csd.Reserved3 = 0;
	cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);

	/*!< Byte 14 */
	tmp = (uint8_t) ((CSD_Tab[3] & 0x0000FF00) >> 8);
	cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
	cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
	cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
	cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
	cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
	cardinfo->SD_csd.ECC = (tmp & 0x03);

	/*!< Byte 15 */
	tmp = (uint8_t) (CSD_Tab[3] & 0x000000FF);
	cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
	cardinfo->SD_csd.Reserved4 = 1;

	/*!< Byte 0 */
	tmp = (uint8_t) ((CID_Tab[0] & 0xFF000000) >> 24);
	cardinfo->SD_cid.ManufacturerID = tmp;

	/*!< Byte 1 */
	tmp = (uint8_t) ((CID_Tab[0] & 0x00FF0000) >> 16);
	cardinfo->SD_cid.OEM_AppliID = tmp << 8;

	/*!< Byte 2 */
	tmp = (uint8_t) ((CID_Tab[0] & 0x000000FF00) >> 8);
	cardinfo->SD_cid.OEM_AppliID |= tmp;

	/*!< Byte 3 */
	tmp = (uint8_t) (CID_Tab[0] & 0x000000FF);
	cardinfo->SD_cid.ProdName1 = tmp << 24;

	/*!< Byte 4 */
	tmp = (uint8_t) ((CID_Tab[1] & 0xFF000000) >> 24);
	cardinfo->SD_cid.ProdName1 |= tmp << 16;

	/*!< Byte 5 */
	tmp = (uint8_t) ((CID_Tab[1] & 0x00FF0000) >> 16);
	cardinfo->SD_cid.ProdName1 |= tmp << 8;

	/*!< Byte 6 */
	tmp = (uint8_t) ((CID_Tab[1] & 0x0000FF00) >> 8);
	cardinfo->SD_cid.ProdName1 |= tmp;

	/*!< Byte 7 */
	tmp = (uint8_t) (CID_Tab[1] & 0x000000FF);
	cardinfo->SD_cid.ProdName2 = tmp;

	/*!< Byte 8 */
	tmp = (uint8_t) ((CID_Tab[2] & 0xFF000000) >> 24);
	cardinfo->SD_cid.ProdRev = tmp;

	/*!< Byte 9 */
	tmp = (uint8_t) ((CID_Tab[2] & 0x00FF0000) >> 16);
	cardinfo->SD_cid.ProdSN = tmp << 24;

	/*!< Byte 10 */
	tmp = (uint8_t) ((CID_Tab[2] & 0x0000FF00) >> 8);
	cardinfo->SD_cid.ProdSN |= tmp << 16;

	/*!< Byte 11 */
	tmp = (uint8_t) (CID_Tab[2] & 0x000000FF);
	cardinfo->SD_cid.ProdSN |= tmp << 8;

	/*!< Byte 12 */
	tmp = (uint8_t) ((CID_Tab[3] & 0xFF000000) >> 24);
	cardinfo->SD_cid.ProdSN |= tmp;

	/*!< Byte 13 */
	tmp = (uint8_t) ((CID_Tab[3] & 0x00FF0000) >> 16);
	cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
	cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;

	/*!< Byte 14 */
	tmp = (uint8_t) ((CID_Tab[3] & 0x0000FF00) >> 8);
	cardinfo->SD_cid.ManufactDate |= tmp;

	/*!< Byte 15 */
	tmp = (uint8_t) (CID_Tab[3] & 0x000000FF);
	cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
	cardinfo->SD_cid.Reserved2 = 1;

	return (errorstatus);
}

/**
 * @brief  Enables wide bus opeartion for the requeseted card if supported by
 *         card.
 * @param  WideMode: Specifies the SD card wide bus mode.
 *   This parameter can be one of the following values:
 *     @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
 *     @arg SDIO_BusWide_4b: 4-bit data transfer
 *     @arg SDIO_BusWide_1b: 1-bit data transfer
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_GetCardStatus (SD_CardStatus *cardstatus)
{
	SD_Error errorstatus = SD_OK;
	uint8_t tmp = 0;

	errorstatus = SD_SendSDStatus ((uint32_t *) SDSTATUS_Tab);

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	/*!< Byte 0 */
	tmp = (uint8_t) ((SDSTATUS_Tab[0] & 0xC0) >> 6);
	cardstatus->DAT_BUS_WIDTH = tmp;

	/*!< Byte 0 */
	tmp = (uint8_t) ((SDSTATUS_Tab[0] & 0x20) >> 5);
	cardstatus->SECURED_MODE = tmp;

	/*!< Byte 2 */
	tmp = (uint8_t) ((SDSTATUS_Tab[2] & 0xFF));
	cardstatus->SD_CARD_TYPE = tmp << 8;

	/*!< Byte 3 */
	tmp = (uint8_t) ((SDSTATUS_Tab[3] & 0xFF));
	cardstatus->SD_CARD_TYPE |= tmp;

	/*!< Byte 4 */
	tmp = (uint8_t) (SDSTATUS_Tab[4] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA = tmp << 24;

	/*!< Byte 5 */
	tmp = (uint8_t) (SDSTATUS_Tab[5] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 16;

	/*!< Byte 6 */
	tmp = (uint8_t) (SDSTATUS_Tab[6] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 8;

	/*!< Byte 7 */
	tmp = (uint8_t) (SDSTATUS_Tab[7] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp;

	/*!< Byte 8 */
	tmp = (uint8_t) ((SDSTATUS_Tab[8] & 0xFF));
	cardstatus->SPEED_CLASS = tmp;

	/*!< Byte 9 */
	tmp = (uint8_t) ((SDSTATUS_Tab[9] & 0xFF));
	cardstatus->PERFORMANCE_MOVE = tmp;

	/*!< Byte 10 */
	tmp = (uint8_t) ((SDSTATUS_Tab[10] & 0xF0) >> 4);
	cardstatus->AU_SIZE = tmp;

	/*!< Byte 11 */
	tmp = (uint8_t) (SDSTATUS_Tab[11] & 0xFF);
	cardstatus->ERASE_SIZE = tmp << 8;

	/*!< Byte 12 */
	tmp = (uint8_t) (SDSTATUS_Tab[12] & 0xFF);
	cardstatus->ERASE_SIZE |= tmp;

	/*!< Byte 13 */
	tmp = (uint8_t) ((SDSTATUS_Tab[13] & 0xFC) >> 2);
	cardstatus->ERASE_TIMEOUT = tmp;

	/*!< Byte 13 */
	tmp = (uint8_t) ((SDSTATUS_Tab[13] & 0x3));
	cardstatus->ERASE_OFFSET = tmp;

	return (errorstatus);
}

/**
 * @brief  Enables wide bus opeartion for the requeseted card if supported by
 *         card.
 * @param  WideMode: Specifies the SD card wide bus mode.
 *   This parameter can be one of the following values:
 *     @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
 *     @arg SDIO_BusWide_4b: 4-bit data transfer
 *     @arg SDIO_BusWide_1b: 1-bit data transfer
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_EnableWideBusOperation (uint32_t WideMode)
{
	SD_Error errorstatus = SD_OK;

	/*!< MMC Card doesn't support this feature */
	if (SDIO_MULTIMEDIA_CARD == CardType) {
		errorstatus = SD_UNSUPPORTED_FEATURE;
		return (errorstatus);
	} else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType)) {
		if (SDIO_BusWide_8b == WideMode) {
			errorstatus = SD_UNSUPPORTED_FEATURE;
			return (errorstatus);
		} else if (SDIO_BusWide_4b == WideMode) {
			errorstatus = SDEnWideBus (ENABLE);

			if (SD_OK == errorstatus) {
				/*!< Configure the SDIO peripheral */
				SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV;
				SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
				SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
				SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
				SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_4b;
				SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
				SDIO_Init (&SDIO_InitStructure);
			}
		} else {
			errorstatus = SDEnWideBus (DISABLE);

			if (SD_OK == errorstatus) {
				/*!< Configure the SDIO peripheral */
				SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV;
				SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
				SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
				SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
				SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
				SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
				SDIO_Init (&SDIO_InitStructure);
			}
		}
	}

	return (errorstatus);
}

/**
 * @brief  Selects od Deselects the corresponding card.
 * @param  addr: Address of the Card to be selected.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_SelectDeselect (uint64_t addr)
{
	SD_Error errorstatus = SD_OK;

	/*!< Send CMD7 SDIO_SEL_DESEL_CARD */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) addr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEL_DESEL_CARD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_SEL_DESEL_CARD );

	return (errorstatus);
}

/**
 * @brief  Allows to read one block from a specified address in a card. The Data
 *         transfer can be managed by DMA mode or Polling mode.
 * @note   This operation should be followed by two functions to check if the
 *         DMA Controller and SD Card status.
 *          - SD_ReadWaitOperation(): this function insure that the DMA
 *            controller has finished all data transfer.
 *          - SD_GetStatus(): to check that the SD Card has finished the
 *            data transfer and it is ready for data.
 * @param  readbuff: pointer to the buffer that will contain the received data
 * @param  ReadAddr: Address from where data are to be read.
 * @param  BlockSize: the SD card Data block size. The Block size should be 512.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_ReadBlock (uint8_t *readbuff, uint64_t ReadAddr, uint16_t BlockSize)
{
	SD_Error errorstatus = SD_OK;
#if defined (SD_POLLING_MODE)
	uint32_t count = 0, *tempbuff = (uint32_t *)readbuff;
#endif

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 0;

	SDIO ->DCTRL = 0x0;

#if defined (SD_DMA_MODE)
	SDIO->MASK |= (SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR);
	SDIO_DMACmd (ENABLE);
	SD_LowLevel_DMA_RxConfig ((uint32_t *) readbuff, BlockSize);
#endif

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD ) {
		BlockSize = 512;
		ReadAddr /= 512;
	}

	/* Set Block Size for Card */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) BlockSize;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_SET_BLOCKLEN );

	if (SD_OK != errorstatus) {
		return (errorstatus);
	}

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) SDIO_DATABLOCKSIZE;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig (&SDIO_DataInitStructure);

	/*!< Send CMD17 READ_SINGLE_BLOCK */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) ReadAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_READ_SINGLE_BLOCK );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

#if defined (SD_POLLING_MODE)
	/*!< In case of single block transfer, no need of stop transfer at all.*/
	/*!< Polling mode */
	while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR))) {
		if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET) {
			for (count = 0; count < 8; count++) {
				*(tempbuff + count) = SDIO_ReadData();
			}
			tempbuff += 8;
		}
	}

	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) {
		SDIO->ICR = (SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	} else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) {
		SDIO->ICR = (SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	} else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) {
		SDIO->ICR = (SDIO_FLAG_RXOVERR);
		errorstatus = SD_RX_OVERRUN;
		return(errorstatus);
	} else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) {
		SDIO->ICR = (SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}
	count = SD_DATATIMEOUT;
	while ((SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) && (count > 0)) {
		*tempbuff = SDIO_ReadData();
		tempbuff++;
		count--;
	}

	/*!< Clear all the static flags */
	SDIO->ICR = (SDIO_STATIC_FLAGS);

#endif

	return (errorstatus);
}

/**
 * @brief  Allows to read blocks from a specified address  in a card.  The Data
 *         transfer can be managed by DMA mode or Polling mode.
 * @note   This operation should be followed by two functions to check if the
 *         DMA Controller and SD Card status.
 *          - SD_ReadWaitOperation(): this function insure that the DMA
 *            controller has finished all data transfer.
 *          - SD_GetStatus(): to check that the SD Card has finished the
 *            data transfer and it is ready for data.
 * @param  readbuff: pointer to the buffer that will contain the received data.
 * @param  ReadAddr: Address from where data are to be read.
 * @param  BlockSize: the SD card Data block size. The Block size should be 512.
 * @param  NumberOfBlocks: number of blocks to be read.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_ReadMultiBlocks (uint8_t *readbuff, uint64_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
	SD_Error errorstatus = SD_OK;
	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 1;

	SDIO ->DCTRL = 0x0;

#if defined (SD_DMA_MODE)
	SDIO->MASK |= (SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR);
	SD_LowLevel_DMA_RxConfig ((uint32_t *) readbuff, (NumberOfBlocks * BlockSize));
	SDIO_DMACmd (ENABLE);
#endif

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD ) {
		BlockSize = 512;
		ReadAddr /= 512;
	}

	/*!< Set Block Size for Card */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) BlockSize;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_SET_BLOCKLEN );

	if (SD_OK != errorstatus) {
		return (errorstatus);
	}

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DATABLOCKSIZE;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig (&SDIO_DataInitStructure);

	/*!< Send CMD18 READ_MULT_BLOCK with argument data address */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) ReadAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_READ_MULT_BLOCK );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	return (errorstatus);
}

SD_Error SD_ReadMultiBlocksFIXED(uint8_t *readbuff, uint64_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks) {
	SD_Error errorstatus = SD_OK;
	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 1;

	SDIO->DCTRL = 0x0;

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) {
		BlockSize = 512;
	} else {
		ReadAddr *= BlockSize; // Convert to Bytes for NON SDHC
	}

	/*!< Set Block Size for Card */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) BlockSize;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

	if (SD_OK != errorstatus) {
		return(errorstatus);
	}

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) SDIO_DATABLOCKSIZE;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	/*!< Send CMD18 READ_MULT_BLOCK with argument data address */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)ReadAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);

	if (errorstatus != SD_OK) {
		return(errorstatus);
	}

	SDIO->MASK |= (SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR);
	SDIO_DMACmd(ENABLE);
	SD_LowLevel_DMA_RxConfig((uint32_t *)readbuff, (NumberOfBlocks * BlockSize));

	return(errorstatus);
}

/**
 * @brief  This function waits until the SDIO DMA data transfer is finished.
 *         This function should be called after SDIO_ReadMultiBlocks() function
 *         to insure that all data sent by the card are already transferred by
 *         the DMA controller.
 * @param  None.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_WaitReadOperation (void)
{
	SD_Error errorstatus = SD_OK;
	volatile uint32_t timeout;

	timeout = SD_DATATIMEOUT;

	while ((DMAEndOfTransfer == 0x00) && (TransferEnd == 0) && (TransferError == SD_OK) && (timeout > 0)) {
		timeout--;
	}

	DMAEndOfTransfer = 0x00;

	timeout = SD_DATATIMEOUT;

	while (((SDIO ->STA & SDIO_FLAG_RXACT)) && (timeout > 0)) {
		timeout--;
	}

	if (StopCondition == 1) {
		errorstatus = SD_StopTransfer ();
		StopCondition = 0;
	}

	if ((timeout == 0) && (errorstatus == SD_OK)) {
		errorstatus = SD_DATA_TIMEOUT;
	}

	/*!< Clear all the static flags */
	SDIO->ICR = (SDIO_STATIC_FLAGS);

	if (TransferError != SD_OK) {
		return (TransferError);
	}

	return (errorstatus);
}

/**
 * @brief  Allows to write one block starting from a specified address in a card.
 *         The Data transfer can be managed by DMA mode or Polling mode.
 * @note   This operation should be followed by two functions to check if the
 *         DMA Controller and SD Card status.
 *          - SD_ReadWaitOperation(): this function insure that the DMA
 *            controller has finished all data transfer.
 *          - SD_GetStatus(): to check that the SD Card has finished the
 *            data transfer and it is ready for data.
 * @param  writebuff: pointer to the buffer that contain the data to be transferred.
 * @param  WriteAddr: Address from where data are to be read.
 * @param  BlockSize: the SD card Data block size. The Block size should be 512.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_WriteBlock (uint8_t *writebuff, uint64_t WriteAddr, uint16_t BlockSize) {
	SD_Error errorstatus = SD_OK;

#if defined (SD_POLLING_MODE)
	uint32_t bytestransferred = 0, count = 0, restwords = 0;
	uint32_t *tempbuff = (uint32_t *)writebuff;
#endif

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 0;

	SDIO->DCTRL = 0x0;

#if defined (SD_DMA_MODE)
	SDIO->MASK |= (SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR);
	SD_LowLevel_DMA_TxConfig ((uint32_t *) writebuff, BlockSize);
	SDIO_DMACmd (ENABLE);
#endif

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD ) {
		BlockSize = 512;
		WriteAddr /= 512;
	}

	/* Set Block Size for Card */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) BlockSize;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_SET_BLOCKLEN );

	if (SD_OK != errorstatus) {
		return (errorstatus);
	}

	/*!< Send CMD24 WRITE_SINGLE_BLOCK */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) WriteAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_WRITE_SINGLE_BLOCK );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) SDIO_DATABLOCKSIZE;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig (&SDIO_DataInitStructure);

	/*!< In case of single data block transfer no need of stop command at all */
#if defined (SD_POLLING_MODE)
	while (!(SDIO->STA & (SDIO_FLAG_DBCKEND | SDIO_FLAG_TXUNDERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_STBITERR))) {
		if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET) {
			if ((512 - bytestransferred) < 32) {
				restwords = ((512 - bytestransferred) % 4 == 0) ? ((512 - bytestransferred) / 4) : (( 512 - bytestransferred) / 4 + 1);
				for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4) {
					SDIO->FIFO = (*tempbuff);
				}
			} else {
				for (count = 0; count < 8; count++) {
					SDIO->FIFO = (*(tempbuff + count));
				}
				tempbuff += 8;
				bytestransferred += 32;
			}
		}
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) {
		SDIO->ICR = (SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	} else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) {
		SDIO->ICR = (SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	} else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET) {
		SDIO->ICR = (SDIO_FLAG_TXUNDERR);
		errorstatus = SD_TX_UNDERRUN;
		return(errorstatus);
	} else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) {
		SDIO->ICR = (SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}

#endif

	return (errorstatus);
}

/**
 * @brief  Allows to write blocks starting from a specified address in a card.
 *         The Data transfer can be managed by DMA mode only.
 * @note   This operation should be followed by two functions to check if the
 *         DMA Controller and SD Card status.
 *          - SD_ReadWaitOperation(): this function insure that the DMA
 *            controller has finished all data transfer.
 *          - SD_GetStatus(): to check that the SD Card has finished the
 *            data transfer and it is ready for data.
 * @param  WriteAddr: Address from where data are to be read.
 * @param  writebuff: pointer to the buffer that contain the data to be transferred.
 * @param  BlockSize: the SD card Data block size. The Block size should be 512.
 * @param  NumberOfBlocks: number of blocks to be written.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_WriteMultiBlocks (uint8_t *writebuff, uint64_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks) {
	SD_Error errorstatus = SD_OK;

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 1;
	SDIO ->DCTRL = 0x0;

#if defined (SD_DMA_MODE)
	SDIO->MASK |= (SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_TXUNDERR | SDIO_IT_STBITERR);
	SD_LowLevel_DMA_TxConfig ((uint32_t *) writebuff, (NumberOfBlocks * BlockSize));
	SDIO_DMACmd (ENABLE);
#endif

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD ) {
		BlockSize = 512;
		WriteAddr /= 512;
	}

	/* Set Block Size for Card */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) BlockSize;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_SET_BLOCKLEN );

	if (SD_OK != errorstatus) {
		return (errorstatus);
	}

	/*!< To improve performance */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) (RCA << 16);
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_APP_CMD );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	/*!< To improve performance */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) NumberOfBlocks;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_SET_BLOCK_COUNT );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	/*!< Send CMD25 WRITE_MULT_BLOCK with argument data address */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) WriteAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_WRITE_MULT_BLOCK );

	if (SD_OK != errorstatus) {
		return (errorstatus);
	}

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) SDIO_DATABLOCKSIZE;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig (&SDIO_DataInitStructure);

	return (errorstatus);
}

/**
 * @brief  Allows to write blocks starting from a specified address in a card.
 *         The Data transfer can be managed by DMA mode only.
 * @note   This operation should be followed by two functions to check if the
 *         DMA Controller and SD Card status.
 *          - SD_ReadWaitOperation(): this function insure that the DMA
 *            controller has finished all data transfer.
 *          - SD_GetStatus(): to check that the SD Card has finished the
 *            data transfer and it is ready for data.
 * @param  WriteAddr: Address from where data are to be read.
 * @param  writebuff: pointer to the buffer that contain the data to be transferred.
 * @param  BlockSize: the SD card Data block size. The Block size should be 512.
 * @param  NumberOfBlocks: number of blocks to be written.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_WriteMultiBlocksFIXED (uint8_t *writebuff, uint64_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks) {
	SD_Error errorstatus = SD_OK;

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 1;

	SDIO->DCTRL = 0x0;

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
		BlockSize = 512;
	else
		WriteAddr *= BlockSize; // Convert to Bytes for NON SDHC

	/* Set Block Size for Card */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) BlockSize;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

	if (SD_OK != errorstatus) {
		return(errorstatus);
	}

	/*!< To improve performance */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) (RCA << 16);
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);


	errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

	if (errorstatus != SD_OK) {
		return(errorstatus);
	}
	/*!< To improve performance */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)NumberOfBlocks;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);

	if (errorstatus != SD_OK) {
		return(errorstatus);
	}


	/*!< Send CMD25 WRITE_MULT_BLOCK with argument data address */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)WriteAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);

	if (SD_OK != errorstatus) {
		return(errorstatus);
	}

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) SDIO_DATABLOCKSIZE;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	SDIO->MASK |= (SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR);
	SDIO_DMACmd(ENABLE);
	SD_LowLevel_DMA_TxConfig((uint32_t *)writebuff, (NumberOfBlocks * BlockSize));

	return (errorstatus);
}

/**
 * @brief  This function waits until the SDIO DMA data transfer is finished.
 *         This function should be called after SDIO_WriteBlock() and
 *         SDIO_WriteMultiBlocks() function to insure that all data sent by the
 *         card are already transferred by the DMA controller.
 * @param  None.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_WaitWriteOperation (void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t timeout;

	timeout = SD_DATATIMEOUT;

	while ((DMAEndOfTransfer == 0x00) && (TransferEnd == 0) && (TransferError == SD_OK) && (timeout > 0)) {
		timeout--;
	}

	DMAEndOfTransfer = 0x00;

	timeout = SD_DATATIMEOUT;

	while (((SDIO ->STA & SDIO_FLAG_TXACT)) && (timeout > 0)) {
		timeout--;
	}

	if (StopCondition == 1) {
		errorstatus = SD_StopTransfer ();
		StopCondition = 0;
	}

	if ((timeout == 0) && (errorstatus == SD_OK)) {
		errorstatus = SD_DATA_TIMEOUT;
	}

	/*!< Clear all the static flags */
	SDIO->ICR =  (SDIO_STATIC_FLAGS );

	if (TransferError != SD_OK) {
		return (TransferError);
	} else {
		return (errorstatus);
	}
}

/**
 * @brief  Gets the cuurent data transfer state.
 * @param  None
 * @retval SDTransferState: Data Transfer state.
 *   This value can be:
 *        - SD_TRANSFER_OK: No data transfer is acting
 *        - SD_TRANSFER_BUSY: Data transfer is acting
 */
SDTransferState SD_GetTransferState (void)
{
	if (SDIO ->STA & (SDIO_FLAG_TXACT | SDIO_FLAG_RXACT)) {
		return (SD_TRANSFER_BUSY);
	} else {
		return (SD_TRANSFER_OK);
	}
}

/**
 * @brief  Aborts an ongoing data transfer.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_StopTransfer (void)
{
	SD_Error errorstatus = SD_OK;

	/*!< Send CMD12 STOP_TRANSMISSION  */
	SDIO_CmdInitStructure.SDIO_Argument = 0x0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_STOP_TRANSMISSION );

	return (errorstatus);
}

/**
 * @brief  Allows to erase memory area specified for the given card.
 * @param  startaddr: the start address.
 * @param  endaddr: the end address.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_Erase (uint64_t startaddr, uint64_t endaddr)
{
	SD_Error errorstatus = SD_OK;
	uint32_t delay = 0;
	__IO uint32_t maxdelay = 0;
	uint8_t cardstate = 0;

	/*!< Check if the card coomnd class supports erase command */
	if (((CSD_Tab[1] >> 20) & SD_CCCC_ERASE )== 0){
		errorstatus = SD_REQUEST_NOT_APPLICABLE;
		return (errorstatus);
	}

	maxdelay = 120000 / ((SDIO ->CLKCR & 0xFF) + 2);

	if (SDIO_GetResponse (SDIO_RESP1) & SD_CARD_LOCKED ) {
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return (errorstatus);
	}

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD ) {
		startaddr /= 512;
		endaddr /= 512;
	}

	/*!< According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33) */
	if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType)) {
		/*!< Send CMD32 SD_ERASE_GRP_START with argument as addr  */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) startaddr;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_ERASE_GRP_START;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand (&SDIO_CmdInitStructure);

		errorstatus = CmdResp1Error (SD_CMD_SD_ERASE_GRP_START );
		if (errorstatus != SD_OK) {
			return (errorstatus);
		}

		/*!< Send CMD33 SD_ERASE_GRP_END with argument as addr  */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) endaddr;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_ERASE_GRP_END;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand (&SDIO_CmdInitStructure);

		errorstatus = CmdResp1Error (SD_CMD_SD_ERASE_GRP_END );
		if (errorstatus != SD_OK) {
			return (errorstatus);
		}
	}

	/*!< Send CMD38 ERASE */
	SDIO_CmdInitStructure.SDIO_Argument = 0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ERASE;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_ERASE );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	for (delay = 0; delay < maxdelay; delay++);

	/*!< Wait till the card is in programming state */
	errorstatus = IsCardProgramming (&cardstate);
	delay = SD_DATATIMEOUT;
	while ((delay > 0) && (errorstatus == SD_OK) && ((SD_CARD_PROGRAMMING == cardstate) || (SD_CARD_RECEIVING == cardstate))) {
		errorstatus = IsCardProgramming (&cardstate);
		delay--;
	}

	return (errorstatus);
}

/**
 * @brief  Returns the current card's status.
 * @param  pcardstatus: pointer to the buffer that will contain the SD card
 *         status (Card Status register).
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_SendStatus (uint32_t *pcardstatus)
{
	SD_Error errorstatus = SD_OK;

	if (pcardstatus == 0) {
		errorstatus = SD_INVALID_PARAMETER;
		return (errorstatus);
	}

	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_SEND_STATUS );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	*pcardstatus = SDIO_GetResponse (SDIO_RESP1);

	return (errorstatus);
}

/**
 * @brief  Returns the current SD card's status.
 * @param  psdstatus: pointer to the buffer that will contain the SD card status
 *         (SD Status register).
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_SendSDStatus (uint32_t *psdstatus)
{
	SD_Error errorstatus = SD_OK;
	uint32_t count = 0;

	if (SDIO_GetResponse (SDIO_RESP1) & SD_CARD_LOCKED ) {
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return (errorstatus);
	}

	/*!< Set block size for card if it is not equal to current block size for card. */
	SDIO_CmdInitStructure.SDIO_Argument = 64;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_SET_BLOCKLEN );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	/*!< CMD55 */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);
	errorstatus = CmdResp1Error (SD_CMD_APP_CMD );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = 64;
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_64b;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig (&SDIO_DataInitStructure);

	/*!< Send ACMD13 SD_APP_STAUS  with argument as card's RCA.*/
	SDIO_CmdInitStructure.SDIO_Argument = 0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_STAUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);
	errorstatus = CmdResp1Error (SD_CMD_SD_APP_STAUS );

	if (errorstatus != SD_OK) {
	return (errorstatus);
	}

	while (!(SDIO ->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR))) {
		if (SDIO_GetFlagStatus (SDIO_FLAG_RXFIFOHF) != RESET) {
			for (count = 0; count < 8; count++) {
				*(psdstatus + count) = SDIO_ReadData ();
			}
			psdstatus += 8;
		}
	}

	if (SDIO_GetFlagStatus (SDIO_FLAG_DTIMEOUT) != RESET) {
		SDIO->ICR =  (SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return (errorstatus);
	} else if (SDIO_GetFlagStatus (SDIO_FLAG_DCRCFAIL) != RESET) {
		SDIO->ICR =  (SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return (errorstatus);
	} else if (SDIO_GetFlagStatus (SDIO_FLAG_RXOVERR) != RESET) {
		SDIO->ICR =  (SDIO_FLAG_RXOVERR);
		errorstatus = SD_RX_OVERRUN;
		return (errorstatus);
	} else if (SDIO_GetFlagStatus (SDIO_FLAG_STBITERR) != RESET) {
		SDIO->ICR =  (SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return (errorstatus);
	}

	count = SD_DATATIMEOUT;
	while ((SDIO_GetFlagStatus (SDIO_FLAG_RXDAVL) != RESET) && (count > 0)) {
		*psdstatus = SDIO_ReadData ();
		psdstatus++;
		count--;
	}
	/*!< Clear all the static status flags*/
	SDIO->ICR =  (SDIO_STATIC_FLAGS );

	return (errorstatus);
}

/**
 * @brief  Allows to process all the interrupts that are high.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_ProcessIRQSrc (void)
{
	if (SDIO_GetITStatus (SDIO_IT_DATAEND) != RESET) {
		TransferError = SD_OK;
		SDIO_ClearITPendingBit (SDIO_IT_DATAEND);
		TransferEnd = 1;
		logf ("SDIO IRQ : TransferEnd = 1, OK\r\n");
	} else if (SDIO_GetITStatus (SDIO_IT_DCRCFAIL) != RESET) {
		SDIO_ClearITPendingBit (SDIO_IT_DCRCFAIL);
		TransferError = SD_DATA_CRC_FAIL;
		logf ("SDIO IRQ : SD_DATA_CRC_FAIL\r\n");
	} else if (SDIO_GetITStatus (SDIO_IT_DTIMEOUT) != RESET) {
		SDIO_ClearITPendingBit (SDIO_IT_DTIMEOUT);
		TransferError = SD_DATA_TIMEOUT;
		logf ("SDIO IRQ : SD_DATA_TIMEOUT\r\n");
	} else if (SDIO_GetITStatus (SDIO_IT_RXOVERR) != RESET) {
		SDIO_ClearITPendingBit (SDIO_IT_RXOVERR);
		TransferError = SD_RX_OVERRUN;
		logf ("SDIO IRQ : SD_RX_OVERRUN\r\n");
	} else if (SDIO_GetITStatus (SDIO_IT_TXUNDERR) != RESET) {
		SDIO_ClearITPendingBit (SDIO_IT_TXUNDERR);
		TransferError = SD_TX_UNDERRUN;
		logf ("SDIO IRQ : SD_TX_UNDERRUN\r\n");
	} else if (SDIO_GetITStatus (SDIO_IT_STBITERR) != RESET) {
		SDIO_ClearITPendingBit (SDIO_IT_STBITERR);
		TransferError = SD_START_BIT_ERR;
		logf ("SDIO IRQ : SD_START_BIT_ERR\r\n");
	}

	SDIO->MASK &= ~(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR | SDIO_IT_RXOVERR | SDIO_IT_STBITERR);
	return (TransferError);
}

/**
 * @brief  This function waits until the SDIO DMA data transfer is finished.
 * @param  None.
 * @retval None.
 */
void SD_ProcessDMAIRQ(void) {
#ifdef SD_SDIO_DMA_STREAM3
	if (DMA2->LISR & SD_SDIO_DMA_FLAG_TCIF) {
#else
	if (DMA2->HISR & SD_SDIO_DMA_FLAG_TCIF) {
#endif
		DMAEndOfTransfer = 0x01;
		DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TCIF | SD_SDIO_DMA_FLAG_FEIF);
	}
}

/**
 * @brief  Checks for error conditions for CMD0.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error CmdError (void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t timeout;

	timeout = SDIO_CMD0TIMEOUT; /*!< 10000 */

	while ((timeout > 0) && (SDIO_GetFlagStatus (SDIO_FLAG_CMDSENT) == RESET)) {
		timeout--;
	}

	if (timeout == 0) {
		errorstatus = SD_CMD_RSP_TIMEOUT;
		return (errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO->ICR =  (SDIO_STATIC_FLAGS );

	return (errorstatus);
}

/**
 * @brief  Checks for error conditions for R7 response.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error CmdResp7Error (void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t timeout = SDIO_CMD0TIMEOUT;

	status = SDIO ->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)) && (timeout > 0)) {
		timeout--;
		status = SDIO ->STA;
	}

	if ((timeout == 0) || (status & SDIO_FLAG_CTIMEOUT)) {
		/*!< Card is not V2.0 complient or card does not support the set voltage range */
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO->ICR =  (SDIO_FLAG_CTIMEOUT);
		return (errorstatus);
	}

	if (status & SDIO_FLAG_CMDREND) {
		/*!< Card is SD V2.0 compliant */
		errorstatus = SD_OK;
		SDIO->ICR =  (SDIO_FLAG_CMDREND);
		return (errorstatus);
	}
	return (errorstatus);
}

/**
 * @brief  Checks for error conditions for R1 response.
 * @param  cmd: The sent command index.
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error CmdResp1Error (uint8_t cmd)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t response_r1;

	status = SDIO ->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT))) {
		status = SDIO ->STA;
	}

	if (status & SDIO_FLAG_CTIMEOUT) {
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO->ICR =  (SDIO_FLAG_CTIMEOUT);
		return (errorstatus);
	} else if (status & SDIO_FLAG_CCRCFAIL) {
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO->ICR =  (SDIO_FLAG_CCRCFAIL);
		return (errorstatus);
	}

	/*!< Check response received is of desired command */
	if (SDIO_GetCommandResponse () != cmd) {
		errorstatus = SD_ILLEGAL_CMD;
		return (errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO->ICR =  (SDIO_STATIC_FLAGS);

	/*!< We have received response, retrieve it for analysis  */
	response_r1 = SDIO_GetResponse (SDIO_RESP1);

	if ((response_r1 & SD_OCR_ERRORBITS )== SD_ALLZERO) {
		return (errorstatus);
	}

	if (response_r1 & SD_OCR_ADDR_OUT_OF_RANGE ) {
		return (SD_ADDR_OUT_OF_RANGE);
	}

	if (response_r1 & SD_OCR_ADDR_MISALIGNED ) {
		return (SD_ADDR_MISALIGNED);
	}

	if (response_r1 & SD_OCR_BLOCK_LEN_ERR ) {
		return (SD_BLOCK_LEN_ERR);
	}

	if (response_r1 & SD_OCR_ERASE_SEQ_ERR ) {
		return (SD_ERASE_SEQ_ERR);
	}

	if (response_r1 & SD_OCR_BAD_ERASE_PARAM ) {
		return (SD_BAD_ERASE_PARAM);
	}

	if (response_r1 & SD_OCR_WRITE_PROT_VIOLATION ) {
		return (SD_WRITE_PROT_VIOLATION);
	}

	if (response_r1 & SD_OCR_LOCK_UNLOCK_FAILED ) {
		return (SD_LOCK_UNLOCK_FAILED);
	}

	if (response_r1 & SD_OCR_COM_CRC_FAILED ) {
		return (SD_COM_CRC_FAILED);
	}

	if (response_r1 & SD_OCR_ILLEGAL_CMD ) {
		return (SD_ILLEGAL_CMD);
	}

	if (response_r1 & SD_OCR_CARD_ECC_FAILED ) {
		return (SD_CARD_ECC_FAILED);
	}

	if (response_r1 & SD_OCR_CC_ERROR ) {
		return (SD_CC_ERROR);
	}

	if (response_r1 & SD_OCR_GENERAL_UNKNOWN_ERROR ) {
		return (SD_GENERAL_UNKNOWN_ERROR);
	}

	if (response_r1 & SD_OCR_STREAM_READ_UNDERRUN ) {
		return (SD_STREAM_READ_UNDERRUN);
	}

	if (response_r1 & SD_OCR_STREAM_WRITE_OVERRUN ) {
		return (SD_STREAM_WRITE_OVERRUN);
	}

	if (response_r1 & SD_OCR_CID_CSD_OVERWRIETE ) {
		return (SD_CID_CSD_OVERWRITE);
	}

	if (response_r1 & SD_OCR_WP_ERASE_SKIP ) {
		return (SD_WP_ERASE_SKIP);
	}

	if (response_r1 & SD_OCR_CARD_ECC_DISABLED ) {
		return (SD_CARD_ECC_DISABLED);
	}

	if (response_r1 & SD_OCR_ERASE_RESET ) {
		return (SD_ERASE_RESET);
	}

	if (response_r1 & SD_OCR_AKE_SEQ_ERROR ) {
		return (SD_AKE_SEQ_ERROR);
	}
	return (errorstatus);
}

/**
 * @brief  Checks for error conditions for R3 (OCR) response.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error CmdResp3Error (void)
{
        SD_Error errorstatus = SD_OK;
        uint32_t status;

        status = SDIO ->STA;

        while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT))) {
                status = SDIO ->STA;
        }

        if (status & SDIO_FLAG_CTIMEOUT) {
                errorstatus = SD_CMD_RSP_TIMEOUT;
                SDIO->ICR =  (SDIO_FLAG_CTIMEOUT);
                return (errorstatus);
        }
        /*!< Clear all the static flags */
        SDIO->ICR =  (SDIO_STATIC_FLAGS );
        return (errorstatus);
}

/**
 * @brief  Checks for error conditions for R2 (CID or CSD) response.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error CmdResp2Error (void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;

	status = SDIO ->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND))) {
		status = SDIO ->STA;
	}

	if (status & SDIO_FLAG_CTIMEOUT) {
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO->ICR =  (SDIO_FLAG_CTIMEOUT);
		return (errorstatus);
	} else if (status & SDIO_FLAG_CCRCFAIL) {
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO->ICR =  (SDIO_FLAG_CCRCFAIL);
		return (errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO->ICR =  (SDIO_STATIC_FLAGS );

	return (errorstatus);
}

/**
 * @brief  Checks for error conditions for R6 (RCA) response.
 * @param  cmd: The sent command index.
 * @param  prca: pointer to the variable that will contain the SD card relative
 *         address RCA.
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error CmdResp6Error (uint8_t cmd, uint16_t *prca)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t response_r1;

	status = SDIO ->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND))) {
		status = SDIO ->STA;
	}

	if (status & SDIO_FLAG_CTIMEOUT) {
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO->ICR =  (SDIO_FLAG_CTIMEOUT);
		return (errorstatus);
	} else if (status & SDIO_FLAG_CCRCFAIL) {
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO->ICR =  (SDIO_FLAG_CCRCFAIL);
		return (errorstatus);
	}

	/*!< Check response received is of desired command */
	if (SDIO_GetCommandResponse () != cmd) {
		errorstatus = SD_ILLEGAL_CMD;
		return (errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO->ICR =  (SDIO_STATIC_FLAGS );

	/*!< We have received response, retrieve it.  */
	response_r1 = SDIO_GetResponse (SDIO_RESP1);

	if (SD_ALLZERO == (response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED ))) {
		*prca = (uint16_t) (response_r1 >> 16);
		return (errorstatus);
	}

	if (response_r1 & SD_R6_GENERAL_UNKNOWN_ERROR ) {
		return (SD_GENERAL_UNKNOWN_ERROR);
	}

	if (response_r1 & SD_R6_ILLEGAL_CMD ) {
		return (SD_ILLEGAL_CMD);
	}

	if (response_r1 & SD_R6_COM_CRC_FAILED ) {
		return (SD_COM_CRC_FAILED);
	}

	return (errorstatus);
}

/**
 * @brief  Enables or disables the SDIO wide bus mode.
 * @param  NewState: new state of the SDIO wide bus mode.
 *   This parameter can be: ENABLE or DISABLE.
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error SDEnWideBus (FunctionalState NewState)
{
	SD_Error errorstatus = SD_OK;

	uint32_t scr[2] = { 0, 0 };

	if (SDIO_GetResponse (SDIO_RESP1) & SD_CARD_LOCKED ) {
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return (errorstatus);
	}

	/*!< Get SCR Register */
	errorstatus = FindSCR (RCA, scr);

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	/*!< If wide bus operation to be enabled */
	if (NewState == ENABLE) {
		/*!< If requested card supports wide bus operation */
		if ((scr[1] & SD_WIDE_BUS_SUPPORT )!= SD_ALLZERO) {
			/*!< Send CMD55 APP_CMD with argument as card's RCA.*/
			SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand (&SDIO_CmdInitStructure);

			errorstatus = CmdResp1Error (SD_CMD_APP_CMD );

			if (errorstatus != SD_OK) {
				return (errorstatus);
			}

			/*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
			SDIO_CmdInitStructure.SDIO_Argument = 0x2;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand (&SDIO_CmdInitStructure);

			errorstatus = CmdResp1Error (SD_CMD_APP_SD_SET_BUSWIDTH );

			if (errorstatus != SD_OK) {
				return (errorstatus);
			}
			return (errorstatus);
		} else {
			errorstatus = SD_REQUEST_NOT_APPLICABLE;
			return (errorstatus);
		}
	} else { /*!< If wide bus operation to be disabled */
		/*!< If requested card supports 1 bit mode operation */
		if ((scr[1] & SD_SINGLE_BUS_SUPPORT )!= SD_ALLZERO) {
			/*!< Send CMD55 APP_CMD with argument as card's RCA.*/
			SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand (&SDIO_CmdInitStructure);

			errorstatus = CmdResp1Error (SD_CMD_APP_CMD );

			if (errorstatus != SD_OK) {
				return (errorstatus);
			}

			/*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
			SDIO_CmdInitStructure.SDIO_Argument = 0x00;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand (&SDIO_CmdInitStructure);

			errorstatus = CmdResp1Error (SD_CMD_APP_SD_SET_BUSWIDTH );

			if (errorstatus != SD_OK) {
				return (errorstatus);
			}

			return (errorstatus);
		} else {
			errorstatus = SD_REQUEST_NOT_APPLICABLE;
			return (errorstatus);
		}
	}
}

/**
 * @brief  Checks if the SD card is in programming state.
 * @param  pstatus: pointer to the variable that will contain the SD card state.
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error IsCardProgramming (uint8_t *pstatus)
{
	SD_Error errorstatus = SD_OK;
	__IO uint32_t respR1 = 0, status = 0;

	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	status = SDIO ->STA;
		while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT))) {
		status = SDIO ->STA;
	}

	if (status & SDIO_FLAG_CTIMEOUT) {
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO->ICR =  (SDIO_FLAG_CTIMEOUT);
		return (errorstatus);
	} else if (status & SDIO_FLAG_CCRCFAIL) {
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO->ICR =  (SDIO_FLAG_CCRCFAIL);
		return (errorstatus);
	}

	status = (uint32_t) SDIO_GetCommandResponse ();

	/*!< Check response received is of desired command */
	if (status != SD_CMD_SEND_STATUS ) {
		errorstatus = SD_ILLEGAL_CMD;
		return (errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO->ICR =  (SDIO_STATIC_FLAGS );

	/*!< We have received response, retrieve it for analysis  */
	respR1 = SDIO_GetResponse (SDIO_RESP1);

	/*!< Find out card status */
	*pstatus = (uint8_t) ((respR1 >> 9) & 0x0000000F);

	if ((respR1 & SD_OCR_ERRORBITS )== SD_ALLZERO) {
		return (errorstatus);
	}

	if (respR1 & SD_OCR_ADDR_OUT_OF_RANGE ) {
		return (SD_ADDR_OUT_OF_RANGE);
	}

	if (respR1 & SD_OCR_ADDR_MISALIGNED ) {
		return (SD_ADDR_MISALIGNED);
	}

	if (respR1 & SD_OCR_BLOCK_LEN_ERR ) {
		return (SD_BLOCK_LEN_ERR);
	}

	if (respR1 & SD_OCR_ERASE_SEQ_ERR ) {
		return (SD_ERASE_SEQ_ERR);
	}

	if (respR1 & SD_OCR_BAD_ERASE_PARAM ) {
		return (SD_BAD_ERASE_PARAM);
	}

	if (respR1 & SD_OCR_WRITE_PROT_VIOLATION ) {
		return (SD_WRITE_PROT_VIOLATION);
	}

	if (respR1 & SD_OCR_LOCK_UNLOCK_FAILED ) {
		return (SD_LOCK_UNLOCK_FAILED);
	}

	if (respR1 & SD_OCR_COM_CRC_FAILED ) {
		return (SD_COM_CRC_FAILED);
	}

	if (respR1 & SD_OCR_ILLEGAL_CMD ) {
		return (SD_ILLEGAL_CMD);
	}

	if (respR1 & SD_OCR_CARD_ECC_FAILED ) {
		return (SD_CARD_ECC_FAILED);
	}

	if (respR1 & SD_OCR_CC_ERROR ) {
		return (SD_CC_ERROR);
	}

	if (respR1 & SD_OCR_GENERAL_UNKNOWN_ERROR ) {
		return (SD_GENERAL_UNKNOWN_ERROR);
	}

	if (respR1 & SD_OCR_STREAM_READ_UNDERRUN ) {
		return (SD_STREAM_READ_UNDERRUN);
	}

	if (respR1 & SD_OCR_STREAM_WRITE_OVERRUN ) {
		return (SD_STREAM_WRITE_OVERRUN);
	}

	if (respR1 & SD_OCR_CID_CSD_OVERWRIETE ) {
		return (SD_CID_CSD_OVERWRITE);
	}

	if (respR1 & SD_OCR_WP_ERASE_SKIP ) {
		return (SD_WP_ERASE_SKIP);
	}

	if (respR1 & SD_OCR_CARD_ECC_DISABLED ) {
		return (SD_CARD_ECC_DISABLED);
	}

	if (respR1 & SD_OCR_ERASE_RESET ) {
		return (SD_ERASE_RESET);
	}

	if (respR1 & SD_OCR_AKE_SEQ_ERROR ) {
		return (SD_AKE_SEQ_ERROR);
	}

	return (errorstatus);
}

/**
 * @brief  Find the SD card SCR register value.
 * @param  rca: selected card address.
 * @param  pscr: pointer to the buffer that will contain the SCR value.
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error FindSCR (uint16_t rca, uint32_t *pscr)
{
	uint32_t index = 0;
	SD_Error errorstatus = SD_OK;
	uint32_t tempscr[2] = { 0, 0 };

	/*!< Set Block Size To 8 Bytes */
	/*!< Send CMD55 APP_CMD with argument as card's RCA */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) 8;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_SET_BLOCKLEN );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	/*!< Send CMD55 APP_CMD with argument as card's RCA */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_APP_CMD );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = 8;
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig (&SDIO_DataInitStructure);

	/*!< Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
	SDIO_CmdInitStructure.SDIO_Argument = 0x0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_SEND_SCR;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand (&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error (SD_CMD_SD_APP_SEND_SCR );

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	while (!(SDIO ->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR))) {
		if (SDIO_GetFlagStatus (SDIO_FLAG_RXDAVL) != RESET) {
			*(tempscr + index) = SDIO_ReadData ();
			index++;
		}
	}

	if (SDIO_GetFlagStatus (SDIO_FLAG_DTIMEOUT) != RESET) {
		SDIO->ICR =  (SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return (errorstatus);
	} else if (SDIO_GetFlagStatus (SDIO_FLAG_DCRCFAIL) != RESET) {
		SDIO->ICR =  (SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return (errorstatus);
	} else if (SDIO_GetFlagStatus (SDIO_FLAG_RXOVERR) != RESET) {
		SDIO->ICR =  (SDIO_FLAG_RXOVERR);
		errorstatus = SD_RX_OVERRUN;
		return (errorstatus);
	} else if (SDIO_GetFlagStatus (SDIO_FLAG_STBITERR) != RESET) {
		SDIO->ICR =  (SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return (errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO->ICR =  (SDIO_STATIC_FLAGS );

	*(pscr + 1) = ((tempscr[0] & SD_0TO7BITS )<< 24)|((tempscr[0] & SD_8TO15BITS )<< 8)|((tempscr[0] & SD_16TO23BITS )>> 8)|((tempscr[0] & SD_24TO31BITS )>> 24);

	*(pscr) = ((tempscr[1] & SD_0TO7BITS )<< 24)|((tempscr[1] & SD_8TO15BITS )<< 8)|((tempscr[1] & SD_16TO23BITS )>> 8)|((tempscr[1] & SD_24TO31BITS )>> 24);

	return (errorstatus);
}

/**
 * @brief  Converts the number of bytes in power of two and returns the power.
 * @param  NumberOfBytes: number of bytes.
 * @retval None
 */
uint8_t convert_from_bytes_to_power_of_two (uint16_t NumberOfBytes)
{
	uint8_t count = 0;

	while (NumberOfBytes != 1) {
		NumberOfBytes >>= 1;
		count++;
	}
	return (count);
}

/**
 * @brief  Switch mode High-Speed
 * @note   This function must be used after "Transfer State"
 * @note   This operation should be followed by the configuration
 *         of PLL to have SDIOCK clock between 67 and 75 MHz
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SD_HighSpeed (void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t scr[2] = { 0, 0 };
	uint32_t SD_SPEC = 0;
	uint8_t hs[64] = { 0 };
	uint32_t count = 0, *tempbuff = (uint32_t *) hs;
	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 0;

	SDIO->DCTRL = 0x0;

	/*!< Get SCR Register */
	errorstatus = FindSCR (RCA, scr);

	if (errorstatus != SD_OK) {
		return (errorstatus);
	}

	/* Test the Version supported by the card*/
	SD_SPEC = (scr[1] & 0x01000000) || (scr[1] & 0x02000000);

	if (SD_SPEC != SD_ALLZERO ) {
		/* Set Block Size for Card */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) 64;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand (&SDIO_CmdInitStructure);

		errorstatus = CmdResp1Error (SD_CMD_SET_BLOCKLEN);

		if (errorstatus != SD_OK) {
			return (errorstatus);
		}

		SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
		SDIO_DataInitStructure.SDIO_DataLength = 64;
		SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_64b;
		SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
		SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
		SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
		SDIO_DataConfig (&SDIO_DataInitStructure);

		/*!< Send CMD6 switch mode */
		SDIO_CmdInitStructure.SDIO_Argument = 0x80FFFF01;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_HS_SWITCH;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand (&SDIO_CmdInitStructure);
		errorstatus = CmdResp1Error (SD_CMD_HS_SWITCH );

		if (errorstatus != SD_OK) {
			return (errorstatus);
		}

		while (!(SDIO ->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR))) {
			if (SDIO_GetFlagStatus (SDIO_FLAG_RXFIFOHF) != RESET) {
				for (count = 0; count < 8; count++) {
					*(tempbuff + count) = SDIO_ReadData ();
				}
				tempbuff += 8;
			}
		}

		if (SDIO_GetFlagStatus (SDIO_FLAG_DTIMEOUT) != RESET) {
			SDIO->ICR =  (SDIO_FLAG_DTIMEOUT);
			errorstatus = SD_DATA_TIMEOUT;
			return (errorstatus);
		} else if (SDIO_GetFlagStatus (SDIO_FLAG_DCRCFAIL) != RESET) {
			SDIO->ICR =  (SDIO_FLAG_DCRCFAIL);
			errorstatus = SD_DATA_CRC_FAIL;
			return (errorstatus);
		} else if (SDIO_GetFlagStatus (SDIO_FLAG_RXOVERR) != RESET) {
			SDIO->ICR =  (SDIO_FLAG_RXOVERR);
			errorstatus = SD_RX_OVERRUN;
			return (errorstatus);
		} else if (SDIO_GetFlagStatus (SDIO_FLAG_STBITERR) != RESET) {
			SDIO->ICR =  (SDIO_FLAG_STBITERR);
			errorstatus = SD_START_BIT_ERR;
			return (errorstatus);
		}
		count = SD_DATATIMEOUT;
		while ((SDIO_GetFlagStatus (SDIO_FLAG_RXDAVL) != RESET) && (count > 0)) {
			*tempbuff = SDIO_ReadData ();
			tempbuff++;
			count--;
		}

		/*!< Clear all the static flags */
		SDIO->ICR =  (SDIO_STATIC_FLAGS );

		/* Test if the switch mode HS is ok */
		if ((hs[13] & 0x2) == 0x2) {
			errorstatus = SD_OK;
		} else {
			errorstatus = SD_UNSUPPORTED_FEATURE;
		}
	}
	return (errorstatus);
}

/**
 ******************************************************************************
 * @author  MCD Application Team (modified by lukasz.iwaszkiewicz@gmail.com)
 * @version V1.0.3
 * @date    13-November-2013
 * @brief   This file provides
 *            - set of firmware functions to manage Leds, push-button and COM ports
 *            - low level initialization functions for SD card (on SDIO) and EEPROM
 *          available on STM324x9I-EVAL evaluation board(MB1045) RevB from
 *          STMicroelectronics.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
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

/**
 * @brief  DeInitializes the SDIO interface.
 * @param  None
 * @retval None
 */
void SD_LowLevel_DeInit(void) {
	/*!< Disable SDIO Clock */
	SDIO_ClockCmd(DISABLE);

	/*!< Set Power State to OFF */
	SDIO_SetPowerState(SDIO_PowerState_OFF);

	/*!< DeInitializes the SDIO peripheral */
	SDIO_DeInit();

	/* Disable the SDIO APB2 Clock */
	RCC->APB2ENR &= ~RCC_APB2ENR_SDIOEN;

#if FATFS_SDIO_4BIT == 1
	TM_GPIO_DeInit(GPIOC, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
	TM_GPIO_SetPullResistor(GPIOC, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12, TM_GPIO_PuPd_DOWN);
#else
	TM_GPIO_DeInit(GPIOC, GPIO_PIN_8 | GPIO_PIN_12);
	TM_GPIO_SetPullResistor(GPIOC, GPIO_PIN_8 | GPIO_PIN_12, TM_GPIO_PuPd_DOWN);
#endif

	TM_GPIO_DeInit(GPIOD, GPIO_PIN_2);
	TM_GPIO_SetPullResistor(GPIOD, GPIO_PIN_2, TM_GPIO_PuPd_DOWN);
}

/**
 * @brief  Initializes the SD Card and put it into StandBy State (Ready for
 *         data transfer).
 * @param  None
 * @retval None
 */
void SD_LowLevel_Init (void) {
#if FATFS_SDIO_4BIT == 1
	TM_GPIO_InitAlternate(GPIOC, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Fast, GPIO_AF_SDIO);
#else
	TM_GPIO_InitAlternate(GPIOC, GPIO_PIN_8 | GPIO_PIN_12, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Fast, GPIO_AF_SDIO);
#endif

	TM_GPIO_InitAlternate(GPIOD, GPIO_PIN_2, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Fast, GPIO_AF_SDIO);

	/* Enable the SDIO APB2 Clock */
	RCC->APB2ENR |= RCC_APB2ENR_SDIOEN;

	/* Enable the DMA2 Clock */
	RCC->AHB1ENR |= SD_SDIO_DMA_CLK;
}

/**
 * @brief  Configures the DMA2 Channel4 for SDIO Tx request.
 * @param  BufferSRC: pointer to the source buffer
 * @param  BufferSize: buffer size
 * @retval None
 */
void SD_LowLevel_DMA_TxConfig (uint32_t *BufferSRC, uint32_t BufferSize) {
	DMA_InitTypeDef SDDMA_InitStructure;

	DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF | SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);

	/* DMA2 Stream3  or Stream6 disable */
	DMA_Cmd(SD_SDIO_DMA_STREAM, DISABLE);

	/* DMA2 Stream3  or Stream6 Config */
	DMA_DeInit(SD_SDIO_DMA_STREAM );

	SDDMA_InitStructure.DMA_Channel = SD_SDIO_DMA_CHANNEL;
	SDDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) SDIO_FIFO_ADDRESS;
	SDDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) BufferSRC;
	SDDMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	SDDMA_InitStructure.DMA_BufferSize = 1;
	SDDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	SDDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	SDDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	SDDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; /* DMA_MemoryDataSize_Word; */
	SDDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	SDDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	SDDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	SDDMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull; /* DMA_FIFOThreshold_Full */
	SDDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single; /* DMA_MemoryBurst_INC4 */
	SDDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
	DMA_Init (SD_SDIO_DMA_STREAM, &SDDMA_InitStructure);
	DMA_ITConfig (SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
	DMA_FlowControllerConfig (SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);

	/* DMA2 Stream3  or Stream6 enable */
	DMA_Cmd(SD_SDIO_DMA_STREAM, ENABLE);
}
/**
 * @brief  Configures the DMA2 Channel4 for SDIO Rx request.
 * @param  BufferDST: pointer to the destination buffer
 * @param  BufferSize: buffer size
 * @retval None
 */
void SD_LowLevel_DMA_RxConfig (uint32_t *BufferDST, uint32_t BufferSize)
{
	DMA_InitTypeDef SDDMA_InitStructure;

	DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF | SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);

	/* DMA2 Stream3  or Stream6 disable */
	DMA_Cmd(SD_SDIO_DMA_STREAM, DISABLE);

	/* DMA2 Stream3 or Stream6 Config */
	DMA_DeInit(SD_SDIO_DMA_STREAM);

	SDDMA_InitStructure.DMA_Channel = SD_SDIO_DMA_CHANNEL;
	SDDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) SDIO_FIFO_ADDRESS;
	SDDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) BufferDST;
	SDDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	SDDMA_InitStructure.DMA_BufferSize = 1;
	SDDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	SDDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	SDDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	SDDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	SDDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	SDDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	SDDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	SDDMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	SDDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
	SDDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
	DMA_Init (SD_SDIO_DMA_STREAM, &SDDMA_InitStructure);
	DMA_ITConfig (SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
	DMA_FlowControllerConfig (SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);

	/* DMA2 Stream3 or Stream6 enable */
	DMA_Cmd(SD_SDIO_DMA_STREAM, ENABLE);
}
