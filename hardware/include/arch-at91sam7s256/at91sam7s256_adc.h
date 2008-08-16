/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_adc.h
 * 
 * Hardware definition for the adc peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from ADC_6051C V1.1
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


#ifndef __AT91SAM7S256_ADC_H
#define __AT91SAM7S256_ADC_H

/* -------------------------------------------------------- */
/* ADC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_ADC
#define AT91C_ID_ADC   	 4 /**< Analog-to-Digital Converter id */
#endif /* AT91C_ID_ADC */

/* -------------------------------------------------------- */
/* ADC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_ADC       	0xFFFD8000 /**< ADC base address */

/* -------------------------------------------------------- */
/* PIO definition for ADC hardware peripheral */
/* -------------------------------------------------------- */
#define AT91C_PA8_ADTRG    	(1 << 8) /**< ADC External Trigger */


/* -------------------------------------------------------- */
/* Register offset definition for ADC hardware peripheral */
/* -------------------------------------------------------- */
#define ADC_CR 	(0x0000) 	/**< ADC Control Register */
#define ADC_MR 	(0x0004) 	/**< ADC Mode Register */
#define ADC_CHER 	(0x0010) 	/**< ADC Channel Enable Register */
#define ADC_CHDR 	(0x0014) 	/**< ADC Channel Disable Register */
#define ADC_CHSR 	(0x0018) 	/**< ADC Channel Status Register */
#define ADC_SR 	(0x001C) 	/**< ADC Status Register */
#define ADC_LCDR 	(0x0020) 	/**< ADC Last Converted Data Register */
#define ADC_IER 	(0x0024) 	/**< ADC Interrupt Enable Register */
#define ADC_IDR 	(0x0028) 	/**< ADC Interrupt Disable Register */
#define ADC_IMR 	(0x002C) 	/**< ADC Interrupt Mask Register */
#define ADC_CDR0 	(0x0030) 	/**< ADC Channel Data Register 0 */
#define ADC_CDR1 	(0x0034) 	/**< ADC Channel Data Register 1 */
#define ADC_CDR2 	(0x0038) 	/**< ADC Channel Data Register 2 */
#define ADC_CDR3 	(0x003C) 	/**< ADC Channel Data Register 3 */
#define ADC_CDR4 	(0x0040) 	/**< ADC Channel Data Register 4 */
#define ADC_CDR5 	(0x0044) 	/**< ADC Channel Data Register 5 */
#define ADC_CDR6 	(0x0048) 	/**< ADC Channel Data Register 6 */
#define ADC_CDR7 	(0x004C) 	/**< ADC Channel Data Register 7 */
#define ADC_RPR 	(0x0100) 	/**< Receive Pointer Register */
#define ADC_RCR 	(0x0104) 	/**< Receive Counter Register */
#define ADC_TPR 	(0x0108) 	/**< Transmit Pointer Register */
#define ADC_TCR 	(0x010C) 	/**< Transmit Counter Register */
#define ADC_RNPR 	(0x0110) 	/**< Receive Next Pointer Register */
#define ADC_RNCR 	(0x0114) 	/**< Receive Next Counter Register */
#define ADC_TNPR 	(0x0118) 	/**< Transmit Next Pointer Register */
#define ADC_TNCR 	(0x011C) 	/**< Transmit Next Counter Register */
#define ADC_PTCR 	(0x0120) 	/**< PDC Transfer Control Register */
#define ADC_PTSR 	(0x0124) 	/**< PDC Transfer Status Register */

