/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_dbgu.h
 * 
 * Hardware definition for the dbgu peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from DBGU_6059D V1.1
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


#ifndef __AT91SAM7S256_DBGU_H
#define __AT91SAM7S256_DBGU_H

/* -------------------------------------------------------- */
/* DBGU ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_SYS
#define AT91C_ID_SYS   	 1 /**< System Peripheral id */
#endif /* AT91C_ID_SYS */

/* -------------------------------------------------------- */
/* DBGU Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_DBGU      	0xFFFFF200 /**< DBGU base address */

/* -------------------------------------------------------- */
/* PIO definition for DBGU hardware peripheral */
/* -------------------------------------------------------- */
#define AT91C_PA9_DRXD     	(1 << 9) /**< DBGU Debug Receive Data */
#define AT91C_PA10_DTXD     	(1 << 10) /**< DBGU Debug Transmit Data */


/* -------------------------------------------------------- */
/* Register offset definition for DBGU hardware peripheral */
/* -------------------------------------------------------- */
#define DBGU_CR 	(0x0000) 	/**< Control Register */
#define DBGU_MR 	(0x0004) 	/**< Mode Register */
#define DBGU_IER 	(0x0008) 	/**< Interrupt Enable Register */
#define DBGU_IDR 	(0x000C) 	/**< Interrupt Disable Register */
#define DBGU_IMR 	(0x0010) 	/**< Interrupt Mask Register */
#define DBGU_CSR 	(0x0014) 	/**< Channel Status Register */
#define DBGU_RHR 	(0x0018) 	/**< Receiver Holding Register */
#define DBGU_THR 	(0x001C) 	/**< Transmitter Holding Register */
#define DBGU_BRGR 	(0x0020) 	/**< Baud Rate Generator Register */
#define DBGU_CIDR 	(0x0040) 	/**< Chip ID Register */
#define DBGU_EXID 	(0x0044) 	/**< Chip ID Extension Register */
#define DBGU_FNTR 	(0x0048) 	/**< Force NTRST Register */
#define DBGU_RPR 	(0x0100) 	/**< Receive Pointer Register */
#define DBGU_RCR 	(0x0104) 	/**< Receive Counter Register */
#define DBGU_TPR 	(0x0108) 	/**< Transmit Pointer Register */
#define DBGU_TCR 	(0x010C) 	/**< Transmit Counter Register */
#define DBGU_RNPR 	(0x0110) 	/**< Receive Next Pointer Register */
#define DBGU_RNCR 	(0x0114) 	/**< Receive Next Counter Register */
#define DBGU_TNPR 	(0x0118) 	/**< Transmit Next Pointer Register */
#define DBGU_TNCR 	(0x011C) 	/**< Transmit Next Counter Register */
#define DBGU_PTCR 	(0x0120) 	/**< PDC Transfer Control Register */
#define DBGU_PTSR 	(0x0124) 	/**< PDC Transfer Status Register */

