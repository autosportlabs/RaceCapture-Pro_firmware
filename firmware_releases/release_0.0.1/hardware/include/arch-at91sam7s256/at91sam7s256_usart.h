/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_usart.h
 * 
 * Hardware definition for the usart peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from  V
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


#ifndef __AT91SAM7S256_USART_H
#define __AT91SAM7S256_USART_H

/* -------------------------------------------------------- */
/* USART ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_US0
#define AT91C_ID_US0   	 6 /**< USART 0 id */
#endif /* AT91C_ID_US0 */
#ifndef AT91C_ID_US1
#define AT91C_ID_US1   	 7 /**< USART 1 id */
#endif /* AT91C_ID_US1 */

/* -------------------------------------------------------- */
/* USART Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_US1       	0xFFFC4000 /**< US1 base address */
#define AT91C_BASE_US0       	0xFFFC0000 /**< US0 base address */

/* -------------------------------------------------------- */
/* PIO definition for USART hardware peripheral */
/* -------------------------------------------------------- */
#define AT91C_PA8_CTS0     	(1 << 8) /**< USART 0 Clear To Send */
#define AT91C_PA7_RTS0     	(1 << 7) /**< USART 0 Ready To Send */
#define AT91C_PA5_RXD0     	(1 << 5) /**< USART 0 Receive Data */
#define AT91C_PA2_SCK0     	(1 << 2) /**< USART 0 Serial Clock */
#define AT91C_PA6_TXD0     	(1 << 6) /**< USART 0 Transmit Data */

#define AT91C_PA25_CTS1     	(1 << 25) /**< USART 1 Clear To Send */
#define AT91C_PA26_DCD1     	(1 << 26) /**< USART 1 Data Carrier Detect */
#define AT91C_PA28_DSR1     	(1 << 28) /**< USART 1 Data Set ready */
#define AT91C_PA27_DTR1     	(1 << 27) /**< USART 1 Data Terminal ready */
#define AT91C_PA29_RI1      	(1 << 29) /**< USART 1 Ring Indicator */
#define AT91C_PA24_RTS1     	(1 << 24) /**< USART 1 Ready To Send */
#define AT91C_PA21_RXD1     	(1 << 21) /**< USART 1 Receive Data */
#define AT91C_PA23_SCK1     	(1 << 23) /**< USART 1 Serial Clock */
#define AT91C_PA22_TXD1     	(1 << 22) /**< USART 1 Transmit Data */


/* -------------------------------------------------------- */
/* Register offset definition for USART hardware peripheral */
/* -------------------------------------------------------- */
#define US_CR 	(0x0000) 	/**< Control Register */
#define US_MR 	(0x0004) 	/**< Mode Register */
#define US_IER 	(0x0008) 	/**< Interrupt Enable Register */
#define US_IDR 	(0x000C) 	/**< Interrupt Disable Register */
#define US_IMR 	(0x0010) 	/**< Interrupt Mask Register */
#define US_CSR 	(0x0014) 	/**< Channel Status Register */
#define US_RHR 	(0x0018) 	/**< Receiver Holding Register */
#define US_THR 	(0x001C) 	/**< Transmitter Holding Register */
#define US_BRGR 	(0x0020) 	/**< Baud Rate Generator Register */
#define US_RTOR 	(0x0024) 	/**< Receiver Time-out Register */
#define US_TTGR 	(0x0028) 	/**< Transmitter Time-guard Register */
#define US_FIDI 	(0x0040) 	/**< FI_DI_Ratio Register */
#define US_NER 	(0x0044) 	/**< Nb Errors Register */
#define US_IF 	(0x004C) 	/**< IRDA_FILTER Register */
#define US_RPR 	(0x0100) 	/**< Receive Pointer Register */
#define US_RCR 	(0x0104) 	/**< Receive Counter Register */
#define US_TPR 	(0x0108) 	/**< Transmit Pointer Register */
#define US_TCR 	(0x010C) 	/**< Transmit Counter Register */
#define US_RNPR 	(0x0110) 	/**< Receive Next Pointer Register */
#define US_RNCR 	(0x0114) 	/**< Receive Next Counter Register */
#define US_TNPR 	(0x0118) 	/**< Transmit Next Pointer Register */
#define US_TNCR 	(0x011C) 	/**< Transmit Next Counter Register */
#define US_PTCR 	(0x0120) 	/**< PDC Transfer Control Register */
#define US_PTSR 	(0x0124) 	/**< PDC Transfer Status Register */

