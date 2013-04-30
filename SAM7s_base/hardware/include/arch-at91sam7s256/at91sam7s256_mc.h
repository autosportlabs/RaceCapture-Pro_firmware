/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_mc.h
 * 
 * Hardware definition for the mc peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from MC_SAM7S V1.3
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


#ifndef __AT91SAM7S256_MC_H
#define __AT91SAM7S256_MC_H

/* -------------------------------------------------------- */
/* MC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_SYS
#define AT91C_ID_SYS   	 1 /**< System Peripheral id */
#endif /* AT91C_ID_SYS */

/* -------------------------------------------------------- */
/* MC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_MC        	0xFFFFFF00 /**< MC base address */

/* -------------------------------------------------------- */
/* PIO definition for MC hardware peripheral */
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/* Register offset definition for MC hardware peripheral */
/* -------------------------------------------------------- */
#define MC_RCR 	(0x0000) 	/**< MC Remap Control Register */
#define MC_ASR 	(0x0004) 	/**< MC Abort Status Register */
#define MC_AASR 	(0x0008) 	/**< MC Abort Address Status Register */
#define MC_FMR 	(0x0060) 	/**< MC Flash Mode Register */
#define MC_FCR 	(0x0064) 	/**< MC Flash Command Register */
#define MC_FSR 	(0x0068) 	/**< MC Flash Status Register */

