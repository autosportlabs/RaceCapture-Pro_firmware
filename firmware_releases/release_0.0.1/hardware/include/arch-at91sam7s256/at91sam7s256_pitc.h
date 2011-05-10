/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_pitc.h
 * 
 * Hardware definition for the pitc peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from PITC_6079A V1.2
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


#ifndef __AT91SAM7S256_PITC_H
#define __AT91SAM7S256_PITC_H

/* -------------------------------------------------------- */
/* PITC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_SYS
#define AT91C_ID_SYS   	 1 /**< System Peripheral id */
#endif /* AT91C_ID_SYS */

/* -------------------------------------------------------- */
/* PITC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_PITC      	0xFFFFFD30 /**< PITC base address */

/* -------------------------------------------------------- */
/* PIO definition for PITC hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for PITC hardware peripheral */
/* -------------------------------------------------------- */
#define PITC_PIMR 	(0x0000) 	/**< Period Interval Mode Register */
#define PITC_PISR 	(0x0004) 	/**< Period Interval Status Register */
#define PITC_PIVR 	(0x0008) 	/**< Period Interval Value Register */
#define PITC_PIIR 	(0x000C) 	/**< Period Interval Image Register */

/* -------------------------------------------------------- */
/* Bitfields definition for PITC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register PITC_PIMR */
#define AT91C_PITC_PIV        (0xFFFFF << 0 ) /**< (PITC) Periodic Interval Value */
#define AT91C_PITC_PITEN      (0x1 << 24) /**< (PITC) Periodic Interval Timer Enabled */
#define AT91C_PITC_PITIEN     (0x1 << 25) /**< (PITC) Periodic Interval Timer Interrupt Enable */
/* --- Register PITC_PISR */
#define AT91C_PITC_PITS       (0x1 << 0 ) /**< (PITC) Periodic Interval Timer Status */
/* --- Register PITC_PIVR */
#define AT91C_PITC_CPIV       (0xFFFFF << 0 ) /**< (PITC) Current Periodic Interval Value */
#define AT91C_PITC_PICNT      (0xFFF << 20) /**< (PITC) Periodic Interval Counter */
/* --- Register PITC_PIIR */
#define AT91C_PITC_CPIV       (0xFFFFF << 0 ) /**< (PITC) Current Periodic Interval Value */
#define AT91C_PITC_PICNT      (0xFFF << 20) /**< (PITC) Periodic Interval Counter */

#endif /* __AT91SAM7S256_PITC_H */