/* -------------------------------------------------------- */
/* Bitfields definition for USART hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register US_CR */
#define AT91C_US_RSTRX        (0x1 << 2 ) /**< (USART) Reset Receiver */
#define AT91C_US_RSTTX        (0x1 << 3 ) /**< (USART) Reset Transmitter */
#define AT91C_US_RXEN         (0x1 << 4 ) /**< (USART) Receiver Enable */
#define AT91C_US_RXDIS        (0x1 << 5 ) /**< (USART) Receiver Disable */
#define AT91C_US_TXEN         (0x1 << 6 ) /**< (USART) Transmitter Enable */
#define AT91C_US_TXDIS        (0x1 << 7 ) /**< (USART) Transmitter Disable */
#define AT91C_US_RSTSTA       (0x1 << 8 ) /**< (USART) Reset Status Bits */
#define AT91C_US_STTBRK       (0x1 << 9 ) /**< (USART) Start Break */
#define AT91C_US_STPBRK       (0x1 << 10) /**< (USART) Stop Break */
#define AT91C_US_STTTO        (0x1 << 11) /**< (USART) Start Time-out */
#define AT91C_US_SENDA        (0x1 << 12) /**< (USART) Send Address */
#define AT91C_US_RSTIT        (0x1 << 13) /**< (USART) Reset Iterations */
#define AT91C_US_RSTNACK      (0x1 << 14) /**< (USART) Reset Non Acknowledge */
#define AT91C_US_RETTO        (0x1 << 15) /**< (USART) Rearm Time-out */
#define AT91C_US_DTREN        (0x1 << 16) /**< (USART) Data Terminal ready Enable */
#define AT91C_US_DTRDIS       (0x1 << 17) /**< (USART) Data Terminal ready Disable */
#define AT91C_US_RTSEN        (0x1 << 18) /**< (USART) Request to Send enable */
#define AT91C_US_RTSDIS       (0x1 << 19) /**< (USART) Request to Send Disable */
/* --- Register US_MR */
#define AT91C_US_USMODE       (0xF << 0 ) /**< (USART) Usart mode */
#define 	AT91C_US_USMODE_NORMAL               0x0 /**< (USART) Normal */
#define 	AT91C_US_USMODE_RS485                0x1 /**< (USART) RS485 */
#define 	AT91C_US_USMODE_HWHSH                0x2 /**< (USART) Hardware Handshaking */
#define 	AT91C_US_USMODE_MODEM                0x3 /**< (USART) Modem */
#define 	AT91C_US_USMODE_ISO7816_0            0x4 /**< (USART) ISO7816 protocol: T = 0 */
#define 	AT91C_US_USMODE_ISO7816_1            0x6 /**< (USART) ISO7816 protocol: T = 1 */
#define 	AT91C_US_USMODE_IRDA                 0x8 /**< (USART) IrDA */
#define 	AT91C_US_USMODE_SWHSH                0xC /**< (USART) Software Handshaking */
#define AT91C_US_CLKS         (0x3 << 4 ) /**< (USART) Clock Selection (Baud Rate generator Input Clock */
#define 	AT91C_US_CLKS_CLOCK                (0x0 <<  4) /**< (USART) Clock */
#define 	AT91C_US_CLKS_FDIV1                (0x1 <<  4) /**< (USART) fdiv1 */
#define 	AT91C_US_CLKS_SLOW                 (0x2 <<  4) /**< (USART) slow_clock (ARM) */
#define 	AT91C_US_CLKS_EXT                  (0x3 <<  4) /**< (USART) External (SCK) */
#define AT91C_US_CHRL         (0x3 << 6 ) /**< (USART) Clock Selection (Baud Rate generator Input Clock */
#define 	AT91C_US_CHRL_5_BITS               (0x0 <<  6) /**< (USART) Character Length: 5 bits */
#define 	AT91C_US_CHRL_6_BITS               (0x1 <<  6) /**< (USART) Character Length: 6 bits */
#define 	AT91C_US_CHRL_7_BITS               (0x2 <<  6) /**< (USART) Character Length: 7 bits */
#define 	AT91C_US_CHRL_8_BITS               (0x3 <<  6) /**< (USART) Character Length: 8 bits */
#define AT91C_US_SYNC         (0x1 << 8 ) /**< (USART) Synchronous Mode Select */
#define AT91C_US_PAR          (0x7 << 9 ) /**< (USART) Parity type */
#define 	AT91C_US_PAR_EVEN                 (0x0 <<  9) /**< (USART) Even Parity */
#define 	AT91C_US_PAR_ODD                  (0x1 <<  9) /**< (USART) Odd Parity */
#define 	AT91C_US_PAR_SPACE                (0x2 <<  9) /**< (USART) Parity forced to 0 (Space) */
#define 	AT91C_US_PAR_MARK                 (0x3 <<  9) /**< (USART) Parity forced to 1 (Mark) */
#define 	AT91C_US_PAR_NONE                 (0x4 <<  9) /**< (USART) No Parity */
#define 	AT91C_US_PAR_MULTI_DROP           (0x6 <<  9) /**< (USART) Multi-drop mode */
#define AT91C_US_NBSTOP       (0x3 << 12) /**< (USART) Number of Stop bits */
#define 	AT91C_US_NBSTOP_1_BIT                (0x0 << 12) /**< (USART) 1 stop bit */
#define 	AT91C_US_NBSTOP_15_BIT               (0x1 << 12) /**< (USART) Asynchronous (SYNC=0) 2 stop bits Synchronous (SYNC=1) 2 stop bits */
#define 	AT91C_US_NBSTOP_2_BIT                (0x2 << 12) /**< (USART) 2 stop bits */
#define AT91C_US_CHMODE       (0x3 << 14) /**< (USART) Channel Mode */
#define 	AT91C_US_CHMODE_NORMAL               (0x0 << 14) /**< (USART) Normal Mode: The USART channel operates as an RX/TX USART. */
#define 	AT91C_US_CHMODE_AUTO                 (0x1 << 14) /**< (USART) Automatic Echo: Receiver Data Input is connected to the TXD pin. */
#define 	AT91C_US_CHMODE_LOCAL                (0x2 << 14) /**< (USART) Local Loopback: Transmitter Output Signal is connected to Receiver Input Signal. */
#define 	AT91C_US_CHMODE_REMOTE               (0x3 << 14) /**< (USART) Remote Loopback: RXD pin is internally connected to TXD pin. */
#define AT91C_US_MSBF         (0x1 << 16) /**< (USART) Bit Order */
#define AT91C_US_MODE9        (0x1 << 17) /**< (USART) 9-bit Character length */
#define AT91C_US_CKLO         (0x1 << 18) /**< (USART) Clock Output Select */
#define AT91C_US_OVER         (0x1 << 19) /**< (USART) Over Sampling Mode */
#define AT91C_US_INACK        (0x1 << 20) /**< (USART) Inhibit Non Acknowledge */
#define AT91C_US_DSNACK       (0x1 << 21) /**< (USART) Disable Successive NACK */
#define AT91C_US_MAX_ITER     (0x1 << 24) /**< (USART) Number of Repetitions */
#define AT91C_US_FILTER       (0x1 << 28) /**< (USART) Receive Line Filter */
/* --- Register US_IER */
#define AT91C_US_RXRDY        (0x1 << 0 ) /**< (USART) RXRDY Interrupt */
#define AT91C_US_TXRDY        (0x1 << 1 ) /**< (USART) TXRDY Interrupt */
#define AT91C_US_RXBRK        (0x1 << 2 ) /**< (USART) Break Received/End of Break */
#define AT91C_US_ENDRX        (0x1 << 3 ) /**< (USART) End of Receive Transfer Interrupt */
#define AT91C_US_ENDTX        (0x1 << 4 ) /**< (USART) End of Transmit Interrupt */
#define AT91C_US_OVRE         (0x1 << 5 ) /**< (USART) Overrun Interrupt */
#define AT91C_US_FRAME        (0x1 << 6 ) /**< (USART) Framing Error Interrupt */
#define AT91C_US_PARE         (0x1 << 7 ) /**< (USART) Parity Error Interrupt */
#define AT91C_US_TIMEOUT      (0x1 << 8 ) /**< (USART) Receiver Time-out */
#define AT91C_US_TXEMPTY      (0x1 << 9 ) /**< (USART) TXEMPTY Interrupt */
#define AT91C_US_ITERATION    (0x1 << 10) /**< (USART) Max number of Repetitions Reached */
#define AT91C_US_TXBUFE       (0x1 << 11) /**< (USART) TXBUFE Interrupt */
#define AT91C_US_RXBUFF       (0x1 << 12) /**< (USART) RXBUFF Interrupt */
#define AT91C_US_NACK         (0x1 << 13) /**< (USART) Non Acknowledge */
#define AT91C_US_RIIC         (0x1 << 16) /**< (USART) Ring INdicator Input Change Flag */
#define AT91C_US_DSRIC        (0x1 << 17) /**< (USART) Data Set Ready Input Change Flag */
#define AT91C_US_DCDIC        (0x1 << 18) /**< (USART) Data Carrier Flag */
#define AT91C_US_CTSIC        (0x1 << 19) /**< (USART) Clear To Send Input Change Flag */
/* --- Register US_IDR */
#define AT91C_US_RXRDY        (0x1 << 0 ) /**< (USART) RXRDY Interrupt */
#define AT91C_US_TXRDY        (0x1 << 1 ) /**< (USART) TXRDY Interrupt */
#define AT91C_US_RXBRK        (0x1 << 2 ) /**< (USART) Break Received/End of Break */
#define AT91C_US_ENDRX        (0x1 << 3 ) /**< (USART) End of Receive Transfer Interrupt */
#define AT91C_US_ENDTX        (0x1 << 4 ) /**< (USART) End of Transmit Interrupt */
#define AT91C_US_OVRE         (0x1 << 5 ) /**< (USART) Overrun Interrupt */
#define AT91C_US_FRAME        (0x1 << 6 ) /**< (USART) Framing Error Interrupt */
#define AT91C_US_PARE         (0x1 << 7 ) /**< (USART) Parity Error Interrupt */
#define AT91C_US_TIMEOUT      (0x1 << 8 ) /**< (USART) Receiver Time-out */
#define AT91C_US_TXEMPTY      (0x1 << 9 ) /**< (USART) TXEMPTY Interrupt */
#define AT91C_US_ITERATION    (0x1 << 10) /**< (USART) Max number of Repetitions Reached */
#define AT91C_US_TXBUFE       (0x1 << 11) /**< (USART) TXBUFE Interrupt */
#define AT91C_US_RXBUFF       (0x1 << 12) /**< (USART) RXBUFF Interrupt */
#define AT91C_US_NACK         (0x1 << 13) /**< (USART) Non Acknowledge */
#define AT91C_US_RIIC         (0x1 << 16) /**< (USART) Ring INdicator Input Change Flag */
#define AT91C_US_DSRIC        (0x1 << 17) /**< (USART) Data Set Ready Input Change Flag */
#define AT91C_US_DCDIC        (0x1 << 18) /**< (USART) Data Carrier Flag */
#define AT91C_US_CTSIC        (0x1 << 19) /**< (USART) Clear To Send Input Change Flag */
/* --- Register US_IMR */
#define AT91C_US_RXRDY        (0x1 << 0 ) /**< (USART) RXRDY Interrupt */
#define AT91C_US_TXRDY        (0x1 << 1 ) /**< (USART) TXRDY Interrupt */
#define AT91C_US_RXBRK        (0x1 << 2 ) /**< (USART) Break Received/End of Break */
#define AT91C_US_ENDRX        (0x1 << 3 ) /**< (USART) End of Receive Transfer Interrupt */
#define AT91C_US_ENDTX        (0x1 << 4 ) /**< (USART) End of Transmit Interrupt */
#define AT91C_US_OVRE         (0x1 << 5 ) /**< (USART) Overrun Interrupt */
#define AT91C_US_FRAME        (0x1 << 6 ) /**< (USART) Framing Error Interrupt */
#define AT91C_US_PARE         (0x1 << 7 ) /**< (USART) Parity Error Interrupt */
#define AT91C_US_TIMEOUT      (0x1 << 8 ) /**< (USART) Receiver Time-out */
#define AT91C_US_TXEMPTY      (0x1 << 9 ) /**< (USART) TXEMPTY Interrupt */
#define AT91C_US_ITERATION    (0x1 << 10) /**< (USART) Max number of Repetitions Reached */
#define AT91C_US_TXBUFE       (0x1 << 11) /**< (USART) TXBUFE Interrupt */
#define AT91C_US_RXBUFF       (0x1 << 12) /**< (USART) RXBUFF Interrupt */
#define AT91C_US_NACK         (0x1 << 13) /**< (USART) Non Acknowledge */
#define AT91C_US_RIIC         (0x1 << 16) /**< (USART) Ring INdicator Input Change Flag */
#define AT91C_US_DSRIC        (0x1 << 17) /**< (USART) Data Set Ready Input Change Flag */
#define AT91C_US_DCDIC        (0x1 << 18) /**< (USART) Data Carrier Flag */
#define AT91C_US_CTSIC        (0x1 << 19) /**< (USART) Clear To Send Input Change Flag */
/* --- Register US_CSR */
#define AT91C_US_RXRDY        (0x1 << 0 ) /**< (USART) RXRDY Interrupt */
#define AT91C_US_TXRDY        (0x1 << 1 ) /**< (USART) TXRDY Interrupt */
#define AT91C_US_RXBRK        (0x1 << 2 ) /**< (USART) Break Received/End of Break */
#define AT91C_US_ENDRX        (0x1 << 3 ) /**< (USART) End of Receive Transfer Interrupt */
#define AT91C_US_ENDTX        (0x1 << 4 ) /**< (USART) End of Transmit Interrupt */
#define AT91C_US_OVRE         (0x1 << 5 ) /**< (USART) Overrun Interrupt */
#define AT91C_US_FRAME        (0x1 << 6 ) /**< (USART) Framing Error Interrupt */
#define AT91C_US_PARE         (0x1 << 7 ) /**< (USART) Parity Error Interrupt */
#define AT91C_US_TIMEOUT      (0x1 << 8 ) /**< (USART) Receiver Time-out */
#define AT91C_US_TXEMPTY      (0x1 << 9 ) /**< (USART) TXEMPTY Interrupt */
#define AT91C_US_ITERATION    (0x1 << 10) /**< (USART) Max number of Repetitions Reached */
#define AT91C_US_TXBUFE       (0x1 << 11) /**< (USART) TXBUFE Interrupt */
#define AT91C_US_RXBUFF       (0x1 << 12) /**< (USART) RXBUFF Interrupt */
#define AT91C_US_NACK         (0x1 << 13) /**< (USART) Non Acknowledge */
#define AT91C_US_RIIC         (0x1 << 16) /**< (USART) Ring INdicator Input Change Flag */
#define AT91C_US_DSRIC        (0x1 << 17) /**< (USART) Data Set Ready Input Change Flag */
#define AT91C_US_DCDIC        (0x1 << 18) /**< (USART) Data Carrier Flag */
#define AT91C_US_CTSIC        (0x1 << 19) /**< (USART) Clear To Send Input Change Flag */
#define AT91C_US_RI           (0x1 << 20) /**< (USART) Image of RI Input */
#define AT91C_US_DSR          (0x1 << 21) /**< (USART) Image of DSR Input */
#define AT91C_US_DCD          (0x1 << 22) /**< (USART) Image of DCD Input */
#define AT91C_US_CTS          (0x1 << 23) /**< (USART) Image of CTS Input */

#endif /* __AT91SAM7S256_USART_H */
