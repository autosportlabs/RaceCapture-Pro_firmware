/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_spi.h
 * 
 * Hardware definition for the spi peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from SPI_6088D V1.3
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#ifndef __AT91SAM7S256_SPI_H
#define __AT91SAM7S256_SPI_H

/* -------------------------------------------------------- */
/* SPI ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_SPI
#define AT91C_ID_SPI   	 5 /**< Serial Peripheral Interface id */
#endif /* AT91C_ID_SPI */

/* -------------------------------------------------------- */
/* SPI Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_SPI       	0xFFFE0000 /**< SPI base address */

/* -------------------------------------------------------- */
/* PIO definition for SPI hardware peripheral */
/* -------------------------------------------------------- */
#define AT91C_PA12_MISO     	(1 << 12) /**< SPI Master In Slave */
#define AT91C_PA13_MOSI     	(1 << 13) /**< SPI Master Out Slave */
#define AT91C_PA11_NPCS0    	(1 << 11) /**< SPI Peripheral Chip Select 0 */
#define AT91C_PA9_NPCS1    	(1 << 9) /**< SPI Peripheral Chip Select 1 */
#define AT91C_PA10_NPCS2    	(1 << 10) /**< SPI Peripheral Chip Select 2 */
#define AT91C_PA5_NPCS3    	(1 << 5) /**< SPI Peripheral Chip Select 3 */
#define AT91C_PA14_SPCK     	(1 << 14) /**< SPI Serial Clock */


/* -------------------------------------------------------- */
/* Register offset definition for SPI hardware peripheral */
/* -------------------------------------------------------- */
#define SPI_CR 	(0x0000) 	/**< Control Register */
#define SPI_MR 	(0x0004) 	/**< Mode Register */
#define SPI_RDR 	(0x0008) 	/**< Receive Data Register */
#define SPI_TDR 	(0x000C) 	/**< Transmit Data Register */
#define SPI_SR 	(0x0010) 	/**< Status Register */
#define SPI_IER 	(0x0014) 	/**< Interrupt Enable Register */
#define SPI_IDR 	(0x0018) 	/**< Interrupt Disable Register */
#define SPI_IMR 	(0x001C) 	/**< Interrupt Mask Register */
#define SPI_CSR 	(0x0030) 	/**< Chip Select Register */
#define SPI_RPR 	(0x0100) 	/**< Receive Pointer Register */
#define SPI_RCR 	(0x0104) 	/**< Receive Counter Register */
#define SPI_TPR 	(0x0108) 	/**< Transmit Pointer Register */
#define SPI_TCR 	(0x010C) 	/**< Transmit Counter Register */
#define SPI_RNPR 	(0x0110) 	/**< Receive Next Pointer Register */
#define SPI_RNCR 	(0x0114) 	/**< Receive Next Counter Register */
#define SPI_TNPR 	(0x0118) 	/**< Transmit Next Pointer Register */
#define SPI_TNCR 	(0x011C) 	/**< Transmit Next Counter Register */
#define SPI_PTCR 	(0x0120) 	/**< PDC Transfer Control Register */
#define SPI_PTSR 	(0x0124) 	/**< PDC Transfer Status Register */

