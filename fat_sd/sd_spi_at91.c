/*-----------------------------------------------------------------------*/
/* MMC/SDSC/SDHC (in SPI mode) control module for AT91SAM7               */
/* (C) Martin Thomas, 2009 - based on the AVR module (C)ChaN, 2007       */
/*-----------------------------------------------------------------------*/

/* Copyright (c) 2009, Martin Thomas, ChaN
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

/*-----------------------------------------------------------------------*/
/*
 * Martin Thomas 9/2009
 * - modified xmit_datablock_dma
 *
 * Martin Thomas 5/2009
 * - based on my older driver from 2006
 * - using ChaN's new driver-skeleton based on his AVR mmc.c
 * - adapted for Olimex SAM7-Pxxx (with AT91SAM7S256)
 *   Hardware SPI, CS @ NCPS0
 * - TODO: disable internal pull-up when external pull-up/downs are installed
 */
/*-----------------------------------------------------------------------*/

#define USE_BOARD_H       1
#define USE_DMA           1
#define USE_DMA_DUMMY_RAM 0
#define USE_SOCKSWITCHES  1
#define USE_POWERCONTROL  0

#if USE_BOARD_H
#include <board.h>
#else /* !USE_BOARD_H */
#include <at91sam7se512/AT91SAM7S256.h>
#endif /* USE_BOARD_H */

#include "diskio.h"
#include "taskUtil.h"

/* Definitions for MMC/SDC command */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define	ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD9	(0x40+9)	/* SEND_CSD */
#define CMD10	(0x40+10)	/* SEND_CID */
#define CMD12	(0x40+12)	/* STOP_TRANSMISSION */
#define ACMD13	(0xC0+13)	/* SD_STATUS (SDC) */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD18	(0x40+18)	/* READ_MULTIPLE_BLOCK */
#define CMD23	(0x40+23)	/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0xC0+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD25	(0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */


/*--------------------------------------------------------------------------

   DMA/PDC - support ( Martin Thomas )

---------------------------------------------------------------------------*/
/* Dummy-Array to kick SPI-transfers thru PDC for block read.
   If the array is placed in flash (with const) the transfer
   is a little slower but this saves 512 bytes of RAM.
   Define #USE_DUMMY_RAM with 1 to place this array in RAM.
 */
#if USE_DMA
#define DMA_DUMMY_SIZE 512
#if USE_DMA_DUMMY_RAM
static
#else
static const
#endif /* USE_DMA_DUMMY_RAM */
BYTE dma_dummy[DMA_DUMMY_SIZE] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
#endif /* USE_DMA */



/*--------------------------------------------------------------------------

   Module private portability Functions & Definitions ( Martin Thomas )

---------------------------------------------------------------------------*/

#define PPIO_BASE_SPI         AT91C_BASE_PIOA
#define PSPI_BASE             AT91C_BASE_SPI
#define SPI_SCBR_MIN          2 /* 1 failed with my cards */
// Chip-Select-Register Number (Array Index of SPI_CSR[])
// here: NPCS0
#define SPI_CSR_NUM           0

#define PMC_ID_CS             AT91C_ID_PIOA
#define PPIO_BASE_CS          AT91C_BASE_PIOA
#define CARD_SELECT_PIN       AT91C_PA11_NPCS0

// select card ( MMC CS = L )
static inline void SELECT( void )
{
	PPIO_BASE_CS->PIO_CODR = CARD_SELECT_PIN;
}

// unselect card MMC CS = H
static inline void DESELECT( void )
{
	PPIO_BASE_CS->PIO_SODR = CARD_SELECT_PIN;
}

#define SOCKWP		0x20			/* Write protect bit-mask (Bit5 set = */
#define SOCKINS		0x10			/* Card detect bit-mask   */

#if USE_SOCKSWITCHES
#ifndef SD_SOCKET_WP_PIN
/// Pin connected to the Write-Protect WP switch of the card socket
#define SD_SOCKET_WP_PIN      AT91C_PIO_PA28
#endif
#ifndef SD_SOCKET_INS_PIN
/// Pin connected to the Insert/Card Detect switch of the card socket
#define SD_SOCKET_INS_PIN     AT91C_PIO_PA29
#endif

