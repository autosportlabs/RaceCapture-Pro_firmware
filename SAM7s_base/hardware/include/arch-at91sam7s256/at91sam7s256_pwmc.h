/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_pwmc.h
 * 
 * Hardware definition for the pwmc peripheral in the ATMEL at91sam7s256 processor
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


#ifndef __AT91SAM7S256_PWMC_H
#define __AT91SAM7S256_PWMC_H

/* -------------------------------------------------------- */
/* PWMC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_PWMC
#define AT91C_ID_PWMC  	10 /**< PWM Controller id */
#endif /* AT91C_ID_PWMC */

/* -------------------------------------------------------- */
/* PWMC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_PWMC      	0xFFFCC000 /**< PWMC base address */

/* -------------------------------------------------------- */
/* PIO definition for PWMC hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for PWMC hardware peripheral */
/* -------------------------------------------------------- */
#define PWMC_MR 	(0x0000) 	/**< PWMC Mode Register */
#define PWMC_ENA 	(0x0004) 	/**< PWMC Enable Register */
#define PWMC_DIS 	(0x0008) 	/**< PWMC Disable Register */
#define PWMC_SR 	(0x000C) 	/**< PWMC Status Register */
#define PWMC_IER 	(0x0010) 	/**< PWMC Interrupt Enable Register */
#define PWMC_IDR 	(0x0014) 	/**< PWMC Interrupt Disable Register */
#define PWMC_IMR 	(0x0018) 	/**< PWMC Interrupt Mask Register */
#define PWMC_ISR 	(0x001C) 	/**< PWMC Interrupt Status Register */
#define PWMC_VR 	(0x00FC) 	/**< PWMC Version Register */
#define PWMC_CH 	(0x0200) 	/**< PWMC Channel */