/* -------------------------------------------------------- */
/* Bitfields definition for ADC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register ADC_CR */
#define AT91C_ADC_SWRST       (0x1 << 0 ) /**< (ADC) Software Reset */
#define AT91C_ADC_START       (0x1 << 1 ) /**< (ADC) Start Conversion */
/* --- Register ADC_MR */
#define AT91C_ADC_TRGEN       (0x1 << 0 ) /**< (ADC) Trigger Enable */
#define 	AT91C_ADC_TRGEN_DIS                  0x0 /**< (ADC) Hradware triggers are disabled. Starting a conversion is only possible by software */
#define 	AT91C_ADC_TRGEN_EN                   0x1 /**< (ADC) Hardware trigger selected by TRGSEL field is enabled. */
#define AT91C_ADC_TRGSEL      (0x7 << 1 ) /**< (ADC) Trigger Selection */
#define 	AT91C_ADC_TRGSEL_TIOA0                (0x0 <<  1) /**< (ADC) Selected TRGSEL = TIAO0 */
#define 	AT91C_ADC_TRGSEL_TIOA1                (0x1 <<  1) /**< (ADC) Selected TRGSEL = TIAO1 */
#define 	AT91C_ADC_TRGSEL_TIOA2                (0x2 <<  1) /**< (ADC) Selected TRGSEL = TIAO2 */
#define 	AT91C_ADC_TRGSEL_TIOA3                (0x3 <<  1) /**< (ADC) Selected TRGSEL = TIAO3 */
#define 	AT91C_ADC_TRGSEL_TIOA4                (0x4 <<  1) /**< (ADC) Selected TRGSEL = TIAO4 */
#define 	AT91C_ADC_TRGSEL_TIOA5                (0x5 <<  1) /**< (ADC) Selected TRGSEL = TIAO5 */
#define 	AT91C_ADC_TRGSEL_EXT                  (0x6 <<  1) /**< (ADC) Selected TRGSEL = External Trigger */
#define AT91C_ADC_LOWRES      (0x1 << 4 ) /**< (ADC) Resolution. */
#define 	AT91C_ADC_LOWRES_10_BIT               (0x0 <<  4) /**< (ADC) 10-bit resolution */
#define 	AT91C_ADC_LOWRES_8_BIT                (0x1 <<  4) /**< (ADC) 8-bit resolution */
#define AT91C_ADC_SLEEP       (0x1 << 5 ) /**< (ADC) Sleep Mode */
#define 	AT91C_ADC_SLEEP_NORMAL_MODE          (0x0 <<  5) /**< (ADC) Normal Mode */
#define 	AT91C_ADC_SLEEP_MODE                 (0x1 <<  5) /**< (ADC) Sleep Mode */
#define AT91C_ADC_PRESCAL     (0x3F << 8 ) /**< (ADC) Prescaler rate selection */
#define AT91C_ADC_STARTUP     (0x1F << 16) /**< (ADC) Startup Time */
#define AT91C_ADC_SHTIM       (0xF << 24) /**< (ADC) Sample & Hold Time */
/* --- Register 	ADC_CHER */
#define AT91C_ADC_CH0         (0x1 << 0 ) /**< (ADC) Channel 0 */
#define AT91C_ADC_CH1         (0x1 << 1 ) /**< (ADC) Channel 1 */
#define AT91C_ADC_CH2         (0x1 << 2 ) /**< (ADC) Channel 2 */
#define AT91C_ADC_CH3         (0x1 << 3 ) /**< (ADC) Channel 3 */
#define AT91C_ADC_CH4         (0x1 << 4 ) /**< (ADC) Channel 4 */
#define AT91C_ADC_CH5         (0x1 << 5 ) /**< (ADC) Channel 5 */
#define AT91C_ADC_CH6         (0x1 << 6 ) /**< (ADC) Channel 6 */
#define AT91C_ADC_CH7         (0x1 << 7 ) /**< (ADC) Channel 7 */
/* --- Register 	ADC_CHDR */
#define AT91C_ADC_CH0         (0x1 << 0 ) /**< (ADC) Channel 0 */
#define AT91C_ADC_CH1         (0x1 << 1 ) /**< (ADC) Channel 1 */
#define AT91C_ADC_CH2         (0x1 << 2 ) /**< (ADC) Channel 2 */
#define AT91C_ADC_CH3         (0x1 << 3 ) /**< (ADC) Channel 3 */
#define AT91C_ADC_CH4         (0x1 << 4 ) /**< (ADC) Channel 4 */
#define AT91C_ADC_CH5         (0x1 << 5 ) /**< (ADC) Channel 5 */
#define AT91C_ADC_CH6         (0x1 << 6 ) /**< (ADC) Channel 6 */
#define AT91C_ADC_CH7         (0x1 << 7 ) /**< (ADC) Channel 7 */
/* --- Register 	ADC_CHSR */
#define AT91C_ADC_CH0         (0x1 << 0 ) /**< (ADC) Channel 0 */
#define AT91C_ADC_CH1         (0x1 << 1 ) /**< (ADC) Channel 1 */
#define AT91C_ADC_CH2         (0x1 << 2 ) /**< (ADC) Channel 2 */
#define AT91C_ADC_CH3         (0x1 << 3 ) /**< (ADC) Channel 3 */
#define AT91C_ADC_CH4         (0x1 << 4 ) /**< (ADC) Channel 4 */
#define AT91C_ADC_CH5         (0x1 << 5 ) /**< (ADC) Channel 5 */
#define AT91C_ADC_CH6         (0x1 << 6 ) /**< (ADC) Channel 6 */
#define AT91C_ADC_CH7         (0x1 << 7 ) /**< (ADC) Channel 7 */
/* --- Register ADC_SR */
#define AT91C_ADC_EOC0        (0x1 << 0 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC1        (0x1 << 1 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC2        (0x1 << 2 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC3        (0x1 << 3 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC4        (0x1 << 4 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC5        (0x1 << 5 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC6        (0x1 << 6 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC7        (0x1 << 7 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_OVRE0       (0x1 << 8 ) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE1       (0x1 << 9 ) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE2       (0x1 << 10) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE3       (0x1 << 11) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE4       (0x1 << 12) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE5       (0x1 << 13) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE6       (0x1 << 14) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE7       (0x1 << 15) /**< (ADC) Overrun Error */
#define AT91C_ADC_DRDY        (0x1 << 16) /**< (ADC) Data Ready */
#define AT91C_ADC_GOVRE       (0x1 << 17) /**< (ADC) General Overrun */
#define AT91C_ADC_ENDRX       (0x1 << 18) /**< (ADC) End of Receiver Transfer */
#define AT91C_ADC_RXBUFF      (0x1 << 19) /**< (ADC) RXBUFF Interrupt */
/* --- Register ADC_LCDR */
#define AT91C_ADC_LDATA       (0x3FF << 0 ) /**< (ADC) Last Data Converted */
/* --- Register ADC_IER */
#define AT91C_ADC_EOC0        (0x1 << 0 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC1        (0x1 << 1 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC2        (0x1 << 2 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC3        (0x1 << 3 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC4        (0x1 << 4 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC5        (0x1 << 5 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC6        (0x1 << 6 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC7        (0x1 << 7 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_OVRE0       (0x1 << 8 ) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE1       (0x1 << 9 ) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE2       (0x1 << 10) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE3       (0x1 << 11) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE4       (0x1 << 12) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE5       (0x1 << 13) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE6       (0x1 << 14) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE7       (0x1 << 15) /**< (ADC) Overrun Error */
#define AT91C_ADC_DRDY        (0x1 << 16) /**< (ADC) Data Ready */
#define AT91C_ADC_GOVRE       (0x1 << 17) /**< (ADC) General Overrun */
#define AT91C_ADC_ENDRX       (0x1 << 18) /**< (ADC) End of Receiver Transfer */
#define AT91C_ADC_RXBUFF      (0x1 << 19) /**< (ADC) RXBUFF Interrupt */
/* --- Register ADC_IDR */
#define AT91C_ADC_EOC0        (0x1 << 0 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC1        (0x1 << 1 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC2        (0x1 << 2 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC3        (0x1 << 3 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC4        (0x1 << 4 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC5        (0x1 << 5 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC6        (0x1 << 6 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC7        (0x1 << 7 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_OVRE0       (0x1 << 8 ) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE1       (0x1 << 9 ) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE2       (0x1 << 10) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE3       (0x1 << 11) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE4       (0x1 << 12) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE5       (0x1 << 13) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE6       (0x1 << 14) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE7       (0x1 << 15) /**< (ADC) Overrun Error */
#define AT91C_ADC_DRDY        (0x1 << 16) /**< (ADC) Data Ready */
#define AT91C_ADC_GOVRE       (0x1 << 17) /**< (ADC) General Overrun */
#define AT91C_ADC_ENDRX       (0x1 << 18) /**< (ADC) End of Receiver Transfer */
#define AT91C_ADC_RXBUFF      (0x1 << 19) /**< (ADC) RXBUFF Interrupt */
/* --- Register ADC_IMR */
#define AT91C_ADC_EOC0        (0x1 << 0 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC1        (0x1 << 1 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC2        (0x1 << 2 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC3        (0x1 << 3 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC4        (0x1 << 4 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC5        (0x1 << 5 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC6        (0x1 << 6 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_EOC7        (0x1 << 7 ) /**< (ADC) End of Conversion */
#define AT91C_ADC_OVRE0       (0x1 << 8 ) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE1       (0x1 << 9 ) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE2       (0x1 << 10) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE3       (0x1 << 11) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE4       (0x1 << 12) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE5       (0x1 << 13) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE6       (0x1 << 14) /**< (ADC) Overrun Error */
#define AT91C_ADC_OVRE7       (0x1 << 15) /**< (ADC) Overrun Error */
#define AT91C_ADC_DRDY        (0x1 << 16) /**< (ADC) Data Ready */
#define AT91C_ADC_GOVRE       (0x1 << 17) /**< (ADC) General Overrun */
#define AT91C_ADC_ENDRX       (0x1 << 18) /**< (ADC) End of Receiver Transfer */
#define AT91C_ADC_RXBUFF      (0x1 << 19) /**< (ADC) RXBUFF Interrupt */
/* --- Register ADC_CDR0 */
#define AT91C_ADC_DATA        (0x3FF << 0 ) /**< (ADC) Converted Data */
/* --- Register ADC_CDR1 */
#define AT91C_ADC_DATA        (0x3FF << 0 ) /**< (ADC) Converted Data */
/* --- Register ADC_CDR2 */
#define AT91C_ADC_DATA        (0x3FF << 0 ) /**< (ADC) Converted Data */
/* --- Register ADC_CDR3 */
#define AT91C_ADC_DATA        (0x3FF << 0 ) /**< (ADC) Converted Data */
/* --- Register ADC_CDR4 */
#define AT91C_ADC_DATA        (0x3FF << 0 ) /**< (ADC) Converted Data */
/* --- Register ADC_CDR5 */
#define AT91C_ADC_DATA        (0x3FF << 0 ) /**< (ADC) Converted Data */
/* --- Register ADC_CDR6 */
#define AT91C_ADC_DATA        (0x3FF << 0 ) /**< (ADC) Converted Data */
/* --- Register ADC_CDR7 */
#define AT91C_ADC_DATA        (0x3FF << 0 ) /**< (ADC) Converted Data */

#endif /* __AT91SAM7S256_ADC_H */
