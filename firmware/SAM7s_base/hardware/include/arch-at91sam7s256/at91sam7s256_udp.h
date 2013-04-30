/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_udp.h
 * 
 * Hardware definition for the udp peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from UDP_SAM7S V1.1
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


#ifndef __AT91SAM7S256_UDP_H
#define __AT91SAM7S256_UDP_H

/* -------------------------------------------------------- */
/* UDP ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_UDP
#define AT91C_ID_UDP   	11 /**< USB Device Port id */
#endif /* AT91C_ID_UDP */

/* -------------------------------------------------------- */
/* UDP Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_UDP       	0xFFFB0000 /**< UDP base address */

/* -------------------------------------------------------- */
/* PIO definition for UDP hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for UDP hardware peripheral */
/* -------------------------------------------------------- */
#define UDP_NUM 	(0x0000) 	/**< Frame Number Register */
#define UDP_GLBSTATE 	(0x0004) 	/**< Global State Register */
#define UDP_FADDR 	(0x0008) 	/**< Function Address Register */
#define UDP_IER 	(0x0010) 	/**< Interrupt Enable Register */
#define UDP_IDR 	(0x0014) 	/**< Interrupt Disable Register */
#define UDP_IMR 	(0x0018) 	/**< Interrupt Mask Register */
#define UDP_ISR 	(0x001C) 	/**< Interrupt Status Register */
#define UDP_ICR 	(0x0020) 	/**< Interrupt Clear Register */
#define UDP_RSTEP 	(0x0028) 	/**< Reset Endpoint Register */
#define UDP_CSR 	(0x0030) 	/**< Endpoint Control and Status Register */
#define UDP_FDR 	(0x0050) 	/**< Endpoint FIFO Data Register */
#define UDP_TXVC 	(0x0074) 	/**< Transceiver Control Register */

