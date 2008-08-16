/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_vreg.h
 * 
 * Hardware definition for the vreg peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from VREG_6085B V1.1
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


#ifndef __AT91SAM7S256_VREG_H
#define __AT91SAM7S256_VREG_H

/* -------------------------------------------------------- */
/* VREG ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_SYS
#define AT91C_ID_SYS   	 1 /**< System Peripheral id */
#endif /* AT91C_ID_SYS */

/* -------------------------------------------------------- */
/* VREG Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_VREG      	0xFFFFFD60 /**< VREG base address */

/* -------------------------------------------------------- */
/* PIO definition for VREG hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for VREG hardware peripheral */
/* -------------------------------------------------------- */
#define VREG_MR 	(0x0000) 	/**< Voltage Regulator Mode Register */

/* -------------------------------------------------------- */
/* Bitfields definition for VREG hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register VREG_MR */
#define AT91C_VREG_PSTDBY     (0x1 << 0 ) /**< (VREG) Voltage Regulator Power Standby Mode */

#endif /* __AT91SAM7S256_VREG_H */
