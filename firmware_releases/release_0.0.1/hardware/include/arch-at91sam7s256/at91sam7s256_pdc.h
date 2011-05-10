/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_pdc.h
 * 
 * Hardware definition for the pdc peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from PDC_6074C V1.2
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


#ifndef __AT91SAM7S256_PDC_H
#define __AT91SAM7S256_PDC_H

/* -------------------------------------------------------- */
/* PDC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* PDC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_PDC_DBGU  	0xFFFFF300 /**< PDC_DBGU base address */
#define AT91C_BASE_PDC_SPI   	0xFFFE0100 /**< PDC_SPI base address */
#define AT91C_BASE_PDC_ADC   	0xFFFD8100 /**< PDC_ADC base address */
#define AT91C_BASE_PDC_SSC   	0xFFFD4100 /**< PDC_SSC base address */
#define AT91C_BASE_PDC_US1   	0xFFFC4100 /**< PDC_US1 base address */
#define AT91C_BASE_PDC_US0   	0xFFFC0100 /**< PDC_US0 base address */

/* -------------------------------------------------------- */
/* PIO definition for PDC hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for PDC hardware peripheral */
/* -------------------------------------------------------- */
#define PDC_RPR 	(0x0000) 	/**< Receive Pointer Register */
#define PDC_RCR 	(0x0004) 	/**< Receive Counter Register */
#define PDC_TPR 	(0x0008) 	/**< Transmit Pointer Register */
#define PDC_TCR 	(0x000C) 	/**< Transmit Counter Register */
#define PDC_RNPR 	(0x0010) 	/**< Receive Next Pointer Register */
#define PDC_RNCR 	(0x0014) 	/**< Receive Next Counter Register */
#define PDC_TNPR 	(0x0018) 	/**< Transmit Next Pointer Register */
#define PDC_TNCR 	(0x001C) 	/**< Transmit Next Counter Register */
#define PDC_PTCR 	(0x0020) 	/**< PDC Transfer Control Register */
#define PDC_PTSR 	(0x0024) 	/**< PDC Transfer Status Register */

/* -------------------------------------------------------- */
/* Bitfields definition for PDC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register PDC_PTCR */
#define AT91C_PDC_RXTEN       (0x1 << 0 ) /**< (PDC) Receiver Transfer Enable */
#define AT91C_PDC_RXTDIS      (0x1 << 1 ) /**< (PDC) Receiver Transfer Disable */
#define AT91C_PDC_TXTEN       (0x1 << 8 ) /**< (PDC) Transmitter Transfer Enable */
#define AT91C_PDC_TXTDIS      (0x1 << 9 ) /**< (PDC) Transmitter Transfer Disable */
/* --- Register PDC_PTSR */
#define AT91C_PDC_RXTEN       (0x1 << 0 ) /**< (PDC) Receiver Transfer Enable */
#define AT91C_PDC_TXTEN       (0x1 << 8 ) /**< (PDC) Transmitter Transfer Enable */

#endif /* __AT91SAM7S256_PDC_H */
