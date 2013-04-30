/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_wdtc.h
 * 
 * Hardware definition for the wdtc peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from WDTC_6080A V1.3
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


#ifndef __AT91SAM7S256_WDTC_H
#define __AT91SAM7S256_WDTC_H

/* -------------------------------------------------------- */
/* WDTC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_SYS
#define AT91C_ID_SYS   	 1 /**< System Peripheral id */
#endif /* AT91C_ID_SYS */

/* -------------------------------------------------------- */
/* WDTC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_WDTC      	0xFFFFFD40 /**< WDTC base address */

/* -------------------------------------------------------- */
/* PIO definition for WDTC hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for WDTC hardware peripheral */
/* -------------------------------------------------------- */
#define WDTC_WDCR 	(0x0000) 	/**< Watchdog Control Register */
#define WDTC_WDMR 	(0x0004) 	/**< Watchdog Mode Register */
#define WDTC_WDSR 	(0x0008) 	/**< Watchdog Status Register */

/* -------------------------------------------------------- */
/* Bitfields definition for WDTC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register WDTC_WDCR */
#define AT91C_WDTC_WDRSTT     (0x1 << 0 ) /**< (WDTC) Watchdog Restart */
#define AT91C_WDTC_KEY        (0xFF << 24) /**< (WDTC) Watchdog KEY Password */
/* --- Register WDTC_WDMR */
#define AT91C_WDTC_WDV        (0xFFF << 0 ) /**< (WDTC) Watchdog Timer Restart */
#define AT91C_WDTC_WDFIEN     (0x1 << 12) /**< (WDTC) Watchdog Fault Interrupt Enable */
#define AT91C_WDTC_WDRSTEN    (0x1 << 13) /**< (WDTC) Watchdog Reset Enable */
#define AT91C_WDTC_WDRPROC    (0x1 << 14) /**< (WDTC) Watchdog Timer Restart */
#define AT91C_WDTC_WDDIS      (0x1 << 15) /**< (WDTC) Watchdog Disable */
#define AT91C_WDTC_WDD        (0xFFF << 16) /**< (WDTC) Watchdog Delta Value */
#define AT91C_WDTC_WDDBGHLT   (0x1 << 28) /**< (WDTC) Watchdog Debug Halt */
#define AT91C_WDTC_WDIDLEHLT  (0x1 << 29) /**< (WDTC) Watchdog Idle Halt */
/* --- Register WDTC_WDSR */
#define AT91C_WDTC_WDUNF      (0x1 << 0 ) /**< (WDTC) Watchdog Underflow */
#define AT91C_WDTC_WDERR      (0x1 << 1 ) /**< (WDTC) Watchdog Error */

#endif /* __AT91SAM7S256_WDTC_H */
