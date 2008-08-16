/* linux/include/asm-arm/arch-at91sam7s256/at91sam7s256_ssc.h
 * 
 * Hardware definition for the ssc peripheral in the ATMEL at91sam7s256 processor
 * 
 * Generated  01/16/2006 (16:36:10) AT91 SW Application Group from SSC_6078A V1.1
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


#ifndef __AT91SAM7S256_SSC_H
#define __AT91SAM7S256_SSC_H

/* -------------------------------------------------------- */
/* SSC ID definitions for  AT91SAM7S256           */
/* -------------------------------------------------------- */
#ifndef AT91C_ID_SSC
#define AT91C_ID_SSC   	 8 /**< Serial Synchronous Controller id */
#endif /* AT91C_ID_SSC */

/* -------------------------------------------------------- */
/* SSC Base Address definitions for  AT91SAM7S256   */
/* -------------------------------------------------------- */
#define AT91C_BASE_SSC       	0xFFFD4000 /**< SSC base address */

/* -------------------------------------------------------- */
/* PIO definition for SSC hardware peripheral */
/* -------------------------------------------------------- */
#define AT91C_PA18_RD       	(1 << 18) /**< SSC Receive Data */
#define AT91C_PA20_RF       	(1 << 20) /**< SSC Receive Frame Sync */
#define AT91C_PA19_RK       	(1 << 19) /**< SSC Receive Clock */
#define AT91C_PA17_TD       	(1 << 17) /**< SSC Transmit data */
#define AT91C_PA15_TF       	(1 << 15) /**< SSC Transmit Frame Sync */
#define AT91C_PA16_TK       	(1 << 16) /**< SSC Transmit Clock */


/* -------------------------------------------------------- */
/* Register offset definition for SSC hardware peripheral */
/* -------------------------------------------------------- */
#define SSC_CR 	(0x0000) 	/**< Control Register */
#define SSC_CMR 	(0x0004) 	/**< Clock Mode Register */
#define SSC_RCMR 	(0x0010) 	/**< Receive Clock ModeRegister */
#define SSC_RFMR 	(0x0014) 	/**< Receive Frame Mode Register */
#define SSC_TCMR 	(0x0018) 	/**< Transmit Clock Mode Register */
#define SSC_TFMR 	(0x001C) 	/**< Transmit Frame Mode Register */
#define SSC_RHR 	(0x0020) 	/**< Receive Holding Register */
#define SSC_THR 	(0x0024) 	/**< Transmit Holding Register */
#define SSC_RSHR 	(0x0030) 	/**< Receive Sync Holding Register */
#define SSC_TSHR 	(0x0034) 	/**< Transmit Sync Holding Register */
#define SSC_SR 	(0x0040) 	/**< Status Register */
#define SSC_IER 	(0x0044) 	/**< Interrupt Enable Register */
#define SSC_IDR 	(0x0048) 	/**< Interrupt Disable Register */
#define SSC_IMR 	(0x004C) 	/**< Interrupt Mask Register */
#define SSC_RPR 	(0x0100) 	/**< Receive Pointer Register */
#define SSC_RCR 	(0x0104) 	/**< Receive Counter Register */
#define SSC_TPR 	(0x0108) 	/**< Transmit Pointer Register */
#define SSC_TCR 	(0x010C) 	/**< Transmit Counter Register */
#define SSC_RNPR 	(0x0110) 	/**< Receive Next Pointer Register */
#define SSC_RNCR 	(0x0114) 	/**< Receive Next Counter Register */
#define SSC_TNPR 	(0x0118) 	/**< Transmit Next Pointer Register */
#define SSC_TNCR 	(0x011C) 	/**< Transmit Next Counter Register */
#define SSC_PTCR 	(0x0120) 	/**< PDC Transfer Control Register */
#define SSC_PTSR 	(0x0124) 	/**< PDC Transfer Status Register */

