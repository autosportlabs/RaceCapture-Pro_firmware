/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_pwmc_ch.h
 * 
 * Hardware definition for the pwmc_ch peripheral in the ATMEL at91sam7s256 processor
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


#ifndef __AT91SAM7S256_PWMC_CH_H
#define __AT91SAM7S256_PWMC_CH_H

/* -------------------------------------------------------- */
/* PWMC_CH ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* PWMC_CH Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_PWMC_CH3  	0xFFFCC260 /**< PWMC_CH3 base address */
#define AT91C_BASE_PWMC_CH2  	0xFFFCC240 /**< PWMC_CH2 base address */
#define AT91C_BASE_PWMC_CH1  	0xFFFCC220 /**< PWMC_CH1 base address */
#define AT91C_BASE_PWMC_CH0  	0xFFFCC200 /**< PWMC_CH0 base address */

/* -------------------------------------------------------- */
/* PIO definition for PWMC_CH hardware peripheral */
/* -------------------------------------------------------- */
#define AT91C_PA11_PWM0     	(1 << 11) /**< PWM Channel 0 */

#define AT91C_PA12_PWM1     	(1 << 12) /**< PWM Channel 1 */

#define AT91C_PA13_PWM2     	(1 << 13) /**< PWM Channel 2 */

#define AT91C_PA7_PWM3     	(1 << 7) /**< PWM Channel 3 */


/* -------------------------------------------------------- */
/* Register offset definition for PWMC_CH hardware peripheral */
/* -------------------------------------------------------- */
#define PWMC_CMR 	(0x0000) 	/**< Channel Mode Register */
#define PWMC_CDTYR 	(0x0004) 	/**< Channel Duty Cycle Register */
#define PWMC_CPRDR 	(0x0008) 	/**< Channel Period Register */
#define PWMC_CCNTR 	(0x000C) 	/**< Channel Counter Register */
#define PWMC_CUPDR 	(0x0010) 	/**< Channel Update Register */
#define PWMC_Reserved 	(0x0014) 	/**< Reserved */

/* -------------------------------------------------------- */
/* Bitfields definition for PWMC_CH hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register PWMC_CMR */
#define AT91C_PWMC_CPRE       (0xF << 0 ) /**< (PWMC_CH) Channel Pre-scaler : PWMC_CLKx */
#define 	AT91C_PWMC_CPRE_MCK                  0x0 /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCK/2                0x1 /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCK/4                0x2 /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCK/8                0x3 /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCK/16               0x4 /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCK/32               0x5 /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCK/64               0x6 /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCK/128              0x7 /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCK/256              0x8 /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCK/512              0x9 /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCK/1024             0xA /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCKA                 0xB /**< (PWMC_CH)  */
#define 	AT91C_PWMC_CPRE_MCKB                 0xC /**< (PWMC_CH)  */
#define AT91C_PWMC_CALG       (0x1 << 8 ) /**< (PWMC_CH) Channel Alignment */
#define AT91C_PWMC_CPOL       (0x1 << 9 ) /**< (PWMC_CH) Channel Polarity */
#define AT91C_PWMC_CPD        (0x1 << 10) /**< (PWMC_CH) Channel Update Period */
/* --- Register PWMC_CDTYR */
#define AT91C_PWMC_CDTY       (0x0 << 0 ) /**< (PWMC_CH) Channel Duty Cycle */
/* --- Register PWMC_CPRDR */
#define AT91C_PWMC_CPRD       (0x0 << 0 ) /**< (PWMC_CH) Channel Period */
/* --- Register PWMC_CCNTR */
#define AT91C_PWMC_CCNT       (0x0 << 0 ) /**< (PWMC_CH) Channel Counter */
/* --- Register PWMC_CUPDR */
#define AT91C_PWMC_CUPD       (0x0 << 0 ) /**< (PWMC_CH) Channel Update */

#endif /* __AT91SAM7S256_PWMC_CH_H */
