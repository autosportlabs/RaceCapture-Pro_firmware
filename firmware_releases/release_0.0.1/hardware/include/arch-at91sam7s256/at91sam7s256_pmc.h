/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_pmc.h
 * 
 * Hardware definition for the pmc peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from PMC_SAM7S_USB V1.4
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


#ifndef __AT91SAM7S256_PMC_H
#define __AT91SAM7S256_PMC_H

/* -------------------------------------------------------- */
/* PMC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_SYS
#define AT91C_ID_SYS   	 1 /**< System Peripheral id */
#endif /* AT91C_ID_SYS */

/* -------------------------------------------------------- */
/* PMC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_PMC       	0xFFFFFC00 /**< PMC base address */

/* -------------------------------------------------------- */
/* PIO definition for PMC hardware peripheral */
/* -------------------------------------------------------- */
#define AT91C_PA6_PCK0     	(1 << 6) /**< PMC Programmable Clock Output 0 */
#define AT91C_PA21_PCK1     	(1 << 21) /**< PMC Programmable Clock Output 1 */
#define AT91C_PA18_PCK2     	(1 << 18) /**< PMC Programmable Clock Output 2 */


/* -------------------------------------------------------- */
/* Register offset definition for PMC hardware peripheral */
/* -------------------------------------------------------- */
#define PMC_SCER 	(0x0000) 	/**< System Clock Enable Register */
#define PMC_SCDR 	(0x0004) 	/**< System Clock Disable Register */
#define PMC_SCSR 	(0x0008) 	/**< System Clock Status Register */
#define PMC_PCER 	(0x0010) 	/**< Peripheral Clock Enable Register */
#define PMC_PCDR 	(0x0014) 	/**< Peripheral Clock Disable Register */
#define PMC_PCSR 	(0x0018) 	/**< Peripheral Clock Status Register */
#define PMC_MOR 	(0x0020) 	/**< Main Oscillator Register */
#define PMC_MCFR 	(0x0024) 	/**< Main Clock  Frequency Register */
#define PMC_PLLR 	(0x002C) 	/**< PLL Register */
#define PMC_MCKR 	(0x0030) 	/**< Master Clock Register */
#define PMC_PCKR 	(0x0040) 	/**< Programmable Clock Register */
#define PMC_IER 	(0x0060) 	/**< Interrupt Enable Register */
#define PMC_IDR 	(0x0064) 	/**< Interrupt Disable Register */
#define PMC_SR 	(0x0068) 	/**< Status Register */
#define PMC_IMR 	(0x006C) 	/**< Interrupt Mask Register */