/* -------------------------------------------------------- */
/* Bitfields definition for MC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register MC_RCR */
#define AT91C_MC_RCB          (0x1 << 0 ) /**< (MC) Remap Command Bit */
/* --- Register MC_ASR */
#define AT91C_MC_UNDADD       (0x1 << 0 ) /**< (MC) Undefined Addess Abort Status */
#define AT91C_MC_MISADD       (0x1 << 1 ) /**< (MC) Misaligned Addess Abort Status */
#define AT91C_MC_ABTSZ        (0x3 << 8 ) /**< (MC) Abort Size Status */
#define 	AT91C_MC_ABTSZ_BYTE                 (0x0 <<  8) /**< (MC) Byte */
#define 	AT91C_MC_ABTSZ_HWORD                (0x1 <<  8) /**< (MC) Half-word */
#define 	AT91C_MC_ABTSZ_WORD                 (0x2 <<  8) /**< (MC) Word */
#define AT91C_MC_ABTTYP       (0x3 << 10) /**< (MC) Abort Type Status */
#define 	AT91C_MC_ABTTYP_DATAR                (0x0 << 10) /**< (MC) Data Read */
#define 	AT91C_MC_ABTTYP_DATAW                (0x1 << 10) /**< (MC) Data Write */
#define 	AT91C_MC_ABTTYP_FETCH                (0x2 << 10) /**< (MC) Code Fetch */
#define AT91C_MC_MST0         (0x1 << 16) /**< (MC) Master 0 Abort Source */
#define AT91C_MC_MST1         (0x1 << 17) /**< (MC) Master 1 Abort Source */
#define AT91C_MC_SVMST0       (0x1 << 24) /**< (MC) Saved Master 0 Abort Source */
#define AT91C_MC_SVMST1       (0x1 << 25) /**< (MC) Saved Master 1 Abort Source */
/* --- Register MC_FMR */
#define AT91C_MC_FRDY         (0x1 << 0 ) /**< (MC) Flash Ready */
#define AT91C_MC_LOCKE        (0x1 << 2 ) /**< (MC) Lock Error */
#define AT91C_MC_PROGE        (0x1 << 3 ) /**< (MC) Programming Error */
#define AT91C_MC_NEBP         (0x1 << 7 ) /**< (MC) No Erase Before Programming */
#define AT91C_MC_FWS          (0x3 << 8 ) /**< (MC) Flash Wait State */
#define 	AT91C_MC_FWS_0FWS                 (0x0 <<  8) /**< (MC) 1 cycle for Read, 2 for Write operations */
#define 	AT91C_MC_FWS_1FWS                 (0x1 <<  8) /**< (MC) 2 cycles for Read, 3 for Write operations */
#define 	AT91C_MC_FWS_2FWS                 (0x2 <<  8) /**< (MC) 3 cycles for Read, 4 for Write operations */
#define 	AT91C_MC_FWS_3FWS                 (0x3 <<  8) /**< (MC) 4 cycles for Read, 4 for Write operations */
#define AT91C_MC_FMCN         (0xFF << 16) /**< (MC) Flash Microsecond Cycle Number */
/* --- Register MC_FCR */
#define AT91C_MC_FCMD         (0xF << 0 ) /**< (MC) Flash Command */
#define 	AT91C_MC_FCMD_START_PROG           0x1 /**< (MC) Starts the programming of th epage specified by PAGEN. */
#define 	AT91C_MC_FCMD_LOCK                 0x2 /**< (MC) Starts a lock sequence of the sector defined by the bits 4 to 7 of the field PAGEN. */
#define 	AT91C_MC_FCMD_PROG_AND_LOCK        0x3 /**< (MC) The lock sequence automatically happens after the programming sequence is completed. */
#define 	AT91C_MC_FCMD_UNLOCK               0x4 /**< (MC) Starts an unlock sequence of the sector defined by the bits 4 to 7 of the field PAGEN. */
#define 	AT91C_MC_FCMD_ERASE_ALL            0x8 /**< (MC) Starts the erase of the entire flash.If at least a page is locked, the command is cancelled. */
#define 	AT91C_MC_FCMD_SET_GP_NVM           0xB /**< (MC) Set General Purpose NVM bits. */
#define 	AT91C_MC_FCMD_CLR_GP_NVM           0xD /**< (MC) Clear General Purpose NVM bits. */
#define 	AT91C_MC_FCMD_SET_SECURITY         0xF /**< (MC) Set Security Bit. */
#define AT91C_MC_PAGEN        (0x3FF << 8 ) /**< (MC) Page Number */
#define AT91C_MC_KEY          (0xFF << 24) /**< (MC) Writing Protect Key */
/* --- Register MC_FSR */
#define AT91C_MC_FRDY         (0x1 << 0 ) /**< (MC) Flash Ready */
#define AT91C_MC_LOCKE        (0x1 << 2 ) /**< (MC) Lock Error */
#define AT91C_MC_PROGE        (0x1 << 3 ) /**< (MC) Programming Error */
#define AT91C_MC_SECURITY     (0x1 << 4 ) /**< (MC) Security Bit Status */
#define AT91C_MC_GPNVM0       (0x1 << 8 ) /**< (MC) Sector 0 Lock Status */
#define AT91C_MC_GPNVM1       (0x1 << 9 ) /**< (MC) Sector 1 Lock Status */
#define AT91C_MC_GPNVM2       (0x1 << 10) /**< (MC) Sector 2 Lock Status */
#define AT91C_MC_GPNVM3       (0x1 << 11) /**< (MC) Sector 3 Lock Status */
#define AT91C_MC_GPNVM4       (0x1 << 12) /**< (MC) Sector 4 Lock Status */
#define AT91C_MC_GPNVM5       (0x1 << 13) /**< (MC) Sector 5 Lock Status */
#define AT91C_MC_GPNVM6       (0x1 << 14) /**< (MC) Sector 6 Lock Status */
#define AT91C_MC_GPNVM7       (0x1 << 15) /**< (MC) Sector 7 Lock Status */
#define AT91C_MC_LOCKS0       (0x1 << 16) /**< (MC) Sector 0 Lock Status */
#define AT91C_MC_LOCKS1       (0x1 << 17) /**< (MC) Sector 1 Lock Status */
#define AT91C_MC_LOCKS2       (0x1 << 18) /**< (MC) Sector 2 Lock Status */
#define AT91C_MC_LOCKS3       (0x1 << 19) /**< (MC) Sector 3 Lock Status */
#define AT91C_MC_LOCKS4       (0x1 << 20) /**< (MC) Sector 4 Lock Status */
#define AT91C_MC_LOCKS5       (0x1 << 21) /**< (MC) Sector 5 Lock Status */
#define AT91C_MC_LOCKS6       (0x1 << 22) /**< (MC) Sector 6 Lock Status */
#define AT91C_MC_LOCKS7       (0x1 << 23) /**< (MC) Sector 7 Lock Status */
#define AT91C_MC_LOCKS8       (0x1 << 24) /**< (MC) Sector 8 Lock Status */
#define AT91C_MC_LOCKS9       (0x1 << 25) /**< (MC) Sector 9 Lock Status */
#define AT91C_MC_LOCKS10      (0x1 << 26) /**< (MC) Sector 10 Lock Status */
#define AT91C_MC_LOCKS11      (0x1 << 27) /**< (MC) Sector 11 Lock Status */
#define AT91C_MC_LOCKS12      (0x1 << 28) /**< (MC) Sector 12 Lock Status */
#define AT91C_MC_LOCKS13      (0x1 << 29) /**< (MC) Sector 13 Lock Status */
#define AT91C_MC_LOCKS14      (0x1 << 30) /**< (MC) Sector 14 Lock Status */
#define AT91C_MC_LOCKS15      (0x1 << 31) /**< (MC) Sector 15 Lock Status */

#endif /* __AT91SAM7S256_MC_H */
