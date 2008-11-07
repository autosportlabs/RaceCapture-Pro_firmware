/*****************************************************************************\
*              efs - General purpose Embedded Filesystem library              *
*          ---------------------------------------------------------          *
*                                                                             *
* Filename : at91_spi.c                                                       *
* Description : This file contains the functions needed to use efs for        *
*               accessing files on an SD-card connected to an AT91(SAM7)      *
*                                                                             *
* This program is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU General Public License                 *
* as published by the Free Software Foundation; version 2                     *
* of the License.                                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
* GNU General Public License for more details.                                *
*                                                                             *
* As a special exception, if other files instantiate templates or             *
* use macros or inline functions from this file, or you compile this          *
* file and link it with other works to produce a work based on this file,     *
* this file does not by itself cause the resulting work to be covered         *
* by the GNU General Public License. However the source code for this         *
* file must still be made available in accordance with section (3) of         *
* the GNU General Public License.                                             *
*                                                                             *
* This exception does not invalidate any other reasons why a work based       *
* on this file might be covered by the GNU General Public License.            *
*                                                                             *
*                                                    (c)2006 Martin Thomas    *
\*****************************************************************************/

/*
TODO: 
- Driver uses simple mode -> implement "DMA"
- use 16-bit mode
- check CPOL
*/

/*****************************************************************************/
#include "interfaces/AT91SAM7S_regs.h"
#include "config.h"
#include "interfaces/arm_spi.h"
#include "interfaces/sd.h"
/*****************************************************************************/

/* Define the following to enable fixed peripheral chip-select.
   This is not a config.h-value since variable peripheral select
   is prefered so other devices can be attached to the AT91 SPI-interface. */
/* #define FIXED_PERIPH */

/*
	AT91SAM7S64 SPI Pins:   Function (A/B)
	PA12 - MISO - AT91C_PA12_MISO  (1/0)
	PA13 - MOSI - AT91C_PA13_MOSI  (1/0)
	PA14 - SCK  - AT91C_PA14_SPCK  (1/0)
	
	Chip-Selects (available on different pins)
	PA11 - NPCS0 - AT91C_PA11_NPCS0 (1/0)
	PA31 - NPCS1 - AT91C_PA31_NPCS1 (1/0)
	PA09 - NPCS1 - AT91C_PA9_NPCS1  (1/0)
	...AT91C_PA3_NPCS3	(0/1)
	...AT91C_PA5_NPCS3	(0/1)
	...AT91C_PA10_NPCS2	(0/1)
	...AT91C_PA22_NPCS3	(0/1)
	...AT91C_PA30_NPCS2 (0/1)
*/

/* here: use NCPS0 @ PA11: */
#define NCPS_PDR_BIT     AT91C_PA11_NPCS0
#define NCPS_ASR_BIT     AT91C_PA11_NPCS0
#define NPCS_BSR_BIT     0
#define SPI_CSR_NUM      0          

/* PCS_0 for NPCS0, PCS_1 for NPCS1 ... */
#define PCS_0 ((0<<0)|(1<<1)|(1<<2)|(1<<3))
#define PCS_1 ((1<<1)|(0<<1)|(1<<2)|(1<<3))
#define PCS_2 ((1<<1)|(1<<1)|(0<<2)|(1<<3))
#define PCS_3 ((1<<1)|(1<<1)|(1<<2)|(0<<3))
/* TODO: ## */
#if (SPI_CSR_NUM == 0)
#define SPI_MR_PCS       PCS_0
#elif (SPI_CSR_NUM == 1)
#define SPI_MR_PCS       PCS_1
#elif (SPI_CSR_NUM == 2)
#define SPI_MR_PCS       PCS_2
#elif (SPI_CSR_NUM == 3)
#define SPI_MR_PCS       PCS_3
#else
#error "SPI_CSR_NUM invalid"
// not realy - when using an external address decoder...
// but this code takes over the complete SPI-interace anyway
#endif