#define PIOA AT91C_BASE_PIOA

static inline BYTE get_SOCKWP(void)
{
	return ( PIOA->PIO_PDSR & SD_SOCKET_WP_PIN ) ? SOCKWP /*protected*/ : 0 /* unprotected */;
}
static inline BYTE get_SOCKINS(void)
{
	return ( PIOA->PIO_PDSR & SD_SOCKET_INS_PIN ) ?  SOCKINS /*no card*/ : 0 /* card inserted */;
}

static void init_SOCKWP_SOCKINS( void )
{
	/* set Pin as Input, no internal pulls, glitch detection */
	AT91C_BASE_PMC->PMC_PCER = ( 1 << PMC_ID_CS ); // enable needed PIO in PMC
	PIOA->PIO_PPUER = SD_SOCKET_WP_PIN; // enable pull-up
	PIOA->PIO_IFER  = SD_SOCKET_WP_PIN; // enable filter
	PIOA->PIO_ODR   = SD_SOCKET_WP_PIN; // disable output
	PIOA->PIO_SODR  = SD_SOCKET_WP_PIN; // set high
	PIOA->PIO_PER   = SD_SOCKET_WP_PIN; // enable
	PIOA->PIO_PPUDR = SD_SOCKET_INS_PIN; // enable pull-up
	PIOA->PIO_IFER  = SD_SOCKET_INS_PIN; // enable filter
	PIOA->PIO_ODR   = SD_SOCKET_INS_PIN; // disable output
	PIOA->PIO_SODR  = SD_SOCKET_INS_PIN; // set high
	PIOA->PIO_PER   = SD_SOCKET_INS_PIN; // enable
}
#else
static inline BYTE get_SOCKWP(void)
{
	return 0; /* fake unprotected */
}
static inline BYTE get_SOCKINS(void)
{
	return 0; /* fake inserted */
}
static void init_SOCKWP_SOCKINS( void )
{
	return;
}
#endif /* USE_SOCKSWITCHES */

static void AT91_spiSetSpeed(BYTE speed)
{
	DWORD reg;

	if ( speed < SPI_SCBR_MIN ) speed = SPI_SCBR_MIN;
	if ( speed > 1 ) speed &= 0xFE;

	reg = PSPI_BASE->SPI_CSR[SPI_CSR_NUM];
	reg = ( reg & ~(AT91C_SPI_SCBR) ) | ( (DWORD)speed << 8 );
	PSPI_BASE->SPI_CSR[SPI_CSR_NUM] = reg;
}

/* general AT91 SPI send/receive */
static inline BYTE AT91_spi_write_read( BYTE outgoing )
{
	BYTE incoming;

	while( !( PSPI_BASE->SPI_SR & AT91C_SPI_TDRE ) ); // transfer complete wait
	PSPI_BASE->SPI_TDR = (WORD)( outgoing );
	while( !( PSPI_BASE->SPI_SR & AT91C_SPI_RDRF ) ); // wait for char
	incoming = (BYTE)( PSPI_BASE->SPI_RDR );

	return incoming;
}

#if USE_DMA

static inline void rcvr_block_dma (
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be multiple of 4) */
)
{
	volatile DWORD dummy;

	PSPI_BASE->SPI_RPR = (DWORD)buff; // destination address
	PSPI_BASE->SPI_RCR = btr; // number of frames (here: frame=byte)
	// SPI PDC TX buffer (dummy bytes):
	PSPI_BASE->SPI_TPR = (DWORD)dma_dummy; // source address
	PSPI_BASE->SPI_TCR = btr; // number of frames (here: frame=byte)
	// enable PDC TX and RX
	PSPI_BASE->SPI_PTCR = AT91C_PDC_TXTEN | AT91C_PDC_RXTEN;
	while( !( (PSPI_BASE->SPI_SR) & AT91C_SPI_RXBUFF ) ) {
		// wait for RX Buffer Full (counters 0)
	}
	dummy = PSPI_BASE->SPI_SR;   // read status to clear flags
	// disable PDC TX and RX
	PSPI_BASE->SPI_PTCR = AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS;
}


