/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_rttc.h
 * 
 * Hardware definition for the rttc peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from RTTC_6081A V1.2
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


#ifndef __AT91SAM7S256_RTTC_H
#define __AT91SAM7S256_RTTC_H

/* -------------------------------------------------------- */
/* RTTC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_SYS
#define AT91C_ID_SYS   	 1 /**< System Peripheral id */
#endif /* AT91C_ID_SYS */

/* -------------------------------------------------------- */
/* RTTC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_RTTC      	0xFFFFFD20 /**< RTTC base address */

/* -------------------------------------------------------- */
/* PIO definition for RTTC hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for RTTC hardware peripheral */
/* -------------------------------------------------------- */
#define RTTC_RTMR 	(0x0000) 	/**< Real-time Mode Register */
#define RTTC_RTAR 	(0x0004) 	/**< Real-time Alarm Register */
#define RTTC_RTVR 	(0x0008) 	/**< Real-time Value Register */
#define RTTC_RTSR 	(0x000C) 	/**< Real-time Status Register */

/* -------------------------------------------------------- */
/* Bitfields definition for RTTC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register RTTC_RTMR */
#define AT91C_RTTC_RTPRES     (0xFFFF << 0 ) /**< (RTTC) Real-time Timer Prescaler Value */
#define AT91C_RTTC_ALMIEN     (0x1 << 16) /**< (RTTC) Alarm Interrupt Enable */
#define AT91C_RTTC_RTTINCIEN  (0x1 << 17) /**< (RTTC) Real Time Timer Increment Interrupt Enable */
#define AT91C_RTTC_RTTRST     (0x1 << 18) /**< (RTTC) Real Time Timer Restart */
/* --- Register RTTC_RTAR */
#define AT91C_RTTC_ALMV       (0x0 << 0 ) /**< (RTTC) Alarm Value */
/* --- Register RTTC_RTVR */
#define AT91C_RTTC_CRTV       (0x0 << 0 ) /**< (RTTC) Current Real-time Value */
/* --- Register RTTC_RTSR */
#define AT91C_RTTC_ALMS       (0x1 << 0 ) /**< (RTTC) Real-time Alarm Status */
#define AT91C_RTTC_RTTINC     (0x1 << 1 ) /**< (RTTC) Real-time Timer Increment */

#endif /* __AT91SAM7S256_RTTC_H */