/* in variable periph. select PSDEC=1 is used
   so the already defined values for SPC_MR_PCS can be
   reused */
#define SPI_TDR_PCS      SPI_MR_PCS

/* SPI prescaler lower limit (the smaller the faster) */
#define SPI_SCBR_MIN     2


esint8 if_initInterface(hwInterface* file, eint8* opts)
{
	euint32 sc;
	
	if_spiInit(file);
	
	if(sd_Init(file)<0)	{
		DBG((TXT("Card failed to init, breaking up...\n")));
		return(-1);
	}
	
	if(sd_State(file)<0){
		DBG((TXT("Card didn't return the ready state, breaking up...\n")));
		return(-2);
	}
	
	/* file->sectorCount=4; */ /* FIXME ASAP!! */
	/* mthomas: - somehow done - see below  */
	
	sd_getDriveSize(file, &sc);
	file->sectorCount = sc/512;
	if( (sc%512) != 0) {
		file->sectorCount--;
	}
	DBG((TXT("Card Capacity is %lu Bytes (%lu Sectors)\n"), sc, file->sectorCount));
	
	if_spiSetSpeed(SPI_SCBR_MIN);
	// if_spiSetSpeed(100); /* debug - slower */
	
	return(0);
}

/*****************************************************************************/ 
/* parts of this function inspired by a spi example found at olimex.com
   not much (nothing?) left from the olimex-code (since too many hardcoded values)
*/
void if_spiInit(hwInterface *iface)
{
	euint8 i;
	
	AT91PS_SPI pSPI      = AT91C_BASE_SPI;
	AT91PS_PIO pPIOA     = AT91C_BASE_PIOA;
	AT91PS_PMC pPMC      = AT91C_BASE_PMC;
	// not used: AT91PS_PDC pPDC_SPI  = AT91C_BASE_PDC_SPI;
	
	// disable PIO from controlling MOSI, MISO, SCK (=hand over to SPI)
	// keep CS untouched - used as GPIO pin during init
	pPIOA->PIO_PDR = AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK; //  | NCPS_PDR_BIT;
	// set pin-functions in PIO Controller
	pPIOA->PIO_ASR = AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK; /// not here: | NCPS_ASR_BIT;
	/// not here: pPIOA->PIO_BSR = NPCS_BSR_BIT;
	
	// set chip-select as output high (unselect card)
	pPIOA->PIO_PER  = NCPS_PDR_BIT; // enable GPIO of CS-pin
	pPIOA->PIO_SODR = NCPS_PDR_BIT; // set high
	pPIOA->PIO_OER  = NCPS_PDR_BIT; // output enable
	
	// enable peripheral clock for SPI ( PID Bit 5 )
	pPMC->PMC_PCER = ( (euint32) 1 << AT91C_ID_SPI ); // n.b. IDs are just bit-numbers
	
	// SPI enable and reset
	pSPI->SPI_CR = AT91C_SPI_SPIEN | AT91C_SPI_SWRST;
	
#ifdef FIXED_PERIPH
	// SPI mode: master, fixed periph. sel., FDIV=0, fault detection disabled
	pSPI->SPI_MR  = AT91C_SPI_MSTR | AT91C_SPI_PS_FIXED | AT91C_SPI_MODFDIS;
	// set PCS for fixed select
	// pSPI->SPI_MR &= 0xFFF0FFFF; // clear old PCS - redundant (AT91lib)
	pSPI->SPI_MR |= ( (SPI_MR_PCS<<16) & AT91C_SPI_PCS ); // set PCS
#else
	// SPI mode: master, variable periph. sel., FDIV=0, fault detection disabled
	// Chip-Select-Decoder Mode (write state of CS-Lines in TDR)
	pSPI->SPI_MR  = AT91C_SPI_MSTR | AT91C_SPI_MODFDIS | AT91C_SPI_PCSDEC ;
#endif
	
	// set chip-select-register
	// 8 bits per transfer, CPOL=1, ClockPhase=0, DLYBCT = 0
	// TODO: Why has CPOL to be active here and non-active on LPC2000?
	//       Take closer look on timing diagrams in datasheets.
	// not working pSPI->SPI_CSR[SPI_CSR_NUM] = AT91C_SPI_CPOL | AT91C_SPI_BITS_8 | AT91C_SPI_NCPHA;
	// not working pSPI->SPI_CSR[SPI_CSR_NUM] = AT91C_SPI_BITS_8 | AT91C_SPI_NCPHA;
	pSPI->SPI_CSR[SPI_CSR_NUM] = AT91C_SPI_CPOL | AT91C_SPI_BITS_8;
	// not working pSPI->SPI_CSR[SPI_CSR_NUM] = AT91C_SPI_BITS_8;
	
	// slow during init
	if_spiSetSpeed(0xFE); 
	
	// enable
	pSPI->SPI_CR = AT91C_SPI_SPIEN;

#if 0
	// a PDC-init has been in the Olimex-code - not needed since
	// the PDC is not used by this version of the interface
	// (Olimex did not use PDC either)
	// enable PDC transmit and receive in "PERIPH_PTCR" (SPI_PTCR)
	pPDC_SPI->PDC_PTCR = AT91C_PDC_TXTEN | AT91C_PDC_RXTEN;
	pSPI->SPI_PTCR = AT91C_PDC_TXTEN | AT91C_PDC_RXTEN;
#endif
  
	/* Send 20 spi commands with card not selected */
	for(i=0;i<21;i++) {
		if_spiSend(iface,0xFF);
	}

	/* enable automatic chip-select */
	// reset PIO-registers of CS-pin to default
	pPIOA->PIO_ODR  = NCPS_PDR_BIT; // input
	pPIOA->PIO_CODR = NCPS_PDR_BIT; // clear
	// disable PIO from controlling the CS pin (=hand over to SPI)
	pPIOA->PIO_PDR = NCPS_PDR_BIT;
	// set pin-functions in PIO Controller (function NCPS for CS-pin)
	pPIOA->PIO_ASR = NCPS_ASR_BIT;
	pPIOA->PIO_BSR = NPCS_BSR_BIT;
}
/*****************************************************************************/