/* -------------------------------------------------------- */
/* Bitfields definition for PWMC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register PWMC_MR */
#define AT91C_PWMC_DIVA       (0xFF << 0 ) /**< (PWMC) CLKA divide factor. */
#define AT91C_PWMC_PREA       (0xF << 8 ) /**< (PWMC) Divider Input Clock Prescaler A */
#define 	AT91C_PWMC_PREA_MCK                  (0x0 <<  8) /**< (PWMC)  */
#define 	AT91C_PWMC_PREA_MCK/2                (0x1 <<  8) /**< (PWMC)  */
#define 	AT91C_PWMC_PREA_MCK/4                (0x2 <<  8) /**< (PWMC)  */
#define 	AT91C_PWMC_PREA_MCK/8                (0x3 <<  8) /**< (PWMC)  */
#define 	AT91C_PWMC_PREA_MCK/16               (0x4 <<  8) /**< (PWMC)  */
#define 	AT91C_PWMC_PREA_MCK/32               (0x5 <<  8) /**< (PWMC)  */
#define 	AT91C_PWMC_PREA_MCK/64               (0x6 <<  8) /**< (PWMC)  */
#define 	AT91C_PWMC_PREA_MCK/128              (0x7 <<  8) /**< (PWMC)  */
#define 	AT91C_PWMC_PREA_MCK/256              (0x8 <<  8) /**< (PWMC)  */
#define AT91C_PWMC_DIVB       (0xFF << 16) /**< (PWMC) CLKB divide factor. */
#define AT91C_PWMC_PREB       (0xF << 24) /**< (PWMC) Divider Input Clock Prescaler B */
#define 	AT91C_PWMC_PREB_MCK                  (0x0 << 24) /**< (PWMC)  */
#define 	AT91C_PWMC_PREB_MCK/2                (0x1 << 24) /**< (PWMC)  */
#define 	AT91C_PWMC_PREB_MCK/4                (0x2 << 24) /**< (PWMC)  */
#define 	AT91C_PWMC_PREB_MCK/8                (0x3 << 24) /**< (PWMC)  */
#define 	AT91C_PWMC_PREB_MCK/16               (0x4 << 24) /**< (PWMC)  */
#define 	AT91C_PWMC_PREB_MCK/32               (0x5 << 24) /**< (PWMC)  */
#define 	AT91C_PWMC_PREB_MCK/64               (0x6 << 24) /**< (PWMC)  */
#define 	AT91C_PWMC_PREB_MCK/128              (0x7 << 24) /**< (PWMC)  */
#define 	AT91C_PWMC_PREB_MCK/256              (0x8 << 24) /**< (PWMC)  */
/* --- Register PWMC_ENA */
#define AT91C_PWMC_CHID0      (0x1 << 0 ) /**< (PWMC) Channel ID 0 */
#define AT91C_PWMC_CHID1      (0x1 << 1 ) /**< (PWMC) Channel ID 1 */
#define AT91C_PWMC_CHID2      (0x1 << 2 ) /**< (PWMC) Channel ID 2 */
#define AT91C_PWMC_CHID3      (0x1 << 3 ) /**< (PWMC) Channel ID 3 */
/* --- Register PWMC_DIS */
#define AT91C_PWMC_CHID0      (0x1 << 0 ) /**< (PWMC) Channel ID 0 */
#define AT91C_PWMC_CHID1      (0x1 << 1 ) /**< (PWMC) Channel ID 1 */
#define AT91C_PWMC_CHID2      (0x1 << 2 ) /**< (PWMC) Channel ID 2 */
#define AT91C_PWMC_CHID3      (0x1 << 3 ) /**< (PWMC) Channel ID 3 */
/* --- Register PWMC_SR */
#define AT91C_PWMC_CHID0      (0x1 << 0 ) /**< (PWMC) Channel ID 0 */
#define AT91C_PWMC_CHID1      (0x1 << 1 ) /**< (PWMC) Channel ID 1 */
#define AT91C_PWMC_CHID2      (0x1 << 2 ) /**< (PWMC) Channel ID 2 */
#define AT91C_PWMC_CHID3      (0x1 << 3 ) /**< (PWMC) Channel ID 3 */
/* --- Register PWMC_IER */
#define AT91C_PWMC_CHID0      (0x1 << 0 ) /**< (PWMC) Channel ID 0 */
#define AT91C_PWMC_CHID1      (0x1 << 1 ) /**< (PWMC) Channel ID 1 */
#define AT91C_PWMC_CHID2      (0x1 << 2 ) /**< (PWMC) Channel ID 2 */
#define AT91C_PWMC_CHID3      (0x1 << 3 ) /**< (PWMC) Channel ID 3 */
/* --- Register PWMC_IDR */
#define AT91C_PWMC_CHID0      (0x1 << 0 ) /**< (PWMC) Channel ID 0 */
#define AT91C_PWMC_CHID1      (0x1 << 1 ) /**< (PWMC) Channel ID 1 */
#define AT91C_PWMC_CHID2      (0x1 << 2 ) /**< (PWMC) Channel ID 2 */
#define AT91C_PWMC_CHID3      (0x1 << 3 ) /**< (PWMC) Channel ID 3 */
/* --- Register PWMC_IMR */
#define AT91C_PWMC_CHID0      (0x1 << 0 ) /**< (PWMC) Channel ID 0 */
#define AT91C_PWMC_CHID1      (0x1 << 1 ) /**< (PWMC) Channel ID 1 */
#define AT91C_PWMC_CHID2      (0x1 << 2 ) /**< (PWMC) Channel ID 2 */
#define AT91C_PWMC_CHID3      (0x1 << 3 ) /**< (PWMC) Channel ID 3 */
/* --- Register PWMC_ISR */
#define AT91C_PWMC_CHID0      (0x1 << 0 ) /**< (PWMC) Channel ID 0 */
#define AT91C_PWMC_CHID1      (0x1 << 1 ) /**< (PWMC) Channel ID 1 */
#define AT91C_PWMC_CHID2      (0x1 << 2 ) /**< (PWMC) Channel ID 2 */
#define AT91C_PWMC_CHID3      (0x1 << 3 ) /**< (PWMC) Channel ID 3 */

#endif /* __AT91SAM7S256_PWMC_H */
