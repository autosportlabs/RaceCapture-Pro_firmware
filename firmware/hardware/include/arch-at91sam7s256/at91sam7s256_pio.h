/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_pio.h
 * 
 * Hardware definition for the pio peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from PIO_6057A V1.2
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


#ifndef __AT91SAM7S256_PIO_H
#define __AT91SAM7S256_PIO_H

/* -------------------------------------------------------- */
/* PIO ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_PIOA
#define AT91C_ID_PIOA  	 2 /**< Parallel IO Controller id */
#endif /* AT91C_ID_PIOA */

/* -------------------------------------------------------- */
/* PIO Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_PIOA      	0xFFFFF400 /**< PIOA base address */

/* -------------------------------------------------------- */
/* PIO definition for PIO hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for PIO hardware peripheral */
/* -------------------------------------------------------- */
#define PIO_PER 	(0x0000) 	/**< PIO Enable Register */
#define PIO_PDR 	(0x0004) 	/**< PIO Disable Register */
#define PIO_PSR 	(0x0008) 	/**< PIO Status Register */
#define PIO_OER 	(0x0010) 	/**< Output Enable Register */
#define PIO_ODR 	(0x0014) 	/**< Output Disable Registerr */
#define PIO_OSR 	(0x0018) 	/**< Output Status Register */
#define PIO_IFER 	(0x0020) 	/**< Input Filter Enable Register */
#define PIO_IFDR 	(0x0024) 	/**< Input Filter Disable Register */
#define PIO_IFSR 	(0x0028) 	/**< Input Filter Status Register */
#define PIO_SODR 	(0x0030) 	/**< Set Output Data Register */
#define PIO_CODR 	(0x0034) 	/**< Clear Output Data Register */
#define PIO_ODSR 	(0x0038) 	/**< Output Data Status Register */
#define PIO_PDSR 	(0x003C) 	/**< Pin Data Status Register */
#define PIO_IER 	(0x0040) 	/**< Interrupt Enable Register */
#define PIO_IDR 	(0x0044) 	/**< Interrupt Disable Register */
#define PIO_IMR 	(0x0048) 	/**< Interrupt Mask Register */
#define PIO_ISR 	(0x004C) 	/**< Interrupt Status Register */
#define PIO_MDER 	(0x0050) 	/**< Multi-driver Enable Register */
#define PIO_MDDR 	(0x0054) 	/**< Multi-driver Disable Register */
#define PIO_MDSR 	(0x0058) 	/**< Multi-driver Status Register */
#define PIO_PPUDR 	(0x0060) 	/**< Pull-up Disable Register */
#define PIO_PPUER 	(0x0064) 	/**< Pull-up Enable Register */
#define PIO_PPUSR 	(0x0068) 	/**< Pull-up Status Register */
#define PIO_ASR 	(0x0070) 	/**< Select A Register */
#define PIO_BSR 	(0x0074) 	/**< Select B Register */
#define PIO_ABSR 	(0x0078) 	/**< AB Select Status Register */
#define PIO_OWER 	(0x00A0) 	/**< Output Write Enable Register */
#define PIO_OWDR 	(0x00A4) 	/**< Output Write Disable Register */
#define PIO_OWSR 	(0x00A8) 	/**< Output Write Status Register */

/* -------------------------------------------------------- */
/* Bitfields definition for PIO hardware peripheral */
/* -------------------------------------------------------- */

#endif /* __AT91SAM7S256_PIO_H */
