/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_sys.h
 * 
 * Hardware definition for the sys peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from SYS_SAM7S V1.2
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


#ifndef __AT91SAM7S256_SYS_H
#define __AT91SAM7S256_SYS_H

/* -------------------------------------------------------- */
/* SYS ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* SYS Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_SYS       	0xFFFFF000 /**< SYS base address */

/* -------------------------------------------------------- */
/* PIO definition for SYS hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for SYS hardware peripheral */
/* -------------------------------------------------------- */
#define AIC_SMR 	(0x0000) 	/**< Source Mode Register */
#define AIC_SVR 	(0x0080) 	/**< Source Vector Register */
#define AIC_IVR 	(0x0100) 	/**< IRQ Vector Register */
#define AIC_FVR 	(0x0104) 	/**< FIQ Vector Register */
#define AIC_ISR 	(0x0108) 	/**< Interrupt Status Register */
#define AIC_IPR 	(0x010C) 	/**< Interrupt Pending Register */
#define AIC_IMR 	(0x0110) 	/**< Interrupt Mask Register */
#define AIC_CISR 	(0x0114) 	/**< Core Interrupt Status Register */
#define AIC_IECR 	(0x0120) 	/**< Interrupt Enable Command Register */
#define AIC_IDCR 	(0x0124) 	/**< Interrupt Disable Command Register */
#define AIC_ICCR 	(0x0128) 	/**< Interrupt Clear Command Register */
#define AIC_ISCR 	(0x012C) 	/**< Interrupt Set Command Register */
#define AIC_EOICR 	(0x0130) 	/**< End of Interrupt Command Register */
#define AIC_SPU 	(0x0134) 	/**< Spurious Vector Register */
#define AIC_DCR 	(0x0138) 	/**< Debug Control Register (Protect) */
#define AIC_FFER 	(0x0140) 	/**< Fast Forcing Enable Register */
#define AIC_FFDR 	(0x0144) 	/**< Fast Forcing Disable Register */
#define AIC_FFSR 	(0x0148) 	/**< Fast Forcing Status Register */
#define DBGU_CR 	(0x0200) 	/**< Control Register */
#define DBGU_MR 	(0x0204) 	/**< Mode Register */
#define DBGU_IER 	(0x0208) 	/**< Interrupt Enable Register */
#define DBGU_IDR 	(0x020C) 	/**< Interrupt Disable Register */
#define DBGU_IMR 	(0x0210) 	/**< Interrupt Mask Register */
#define DBGU_CSR 	(0x0214) 	/**< Channel Status Register */
#define DBGU_RHR 	(0x0218) 	/**< Receiver Holding Register */
#define DBGU_THR 	(0x021C) 	/**< Transmitter Holding Register */
#define DBGU_BRGR 	(0x0220) 	/**< Baud Rate Generator Register */
#define DBGU_CIDR 	(0x0240) 	/**< Chip ID Register */
#define DBGU_EXID 	(0x0244) 	/**< Chip ID Extension Register */
#define DBGU_FNTR 	(0x0248) 	/**< Force NTRST Register */
#define DBGU_RPR 	(0x0300) 	/**< Receive Pointer Register */
#define DBGU_RCR 	(0x0304) 	/**< Receive Counter Register */
#define DBGU_TPR 	(0x0308) 	/**< Transmit Pointer Register */
#define DBGU_TCR 	(0x030C) 	/**< Transmit Counter Register */
#define DBGU_RNPR 	(0x0310) 	/**< Receive Next Pointer Register */
#define DBGU_RNCR 	(0x0314) 	/**< Receive Next Counter Register */
#define DBGU_TNPR 	(0x0318) 	/**< Transmit Next Pointer Register */
#define DBGU_TNCR 	(0x031C) 	/**< Transmit Next Counter Register */
#define DBGU_PTCR 	(0x0320) 	/**< PDC Transfer Control Register */
#define DBGU_PTSR 	(0x0324) 	/**< PDC Transfer Status Register */
#define PIOA_PER 	(0x0400) 	/**< PIO Enable Register */
#define PIOA_PDR 	(0x0404) 	/**< PIO Disable Register */
#define PIOA_PSR 	(0x0408) 	/**< PIO Status Register */
#define PIOA_OER 	(0x0410) 	/**< Output Enable Register */
#define PIOA_ODR 	(0x0414) 	/**< Output Disable Registerr */
#define PIOA_OSR 	(0x0418) 	/**< Output Status Register */
#define PIOA_IFER 	(0x0420) 	/**< Input Filter Enable Register */
#define PIOA_IFDR 	(0x0424) 	/**< Input Filter Disable Register */
#define PIOA_IFSR 	(0x0428) 	/**< Input Filter Status Register */
#define PIOA_SODR 	(0x0430) 	/**< Set Output Data Register */
#define PIOA_CODR 	(0x0434) 	/**< Clear Output Data Register */
#define PIOA_ODSR 	(0x0438) 	/**< Output Data Status Register */
#define PIOA_PDSR 	(0x043C) 	/**< Pin Data Status Register */
#define PIOA_IER 	(0x0440) 	/**< Interrupt Enable Register */
#define PIOA_IDR 	(0x0444) 	/**< Interrupt Disable Register */
#define PIOA_IMR 	(0x0448) 	/**< Interrupt Mask Register */
#define PIOA_ISR 	(0x044C) 	/**< Interrupt Status Register */
#define PIOA_MDER 	(0x0450) 	/**< Multi-driver Enable Register */
#define PIOA_MDDR 	(0x0454) 	/**< Multi-driver Disable Register */
#define PIOA_MDSR 	(0x0458) 	/**< Multi-driver Status Register */
#define PIOA_PPUDR 	(0x0460) 	/**< Pull-up Disable Register */
#define PIOA_PPUER 	(0x0464) 	/**< Pull-up Enable Register */
#define PIOA_PPUSR 	(0x0468) 	/**< Pull-up Status Register */
#define PIOA_ASR 	(0x0470) 	/**< Select A Register */
#define PIOA_BSR 	(0x0474) 	/**< Select B Register */
#define PIOA_ABSR 	(0x0478) 	/**< AB Select Status Register */
#define PIOA_OWER 	(0x04A0) 	/**< Output Write Enable Register */
#define PIOA_OWDR 	(0x04A4) 	/**< Output Write Disable Register */
#define PIOA_OWSR 	(0x04A8) 	/**< Output Write Status Register */
#define PMC_SCER 	(0x0C00) 	/**< System Clock Enable Register */
#define PMC_SCDR 	(0x0C04) 	/**< System Clock Disable Register */
#define PMC_SCSR 	(0x0C08) 	/**< System Clock Status Register */
#define PMC_PCER 	(0x0C10) 	/**< Peripheral Clock Enable Register */
#define PMC_PCDR 	(0x0C14) 	/**< Peripheral Clock Disable Register */
#define PMC_PCSR 	(0x0C18) 	/**< Peripheral Clock Status Register */
#define PMC_MOR 	(0x0C20) 	/**< Main Oscillator Register */
#define PMC_MCFR 	(0x0C24) 	/**< Main Clock  Frequency Register */
#define PMC_PLLR 	(0x0C2C) 	/**< PLL Register */
#define PMC_MCKR 	(0x0C30) 	/**< Master Clock Register */
#define PMC_PCKR 	(0x0C40) 	/**< Programmable Clock Register */
#define PMC_IER 	(0x0C60) 	/**< Interrupt Enable Register */
#define PMC_IDR 	(0x0C64) 	/**< Interrupt Disable Register */
#define PMC_SR 	(0x0C68) 	/**< Status Register */
#define PMC_IMR 	(0x0C6C) 	/**< Interrupt Mask Register */
#define RSTC_RCR 	(0x0D00) 	/**< Reset Control Register */
#define RSTC_RSR 	(0x0D04) 	/**< Reset Status Register */
#define RSTC_RMR 	(0x0D08) 	/**< Reset Mode Register */
#define RTTC_RTMR 	(0x0D20) 	/**< Real-time Mode Register */
#define RTTC_RTAR 	(0x0D24) 	/**< Real-time Alarm Register */
#define RTTC_RTVR 	(0x0D28) 	/**< Real-time Value Register */
#define RTTC_RTSR 	(0x0D2C) 	/**< Real-time Status Register */
#define PITC_PIMR 	(0x0D30) 	/**< Period Interval Mode Register */
#define PITC_PISR 	(0x0D34) 	/**< Period Interval Status Register */
#define PITC_PIVR 	(0x0D38) 	/**< Period Interval Value Register */
#define PITC_PIIR 	(0x0D3C) 	/**< Period Interval Image Register */
#define WDTC_WDCR 	(0x0D40) 	/**< Watchdog Control Register */
#define WDTC_WDMR 	(0x0D44) 	/**< Watchdog Mode Register */
#define WDTC_WDSR 	(0x0D48) 	/**< Watchdog Status Register */
#define VREG_MR 	(0x0D60) 	/**< Voltage Regulator Mode Register */

/* -------------------------------------------------------- */
/* Bitfields definition for SYS hardware peripheral */
/* -------------------------------------------------------- */

#endif /* __AT91SAM7S256_SYS_H */