/* -------------------------------------------------------- */
/* Bitfields definition for DBGU hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register DBGU_CR */
#define AT91C_US_RSTRX        (0x1 << 2 ) /**< (DBGU) Reset Receiver */
#define AT91C_US_RSTTX        (0x1 << 3 ) /**< (DBGU) Reset Transmitter */
#define AT91C_US_RXEN         (0x1 << 4 ) /**< (DBGU) Receiver Enable */
#define AT91C_US_RXDIS        (0x1 << 5 ) /**< (DBGU) Receiver Disable */
#define AT91C_US_TXEN         (0x1 << 6 ) /**< (DBGU) Transmitter Enable */
#define AT91C_US_TXDIS        (0x1 << 7 ) /**< (DBGU) Transmitter Disable */
#define AT91C_US_RSTSTA       (0x1 << 8 ) /**< (DBGU) Reset Status Bits */
/* --- Register DBGU_MR */
#define AT91C_US_PAR          (0x7 << 9 ) /**< (DBGU) Parity type */
#define 	AT91C_US_PAR_EVEN                 (0x0 <<  9) /**< (DBGU) Even Parity */
#define 	AT91C_US_PAR_ODD                  (0x1 <<  9) /**< (DBGU) Odd Parity */
#define 	AT91C_US_PAR_SPACE                (0x2 <<  9) /**< (DBGU) Parity forced to 0 (Space) */
#define 	AT91C_US_PAR_MARK                 (0x3 <<  9) /**< (DBGU) Parity forced to 1 (Mark) */
#define 	AT91C_US_PAR_NONE                 (0x4 <<  9) /**< (DBGU) No Parity */
#define 	AT91C_US_PAR_MULTI_DROP           (0x6 <<  9) /**< (DBGU) Multi-drop mode */
#define AT91C_US_CHMODE       (0x3 << 14) /**< (DBGU) Channel Mode */
#define 	AT91C_US_CHMODE_NORMAL               (0x0 << 14) /**< (DBGU) Normal Mode: The USART channel operates as an RX/TX USART. */
#define 	AT91C_US_CHMODE_AUTO                 (0x1 << 14) /**< (DBGU) Automatic Echo: Receiver Data Input is connected to the TXD pin. */
#define 	AT91C_US_CHMODE_LOCAL                (0x2 << 14) /**< (DBGU) Local Loopback: Transmitter Output Signal is connected to Receiver Input Signal. */
#define 	AT91C_US_CHMODE_REMOTE               (0x3 << 14) /**< (DBGU) Remote Loopback: RXD pin is internally connected to TXD pin. */
/* --- Register DBGU_IER */
#define AT91C_US_RXRDY        (0x1 << 0 ) /**< (DBGU) RXRDY Interrupt */
#define AT91C_US_TXRDY        (0x1 << 1 ) /**< (DBGU) TXRDY Interrupt */
#define AT91C_US_ENDRX        (0x1 << 3 ) /**< (DBGU) End of Receive Transfer Interrupt */
#define AT91C_US_ENDTX        (0x1 << 4 ) /**< (DBGU) End of Transmit Interrupt */
#define AT91C_US_OVRE         (0x1 << 5 ) /**< (DBGU) Overrun Interrupt */
#define AT91C_US_FRAME        (0x1 << 6 ) /**< (DBGU) Framing Error Interrupt */
#define AT91C_US_PARE         (0x1 << 7 ) /**< (DBGU) Parity Error Interrupt */
#define AT91C_US_TXEMPTY      (0x1 << 9 ) /**< (DBGU) TXEMPTY Interrupt */
#define AT91C_US_TXBUFE       (0x1 << 11) /**< (DBGU) TXBUFE Interrupt */
#define AT91C_US_RXBUFF       (0x1 << 12) /**< (DBGU) RXBUFF Interrupt */
#define AT91C_US_COMM_TX      (0x1 << 30) /**< (DBGU) COMM_TX Interrupt */
#define AT91C_US_COMM_RX      (0x1 << 31) /**< (DBGU) COMM_RX Interrupt */
/* --- Register DBGU_IDR */
#define AT91C_US_RXRDY        (0x1 << 0 ) /**< (DBGU) RXRDY Interrupt */
#define AT91C_US_TXRDY        (0x1 << 1 ) /**< (DBGU) TXRDY Interrupt */
#define AT91C_US_ENDRX        (0x1 << 3 ) /**< (DBGU) End of Receive Transfer Interrupt */
#define AT91C_US_ENDTX        (0x1 << 4 ) /**< (DBGU) End of Transmit Interrupt */
#define AT91C_US_OVRE         (0x1 << 5 ) /**< (DBGU) Overrun Interrupt */
#define AT91C_US_FRAME        (0x1 << 6 ) /**< (DBGU) Framing Error Interrupt */
#define AT91C_US_PARE         (0x1 << 7 ) /**< (DBGU) Parity Error Interrupt */
#define AT91C_US_TXEMPTY      (0x1 << 9 ) /**< (DBGU) TXEMPTY Interrupt */
#define AT91C_US_TXBUFE       (0x1 << 11) /**< (DBGU) TXBUFE Interrupt */
#define AT91C_US_RXBUFF       (0x1 << 12) /**< (DBGU) RXBUFF Interrupt */
#define AT91C_US_COMM_TX      (0x1 << 30) /**< (DBGU) COMM_TX Interrupt */
#define AT91C_US_COMM_RX      (0x1 << 31) /**< (DBGU) COMM_RX Interrupt */
/* --- Register DBGU_IMR */
#define AT91C_US_RXRDY        (0x1 << 0 ) /**< (DBGU) RXRDY Interrupt */
#define AT91C_US_TXRDY        (0x1 << 1 ) /**< (DBGU) TXRDY Interrupt */
#define AT91C_US_ENDRX        (0x1 << 3 ) /**< (DBGU) End of Receive Transfer Interrupt */
#define AT91C_US_ENDTX        (0x1 << 4 ) /**< (DBGU) End of Transmit Interrupt */
#define AT91C_US_OVRE         (0x1 << 5 ) /**< (DBGU) Overrun Interrupt */
#define AT91C_US_FRAME        (0x1 << 6 ) /**< (DBGU) Framing Error Interrupt */
#define AT91C_US_PARE         (0x1 << 7 ) /**< (DBGU) Parity Error Interrupt */
#define AT91C_US_TXEMPTY      (0x1 << 9 ) /**< (DBGU) TXEMPTY Interrupt */
#define AT91C_US_TXBUFE       (0x1 << 11) /**< (DBGU) TXBUFE Interrupt */
#define AT91C_US_RXBUFF       (0x1 << 12) /**< (DBGU) RXBUFF Interrupt */
#define AT91C_US_COMM_TX      (0x1 << 30) /**< (DBGU) COMM_TX Interrupt */
#define AT91C_US_COMM_RX      (0x1 << 31) /**< (DBGU) COMM_RX Interrupt */
/* --- Register DBGU_CSR */
#define AT91C_US_RXRDY        (0x1 << 0 ) /**< (DBGU) RXRDY Interrupt */
#define AT91C_US_TXRDY        (0x1 << 1 ) /**< (DBGU) TXRDY Interrupt */
#define AT91C_US_ENDRX        (0x1 << 3 ) /**< (DBGU) End of Receive Transfer Interrupt */
#define AT91C_US_ENDTX        (0x1 << 4 ) /**< (DBGU) End of Transmit Interrupt */
#define AT91C_US_OVRE         (0x1 << 5 ) /**< (DBGU) Overrun Interrupt */
#define AT91C_US_FRAME        (0x1 << 6 ) /**< (DBGU) Framing Error Interrupt */
#define AT91C_US_PARE         (0x1 << 7 ) /**< (DBGU) Parity Error Interrupt */
#define AT91C_US_TXEMPTY      (0x1 << 9 ) /**< (DBGU) TXEMPTY Interrupt */
#define AT91C_US_TXBUFE       (0x1 << 11) /**< (DBGU) TXBUFE Interrupt */
#define AT91C_US_RXBUFF       (0x1 << 12) /**< (DBGU) RXBUFF Interrupt */
#define AT91C_US_COMM_TX      (0x1 << 30) /**< (DBGU) COMM_TX Interrupt */
#define AT91C_US_COMM_RX      (0x1 << 31) /**< (DBGU) COMM_RX Interrupt */
/* --- Register DBGU_FNTR */
#define AT91C_US_FORCE_NTRST  (0x1 << 0 ) /**< (DBGU) Force NTRST in JTAG */

#endif /* __AT91SAM7S256_DBGU_H */