#define XMIT_TEST 2

#if (XMIT_TEST==1)

// TODO: temporary workaround since RCR=0, end RXTDIS did not
// work reliably. Find out why writes may fail if PDC receive is
// disabled and RCR is set to 0.
static BYTE xmit_rec_dummy[512];

static inline void xmit_datablock_dma (
	const BYTE *buff	/* 512 byte data block to be transmitted */
)
{
	volatile DWORD dummy;

	// Dummy Receive-Buffering
	PSPI_BASE->SPI_RPR  = (DWORD)xmit_rec_dummy;
	PSPI_BASE->SPI_RCR  = 512;
	// SPI PDC TX buffer
	PSPI_BASE->SPI_TPR = (DWORD)buff;  // source address
	PSPI_BASE->SPI_TCR = 512;          // number of frames
	// Transmitter transfer enable
	PSPI_BASE->SPI_PTCR = AT91C_PDC_TXTEN | AT91C_PDC_RXTEN;
	while( !( (PSPI_BASE->SPI_SR) & AT91C_SPI_RXBUFF ) ) {
			// wait for RX Buffer Full (counters 0)
	}
	dummy = PSPI_BASE->SPI_SR;   // read status to clear flags
	// Transmitter transfer disable
	PSPI_BASE->SPI_PTCR = AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS;
}

#elif (XMIT_TEST==2)
// seems to work - of it does not work for you select XMIT_TEST 1
// The following may be full of paranoia checks, but a few tests worked as
// expected and I will not change this before more tests can be done.
static inline void xmit_datablock_dma (
	const BYTE *buff	/* 512 byte data block to be transmitted */
)
{
	volatile DWORD dummy;

	// SPI PDC TX buffer
	PSPI_BASE->SPI_RPR   = (DWORD)&dummy;
	PSPI_BASE->SPI_RPR   = 0;
	PSPI_BASE->SPI_RCR   = 0;
	PSPI_BASE->SPI_RNCR  = 0;
	PSPI_BASE->SPI_TPR   = (DWORD)buff;  // source address
	PSPI_BASE->SPI_TCR   = 512;          // number of frames
	// Transmitter transfer enable
	PSPI_BASE->SPI_PTCR = AT91C_PDC_TXTEN;
	while( PSPI_BASE->SPI_TCR ) {
		// wait for transmit PDC counter 0
	}
	while( !( (PSPI_BASE->SPI_SR) & AT91C_SPI_TXEMPTY ) ) {
		// wait for TDR and shifter empty
	}
	// "When the received data is read, the RDRF bit is cleared."
	dummy = PSPI_BASE->SPI_RDR;
	// "The user has to read the status register to clear the OVRES bit."
	dummy = PSPI_BASE->SPI_SR;
	// Transmitter transfer disable
	PSPI_BASE->SPI_PTCR = AT91C_PDC_TXTDIS;
}
#else
#error "unknown xmit selection"
#endif


#endif /* USE_DMA */

/*--------------------------------------------------------------------------

   Module Private Functions ( ChaN )

---------------------------------------------------------------------------*/

static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
BYTE Timer1, Timer2;	/* 100Hz decrement timer */

static
BYTE CardType;			/* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */


