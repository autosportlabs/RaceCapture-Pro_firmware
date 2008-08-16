/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_ckgr.h
 * 
 * Hardware definition for the ckgr peripheral in the ATMEL at91sam7s256 processor
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


#ifndef __AT91SAM7S256_CKGR_H
#define __AT91SAM7S256_CKGR_H

/* -------------------------------------------------------- */
/* CKGR ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* CKGR Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_CKGR      	0xFFFFFC20 /**< CKGR base address */

/* -------------------------------------------------------- */
/* PIO definition for CKGR hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for CKGR hardware peripheral */
/* -------------------------------------------------------- */
#define CKGR_MOR 	(0x0000) 	/**< Main Oscillator Register */
#define CKGR_MCFR 	(0x0004) 	/**< Main Clock  Frequency Register */
#define CKGR_PLLR 	(0x000C) 	/**< PLL Register */

/* -------------------------------------------------------- */
/* Bitfields definition for CKGR hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register CKGR_MOR */
#define AT91C_CKGR_MOSCEN     (0x1 << 0 ) /**< (CKGR) Main Oscillator Enable */
#define AT91C_CKGR_OSCBYPASS  (0x1 << 1 ) /**< (CKGR) Main Oscillator Bypass */
#define AT91C_CKGR_OSCOUNT    (0xFF << 8 ) /**< (CKGR) Main Oscillator Start-up Time */
/* --- Register CKGR_MCFR */
#define AT91C_CKGR_MAINF      (0xFFFF << 0 ) /**< (CKGR) Main Clock Frequency */
#define AT91C_CKGR_MAINRDY    (0x1 << 16) /**< (CKGR) Main Clock Ready */
/* --- Register CKGR_PLLR */
#define AT91C_CKGR_DIV        (0xFF << 0 ) /**< (CKGR) Divider Selected */
#define 	AT91C_CKGR_DIV_0                    0x0 /**< (CKGR) Divider output is 0 */
#define 	AT91C_CKGR_DIV_BYPASS               0x1 /**< (CKGR) Divider is bypassed */
#define AT91C_CKGR_PLLCOUNT   (0x3F << 8 ) /**< (CKGR) PLL Counter */
#define AT91C_CKGR_OUT        (0x3 << 14) /**< (CKGR) PLL Output Frequency Range */
#define 	AT91C_CKGR_OUT_0                    (0x0 << 14) /**< (CKGR) Please refer to the PLL datasheet */
#define 	AT91C_CKGR_OUT_1                    (0x1 << 14) /**< (CKGR) Please refer to the PLL datasheet */
#define 	AT91C_CKGR_OUT_2                    (0x2 << 14) /**< (CKGR) Please refer to the PLL datasheet */
#define 	AT91C_CKGR_OUT_3                    (0x3 << 14) /**< (CKGR) Please refer to the PLL datasheet */
#define AT91C_CKGR_MUL        (0x7FF << 16) /**< (CKGR) PLL Multiplier */
#define AT91C_CKGR_USBDIV     (0x3 << 28) /**< (CKGR) Divider for USB Clocks */
#define 	AT91C_CKGR_USBDIV_0                    (0x0 << 28) /**< (CKGR) Divider output is PLL clock output */
#define 	AT91C_CKGR_USBDIV_1                    (0x1 << 28) /**< (CKGR) Divider output is PLL clock output divided by 2 */
#define 	AT91C_CKGR_USBDIV_2                    (0x2 << 28) /**< (CKGR) Divider output is PLL clock output divided by 4 */

#endif /* __AT91SAM7S256_CKGR_H */