void if_spiSetSpeed(euint8 speed)
{
	euint32 reg;
	AT91PS_SPI pSPI      = AT91C_BASE_SPI;

	if ( speed < SPI_SCBR_MIN ) speed = SPI_SCBR_MIN;
	if ( speed > 1 ) speed &= 0xFE;

	reg = pSPI->SPI_CSR[SPI_CSR_NUM];
	reg = ( reg & ~(AT91C_SPI_SCBR) ) | ( (euint32)speed << 8 );
	pSPI->SPI_CSR[SPI_CSR_NUM] = reg;
}
/*****************************************************************************/

euint8 if_spiSend(hwInterface *iface, euint8 outgoing)
{
	euint8 incoming;
	
	AT91PS_SPI pSPI      = AT91C_BASE_SPI;
	
	while( !( pSPI->SPI_SR & AT91C_SPI_TDRE ) ); // transfer compl. wait
#ifdef FIXED_PERIPH
	pSPI->SPI_TDR = (euint16)( outgoing );
#else
	pSPI->SPI_TDR = ( (euint16)(outgoing) | ((euint32)(SPI_TDR_PCS)<<16) );
#endif

	while( !( pSPI->SPI_SR & AT91C_SPI_RDRF ) ); // wait for char
	incoming = (euint8)( pSPI->SPI_RDR );

	return incoming;
}
/*****************************************************************************/

esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf)
{
	return(sd_readSector(file,address,buf,512));
}
/*****************************************************************************/

esint8 if_writeBuf(hwInterface* file, euint32 address, euint8* buf)
{
	return( sd_writeSector(file, address, buf) );
}
/*****************************************************************************/

esint8 if_setPos(hwInterface* file,euint32 address)
{
	return(0);
}
/*****************************************************************************/ 