/*-----------------------------------------------------------------------*/
/* Init SPI-Interface (Platform dependent)                               */
/*-----------------------------------------------------------------------*/
static void init_spi( void )
{
	// set SPI pins to the peripheral function
	PPIO_BASE_SPI->PIO_ASR = AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK;
	// disable PIO from controlling the pins (so they are used for peripheral)
	PPIO_BASE_SPI->PIO_PDR = AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK;

	/* set chip-select as output high (unselect card) */
	/* manual control for CS !                        */
	AT91C_BASE_PMC->PMC_PCER = ( 1 << PMC_ID_CS ); // enable needed PIO in PMC
	PPIO_BASE_CS->PIO_PER  = CARD_SELECT_PIN;  // enable GPIO of CS-pin (disable peripheral functions)
	PPIO_BASE_CS->PIO_SODR = CARD_SELECT_PIN;  // set high
	PPIO_BASE_CS->PIO_OER  = CARD_SELECT_PIN;  // output enable

	// enable peripheral clock for SPI ( PID Bit 5 )
	AT91C_BASE_PMC->PMC_PCER = ( 1 << AT91C_ID_SPI ); // n.b. IDs are just bit-numbers

	// SPI disable
	PSPI_BASE->SPI_CR = AT91C_SPI_SPIDIS;

#if USE_DMA
	// init the SPI's PDC-controller:
	// disable PDC TX and RX
	PSPI_BASE->SPI_PTCR = AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS;
	// init counters and buffer-pointers to 0
	// "next" TX
	PSPI_BASE->SPI_TNPR = 0;
	PSPI_BASE->SPI_TNCR = 0;
	// "next" RX
	PSPI_BASE->SPI_RNPR = 0;
	PSPI_BASE->SPI_RNCR = 0;
	// TX
	PSPI_BASE->SPI_TPR = 0;
	PSPI_BASE->SPI_TCR = 0;
	// RX
	PSPI_BASE->SPI_RPR = 0;
	PSPI_BASE->SPI_RCR = 0;
#endif /* USE_DMA */

	// SPI enable and reset
	// "It seems that the state machine for revB version needs to have 2 SPI
	// software reset to properly reset the state machine."
	PSPI_BASE->SPI_CR = AT91C_SPI_SWRST;
	PSPI_BASE->SPI_CR = AT91C_SPI_SWRST;
	PSPI_BASE->SPI_CR = AT91C_SPI_SPIEN;

	// SPI mode: master, FDIV=0, fault detection disabled
	PSPI_BASE->SPI_MR  = AT91C_SPI_MSTR | AT91C_SPI_MODFDIS;

	// set chip-select-register
	// 8 bits per transfer, CPOL=1, ClockPhase=0, DLYBCT = 0
	PSPI_BASE->SPI_CSR[SPI_CSR_NUM] = AT91C_SPI_CPOL | AT91C_SPI_BITS_8;

	// slow during init
	AT91_spiSetSpeed(0xFE);

	// enable SPI
	PSPI_BASE->SPI_CR = AT91C_SPI_SPIEN;
}

static void close_spi( void )
{
	// enable PIO-control for Pins
	PPIO_BASE_SPI->PIO_PER = AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK;
	// disable PDC TX and RX
	PSPI_BASE->SPI_PTCR = AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS;
	// disable SPI
	PSPI_BASE->SPI_CR = AT91C_SPI_SPIDIS;
	// disable peripheral clock
	AT91C_BASE_PMC->PMC_PCDR = ( 1 << AT91C_ID_SPI );
}



/*-----------------------------------------------------------------------*/
/* Transmit a byte to MMC via SPI  (Platform dependent)                  */
/*-----------------------------------------------------------------------*/

static inline void xmit_spi( BYTE dat )
{
	AT91_spi_write_read( dat );
}



/*-----------------------------------------------------------------------*/
/* Receive a byte from MMC via SPI  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/

static inline BYTE rcvr_spi (void)
{
	return AT91_spi_write_read( 0xff );
}

/* replacement for the AVR-Macro */
static inline void rcvr_spi_m( BYTE *dst )
{
	*dst = rcvr_spi();
}