/* -------------------------------------------------------- */
/* Bitfields definition for PMC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register PMC_SCER */
#define AT91C_PMC_PCK         (0x1 << 0 ) /**< (PMC) Processor Clock */
#define AT91C_PMC_UDP         (0x1 << 7 ) /**< (PMC) USB Device Port Clock */
#define AT91C_PMC_PCK0        (0x1 << 8 ) /**< (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK1        (0x1 << 9 ) /**< (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK2        (0x1 << 10) /**< (PMC) Programmable Clock Output */
/* --- Register PMC_SCDR */
#define AT91C_PMC_PCK         (0x1 << 0 ) /**< (PMC) Processor Clock */
#define AT91C_PMC_UDP         (0x1 << 7 ) /**< (PMC) USB Device Port Clock */
#define AT91C_PMC_PCK0        (0x1 << 8 ) /**< (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK1        (0x1 << 9 ) /**< (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK2        (0x1 << 10) /**< (PMC) Programmable Clock Output */
/* --- Register PMC_SCSR */
#define AT91C_PMC_PCK         (0x1 << 0 ) /**< (PMC) Processor Clock */
#define AT91C_PMC_UDP         (0x1 << 7 ) /**< (PMC) USB Device Port Clock */
#define AT91C_PMC_PCK0        (0x1 << 8 ) /**< (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK1        (0x1 << 9 ) /**< (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK2        (0x1 << 10) /**< (PMC) Programmable Clock Output */
/* --- Register CKGR_MOR */
#define AT91C_CKGR_MOSCEN     (0x1 << 0 ) /**< (PMC) Main Oscillator Enable */
#define AT91C_CKGR_OSCBYPASS  (0x1 << 1 ) /**< (PMC) Main Oscillator Bypass */
#define AT91C_CKGR_OSCOUNT    (0xFF << 8 ) /**< (PMC) Main Oscillator Start-up Time */
/* --- Register CKGR_MCFR */
#define AT91C_CKGR_MAINF      (0xFFFF << 0 ) /**< (PMC) Main Clock Frequency */
#define AT91C_CKGR_MAINRDY    (0x1 << 16) /**< (PMC) Main Clock Ready */
/* --- Register CKGR_PLLR */
#define AT91C_CKGR_DIV        (0xFF << 0 ) /**< (PMC) Divider Selected */
#define 	AT91C_CKGR_DIV_0                    0x0 /**< (PMC) Divider output is 0 */
#define 	AT91C_CKGR_DIV_BYPASS               0x1 /**< (PMC) Divider is bypassed */
#define AT91C_CKGR_PLLCOUNT   (0x3F << 8 ) /**< (PMC) PLL Counter */
#define AT91C_CKGR_OUT        (0x3 << 14) /**< (PMC) PLL Output Frequency Range */
#define 	AT91C_CKGR_OUT_0                    (0x0 << 14) /**< (PMC) Please refer to the PLL datasheet */
#define 	AT91C_CKGR_OUT_1                    (0x1 << 14) /**< (PMC) Please refer to the PLL datasheet */
#define 	AT91C_CKGR_OUT_2                    (0x2 << 14) /**< (PMC) Please refer to the PLL datasheet */
#define 	AT91C_CKGR_OUT_3                    (0x3 << 14) /**< (PMC) Please refer to the PLL datasheet */
#define AT91C_CKGR_MUL        (0x7FF << 16) /**< (PMC) PLL Multiplier */
#define AT91C_CKGR_USBDIV     (0x3 << 28) /**< (PMC) Divider for USB Clocks */
#define 	AT91C_CKGR_USBDIV_0                    (0x0 << 28) /**< (PMC) Divider output is PLL clock output */
#define 	AT91C_CKGR_USBDIV_1                    (0x1 << 28) /**< (PMC) Divider output is PLL clock output divided by 2 */
#define 	AT91C_CKGR_USBDIV_2                    (0x2 << 28) /**< (PMC) Divider output is PLL clock output divided by 4 */
/* --- Register PMC_MCKR */
#define AT91C_PMC_CSS         (0x3 << 0 ) /**< (PMC) Programmable Clock Selection */
#define 	AT91C_PMC_CSS_SLOW_CLK             0x0 /**< (PMC) Slow Clock is selected */
#define 	AT91C_PMC_CSS_MAIN_CLK             0x1 /**< (PMC) Main Clock is selected */
#define 	AT91C_PMC_CSS_PLL_CLK              0x3 /**< (PMC) Clock from PLL is selected */
#define AT91C_PMC_PRES        (0x7 << 2 ) /**< (PMC) Programmable Clock Prescaler */
#define 	AT91C_PMC_PRES_CLK                  (0x0 <<  2) /**< (PMC) Selected clock */
#define 	AT91C_PMC_PRES_CLK_2                (0x1 <<  2) /**< (PMC) Selected clock divided by 2 */
#define 	AT91C_PMC_PRES_CLK_4                (0x2 <<  2) /**< (PMC) Selected clock divided by 4 */
#define 	AT91C_PMC_PRES_CLK_8                (0x3 <<  2) /**< (PMC) Selected clock divided by 8 */
#define 	AT91C_PMC_PRES_CLK_16               (0x4 <<  2) /**< (PMC) Selected clock divided by 16 */
#define 	AT91C_PMC_PRES_CLK_32               (0x5 <<  2) /**< (PMC) Selected clock divided by 32 */
#define 	AT91C_PMC_PRES_CLK_64               (0x6 <<  2) /**< (PMC) Selected clock divided by 64 */
/* --- Register PMC_PCKR */
#define AT91C_PMC_CSS         (0x3 << 0 ) /**< (PMC) Programmable Clock Selection */
#define 	AT91C_PMC_CSS_SLOW_CLK             0x0 /**< (PMC) Slow Clock is selected */
#define 	AT91C_PMC_CSS_MAIN_CLK             0x1 /**< (PMC) Main Clock is selected */
#define 	AT91C_PMC_CSS_PLL_CLK              0x3 /**< (PMC) Clock from PLL is selected */
#define AT91C_PMC_PRES        (0x7 << 2 ) /**< (PMC) Programmable Clock Prescaler */
#define 	AT91C_PMC_PRES_CLK                  (0x0 <<  2) /**< (PMC) Selected clock */
#define 	AT91C_PMC_PRES_CLK_2                (0x1 <<  2) /**< (PMC) Selected clock divided by 2 */
#define 	AT91C_PMC_PRES_CLK_4                (0x2 <<  2) /**< (PMC) Selected clock divided by 4 */
#define 	AT91C_PMC_PRES_CLK_8                (0x3 <<  2) /**< (PMC) Selected clock divided by 8 */
#define 	AT91C_PMC_PRES_CLK_16               (0x4 <<  2) /**< (PMC) Selected clock divided by 16 */
#define 	AT91C_PMC_PRES_CLK_32               (0x5 <<  2) /**< (PMC) Selected clock divided by 32 */
#define 	AT91C_PMC_PRES_CLK_64               (0x6 <<  2) /**< (PMC) Selected clock divided by 64 */
/* --- Register PMC_IER */
#define AT91C_PMC_MOSCS       (0x1 << 0 ) /**< (PMC) MOSC Status/Enable/Disable/Mask */
#define AT91C_PMC_LOCK        (0x1 << 2 ) /**< (PMC) PLL Status/Enable/Disable/Mask */
#define AT91C_PMC_MCKRDY      (0x1 << 3 ) /**< (PMC) MCK_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK0RDY     (0x1 << 8 ) /**< (PMC) PCK0_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK1RDY     (0x1 << 9 ) /**< (PMC) PCK1_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK2RDY     (0x1 << 10) /**< (PMC) PCK2_RDY Status/Enable/Disable/Mask */
/* --- Register PMC_IDR */
#define AT91C_PMC_MOSCS       (0x1 << 0 ) /**< (PMC) MOSC Status/Enable/Disable/Mask */
#define AT91C_PMC_LOCK        (0x1 << 2 ) /**< (PMC) PLL Status/Enable/Disable/Mask */
#define AT91C_PMC_MCKRDY      (0x1 << 3 ) /**< (PMC) MCK_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK0RDY     (0x1 << 8 ) /**< (PMC) PCK0_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK1RDY     (0x1 << 9 ) /**< (PMC) PCK1_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK2RDY     (0x1 << 10) /**< (PMC) PCK2_RDY Status/Enable/Disable/Mask */
/* --- Register PMC_SR */
#define AT91C_PMC_MOSCS       (0x1 << 0 ) /**< (PMC) MOSC Status/Enable/Disable/Mask */
#define AT91C_PMC_LOCK        (0x1 << 2 ) /**< (PMC) PLL Status/Enable/Disable/Mask */
#define AT91C_PMC_MCKRDY      (0x1 << 3 ) /**< (PMC) MCK_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK0RDY     (0x1 << 8 ) /**< (PMC) PCK0_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK1RDY     (0x1 << 9 ) /**< (PMC) PCK1_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK2RDY     (0x1 << 10) /**< (PMC) PCK2_RDY Status/Enable/Disable/Mask */
/* --- Register PMC_IMR */
#define AT91C_PMC_MOSCS       (0x1 << 0 ) /**< (PMC) MOSC Status/Enable/Disable/Mask */
#define AT91C_PMC_LOCK        (0x1 << 2 ) /**< (PMC) PLL Status/Enable/Disable/Mask */
#define AT91C_PMC_MCKRDY      (0x1 << 3 ) /**< (PMC) MCK_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK0RDY     (0x1 << 8 ) /**< (PMC) PCK0_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK1RDY     (0x1 << 9 ) /**< (PMC) PCK1_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK2RDY     (0x1 << 10) /**< (PMC) PCK2_RDY Status/Enable/Disable/Mask */

#endif /* __AT91SAM7S256_PMC_H */
