/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_rstc.h
 * 
 * Hardware definition for the rstc peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from RSTC_SAM7S V1.2
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


#ifndef __AT91SAM7S256_RSTC_H
#define __AT91SAM7S256_RSTC_H

/* -------------------------------------------------------- */
/* RSTC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_SYS
#define AT91C_ID_SYS   	 1 /**< System Peripheral id */
#endif /* AT91C_ID_SYS */

/* -------------------------------------------------------- */
/* RSTC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_RSTC      	0xFFFFFD00 /**< RSTC base address */

/* -------------------------------------------------------- */
/* PIO definition for RSTC hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for RSTC hardware peripheral */
/* -------------------------------------------------------- */
#define RSTC_RCR 	(0x0000) 	/**< Reset Control Register */
#define RSTC_RSR 	(0x0004) 	/**< Reset Status Register */
#define RSTC_RMR 	(0x0008) 	/**< Reset Mode Register */

/* -------------------------------------------------------- */
/* Bitfields definition for RSTC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register RSTC_RCR */
#define AT91C_RSTC_PROCRST    (0x1 << 0 ) /**< (RSTC) Processor Reset */
#define AT91C_RSTC_PERRST     (0x1 << 2 ) /**< (RSTC) Peripheral Reset */
#define AT91C_RSTC_EXTRST     (0x1 << 3 ) /**< (RSTC) External Reset */
#define AT91C_RSTC_KEY        (0xFF << 24) /**< (RSTC) Password */
/* --- Register RSTC_RSR */
#define AT91C_RSTC_URSTS      (0x1 << 0 ) /**< (RSTC) User Reset Status */
#define AT91C_RSTC_BODSTS     (0x1 << 1 ) /**< (RSTC) Brownout Detection Status */
#define AT91C_RSTC_RSTTYP     (0x7 << 8 ) /**< (RSTC) Reset Type */
#define 	AT91C_RSTC_RSTTYP_POWERUP              (0x0 <<  8) /**< (RSTC) Power-up Reset. VDDCORE rising. */
#define 	AT91C_RSTC_RSTTYP_WAKEUP               (0x1 <<  8) /**< (RSTC) WakeUp Reset. VDDCORE rising. */
#define 	AT91C_RSTC_RSTTYP_WATCHDOG             (0x2 <<  8) /**< (RSTC) Watchdog Reset. Watchdog overflow occured. */
#define 	AT91C_RSTC_RSTTYP_SOFTWARE             (0x3 <<  8) /**< (RSTC) Software Reset. Processor reset required by the software. */
#define 	AT91C_RSTC_RSTTYP_USER                 (0x4 <<  8) /**< (RSTC) User Reset. NRST pin detected low. */
#define 	AT91C_RSTC_RSTTYP_BROWNOUT             (0x5 <<  8) /**< (RSTC) Brownout Reset occured. */
#define AT91C_RSTC_NRSTL      (0x1 << 16) /**< (RSTC) NRST pin level */
#define AT91C_RSTC_SRCMP      (0x1 << 17) /**< (RSTC) Software Reset Command in Progress. */
/* --- Register RSTC_RMR */
#define AT91C_RSTC_URSTEN     (0x1 << 0 ) /**< (RSTC) User Reset Enable */
#define AT91C_RSTC_URSTIEN    (0x1 << 4 ) /**< (RSTC) User Reset Interrupt Enable */
#define AT91C_RSTC_ERSTL      (0xF << 8 ) /**< (RSTC) User Reset Length */
#define AT91C_RSTC_BODIEN     (0x1 << 16) /**< (RSTC) Brownout Detection Interrupt Enable */
#define AT91C_RSTC_KEY        (0xFF << 24) /**< (RSTC) Password */

#endif /* __AT91SAM7S256_RSTC_H */
