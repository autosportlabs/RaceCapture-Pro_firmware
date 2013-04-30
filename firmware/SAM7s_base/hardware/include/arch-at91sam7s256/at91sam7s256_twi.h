/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_twi.h
 * 
 * Hardware definition for the twi peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from TWI_6061A V1.1
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


#ifndef __AT91SAM7S256_TWI_H
#define __AT91SAM7S256_TWI_H

/* -------------------------------------------------------- */
/* TWI ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_TWI
#define AT91C_ID_TWI   	 9 /**< Two-Wire Interface id */
#endif /* AT91C_ID_TWI */

/* -------------------------------------------------------- */
/* TWI Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_TWI       	0xFFFB8000 /**< TWI base address */

/* -------------------------------------------------------- */
/* PIO definition for TWI hardware peripheral */
/* -------------------------------------------------------- */
#define AT91C_PA4_TWCK     	(1 << 4) /**< TWI Two-wire Serial Clock */
#define AT91C_PA3_TWD      	(1 << 3) /**< TWI Two-wire Serial Data */


/* -------------------------------------------------------- */
/* Register offset definition for TWI hardware peripheral */
/* -------------------------------------------------------- */
#define TWI_CR 	(0x0000) 	/**< Control Register */
#define TWI_MMR 	(0x0004) 	/**< Master Mode Register */
#define TWI_IADR 	(0x000C) 	/**< Internal Address Register */
#define TWI_CWGR 	(0x0010) 	/**< Clock Waveform Generator Register */
#define TWI_SR 	(0x0020) 	/**< Status Register */
#define TWI_IER 	(0x0024) 	/**< Interrupt Enable Register */
#define TWI_IDR 	(0x0028) 	/**< Interrupt Disable Register */
#define TWI_IMR 	(0x002C) 	/**< Interrupt Mask Register */
#define TWI_RHR 	(0x0030) 	/**< Receive Holding Register */
#define TWI_THR 	(0x0034) 	/**< Transmit Holding Register */

/* -------------------------------------------------------- */
/* Bitfields definition for TWI hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register TWI_CR */
#define AT91C_TWI_START       (0x1 << 0 ) /**< (TWI) Send a START Condition */
#define AT91C_TWI_STOP        (0x1 << 1 ) /**< (TWI) Send a STOP Condition */
#define AT91C_TWI_MSEN        (0x1 << 2 ) /**< (TWI) TWI Master Transfer Enabled */
#define AT91C_TWI_MSDIS       (0x1 << 3 ) /**< (TWI) TWI Master Transfer Disabled */
#define AT91C_TWI_SWRST       (0x1 << 7 ) /**< (TWI) Software Reset */
/* --- Register TWI_MMR */
#define AT91C_TWI_IADRSZ      (0x3 << 8 ) /**< (TWI) Internal Device Address Size */
#define 	AT91C_TWI_IADRSZ_NO                   (0x0 <<  8) /**< (TWI) No internal device address */
#define 	AT91C_TWI_IADRSZ_1_BYTE               (0x1 <<  8) /**< (TWI) One-byte internal device address */
#define 	AT91C_TWI_IADRSZ_2_BYTE               (0x2 <<  8) /**< (TWI) Two-byte internal device address */
#define 	AT91C_TWI_IADRSZ_3_BYTE               (0x3 <<  8) /**< (TWI) Three-byte internal device address */
#define AT91C_TWI_MREAD       (0x1 << 12) /**< (TWI) Master Read Direction */
#define AT91C_TWI_DADR        (0x7F << 16) /**< (TWI) Device Address */
/* --- Register TWI_CWGR */
#define AT91C_TWI_CLDIV       (0xFF << 0 ) /**< (TWI) Clock Low Divider */
#define AT91C_TWI_CHDIV       (0xFF << 8 ) /**< (TWI) Clock High Divider */
#define AT91C_TWI_CKDIV       (0x7 << 16) /**< (TWI) Clock Divider */
/* --- Register TWI_SR */
#define AT91C_TWI_TXCOMP      (0x1 << 0 ) /**< (TWI) Transmission Completed */
#define AT91C_TWI_RXRDY       (0x1 << 1 ) /**< (TWI) Receive holding register ReaDY */
#define AT91C_TWI_TXRDY       (0x1 << 2 ) /**< (TWI) Transmit holding register ReaDY */
#define AT91C_TWI_OVRE        (0x1 << 6 ) /**< (TWI) Overrun Error */
#define AT91C_TWI_UNRE        (0x1 << 7 ) /**< (TWI) Underrun Error */
#define AT91C_TWI_NACK        (0x1 << 8 ) /**< (TWI) Not Acknowledged */
/* --- Register TWI_IER */
#define AT91C_TWI_TXCOMP      (0x1 << 0 ) /**< (TWI) Transmission Completed */
#define AT91C_TWI_RXRDY       (0x1 << 1 ) /**< (TWI) Receive holding register ReaDY */
#define AT91C_TWI_TXRDY       (0x1 << 2 ) /**< (TWI) Transmit holding register ReaDY */
#define AT91C_TWI_OVRE        (0x1 << 6 ) /**< (TWI) Overrun Error */
#define AT91C_TWI_UNRE        (0x1 << 7 ) /**< (TWI) Underrun Error */
#define AT91C_TWI_NACK        (0x1 << 8 ) /**< (TWI) Not Acknowledged */
/* --- Register TWI_IDR */
#define AT91C_TWI_TXCOMP      (0x1 << 0 ) /**< (TWI) Transmission Completed */
#define AT91C_TWI_RXRDY       (0x1 << 1 ) /**< (TWI) Receive holding register ReaDY */
#define AT91C_TWI_TXRDY       (0x1 << 2 ) /**< (TWI) Transmit holding register ReaDY */
#define AT91C_TWI_OVRE        (0x1 << 6 ) /**< (TWI) Overrun Error */
#define AT91C_TWI_UNRE        (0x1 << 7 ) /**< (TWI) Underrun Error */
#define AT91C_TWI_NACK        (0x1 << 8 ) /**< (TWI) Not Acknowledged */
/* --- Register TWI_IMR */
#define AT91C_TWI_TXCOMP      (0x1 << 0 ) /**< (TWI) Transmission Completed */
#define AT91C_TWI_RXRDY       (0x1 << 1 ) /**< (TWI) Receive holding register ReaDY */
#define AT91C_TWI_TXRDY       (0x1 << 2 ) /**< (TWI) Transmit holding register ReaDY */
#define AT91C_TWI_OVRE        (0x1 << 6 ) /**< (TWI) Overrun Error */
#define AT91C_TWI_UNRE        (0x1 << 7 ) /**< (TWI) Underrun Error */
#define AT91C_TWI_NACK        (0x1 << 8 ) /**< (TWI) Not Acknowledged */

#endif /* __AT91SAM7S256_TWI_H */