/* -------------------------------------------------------- */
/* Bitfields definition for SSC hardware peripheral */
/* -------------------------------------------------------- */
/* --- Register SSC_CR */
#define AT91C_SSC_RXEN        (0x1 << 0 ) /**< (SSC) Receive Enable */
#define AT91C_SSC_RXDIS       (0x1 << 1 ) /**< (SSC) Receive Disable */
#define AT91C_SSC_TXEN        (0x1 << 8 ) /**< (SSC) Transmit Enable */
#define AT91C_SSC_TXDIS       (0x1 << 9 ) /**< (SSC) Transmit Disable */
#define AT91C_SSC_SWRST       (0x1 << 15) /**< (SSC) Software Reset */
/* --- Register SSC_RCMR */
#define AT91C_SSC_CKS         (0x3 << 0 ) /**< (SSC) Receive/Transmit Clock Selection */
#define 	AT91C_SSC_CKS_DIV                  0x0 /**< (SSC) Divided Clock */
#define 	AT91C_SSC_CKS_TK                   0x1 /**< (SSC) TK Clock signal */
#define 	AT91C_SSC_CKS_RK                   0x2 /**< (SSC) RK pin */
#define AT91C_SSC_CKO         (0x7 << 2 ) /**< (SSC) Receive/Transmit Clock Output Mode Selection */
#define 	AT91C_SSC_CKO_NONE                 (0x0 <<  2) /**< (SSC) Receive/Transmit Clock Output Mode: None RK pin: Input-only */
#define 	AT91C_SSC_CKO_CONTINOUS            (0x1 <<  2) /**< (SSC) Continuous Receive/Transmit Clock RK pin: Output */
#define 	AT91C_SSC_CKO_DATA_TX              (0x2 <<  2) /**< (SSC) Receive/Transmit Clock only during data transfers RK pin: Output */
#define AT91C_SSC_CKI         (0x1 << 5 ) /**< (SSC) Receive/Transmit Clock Inversion */
#define AT91C_SSC_START       (0xF << 8 ) /**< (SSC) Receive/Transmit Start Selection */
#define 	AT91C_SSC_START_CONTINOUS            (0x0 <<  8) /**< (SSC) Continuous, as soon as the receiver is enabled, and immediately after the end of transfer of the previous data. */
#define 	AT91C_SSC_START_TX                   (0x1 <<  8) /**< (SSC) Transmit/Receive start */
#define 	AT91C_SSC_START_LOW_RF               (0x2 <<  8) /**< (SSC) Detection of a low level on RF input */
#define 	AT91C_SSC_START_HIGH_RF              (0x3 <<  8) /**< (SSC) Detection of a high level on RF input */
#define 	AT91C_SSC_START_FALL_RF              (0x4 <<  8) /**< (SSC) Detection of a falling edge on RF input */
#define 	AT91C_SSC_START_RISE_RF              (0x5 <<  8) /**< (SSC) Detection of a rising edge on RF input */
#define 	AT91C_SSC_START_LEVEL_RF             (0x6 <<  8) /**< (SSC) Detection of any level change on RF input */
#define 	AT91C_SSC_START_EDGE_RF              (0x7 <<  8) /**< (SSC) Detection of any edge on RF input */
#define 	AT91C_SSC_START_0                    (0x8 <<  8) /**< (SSC) Compare 0 */
#define AT91C_SSC_STTDLY      (0xFF << 16) /**< (SSC) Receive/Transmit Start Delay */
#define AT91C_SSC_PERIOD      (0xFF << 24) /**< (SSC) Receive/Transmit Period Divider Selection */
/* --- Register SSC_RFMR */
#define AT91C_SSC_DATLEN      (0x1F << 0 ) /**< (SSC) Data Length */
#define AT91C_SSC_LOOP        (0x1 << 5 ) /**< (SSC) Loop Mode */
#define AT91C_SSC_MSBF        (0x1 << 7 ) /**< (SSC) Most Significant Bit First */
#define AT91C_SSC_DATNB       (0xF << 8 ) /**< (SSC) Data Number per Frame */
#define AT91C_SSC_FSLEN       (0xF << 16) /**< (SSC) Receive/Transmit Frame Sync length */
#define AT91C_SSC_FSOS        (0x7 << 20) /**< (SSC) Receive/Transmit Frame Sync Output Selection */
#define 	AT91C_SSC_FSOS_NONE                 (0x0 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: None RK pin Input-only */
#define 	AT91C_SSC_FSOS_NEGATIVE             (0x1 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: Negative Pulse */
#define 	AT91C_SSC_FSOS_POSITIVE             (0x2 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: Positive Pulse */
#define 	AT91C_SSC_FSOS_LOW                  (0x3 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: Driver Low during data transfer */
#define 	AT91C_SSC_FSOS_HIGH                 (0x4 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: Driver High during data transfer */
#define 	AT91C_SSC_FSOS_TOGGLE               (0x5 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: Toggling at each start of data transfer */
#define AT91C_SSC_FSEDGE      (0x1 << 24) /**< (SSC) Frame Sync Edge Detection */
/* --- Register SSC_TCMR */
#define AT91C_SSC_CKS         (0x3 << 0 ) /**< (SSC) Receive/Transmit Clock Selection */
#define 	AT91C_SSC_CKS_DIV                  0x0 /**< (SSC) Divided Clock */
#define 	AT91C_SSC_CKS_TK                   0x1 /**< (SSC) TK Clock signal */
#define 	AT91C_SSC_CKS_RK                   0x2 /**< (SSC) RK pin */
#define AT91C_SSC_CKO         (0x7 << 2 ) /**< (SSC) Receive/Transmit Clock Output Mode Selection */
#define 	AT91C_SSC_CKO_NONE                 (0x0 <<  2) /**< (SSC) Receive/Transmit Clock Output Mode: None RK pin: Input-only */
#define 	AT91C_SSC_CKO_CONTINOUS            (0x1 <<  2) /**< (SSC) Continuous Receive/Transmit Clock RK pin: Output */
#define 	AT91C_SSC_CKO_DATA_TX              (0x2 <<  2) /**< (SSC) Receive/Transmit Clock only during data transfers RK pin: Output */
#define AT91C_SSC_CKI         (0x1 << 5 ) /**< (SSC) Receive/Transmit Clock Inversion */
#define AT91C_SSC_START       (0xF << 8 ) /**< (SSC) Receive/Transmit Start Selection */
#define 	AT91C_SSC_START_CONTINOUS            (0x0 <<  8) /**< (SSC) Continuous, as soon as the receiver is enabled, and immediately after the end of transfer of the previous data. */
#define 	AT91C_SSC_START_TX                   (0x1 <<  8) /**< (SSC) Transmit/Receive start */
#define 	AT91C_SSC_START_LOW_RF               (0x2 <<  8) /**< (SSC) Detection of a low level on RF input */
#define 	AT91C_SSC_START_HIGH_RF              (0x3 <<  8) /**< (SSC) Detection of a high level on RF input */
#define 	AT91C_SSC_START_FALL_RF              (0x4 <<  8) /**< (SSC) Detection of a falling edge on RF input */
#define 	AT91C_SSC_START_RISE_RF              (0x5 <<  8) /**< (SSC) Detection of a rising edge on RF input */
#define 	AT91C_SSC_START_LEVEL_RF             (0x6 <<  8) /**< (SSC) Detection of any level change on RF input */
#define 	AT91C_SSC_START_EDGE_RF              (0x7 <<  8) /**< (SSC) Detection of any edge on RF input */
#define 	AT91C_SSC_START_0                    (0x8 <<  8) /**< (SSC) Compare 0 */
#define AT91C_SSC_STTDLY      (0xFF << 16) /**< (SSC) Receive/Transmit Start Delay */
#define AT91C_SSC_PERIOD      (0xFF << 24) /**< (SSC) Receive/Transmit Period Divider Selection */
/* --- Register SSC_TFMR */
#define AT91C_SSC_DATLEN      (0x1F << 0 ) /**< (SSC) Data Length */
#define AT91C_SSC_DATDEF      (0x1 << 5 ) /**< (SSC) Data Default Value */
#define AT91C_SSC_MSBF        (0x1 << 7 ) /**< (SSC) Most Significant Bit First */
#define AT91C_SSC_DATNB       (0xF << 8 ) /**< (SSC) Data Number per Frame */
#define AT91C_SSC_FSLEN       (0xF << 16) /**< (SSC) Receive/Transmit Frame Sync length */
#define AT91C_SSC_FSOS        (0x7 << 20) /**< (SSC) Receive/Transmit Frame Sync Output Selection */
#define 	AT91C_SSC_FSOS_NONE                 (0x0 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: None RK pin Input-only */
#define 	AT91C_SSC_FSOS_NEGATIVE             (0x1 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: Negative Pulse */
#define 	AT91C_SSC_FSOS_POSITIVE             (0x2 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: Positive Pulse */
#define 	AT91C_SSC_FSOS_LOW                  (0x3 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: Driver Low during data transfer */
#define 	AT91C_SSC_FSOS_HIGH                 (0x4 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: Driver High during data transfer */
#define 	AT91C_SSC_FSOS_TOGGLE               (0x5 << 20) /**< (SSC) Selected Receive/Transmit Frame Sync Signal: Toggling at each start of data transfer */
#define AT91C_SSC_FSDEN       (0x1 << 23) /**< (SSC) Frame Sync Data Enable */
#define AT91C_SSC_FSEDGE      (0x1 << 24) /**< (SSC) Frame Sync Edge Detection */
/* --- Register SSC_SR */
#define AT91C_SSC_TXRDY       (0x1 << 0 ) /**< (SSC) Transmit Ready */
#define AT91C_SSC_TXEMPTY     (0x1 << 1 ) /**< (SSC) Transmit Empty */
#define AT91C_SSC_ENDTX       (0x1 << 2 ) /**< (SSC) End Of Transmission */
#define AT91C_SSC_TXBUFE      (0x1 << 3 ) /**< (SSC) Transmit Buffer Empty */
#define AT91C_SSC_RXRDY       (0x1 << 4 ) /**< (SSC) Receive Ready */
#define AT91C_SSC_OVRUN       (0x1 << 5 ) /**< (SSC) Receive Overrun */
#define AT91C_SSC_ENDRX       (0x1 << 6 ) /**< (SSC) End of Reception */
#define AT91C_SSC_RXBUFF      (0x1 << 7 ) /**< (SSC) Receive Buffer Full */
#define AT91C_SSC_TXSYN       (0x1 << 10) /**< (SSC) Transmit Sync */
#define AT91C_SSC_RXSYN       (0x1 << 11) /**< (SSC) Receive Sync */
#define AT91C_SSC_TXENA       (0x1 << 16) /**< (SSC) Transmit Enable */
#define AT91C_SSC_RXENA       (0x1 << 17) /**< (SSC) Receive Enable */
/* --- Register SSC_IER */
#define AT91C_SSC_TXRDY       (0x1 << 0 ) /**< (SSC) Transmit Ready */
#define AT91C_SSC_TXEMPTY     (0x1 << 1 ) /**< (SSC) Transmit Empty */
#define AT91C_SSC_ENDTX       (0x1 << 2 ) /**< (SSC) End Of Transmission */
#define AT91C_SSC_TXBUFE      (0x1 << 3 ) /**< (SSC) Transmit Buffer Empty */
#define AT91C_SSC_RXRDY       (0x1 << 4 ) /**< (SSC) Receive Ready */
#define AT91C_SSC_OVRUN       (0x1 << 5 ) /**< (SSC) Receive Overrun */
#define AT91C_SSC_ENDRX       (0x1 << 6 ) /**< (SSC) End of Reception */
#define AT91C_SSC_RXBUFF      (0x1 << 7 ) /**< (SSC) Receive Buffer Full */
#define AT91C_SSC_TXSYN       (0x1 << 10) /**< (SSC) Transmit Sync */
#define AT91C_SSC_RXSYN       (0x1 << 11) /**< (SSC) Receive Sync */
/* --- Register SSC_IDR */
#define AT91C_SSC_TXRDY       (0x1 << 0 ) /**< (SSC) Transmit Ready */
#define AT91C_SSC_TXEMPTY     (0x1 << 1 ) /**< (SSC) Transmit Empty */
#define AT91C_SSC_ENDTX       (0x1 << 2 ) /**< (SSC) End Of Transmission */
#define AT91C_SSC_TXBUFE      (0x1 << 3 ) /**< (SSC) Transmit Buffer Empty */
#define AT91C_SSC_RXRDY       (0x1 << 4 ) /**< (SSC) Receive Ready */
#define AT91C_SSC_OVRUN       (0x1 << 5 ) /**< (SSC) Receive Overrun */
#define AT91C_SSC_ENDRX       (0x1 << 6 ) /**< (SSC) End of Reception */
#define AT91C_SSC_RXBUFF      (0x1 << 7 ) /**< (SSC) Receive Buffer Full */
#define AT91C_SSC_TXSYN       (0x1 << 10) /**< (SSC) Transmit Sync */
#define AT91C_SSC_RXSYN       (0x1 << 11) /**< (SSC) Receive Sync */
/* --- Register SSC_IMR */
#define AT91C_SSC_TXRDY       (0x1 << 0 ) /**< (SSC) Transmit Ready */
#define AT91C_SSC_TXEMPTY     (0x1 << 1 ) /**< (SSC) Transmit Empty */
#define AT91C_SSC_ENDTX       (0x1 << 2 ) /**< (SSC) End Of Transmission */
#define AT91C_SSC_TXBUFE      (0x1 << 3 ) /**< (SSC) Transmit Buffer Empty */
#define AT91C_SSC_RXRDY       (0x1 << 4 ) /**< (SSC) Receive Ready */
#define AT91C_SSC_OVRUN       (0x1 << 5 ) /**< (SSC) Receive Overrun */
#define AT91C_SSC_ENDRX       (0x1 << 6 ) /**< (SSC) End of Reception */
#define AT91C_SSC_RXBUFF      (0x1 << 7 ) /**< (SSC) Receive Buffer Full */
#define AT91C_SSC_TXSYN       (0x1 << 10) /**< (SSC) Transmit Sync */
#define AT91C_SSC_RXSYN       (0x1 << 11) /**< (SSC) Receive Sync */

#endif /* __AT91SAM7S256_SSC_H */
