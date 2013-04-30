/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_tc.h
 * 
 * Hardware definition for the tc peripheral in the ATMEL at91sam7s256 processor
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


#ifndef __AT91SAM7S256_TC_H
#define __AT91SAM7S256_TC_H

/* -------------------------------------------------------- */
/* TC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_TC0
#define AT91C_ID_TC0   	12 /**< Timer Counter 0 id */
#endif /* AT91C_ID_TC0 */
#ifndef AT91C_ID_TC1
#define AT91C_ID_TC1   	13 /**< Timer Counter 1 id */
#endif /* AT91C_ID_TC1 */
#ifndef AT91C_ID_TC2
#define AT91C_ID_TC2   	14 /**< Timer Counter 2 id */
#endif /* AT91C_ID_TC2 */

/* -------------------------------------------------------- */
/* TC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_TC0       	0xFFFA0000 /**< TC0 base address */
#define AT91C_BASE_TC1       	0xFFFA0040 /**< TC1 base address */
#define AT91C_BASE_TC2       	0xFFFA0080 /**< TC2 base address */

/* -------------------------------------------------------- */
/* PIO definition for TC hardware peripheral */
/* -------------------------------------------------------- */
#define AT91C_PA4_TCLK0    	(1 << 4) /**< Timer Counter 0 external clock input */
#define AT91C_PA0_TIOA0    	(1 << 0) /**< Timer Counter 0 Multipurpose Timer I/O Pin A */
#define AT91C_PA1_TIOB0    	(1 << 1) /**< Timer Counter 0 Multipurpose Timer I/O Pin B */

#define AT91C_PA28_TCLK1    	(1 << 28) /**< Timer Counter 1 external clock input */
#define AT91C_PA15_TIOA1    	(1 << 15) /**< Timer Counter 1 Multipurpose Timer I/O Pin A */
#define AT91C_PA16_TIOB1    	(1 << 16) /**< Timer Counter 1 Multipurpose Timer I/O Pin B */

#define AT91C_PA29_TCLK2    	(1 << 29) /**< Timer Counter 2 external clock input */
#define AT91C_PA26_TIOA2    	(1 << 26) /**< Timer Counter 2 Multipurpose Timer I/O Pin A */
#define AT91C_PA27_TIOB2    	(1 << 27) /**< Timer Counter 2 Multipurpose Timer I/O Pin B */


/* -------------------------------------------------------- */
/* Register offset definition for TC hardware peripheral */
/* -------------------------------------------------------- */
#define TC_CCR 	(0x0000) 	/**< Channel Control Register */
#define TC_CMR 	(0x0004) 	/**< Channel Mode Register (Capture Mode / Waveform Mode) */
#define TC_CV 	(0x0010) 	/**< Counter Value */
#define TC_RA 	(0x0014) 	/**< Register A */
#define TC_RB 	(0x0018) 	/**< Register B */
#define TC_RC 	(0x001C) 	/**< Register C */
#define TC_SR 	(0x0020) 	/**< Status Register */
#define TC_IER 	(0x0024) 	/**< Interrupt Enable Register */
#define TC_IDR 	(0x0028) 	/**< Interrupt Disable Register */
#define TC_IMR 	(0x002C) 	/**< Interrupt Mask Register */