/* -------------------------------------------------------- */
/* Bitfields definition for SPI hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register SPI_CR */
#define AT91C_SPI_SPIEN       (0x1 << 0 ) /**< (SPI) SPI Enable */
#define AT91C_SPI_SPIDIS      (0x1 << 1 ) /**< (SPI) SPI Disable */
#define AT91C_SPI_SWRST       (0x1 << 7 ) /**< (SPI) SPI Software reset */
#define AT91C_SPI_LASTXFER    (0x1 << 24) /**< (SPI) SPI Last Transfer */
/* --- Register SPI_MR */
#define AT91C_SPI_MSTR        (0x1 << 0 ) /**< (SPI) Master/Slave Mode */
#define AT91C_SPI_PS          (0x1 << 1 ) /**< (SPI) Peripheral Select */
#define 	AT91C_SPI_PS_FIXED                (0x0 <<  1) /**< (SPI) Fixed Peripheral Select */
#define 	AT91C_SPI_PS_VARIABLE             (0x1 <<  1) /**< (SPI) Variable Peripheral Select */
#define AT91C_SPI_PCSDEC      (0x1 << 2 ) /**< (SPI) Chip Select Decode */
#define AT91C_SPI_FDIV        (0x1 << 3 ) /**< (SPI) Clock Selection */
#define AT91C_SPI_MODFDIS     (0x1 << 4 ) /**< (SPI) Mode Fault Detection */
#define AT91C_SPI_LLB         (0x1 << 7 ) /**< (SPI) Clock Selection */
#define AT91C_SPI_PCS         (0xF << 16) /**< (SPI) Peripheral Chip Select */
#define AT91C_SPI_DLYBCS      (0xFF << 24) /**< (SPI) Delay Between Chip Selects */
/* --- Register SPI_RDR */
#define AT91C_SPI_RD          (0xFFFF << 0 ) /**< (SPI) Receive Data */
#define AT91C_SPI_RPCS        (0xF << 16) /**< (SPI) Peripheral Chip Select Status */
/* --- Register SPI_TDR */
#define AT91C_SPI_TD          (0xFFFF << 0 ) /**< (SPI) Transmit Data */
#define AT91C_SPI_TPCS        (0xF << 16) /**< (SPI) Peripheral Chip Select Status */
#define AT91C_SPI_LASTXFER    (0x1 << 24) /**< (SPI) SPI Last Transfer */
/* --- Register SPI_SR */
#define AT91C_SPI_RDRF        (0x1 << 0 ) /**< (SPI) Receive Data Register Full */
#define AT91C_SPI_TDRE        (0x1 << 1 ) /**< (SPI) Transmit Data Register Empty */
#define AT91C_SPI_MODF        (0x1 << 2 ) /**< (SPI) Mode Fault Error */
#define AT91C_SPI_OVRES       (0x1 << 3 ) /**< (SPI) Overrun Error Status */
#define AT91C_SPI_ENDRX       (0x1 << 4 ) /**< (SPI) End of Receiver Transfer */
#define AT91C_SPI_ENDTX       (0x1 << 5 ) /**< (SPI) End of Receiver Transfer */
#define AT91C_SPI_RXBUFF      (0x1 << 6 ) /**< (SPI) RXBUFF Interrupt */
#define AT91C_SPI_TXBUFE      (0x1 << 7 ) /**< (SPI) TXBUFE Interrupt */
#define AT91C_SPI_NSSR        (0x1 << 8 ) /**< (SPI) NSSR Interrupt */
#define AT91C_SPI_TXEMPTY     (0x1 << 9 ) /**< (SPI) TXEMPTY Interrupt */
#define AT91C_SPI_SPIENS      (0x1 << 16) /**< (SPI) Enable Status */
/* --- Register SPI_IER */
#define AT91C_SPI_RDRF        (0x1 << 0 ) /**< (SPI) Receive Data Register Full */
#define AT91C_SPI_TDRE        (0x1 << 1 ) /**< (SPI) Transmit Data Register Empty */
#define AT91C_SPI_MODF        (0x1 << 2 ) /**< (SPI) Mode Fault Error */
#define AT91C_SPI_OVRES       (0x1 << 3 ) /**< (SPI) Overrun Error Status */
#define AT91C_SPI_ENDRX       (0x1 << 4 ) /**< (SPI) End of Receiver Transfer */
#define AT91C_SPI_ENDTX       (0x1 << 5 ) /**< (SPI) End of Receiver Transfer */
#define AT91C_SPI_RXBUFF      (0x1 << 6 ) /**< (SPI) RXBUFF Interrupt */
#define AT91C_SPI_TXBUFE      (0x1 << 7 ) /**< (SPI) TXBUFE Interrupt */
#define AT91C_SPI_NSSR        (0x1 << 8 ) /**< (SPI) NSSR Interrupt */
#define AT91C_SPI_TXEMPTY     (0x1 << 9 ) /**< (SPI) TXEMPTY Interrupt */
/* --- Register SPI_IDR */
#define AT91C_SPI_RDRF        (0x1 << 0 ) /**< (SPI) Receive Data Register Full */
#define AT91C_SPI_TDRE        (0x1 << 1 ) /**< (SPI) Transmit Data Register Empty */
#define AT91C_SPI_MODF        (0x1 << 2 ) /**< (SPI) Mode Fault Error */
#define AT91C_SPI_OVRES       (0x1 << 3 ) /**< (SPI) Overrun Error Status */
#define AT91C_SPI_ENDRX       (0x1 << 4 ) /**< (SPI) End of Receiver Transfer */
#define AT91C_SPI_ENDTX       (0x1 << 5 ) /**< (SPI) End of Receiver Transfer */
#define AT91C_SPI_RXBUFF      (0x1 << 6 ) /**< (SPI) RXBUFF Interrupt */
#define AT91C_SPI_TXBUFE      (0x1 << 7 ) /**< (SPI) TXBUFE Interrupt */
#define AT91C_SPI_NSSR        (0x1 << 8 ) /**< (SPI) NSSR Interrupt */
#define AT91C_SPI_TXEMPTY     (0x1 << 9 ) /**< (SPI) TXEMPTY Interrupt */
/* --- Register SPI_IMR */
#define AT91C_SPI_RDRF        (0x1 << 0 ) /**< (SPI) Receive Data Register Full */
#define AT91C_SPI_TDRE        (0x1 << 1 ) /**< (SPI) Transmit Data Register Empty */
#define AT91C_SPI_MODF        (0x1 << 2 ) /**< (SPI) Mode Fault Error */
#define AT91C_SPI_OVRES       (0x1 << 3 ) /**< (SPI) Overrun Error Status */
#define AT91C_SPI_ENDRX       (0x1 << 4 ) /**< (SPI) End of Receiver Transfer */
#define AT91C_SPI_ENDTX       (0x1 << 5 ) /**< (SPI) End of Receiver Transfer */
#define AT91C_SPI_RXBUFF      (0x1 << 6 ) /**< (SPI) RXBUFF Interrupt */
#define AT91C_SPI_TXBUFE      (0x1 << 7 ) /**< (SPI) TXBUFE Interrupt */
#define AT91C_SPI_NSSR        (0x1 << 8 ) /**< (SPI) NSSR Interrupt */
#define AT91C_SPI_TXEMPTY     (0x1 << 9 ) /**< (SPI) TXEMPTY Interrupt */
/* --- Register SPI_CSR */
#define AT91C_SPI_CPOL        (0x1 << 0 ) /**< (SPI) Clock Polarity */
#define AT91C_SPI_NCPHA       (0x1 << 1 ) /**< (SPI) Clock Phase */
#define AT91C_SPI_CSAAT       (0x1 << 3 ) /**< (SPI) Chip Select Active After Transfer */
#define AT91C_SPI_BITS        (0xF << 4 ) /**< (SPI) Bits Per Transfer */
#define 	AT91C_SPI_BITS_8                    (0x0 <<  4) /**< (SPI) 8 Bits Per transfer */
#define 	AT91C_SPI_BITS_9                    (0x1 <<  4) /**< (SPI) 9 Bits Per transfer */
#define 	AT91C_SPI_BITS_10                   (0x2 <<  4) /**< (SPI) 10 Bits Per transfer */
#define 	AT91C_SPI_BITS_11                   (0x3 <<  4) /**< (SPI) 11 Bits Per transfer */
#define 	AT91C_SPI_BITS_12                   (0x4 <<  4) /**< (SPI) 12 Bits Per transfer */
#define 	AT91C_SPI_BITS_13                   (0x5 <<  4) /**< (SPI) 13 Bits Per transfer */
#define 	AT91C_SPI_BITS_14                   (0x6 <<  4) /**< (SPI) 14 Bits Per transfer */
#define 	AT91C_SPI_BITS_15                   (0x7 <<  4) /**< (SPI) 15 Bits Per transfer */
#define 	AT91C_SPI_BITS_16                   (0x8 <<  4) /**< (SPI) 16 Bits Per transfer */
#define AT91C_SPI_SCBR        (0xFF << 8 ) /**< (SPI) Serial Clock Baud Rate */
#define AT91C_SPI_DLYBS       (0xFF << 16) /**< (SPI) Delay Before SPCK */
#define AT91C_SPI_DLYBCT      (0xFF << 24) /**< (SPI) Delay Between Consecutive Transfers */

#endif /* __AT91SAM7S256_SPI_H */