/* -------------------------------------------------------- */
/* Bitfields definition for UDP hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register UDP_FRM_NUM */
#define AT91C_UDP_FRM_NUM     (0x7FF << 0 ) /**< (UDP) Frame Number as Defined in the Packet Field Formats */
#define AT91C_UDP_FRM_ERR     (0x1 << 16) /**< (UDP) Frame Error */
#define AT91C_UDP_FRM_OK      (0x1 << 17) /**< (UDP) Frame OK */
/* --- Register UDP_GLB_STATE */
#define AT91C_UDP_FADDEN      (0x1 << 0 ) /**< (UDP) Function Address Enable */
#define AT91C_UDP_CONFG       (0x1 << 1 ) /**< (UDP) Configured */
#define AT91C_UDP_ESR         (0x1 << 2 ) /**< (UDP) Enable Send Resume */
#define AT91C_UDP_RSMINPR     (0x1 << 3 ) /**< (UDP) A Resume Has Been Sent to the Host */
#define AT91C_UDP_RMWUPE      (0x1 << 4 ) /**< (UDP) Remote Wake Up Enable */
/* --- Register UDP_FADDR */
#define AT91C_UDP_FADD        (0xFF << 0 ) /**< (UDP) Function Address Value */
#define AT91C_UDP_FEN         (0x1 << 8 ) /**< (UDP) Function Enable */
/* --- Register UDP_IER */
#define AT91C_UDP_EPINT0      (0x1 << 0 ) /**< (UDP) Endpoint 0 Interrupt */
#define AT91C_UDP_EPINT1      (0x1 << 1 ) /**< (UDP) Endpoint 0 Interrupt */
#define AT91C_UDP_EPINT2      (0x1 << 2 ) /**< (UDP) Endpoint 2 Interrupt */
#define AT91C_UDP_EPINT3      (0x1 << 3 ) /**< (UDP) Endpoint 3 Interrupt */
#define AT91C_UDP_RXSUSP      (0x1 << 8 ) /**< (UDP) USB Suspend Interrupt */
#define AT91C_UDP_RXRSM       (0x1 << 9 ) /**< (UDP) USB Resume Interrupt */
#define AT91C_UDP_EXTRSM      (0x1 << 10) /**< (UDP) USB External Resume Interrupt */
#define AT91C_UDP_SOFINT      (0x1 << 11) /**< (UDP) USB Start Of frame Interrupt */
#define AT91C_UDP_WAKEUP      (0x1 << 13) /**< (UDP) USB Resume Interrupt */
/* --- Register UDP_IDR */
#define AT91C_UDP_EPINT0      (0x1 << 0 ) /**< (UDP) Endpoint 0 Interrupt */
#define AT91C_UDP_EPINT1      (0x1 << 1 ) /**< (UDP) Endpoint 0 Interrupt */
#define AT91C_UDP_EPINT2      (0x1 << 2 ) /**< (UDP) Endpoint 2 Interrupt */
#define AT91C_UDP_EPINT3      (0x1 << 3 ) /**< (UDP) Endpoint 3 Interrupt */
#define AT91C_UDP_RXSUSP      (0x1 << 8 ) /**< (UDP) USB Suspend Interrupt */
#define AT91C_UDP_RXRSM       (0x1 << 9 ) /**< (UDP) USB Resume Interrupt */
#define AT91C_UDP_EXTRSM      (0x1 << 10) /**< (UDP) USB External Resume Interrupt */
#define AT91C_UDP_SOFINT      (0x1 << 11) /**< (UDP) USB Start Of frame Interrupt */
#define AT91C_UDP_WAKEUP      (0x1 << 13) /**< (UDP) USB Resume Interrupt */
/* --- Register UDP_IMR */
#define AT91C_UDP_EPINT0      (0x1 << 0 ) /**< (UDP) Endpoint 0 Interrupt */
#define AT91C_UDP_EPINT1      (0x1 << 1 ) /**< (UDP) Endpoint 0 Interrupt */
#define AT91C_UDP_EPINT2      (0x1 << 2 ) /**< (UDP) Endpoint 2 Interrupt */
#define AT91C_UDP_EPINT3      (0x1 << 3 ) /**< (UDP) Endpoint 3 Interrupt */
#define AT91C_UDP_RXSUSP      (0x1 << 8 ) /**< (UDP) USB Suspend Interrupt */
#define AT91C_UDP_RXRSM       (0x1 << 9 ) /**< (UDP) USB Resume Interrupt */
#define AT91C_UDP_EXTRSM      (0x1 << 10) /**< (UDP) USB External Resume Interrupt */
#define AT91C_UDP_SOFINT      (0x1 << 11) /**< (UDP) USB Start Of frame Interrupt */
#define AT91C_UDP_WAKEUP      (0x1 << 13) /**< (UDP) USB Resume Interrupt */
/* --- Register UDP_ISR */
#define AT91C_UDP_EPINT0      (0x1 << 0 ) /**< (UDP) Endpoint 0 Interrupt */
#define AT91C_UDP_EPINT1      (0x1 << 1 ) /**< (UDP) Endpoint 0 Interrupt */
#define AT91C_UDP_EPINT2      (0x1 << 2 ) /**< (UDP) Endpoint 2 Interrupt */
#define AT91C_UDP_EPINT3      (0x1 << 3 ) /**< (UDP) Endpoint 3 Interrupt */
#define AT91C_UDP_RXSUSP      (0x1 << 8 ) /**< (UDP) USB Suspend Interrupt */
#define AT91C_UDP_RXRSM       (0x1 << 9 ) /**< (UDP) USB Resume Interrupt */
#define AT91C_UDP_EXTRSM      (0x1 << 10) /**< (UDP) USB External Resume Interrupt */
#define AT91C_UDP_SOFINT      (0x1 << 11) /**< (UDP) USB Start Of frame Interrupt */
#define AT91C_UDP_ENDBUSRES   (0x1 << 12) /**< (UDP) USB End Of Bus Reset Interrupt */
#define AT91C_UDP_WAKEUP      (0x1 << 13) /**< (UDP) USB Resume Interrupt */
/* --- Register UDP_ICR */
#define AT91C_UDP_EPINT0      (0x1 << 0 ) /**< (UDP) Endpoint 0 Interrupt */
#define AT91C_UDP_EPINT1      (0x1 << 1 ) /**< (UDP) Endpoint 0 Interrupt */
#define AT91C_UDP_EPINT2      (0x1 << 2 ) /**< (UDP) Endpoint 2 Interrupt */
#define AT91C_UDP_EPINT3      (0x1 << 3 ) /**< (UDP) Endpoint 3 Interrupt */
#define AT91C_UDP_RXSUSP      (0x1 << 8 ) /**< (UDP) USB Suspend Interrupt */
#define AT91C_UDP_RXRSM       (0x1 << 9 ) /**< (UDP) USB Resume Interrupt */
#define AT91C_UDP_EXTRSM      (0x1 << 10) /**< (UDP) USB External Resume Interrupt */
#define AT91C_UDP_SOFINT      (0x1 << 11) /**< (UDP) USB Start Of frame Interrupt */
#define AT91C_UDP_WAKEUP      (0x1 << 13) /**< (UDP) USB Resume Interrupt */
/* --- Register UDP_RST_EP */
#define AT91C_UDP_EP0         (0x1 << 0 ) /**< (UDP) Reset Endpoint 0 */
#define AT91C_UDP_EP1         (0x1 << 1 ) /**< (UDP) Reset Endpoint 1 */
#define AT91C_UDP_EP2         (0x1 << 2 ) /**< (UDP) Reset Endpoint 2 */
#define AT91C_UDP_EP3         (0x1 << 3 ) /**< (UDP) Reset Endpoint 3 */
/* --- Register UDP_CSR */
#define AT91C_UDP_TXCOMP      (0x1 << 0 ) /**< (UDP) Generates an IN packet with data previously written in the DPR */
#define AT91C_UDP_RX_DATA_BK0 (0x1 << 1 ) /**< (UDP) Receive Data Bank 0 */
#define AT91C_UDP_RXSETUP     (0x1 << 2 ) /**< (UDP) Sends STALL to the Host (Control endpoints) */
#define AT91C_UDP_ISOERROR    (0x1 << 3 ) /**< (UDP) Isochronous error (Isochronous endpoints) */
#define AT91C_UDP_TXPKTRDY    (0x1 << 4 ) /**< (UDP) Transmit Packet Ready */
#define AT91C_UDP_FORCESTALL  (0x1 << 5 ) /**< (UDP) Force Stall (used by Control, Bulk and Isochronous endpoints). */
#define AT91C_UDP_RX_DATA_BK1 (0x1 << 6 ) /**< (UDP) Receive Data Bank 1 (only used by endpoints with ping-pong attributes). */
#define AT91C_UDP_DIR         (0x1 << 7 ) /**< (UDP) Transfer Direction */
#define AT91C_UDP_EPTYPE      (0x7 << 8 ) /**< (UDP) Endpoint type */
#define 	AT91C_UDP_EPTYPE_CTRL                 (0x0 <<  8) /**< (UDP) Control */
#define 	AT91C_UDP_EPTYPE_ISO_OUT              (0x1 <<  8) /**< (UDP) Isochronous OUT */
#define 	AT91C_UDP_EPTYPE_BULK_OUT             (0x2 <<  8) /**< (UDP) Bulk OUT */
#define 	AT91C_UDP_EPTYPE_INT_OUT              (0x3 <<  8) /**< (UDP) Interrupt OUT */
#define 	AT91C_UDP_EPTYPE_ISO_IN               (0x5 <<  8) /**< (UDP) Isochronous IN */
#define 	AT91C_UDP_EPTYPE_BULK_IN              (0x6 <<  8) /**< (UDP) Bulk IN */
#define 	AT91C_UDP_EPTYPE_INT_IN               (0x7 <<  8) /**< (UDP) Interrupt IN */
#define AT91C_UDP_DTGLE       (0x1 << 11) /**< (UDP) Data Toggle */
#define AT91C_UDP_EPEDS       (0x1 << 15) /**< (UDP) Endpoint Enable Disable */
#define AT91C_UDP_RXBYTECNT   (0x7FF << 16) /**< (UDP) Number Of Bytes Available in the FIFO */
/* --- Register UDP_TXVC */
#define AT91C_UDP_TXVDIS      (0x1 << 8 ) /**< (UDP)  */

#endif /* __AT91SAM7S256_UDP_H */
