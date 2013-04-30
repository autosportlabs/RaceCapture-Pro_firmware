/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_tcb.h
 * 
 * Hardware definition for the tcb peripheral in the ATMEL at91sam7s256 processor
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


#ifndef __AT91SAM7S256_TCB_H
#define __AT91SAM7S256_TCB_H

/* -------------------------------------------------------- */
/* TCB ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* TCB Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_TCB       	0xFFFA0000 /**< TCB base address */

/* -------------------------------------------------------- */
/* PIO definition for TCB hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for TCB hardware peripheral */
/* -------------------------------------------------------- */
#define TCB_TC0 	(0x0000) 	/**< TC Channel 0 */
#define TCB_TC1 	(0x0040) 	/**< TC Channel 1 */
#define TCB_TC2 	(0x0080) 	/**< TC Channel 2 */
#define TCB_BCR 	(0x00C0) 	/**< TC Block Control Register */
#define TCB_BMR 	(0x00C4) 	/**< TC Block Mode Register */

/* -------------------------------------------------------- */
/* Bitfields definition for TCB hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register TCB_BCR */
#define AT91C_TCB_SYNC        (0x1 << 0 ) /**< (TCB) Synchro Command */
/* --- Register TCB_BMR */
#define AT91C_TCB_TC0XC0S     (0x3 << 0 ) /**< (TCB) External Clock Signal 0 Selection */
#define 	AT91C_TCB_TC0XC0S_TCLK0                0x0 /**< (TCB) TCLK0 connected to XC0 */
#define 	AT91C_TCB_TC0XC0S_NONE                 0x1 /**< (TCB) None signal connected to XC0 */
#define 	AT91C_TCB_TC0XC0S_TIOA1                0x2 /**< (TCB) TIOA1 connected to XC0 */
#define 	AT91C_TCB_TC0XC0S_TIOA2                0x3 /**< (TCB) TIOA2 connected to XC0 */
#define AT91C_TCB_TC1XC1S     (0x3 << 2 ) /**< (TCB) External Clock Signal 1 Selection */
#define 	AT91C_TCB_TC1XC1S_TCLK1                (0x0 <<  2) /**< (TCB) TCLK1 connected to XC1 */
#define 	AT91C_TCB_TC1XC1S_NONE                 (0x1 <<  2) /**< (TCB) None signal connected to XC1 */
#define 	AT91C_TCB_TC1XC1S_TIOA0                (0x2 <<  2) /**< (TCB) TIOA0 connected to XC1 */
#define 	AT91C_TCB_TC1XC1S_TIOA2                (0x3 <<  2) /**< (TCB) TIOA2 connected to XC1 */
#define AT91C_TCB_TC2XC2S     (0x3 << 4 ) /**< (TCB) External Clock Signal 2 Selection */
#define 	AT91C_TCB_TC2XC2S_TCLK2                (0x0 <<  4) /**< (TCB) TCLK2 connected to XC2 */
#define 	AT91C_TCB_TC2XC2S_NONE                 (0x1 <<  4) /**< (TCB) None signal connected to XC2 */
#define 	AT91C_TCB_TC2XC2S_TIOA0                (0x2 <<  4) /**< (TCB) TIOA0 connected to XC2 */
#define 	AT91C_TCB_TC2XC2S_TIOA1                (0x3 <<  4) /**< (TCB) TIOA2 connected to XC2 */

#endif /* __AT91SAM7S256_TCB_H */