/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
BYTE wait_ready (void)
{
	BYTE res;

	portTickType start = xTaskGetTickCount();

	rcvr_spi();
	do
		res = rcvr_spi();
	while ((res != 0xFF) && !isTimeoutMs(start,500));

	return res;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void release_spi (void)
{
	DESELECT();
	rcvr_spi();
}



/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

static
void power_on (void)
{
	init_SOCKWP_SOCKINS();      /* Init socket-switches */
	init_spi();
}

static
void power_off (void)
{
	SELECT();				/* Wait for card ready */
	wait_ready();
	close_spi();
	Stat |= STA_NOINIT;		/* Set STA_NOINIT */
}

static
int chk_power(void)		/* Socket power state: 0=off, 1=on */
{
	return 1;
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
BOOL rcvr_datablock (
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be multiple of 4) */
)
{
	BYTE token;



	portTickType timer1Start = xTaskGetTickCount();
	do {							/* Wait for data packet in timeout of 100ms */
		token = rcvr_spi();
	} while ((token == 0xFF) && ! isTimeoutMs(timer1Start,100));
	if(token != 0xFE) return FALSE;	/* If not valid data token, return with error */

#if USE_DMA
	rcvr_block_dma( buff, btr );
#else
	do {							/* Receive the data block into buffer */
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
	} while (btr -= 4);
#endif /* USE_DMA */
	rcvr_spi();						/* Discard CRC */
	rcvr_spi();

	return TRUE;					/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
static
BOOL xmit_datablock (
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data/Stop token */
)
{
	BYTE resp, wc;


	if (wait_ready() != 0xFF) return FALSE;

	xmit_spi(token);					/* Xmit data token */
	if (token != 0xFD) {	/* Is data token */
		wc = 0;
#if USE_DMA
		xmit_datablock_dma( buff );
#else
		do {							/* Xmit the 512 byte data block to MMC */
			xmit_spi(*buff++);
			xmit_spi(*buff++);
		} while (--wc);
#endif /* USE_DMA */
		xmit_spi(0xFF);					/* CRC (Dummy) */
		xmit_spi(0xFF);
		resp = rcvr_spi();				/* Receive data response */
		if ((resp & 0x1F) != 0x05)		/* If not accepted, return with error */
			return FALSE;
	}

	return TRUE;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequence of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	DESELECT();
	SELECT();
	if (wait_ready() != 0xFF) return 0xFF;

	/* Send command packet */
	xmit_spi(cmd);						/* Start + Command index */
	xmit_spi((BYTE)(arg >> 24));		/* Argument[31..24] */
	xmit_spi((BYTE)(arg >> 16));		/* Argument[23..16] */
	xmit_spi((BYTE)(arg >> 8));			/* Argument[15..8] */
	xmit_spi((BYTE)arg);				/* Argument[7..0] */
	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive command response */
	if (cmd == CMD12) rcvr_spi();		/* Skip a stuff byte when stop reading */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do
		res = rcvr_spi();
	while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive number (0) */
)
{
	BYTE n, cmd, ty, ocr[4];

#if 0
	static BYTE pv;
	pv = get_SOCKWP() | get_SOCKINS();
	if (pv & SOCKINS) /* INS = H (Socket empty) */
		xprintf("not inserted\n");
	else {
		/* INS = L (Card inserted) */
		xprintf("inserted\n");
		if (pv & SOCKWP) /* WP is H (write protected) */
			xprintf("protected\n");
		else
			/* WP is L (write enabled) */
			xprintf("unprotected\n");
	}
#endif

	if (drv) return STA_NOINIT;			/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	power_on();							/* Force socket power on */
	for (n = 10; n; n--) rcvr_spi();	/* 80 dummy clocks */

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		portTickType timer1Start = xTaskGetTickCount();
		Timer1 = 100;						/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDHC */
			for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();		/* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at vdd range of 2.7-3.6V */
				while (!isTimeoutMs(timer1Start,1000) && send_cmd(ACMD41, 1UL << 30));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (!isTimeoutMs(timer1Start,1000) && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			}
		} else {							/* SDSC or MMC */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDSC */
			} else {
				ty = CT_MMC; cmd = CMD1;		/* MMC */
			}
			while (!isTimeoutMs(timer1Start,1000) && send_cmd(cmd, 0));			/* Wait for leaving idle state */
			if (isTimeoutMs(timer1Start,1000) || send_cmd(CMD16, 512) != 0)	/* Set R/W block length to 512 */
				ty = 0;
		}
	}
	CardType = ty;
	release_spi();

	if (ty) {			/* Initialization succeeded */
		Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */
		AT91_spiSetSpeed(SPI_SCBR_MIN);
	} else {			/* Initialization failed */
		power_off();
	}

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive number (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,			/* Physical drive number (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block read */
		if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512))
			count = 0;
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY == 0 */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL != 0
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive number (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	WORD csize;


	if (drv) return RES_PARERR;

	res = RES_ERROR;

	if (ctrl == CTRL_POWER) {
		switch (*ptr) {
		case 0:		/* Sub control code == 0 (POWER_OFF) */
			if (chk_power())
				power_off();		/* Power off */
			res = RES_OK;
			break;
		case 1:		/* Sub control code == 1 (POWER_ON) */
			power_on();				/* Power on */
			res = RES_OK;
			break;
		case 2:		/* Sub control code == 2 (POWER_GET) */
			*(ptr+1) = (BYTE)chk_power();
			res = RES_OK;
			break;
		default :
			res = RES_PARERR;
		}
	}
	else {
		if (Stat & STA_NOINIT) return RES_NOTRDY;

		switch (ctrl) {
		case CTRL_SYNC :		/* Make sure that no pending write process */
			SELECT();
			if (wait_ready() == 0xFF)
				res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
					csize = csd[9] + ((WORD)csd[8] << 8) + 1;
					*(DWORD*)buff = (DWORD)csize << 10;
				} else {					/* SDC ver 1.XX or MMC*/
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = (DWORD)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			if (CardType & CT_SD2) {			/* SDC ver 2.00 */
				if (send_cmd(ACMD13, 0) == 0) {		/* Read SD status */
					rcvr_spi();
					if (rcvr_datablock(csd, 16)) {				/* Read partial block */
						for (n = 64 - 16; n; n--) rcvr_spi();	/* Purge trailing data */
						*(DWORD*)buff = 16UL << (csd[10] >> 4);
						res = RES_OK;
					}
				}
			} else {					/* SDC ver 1.XX or MMC */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
					if (CardType & CT_SD1) {			/* SDC ver 1.XX */
						*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} else {					/* MMC */
						*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
					res = RES_OK;
				}
			}
			break;

		case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
			*ptr = CardType;
			res = RES_OK;
			break;

		case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
			if (send_cmd(CMD9, 0) == 0		/* READ_CSD */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
			if (send_cmd(CMD10, 0) == 0		/* READ_CID */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
				for (n = 4; n; n--) *ptr++ = rcvr_spi();
				res = RES_OK;
			}
			break;

		case MMC_GET_SDSTAT :	/* Receive SD status as a data block (64 bytes) */
			if (send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
				rcvr_spi();
				if (rcvr_datablock(ptr, 64))
					res = RES_OK;
			}
			break;

		default:
			res = RES_PARERR;
		}

		release_spi();
	}

	return res;
}
#endif /* _USE_IOCTL != 0 */


/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */

inline void disk_timerproc (void)
{
	static BYTE pv;
	BYTE n, s;


	n = Timer1;						/* 100Hz decrement timer */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;

	n = pv;
	pv = get_SOCKWP() | get_SOCKINS();
	// pv = ( ( PIOA->PIO_PDSR & SD_SOCKET_WP_PIN ) ? SOCKWP /*protected*/ : 0 /* unprotected */ )
	//	| ( ( PIOA->PIO_PDSR & SD_SOCKET_INS_PIN ) ?  SOCKINS /*no card*/ : 0 /* card inserted */ );

	if (n == pv) {					/* Have contacts stabled? */
		s = Stat;

		if (pv & SOCKWP)			/* WP is H (write protected) */
			s |= STA_PROTECT;
		else						/* WP is L (write enabled) */
			s &= ~STA_PROTECT;

		if (pv & SOCKINS)			/* INS = H (Socket empty) */
			s |= (STA_NODISK | STA_NOINIT);
		else						/* INS = L (Card inserted) */
			s &= ~STA_NODISK;

		Stat = s;
	}
}

