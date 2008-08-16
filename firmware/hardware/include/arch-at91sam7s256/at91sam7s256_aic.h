/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_aic.h
 * 
 * Hardware definition for the aic peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from AIC_6075B V1.3
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


#ifndef __AT91SAM7S256_AIC_H
#define __AT91SAM7S256_AIC_H

/* -------------------------------------------------------- */
/* AIC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_FIQ
#define AT91C_ID_FIQ   	 0 /**< Advanced Interrupt Controller (FIQ) id */
#endif /* AT91C_ID_FIQ */
#ifndef AT91C_ID_IRQ0
#define AT91C_ID_IRQ0  	30 /**< Advanced Interrupt Controller (IRQ0) id */
#endif /* AT91C_ID_IRQ0 */
#ifndef AT91C_ID_IRQ1
#define AT91C_ID_IRQ1  	31 /**< Advanced Interrupt Controller (IRQ1) id */
#endif /* AT91C_ID_IRQ1 */

/* -------------------------------------------------------- */
/* AIC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_AIC       	0xFFFFF000 /**< AIC base address */

/* -------------------------------------------------------- */
/* PIO definition for AIC hardware peripheral */
/* -------------------------------------------------------- */
#define AT91C_PA19_FIQ      	(1 << 19) /**< AIC Fast Interrupt Input */
#define AT91C_PA20_IRQ0     	(1 << 20) /**< External Interrupt 0 */
#define AT91C_PA30_IRQ1     	(1 << 30) /**< External Interrupt 1 */


/* -------------------------------------------------------- */
/* Register offset definition for AIC hardware peripheral */
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

/* -------------------------------------------------------- */
/* Bitfields definition for AIC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register AIC_SMR */
#define AT91C_AIC_PRIOR       (0x7 << 0 ) /**< (AIC) Priority Level */
#define 	AT91C_AIC_PRIOR_LOWEST               0x0 /**< (AIC) Lowest priority level */
#define 	AT91C_AIC_PRIOR_HIGHEST              0x7 /**< (AIC) Highest priority level */
#define AT91C_AIC_SRCTYPE     (0x3 << 5 ) /**< (AIC) Interrupt Source Type */
#define 	AT91C_AIC_SRCTYPE_EXT_LOW_LEVEL        (0x0 <<  5) /**< (AIC) External Sources Code Label Low-level Sensitive */
#define 	AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL       (0x0 <<  5) /**< (AIC) Internal Sources Code Label High-level Sensitive */
#define 	AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE    (0x1 <<  5) /**< (AIC) Internal Sources Code Label Positive Edge triggered */
#define 	AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE    (0x1 <<  5) /**< (AIC) External Sources Code Label Negative Edge triggered */
#define 	AT91C_AIC_SRCTYPE_HIGH_LEVEL           (0x2 <<  5) /**< (AIC) Internal Or External Sources Code Label High-level Sensitive */
#define 	AT91C_AIC_SRCTYPE_POSITIVE_EDGE        (0x3 <<  5) /**< (AIC) Internal Or External Sources Code Label Positive Edge triggered */
/* --- Register AIC_CISR */
#define AT91C_AIC_NFIQ        (0x1 << 0 ) /**< (AIC) NFIQ Status */
#define AT91C_AIC_NIRQ        (0x1 << 1 ) /**< (AIC) NIRQ Status */
/* --- Register AIC_DCR */
#define AT91C_AIC_DCR_PROT    (0x1 << 0 ) /**< (AIC) Protection Mode */
#define AT91C_AIC_DCR_GMSK    (0x1 << 1 ) /**< (AIC) General Mask */

#endif /* __AT91SAM7S256_AIC_H */