/* -------------------------------------------------------- */
/* Bitfields definition for TC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register TC_CCR */
#define AT91C_TC_CLKEN        (0x1 << 0 ) /**< (TC) Counter Clock Enable Command */
#define AT91C_TC_CLKDIS       (0x1 << 1 ) /**< (TC) Counter Clock Disable Command */
#define AT91C_TC_SWTRG        (0x1 << 2 ) /**< (TC) Software Trigger Command */
/* --- Register TC_CMR */
#define AT91C_TC_CLKS         (0x7 << 0 ) /**< (TC) Clock Selection */
#define 	AT91C_TC_CLKS_TIMER_DIV1_CLOCK     0x0 /**< (TC) Clock selected: TIMER_DIV1_CLOCK */
#define 	AT91C_TC_CLKS_TIMER_DIV2_CLOCK     0x1 /**< (TC) Clock selected: TIMER_DIV2_CLOCK */
#define 	AT91C_TC_CLKS_TIMER_DIV3_CLOCK     0x2 /**< (TC) Clock selected: TIMER_DIV3_CLOCK */
#define 	AT91C_TC_CLKS_TIMER_DIV4_CLOCK     0x3 /**< (TC) Clock selected: TIMER_DIV4_CLOCK */
#define 	AT91C_TC_CLKS_TIMER_DIV5_CLOCK     0x4 /**< (TC) Clock selected: TIMER_DIV5_CLOCK */
#define 	AT91C_TC_CLKS_XC0                  0x5 /**< (TC) Clock selected: XC0 */
#define 	AT91C_TC_CLKS_XC1                  0x6 /**< (TC) Clock selected: XC1 */
#define 	AT91C_TC_CLKS_XC2                  0x7 /**< (TC) Clock selected: XC2 */
#define AT91C_TC_CLKS         (0x7 << 0 ) /**< (TC) Clock Selection */
#define 	AT91C_TC_CLKS_TIMER_DIV1_CLOCK     0x0 /**< (TC) Clock selected: TIMER_DIV1_CLOCK */
#define 	AT91C_TC_CLKS_TIMER_DIV2_CLOCK     0x1 /**< (TC) Clock selected: TIMER_DIV2_CLOCK */
#define 	AT91C_TC_CLKS_TIMER_DIV3_CLOCK     0x2 /**< (TC) Clock selected: TIMER_DIV3_CLOCK */
#define 	AT91C_TC_CLKS_TIMER_DIV4_CLOCK     0x3 /**< (TC) Clock selected: TIMER_DIV4_CLOCK */
#define 	AT91C_TC_CLKS_TIMER_DIV5_CLOCK     0x4 /**< (TC) Clock selected: TIMER_DIV5_CLOCK */
#define 	AT91C_TC_CLKS_XC0                  0x5 /**< (TC) Clock selected: XC0 */
#define 	AT91C_TC_CLKS_XC1                  0x6 /**< (TC) Clock selected: XC1 */
#define 	AT91C_TC_CLKS_XC2                  0x7 /**< (TC) Clock selected: XC2 */
#define AT91C_TC_CLKI         (0x1 << 3 ) /**< (TC) Clock Invert */
#define AT91C_TC_CLKI         (0x1 << 3 ) /**< (TC) Clock Invert */
#define AT91C_TC_BURST        (0x3 << 4 ) /**< (TC) Burst Signal Selection */
#define 	AT91C_TC_BURST_NONE                 (0x0 <<  4) /**< (TC) The clock is not gated by an external signal */
#define 	AT91C_TC_BURST_XC0                  (0x1 <<  4) /**< (TC) XC0 is ANDed with the selected clock */
#define 	AT91C_TC_BURST_XC1                  (0x2 <<  4) /**< (TC) XC1 is ANDed with the selected clock */
#define 	AT91C_TC_BURST_XC2                  (0x3 <<  4) /**< (TC) XC2 is ANDed with the selected clock */
#define AT91C_TC_BURST        (0x3 << 4 ) /**< (TC) Burst Signal Selection */
#define 	AT91C_TC_BURST_NONE                 (0x0 <<  4) /**< (TC) The clock is not gated by an external signal */
#define 	AT91C_TC_BURST_XC0                  (0x1 <<  4) /**< (TC) XC0 is ANDed with the selected clock */
#define 	AT91C_TC_BURST_XC1                  (0x2 <<  4) /**< (TC) XC1 is ANDed with the selected clock */
#define 	AT91C_TC_BURST_XC2                  (0x3 <<  4) /**< (TC) XC2 is ANDed with the selected clock */
#define AT91C_TC_CPCSTOP      (0x1 << 6 ) /**< (TC) Counter Clock Stopped with RC Compare */
#define AT91C_TC_LDBSTOP      (0x1 << 6 ) /**< (TC) Counter Clock Stopped with RB Loading */
#define AT91C_TC_LDBDIS       (0x1 << 7 ) /**< (TC) Counter Clock Disabled with RB Loading */
#define AT91C_TC_CPCDIS       (0x1 << 7 ) /**< (TC) Counter Clock Disable with RC Compare */
#define AT91C_TC_ETRGEDG      (0x3 << 8 ) /**< (TC) External Trigger Edge Selection */
#define 	AT91C_TC_ETRGEDG_NONE                 (0x0 <<  8) /**< (TC) Edge: None */
#define 	AT91C_TC_ETRGEDG_RISING               (0x1 <<  8) /**< (TC) Edge: rising edge */
#define 	AT91C_TC_ETRGEDG_FALLING              (0x2 <<  8) /**< (TC) Edge: falling edge */
#define 	AT91C_TC_ETRGEDG_BOTH                 (0x3 <<  8) /**< (TC) Edge: each edge */
#define AT91C_TC_EEVTEDG      (0x3 << 8 ) /**< (TC) External Event Edge Selection */
#define 	AT91C_TC_EEVTEDG_NONE                 (0x0 <<  8) /**< (TC) Edge: None */
#define 	AT91C_TC_EEVTEDG_RISING               (0x1 <<  8) /**< (TC) Edge: rising edge */
#define 	AT91C_TC_EEVTEDG_FALLING              (0x2 <<  8) /**< (TC) Edge: falling edge */
#define 	AT91C_TC_EEVTEDG_BOTH                 (0x3 <<  8) /**< (TC) Edge: each edge */
#define AT91C_TC_ABETRG       (0x1 << 10) /**< (TC) TIOA or TIOB External Trigger Selection */
#define AT91C_TC_EEVT         (0x3 << 10) /**< (TC) External Event  Selection */
#define 	AT91C_TC_EEVT_TIOB                 (0x0 << 10) /**< (TC) Signal selected as external event: TIOB TIOB direction: input */
#define 	AT91C_TC_EEVT_XC0                  (0x1 << 10) /**< (TC) Signal selected as external event: XC0 TIOB direction: output */
#define 	AT91C_TC_EEVT_XC1                  (0x2 << 10) /**< (TC) Signal selected as external event: XC1 TIOB direction: output */
#define 	AT91C_TC_EEVT_XC2                  (0x3 << 10) /**< (TC) Signal selected as external event: XC2 TIOB direction: output */
#define AT91C_TC_ENETRG       (0x1 << 12) /**< (TC) External Event Trigger enable */
#define AT91C_TC_WAVESEL      (0x3 << 13) /**< (TC) Waveform  Selection */
#define 	AT91C_TC_WAVESEL_UP                   (0x0 << 13) /**< (TC) UP mode without atomatic trigger on RC Compare */
#define 	AT91C_TC_WAVESEL_UPDOWN               (0x1 << 13) /**< (TC) UPDOWN mode without automatic trigger on RC Compare */
#define 	AT91C_TC_WAVESEL_UP_AUTO              (0x2 << 13) /**< (TC) UP mode with automatic trigger on RC Compare */
#define 	AT91C_TC_WAVESEL_UPDOWN_AUTO          (0x3 << 13) /**< (TC) UPDOWN mode with automatic trigger on RC Compare */
#define AT91C_TC_CPCTRG       (0x1 << 14) /**< (TC) RC Compare Trigger Enable */
#define AT91C_TC_WAVE         (0x1 << 15) /**< (TC)  */
#define AT91C_TC_WAVE         (0x1 << 15) /**< (TC)  */
#define AT91C_TC_LDRA         (0x3 << 16) /**< (TC) RA Loading Selection */
#define 	AT91C_TC_LDRA_NONE                 (0x0 << 16) /**< (TC) Edge: None */
#define 	AT91C_TC_LDRA_RISING               (0x1 << 16) /**< (TC) Edge: rising edge of TIOA */
#define 	AT91C_TC_LDRA_FALLING              (0x2 << 16) /**< (TC) Edge: falling edge of TIOA */
#define 	AT91C_TC_LDRA_BOTH                 (0x3 << 16) /**< (TC) Edge: each edge of TIOA */
#define AT91C_TC_ACPA         (0x3 << 16) /**< (TC) RA Compare Effect on TIOA */
#define 	AT91C_TC_ACPA_NONE                 (0x0 << 16) /**< (TC) Effect: none */
#define 	AT91C_TC_ACPA_SET                  (0x1 << 16) /**< (TC) Effect: set */
#define 	AT91C_TC_ACPA_CLEAR                (0x2 << 16) /**< (TC) Effect: clear */
#define 	AT91C_TC_ACPA_TOGGLE               (0x3 << 16) /**< (TC) Effect: toggle */
#define AT91C_TC_LDRB         (0x3 << 18) /**< (TC) RB Loading Selection */
#define 	AT91C_TC_LDRB_NONE                 (0x0 << 18) /**< (TC) Edge: None */
#define 	AT91C_TC_LDRB_RISING               (0x1 << 18) /**< (TC) Edge: rising edge of TIOA */
#define 	AT91C_TC_LDRB_FALLING              (0x2 << 18) /**< (TC) Edge: falling edge of TIOA */
#define 	AT91C_TC_LDRB_BOTH                 (0x3 << 18) /**< (TC) Edge: each edge of TIOA */
#define AT91C_TC_ACPC         (0x3 << 18) /**< (TC) RC Compare Effect on TIOA */
#define 	AT91C_TC_ACPC_NONE                 (0x0 << 18) /**< (TC) Effect: none */
#define 	AT91C_TC_ACPC_SET                  (0x1 << 18) /**< (TC) Effect: set */
#define 	AT91C_TC_ACPC_CLEAR                (0x2 << 18) /**< (TC) Effect: clear */
#define 	AT91C_TC_ACPC_TOGGLE               (0x3 << 18) /**< (TC) Effect: toggle */
#define AT91C_TC_AEEVT        (0x3 << 20) /**< (TC) External Event Effect on TIOA */
#define 	AT91C_TC_AEEVT_NONE                 (0x0 << 20) /**< (TC) Effect: none */
#define 	AT91C_TC_AEEVT_SET                  (0x1 << 20) /**< (TC) Effect: set */
#define 	AT91C_TC_AEEVT_CLEAR                (0x2 << 20) /**< (TC) Effect: clear */
#define 	AT91C_TC_AEEVT_TOGGLE               (0x3 << 20) /**< (TC) Effect: toggle */
#define AT91C_TC_ASWTRG       (0x3 << 22) /**< (TC) Software Trigger Effect on TIOA */
#define 	AT91C_TC_ASWTRG_NONE                 (0x0 << 22) /**< (TC) Effect: none */
#define 	AT91C_TC_ASWTRG_SET                  (0x1 << 22) /**< (TC) Effect: set */
#define 	AT91C_TC_ASWTRG_CLEAR                (0x2 << 22) /**< (TC) Effect: clear */
#define 	AT91C_TC_ASWTRG_TOGGLE               (0x3 << 22) /**< (TC) Effect: toggle */
#define AT91C_TC_BCPB         (0x3 << 24) /**< (TC) RB Compare Effect on TIOB */
#define 	AT91C_TC_BCPB_NONE                 (0x0 << 24) /**< (TC) Effect: none */
#define 	AT91C_TC_BCPB_SET                  (0x1 << 24) /**< (TC) Effect: set */
#define 	AT91C_TC_BCPB_CLEAR                (0x2 << 24) /**< (TC) Effect: clear */
#define 	AT91C_TC_BCPB_TOGGLE               (0x3 << 24) /**< (TC) Effect: toggle */
#define AT91C_TC_BCPC         (0x3 << 26) /**< (TC) RC Compare Effect on TIOB */
#define 	AT91C_TC_BCPC_NONE                 (0x0 << 26) /**< (TC) Effect: none */
#define 	AT91C_TC_BCPC_SET                  (0x1 << 26) /**< (TC) Effect: set */
#define 	AT91C_TC_BCPC_CLEAR                (0x2 << 26) /**< (TC) Effect: clear */
#define 	AT91C_TC_BCPC_TOGGLE               (0x3 << 26) /**< (TC) Effect: toggle */
#define AT91C_TC_BEEVT        (0x3 << 28) /**< (TC) External Event Effect on TIOB */
#define 	AT91C_TC_BEEVT_NONE                 (0x0 << 28) /**< (TC) Effect: none */
#define 	AT91C_TC_BEEVT_SET                  (0x1 << 28) /**< (TC) Effect: set */
#define 	AT91C_TC_BEEVT_CLEAR                (0x2 << 28) /**< (TC) Effect: clear */
#define 	AT91C_TC_BEEVT_TOGGLE               (0x3 << 28) /**< (TC) Effect: toggle */
#define AT91C_TC_BSWTRG       (0x3 << 30) /**< (TC) Software Trigger Effect on TIOB */
#define 	AT91C_TC_BSWTRG_NONE                 (0x0 << 30) /**< (TC) Effect: none */
#define 	AT91C_TC_BSWTRG_SET                  (0x1 << 30) /**< (TC) Effect: set */
#define 	AT91C_TC_BSWTRG_CLEAR                (0x2 << 30) /**< (TC) Effect: clear */
#define 	AT91C_TC_BSWTRG_TOGGLE               (0x3 << 30) /**< (TC) Effect: toggle */
/* --- Register TC_SR */
#define AT91C_TC_COVFS        (0x1 << 0 ) /**< (TC) Counter Overflow */
#define AT91C_TC_LOVRS        (0x1 << 1 ) /**< (TC) Load Overrun */
#define AT91C_TC_CPAS         (0x1 << 2 ) /**< (TC) RA Compare */
#define AT91C_TC_CPBS         (0x1 << 3 ) /**< (TC) RB Compare */
#define AT91C_TC_CPCS         (0x1 << 4 ) /**< (TC) RC Compare */
#define AT91C_TC_LDRAS        (0x1 << 5 ) /**< (TC) RA Loading */
#define AT91C_TC_LDRBS        (0x1 << 6 ) /**< (TC) RB Loading */
#define AT91C_TC_ETRGS        (0x1 << 7 ) /**< (TC) External Trigger */
#define AT91C_TC_CLKSTA       (0x1 << 16) /**< (TC) Clock Enabling */
#define AT91C_TC_MTIOA        (0x1 << 17) /**< (TC) TIOA Mirror */
#define AT91C_TC_MTIOB        (0x1 << 18) /**< (TC) TIOA Mirror */
/* --- Register TC_IER */
#define AT91C_TC_COVFS        (0x1 << 0 ) /**< (TC) Counter Overflow */
#define AT91C_TC_LOVRS        (0x1 << 1 ) /**< (TC) Load Overrun */
#define AT91C_TC_CPAS         (0x1 << 2 ) /**< (TC) RA Compare */
#define AT91C_TC_CPBS         (0x1 << 3 ) /**< (TC) RB Compare */
#define AT91C_TC_CPCS         (0x1 << 4 ) /**< (TC) RC Compare */
#define AT91C_TC_LDRAS        (0x1 << 5 ) /**< (TC) RA Loading */
#define AT91C_TC_LDRBS        (0x1 << 6 ) /**< (TC) RB Loading */
#define AT91C_TC_ETRGS        (0x1 << 7 ) /**< (TC) External Trigger */
/* --- Register TC_IDR */
#define AT91C_TC_COVFS        (0x1 << 0 ) /**< (TC) Counter Overflow */
#define AT91C_TC_LOVRS        (0x1 << 1 ) /**< (TC) Load Overrun */
#define AT91C_TC_CPAS         (0x1 << 2 ) /**< (TC) RA Compare */
#define AT91C_TC_CPBS         (0x1 << 3 ) /**< (TC) RB Compare */
#define AT91C_TC_CPCS         (0x1 << 4 ) /**< (TC) RC Compare */
#define AT91C_TC_LDRAS        (0x1 << 5 ) /**< (TC) RA Loading */
#define AT91C_TC_LDRBS        (0x1 << 6 ) /**< (TC) RB Loading */
#define AT91C_TC_ETRGS        (0x1 << 7 ) /**< (TC) External Trigger */
/* --- Register TC_IMR */
#define AT91C_TC_COVFS        (0x1 << 0 ) /**< (TC) Counter Overflow */
#define AT91C_TC_LOVRS        (0x1 << 1 ) /**< (TC) Load Overrun */
#define AT91C_TC_CPAS         (0x1 << 2 ) /**< (TC) RA Compare */
#define AT91C_TC_CPBS         (0x1 << 3 ) /**< (TC) RB Compare */
#define AT91C_TC_CPCS         (0x1 << 4 ) /**< (TC) RC Compare */
#define AT91C_TC_LDRAS        (0x1 << 5 ) /**< (TC) RA Loading */
#define AT91C_TC_LDRBS        (0x1 << 6 ) /**< (TC) RB Loading */
#define AT91C_TC_ETRGS        (0x1 << 7 ) /**< (TC) External Trigger */

#endif /* __AT91SAM7S256_TC_H */
