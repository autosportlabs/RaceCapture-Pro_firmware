/* This is a reduced version of the AT91SAM7S*.h files provided by
   ATMEL Microcontroller Software Support  -  ROUSSET  -
   
   This file is used by the EFSL-interface to reduced the
   size of the distribution-archive. It can be replaced 
   by the files from Atmel.
   
   Done by Martin Thomas, 2006
*/

#ifndef AT91SAM7S64_REGS_H
#define AT91SAM7S64_REGS_H

typedef volatile unsigned long AT91_REG;// Hardware register definition

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Serial Parallel Interface
// *****************************************************************************
typedef struct _AT91S_SPI {
	AT91_REG	 SPI_CR; 	// Control Register
	AT91_REG	 SPI_MR; 	// Mode Register
	AT91_REG	 SPI_RDR; 	// Receive Data Register
	AT91_REG	 SPI_TDR; 	// Transmit Data Register
	AT91_REG	 SPI_SR; 	// Status Register
	AT91_REG	 SPI_IER; 	// Interrupt Enable Register
	AT91_REG	 SPI_IDR; 	// Interrupt Disable Register
	AT91_REG	 SPI_IMR; 	// Interrupt Mask Register
	AT91_REG	 Reserved0[4]; 	// 
	AT91_REG	 SPI_CSR[4]; 	// Chip Select Register
	AT91_REG	 Reserved1[48]; 	// 
	AT91_REG	 SPI_RPR; 	// Receive Pointer Register
	AT91_REG	 SPI_RCR; 	// Receive Counter Register
	AT91_REG	 SPI_TPR; 	// Transmit Pointer Register
	AT91_REG	 SPI_TCR; 	// Transmit Counter Register
	AT91_REG	 SPI_RNPR; 	// Receive Next Pointer Register
	AT91_REG	 SPI_RNCR; 	// Receive Next Counter Register
	AT91_REG	 SPI_TNPR; 	// Transmit Next Pointer Register
	AT91_REG	 SPI_TNCR; 	// Transmit Next Counter Register
	AT91_REG	 SPI_PTCR; 	// PDC Transfer Control Register
	AT91_REG	 SPI_PTSR; 	// PDC Transfer Status Register
} AT91S_SPI, *AT91PS_SPI;

// -------- SPI_CR : (SPI Offset: 0x0) SPI Control Register -------- 
#define AT91C_SPI_SPIEN       ((unsigned int) 0x1 <<  0) // (SPI) SPI Enable
#define AT91C_SPI_SPIDIS      ((unsigned int) 0x1 <<  1) // (SPI) SPI Disable
#define AT91C_SPI_SWRST       ((unsigned int) 0x1 <<  7) // (SPI) SPI Software reset
#define AT91C_SPI_LASTXFER    ((unsigned int) 0x1 << 24) // (SPI) SPI Last Transfer
// -------- SPI_MR : (SPI Offset: 0x4) SPI Mode Register -------- 
#define AT91C_SPI_MSTR        ((unsigned int) 0x1 <<  0) // (SPI) Master/Slave Mode
#define AT91C_SPI_PS          ((unsigned int) 0x1 <<  1) // (SPI) Peripheral Select
#define 	AT91C_SPI_PS_FIXED                ((unsigned int) 0x0 <<  1) // (SPI) Fixed Peripheral Select
#define 	AT91C_SPI_PS_VARIABLE             ((unsigned int) 0x1 <<  1) // (SPI) Variable Peripheral Select
#define AT91C_SPI_PCSDEC      ((unsigned int) 0x1 <<  2) // (SPI) Chip Select Decode
#define AT91C_SPI_FDIV        ((unsigned int) 0x1 <<  3) // (SPI) Clock Selection
#define AT91C_SPI_MODFDIS     ((unsigned int) 0x1 <<  4) // (SPI) Mode Fault Detection
#define AT91C_SPI_LLB         ((unsigned int) 0x1 <<  7) // (SPI) Clock Selection
#define AT91C_SPI_PCS         ((unsigned int) 0xF << 16) // (SPI) Peripheral Chip Select
#define AT91C_SPI_DLYBCS      ((unsigned int) 0xFF << 24) // (SPI) Delay Between Chip Selects
// -------- SPI_RDR : (SPI Offset: 0x8) Receive Data Register -------- 
#define AT91C_SPI_RD          ((unsigned int) 0xFFFF <<  0) // (SPI) Receive Data
#define AT91C_SPI_RPCS        ((unsigned int) 0xF << 16) // (SPI) Peripheral Chip Select Status
// -------- SPI_TDR : (SPI Offset: 0xc) Transmit Data Register -------- 
#define AT91C_SPI_TD          ((unsigned int) 0xFFFF <<  0) // (SPI) Transmit Data
#define AT91C_SPI_TPCS        ((unsigned int) 0xF << 16) // (SPI) Peripheral Chip Select Status
// -------- SPI_SR : (SPI Offset: 0x10) Status Register -------- 
#define AT91C_SPI_RDRF        ((unsigned int) 0x1 <<  0) // (SPI) Receive Data Register Full
#define AT91C_SPI_TDRE        ((unsigned int) 0x1 <<  1) // (SPI) Transmit Data Register Empty
#define AT91C_SPI_MODF        ((unsigned int) 0x1 <<  2) // (SPI) Mode Fault Error
#define AT91C_SPI_OVRES       ((unsigned int) 0x1 <<  3) // (SPI) Overrun Error Status
#define AT91C_SPI_ENDRX       ((unsigned int) 0x1 <<  4) // (SPI) End of Receiver Transfer
#define AT91C_SPI_ENDTX       ((unsigned int) 0x1 <<  5) // (SPI) End of Receiver Transfer
#define AT91C_SPI_RXBUFF      ((unsigned int) 0x1 <<  6) // (SPI) RXBUFF Interrupt
#define AT91C_SPI_TXBUFE      ((unsigned int) 0x1 <<  7) // (SPI) TXBUFE Interrupt
#define AT91C_SPI_NSSR        ((unsigned int) 0x1 <<  8) // (SPI) NSSR Interrupt
#define AT91C_SPI_TXEMPTY     ((unsigned int) 0x1 <<  9) // (SPI) TXEMPTY Interrupt
#define AT91C_SPI_SPIENS      ((unsigned int) 0x1 << 16) // (SPI) Enable Status
// -------- SPI_IER : (SPI Offset: 0x14) Interrupt Enable Register -------- 
// -------- SPI_IDR : (SPI Offset: 0x18) Interrupt Disable Register -------- 
// -------- SPI_IMR : (SPI Offset: 0x1c) Interrupt Mask Register -------- 
// -------- SPI_CSR : (SPI Offset: 0x30) Chip Select Register -------- 
#define AT91C_SPI_CPOL        ((unsigned int) 0x1 <<  0) // (SPI) Clock Polarity
#define AT91C_SPI_NCPHA       ((unsigned int) 0x1 <<  1) // (SPI) Clock Phase
#define AT91C_SPI_CSAAT       ((unsigned int) 0x1 <<  3) // (SPI) Chip Select Active After Transfer
#define AT91C_SPI_BITS        ((unsigned int) 0xF <<  4) // (SPI) Bits Per Transfer
#define 	AT91C_SPI_BITS_8                    ((unsigned int) 0x0 <<  4) // (SPI) 8 Bits Per transfer
#define 	AT91C_SPI_BITS_9                    ((unsigned int) 0x1 <<  4) // (SPI) 9 Bits Per transfer
#define 	AT91C_SPI_BITS_10                   ((unsigned int) 0x2 <<  4) // (SPI) 10 Bits Per transfer
#define 	AT91C_SPI_BITS_11                   ((unsigned int) 0x3 <<  4) // (SPI) 11 Bits Per transfer
#define 	AT91C_SPI_BITS_12                   ((unsigned int) 0x4 <<  4) // (SPI) 12 Bits Per transfer
#define 	AT91C_SPI_BITS_13                   ((unsigned int) 0x5 <<  4) // (SPI) 13 Bits Per transfer
#define 	AT91C_SPI_BITS_14                   ((unsigned int) 0x6 <<  4) // (SPI) 14 Bits Per transfer
#define 	AT91C_SPI_BITS_15                   ((unsigned int) 0x7 <<  4) // (SPI) 15 Bits Per transfer
#define 	AT91C_SPI_BITS_16                   ((unsigned int) 0x8 <<  4) // (SPI) 16 Bits Per transfer
#define AT91C_SPI_SCBR        ((unsigned int) 0xFF <<  8) // (SPI) Serial Clock Baud Rate
#define AT91C_SPI_DLYBS       ((unsigned int) 0xFF << 16) // (SPI) Delay Before SPCK
#define AT91C_SPI_DLYBCT      ((unsigned int) 0xFF << 24) // (SPI) Delay Between Consecutive Transfers

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Parallel Input Output Controler
// *****************************************************************************
typedef struct _AT91S_PIO {
	AT91_REG	 PIO_PER; 	// PIO Enable Register
	AT91_REG	 PIO_PDR; 	// PIO Disable Register
	AT91_REG	 PIO_PSR; 	// PIO Status Register
	AT91_REG	 Reserved0[1]; 	// 
	AT91_REG	 PIO_OER; 	// Output Enable Register
	AT91_REG	 PIO_ODR; 	// Output Disable Registerr
	AT91_REG	 PIO_OSR; 	// Output Status Register
	AT91_REG	 Reserved1[1]; 	// 
	AT91_REG	 PIO_IFER; 	// Input Filter Enable Register
	AT91_REG	 PIO_IFDR; 	// Input Filter Disable Register
	AT91_REG	 PIO_IFSR; 	// Input Filter Status Register
	AT91_REG	 Reserved2[1]; 	// 
	AT91_REG	 PIO_SODR; 	// Set Output Data Register
	AT91_REG	 PIO_CODR; 	// Clear Output Data Register
	AT91_REG	 PIO_ODSR; 	// Output Data Status Register
	AT91_REG	 PIO_PDSR; 	// Pin Data Status Register
	AT91_REG	 PIO_IER; 	// Interrupt Enable Register
	AT91_REG	 PIO_IDR; 	// Interrupt Disable Register
	AT91_REG	 PIO_IMR; 	// Interrupt Mask Register
	AT91_REG	 PIO_ISR; 	// Interrupt Status Register
	AT91_REG	 PIO_MDER; 	// Multi-driver Enable Register
	AT91_REG	 PIO_MDDR; 	// Multi-driver Disable Register
	AT91_REG	 PIO_MDSR; 	// Multi-driver Status Register
	AT91_REG	 Reserved3[1]; 	// 
	AT91_REG	 PIO_PPUDR; 	// Pull-up Disable Register
	AT91_REG	 PIO_PPUER; 	// Pull-up Enable Register
	AT91_REG	 PIO_PPUSR; 	// Pull-up Status Register
	AT91_REG	 Reserved4[1]; 	// 
	AT91_REG	 PIO_ASR; 	// Select A Register
	AT91_REG	 PIO_BSR; 	// Select B Register
	AT91_REG	 PIO_ABSR; 	// AB Select Status Register
	AT91_REG	 Reserved5[9]; 	// 
	AT91_REG	 PIO_OWER; 	// Output Write Enable Register
	AT91_REG	 PIO_OWDR; 	// Output Write Disable Register
	AT91_REG	 PIO_OWSR; 	// Output Write Status Register
} AT91S_PIO, *AT91PS_PIO;

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Power Management Controler
// *****************************************************************************
typedef struct _AT91S_PMC {
	AT91_REG	 PMC_SCER; 	// System Clock Enable Register
	AT91_REG	 PMC_SCDR; 	// System Clock Disable Register
	AT91_REG	 PMC_SCSR; 	// System Clock Status Register
	AT91_REG	 Reserved0[1]; 	// 
	AT91_REG	 PMC_PCER; 	// Peripheral Clock Enable Register
	AT91_REG	 PMC_PCDR; 	// Peripheral Clock Disable Register
	AT91_REG	 PMC_PCSR; 	// Peripheral Clock Status Register
	AT91_REG	 Reserved1[1]; 	// 
	AT91_REG	 PMC_MOR; 	// Main Oscillator Register
	AT91_REG	 PMC_MCFR; 	// Main Clock  Frequency Register
	AT91_REG	 Reserved2[1]; 	// 
	AT91_REG	 PMC_PLLR; 	// PLL Register
	AT91_REG	 PMC_MCKR; 	// Master Clock Register
	AT91_REG	 Reserved3[3]; 	// 
	AT91_REG	 PMC_PCKR[3]; 	// Programmable Clock Register
	AT91_REG	 Reserved4[5]; 	// 
	AT91_REG	 PMC_IER; 	// Interrupt Enable Register
	AT91_REG	 PMC_IDR; 	// Interrupt Disable Register
	AT91_REG	 PMC_SR; 	// Status Register
	AT91_REG	 PMC_IMR; 	// Interrupt Mask Register
} AT91S_PMC, *AT91PS_PMC;

// *****************************************************************************
//               PIO DEFINITIONS FOR AT91SAM7S256
// *****************************************************************************
#define AT91C_PIO_PA0        ((unsigned int) 1 <<  0) // Pin Controlled by PA0
#define AT91C_PA0_PWM0     ((unsigned int) AT91C_PIO_PA0) //  PWM Channel 0
#define AT91C_PA0_TIOA0    ((unsigned int) AT91C_PIO_PA0) //  Timer Counter 0 Multipurpose Timer I/O Pin A
#define AT91C_PIO_PA1        ((unsigned int) 1 <<  1) // Pin Controlled by PA1
#define AT91C_PA1_PWM1     ((unsigned int) AT91C_PIO_PA1) //  PWM Channel 1
#define AT91C_PA1_TIOB0    ((unsigned int) AT91C_PIO_PA1) //  Timer Counter 0 Multipurpose Timer I/O Pin B
#define AT91C_PIO_PA10       ((unsigned int) 1 << 10) // Pin Controlled by PA10
#define AT91C_PA10_DTXD     ((unsigned int) AT91C_PIO_PA10) //  DBGU Debug Transmit Data
#define AT91C_PA10_NPCS2    ((unsigned int) AT91C_PIO_PA10) //  SPI Peripheral Chip Select 2
#define AT91C_PIO_PA11       ((unsigned int) 1 << 11) // Pin Controlled by PA11
#define AT91C_PA11_NPCS0    ((unsigned int) AT91C_PIO_PA11) //  SPI Peripheral Chip Select 0
#define AT91C_PA11_PWM0     ((unsigned int) AT91C_PIO_PA11) //  PWM Channel 0
#define AT91C_PIO_PA12       ((unsigned int) 1 << 12) // Pin Controlled by PA12
#define AT91C_PA12_MISO     ((unsigned int) AT91C_PIO_PA12) //  SPI Master In Slave
#define AT91C_PA12_PWM1     ((unsigned int) AT91C_PIO_PA12) //  PWM Channel 1
#define AT91C_PIO_PA13       ((unsigned int) 1 << 13) // Pin Controlled by PA13
#define AT91C_PA13_MOSI     ((unsigned int) AT91C_PIO_PA13) //  SPI Master Out Slave
#define AT91C_PA13_PWM2     ((unsigned int) AT91C_PIO_PA13) //  PWM Channel 2
#define AT91C_PIO_PA14       ((unsigned int) 1 << 14) // Pin Controlled by PA14
#define AT91C_PA14_SPCK     ((unsigned int) AT91C_PIO_PA14) //  SPI Serial Clock
#define AT91C_PA14_PWM3     ((unsigned int) AT91C_PIO_PA14) //  PWM Channel 3
#define AT91C_PIO_PA15       ((unsigned int) 1 << 15) // Pin Controlled by PA15
#define AT91C_PA15_TF       ((unsigned int) AT91C_PIO_PA15) //  SSC Transmit Frame Sync
#define AT91C_PA15_TIOA1    ((unsigned int) AT91C_PIO_PA15) //  Timer Counter 1 Multipurpose Timer I/O Pin A
#define AT91C_PIO_PA16       ((unsigned int) 1 << 16) // Pin Controlled by PA16
#define AT91C_PA16_TK       ((unsigned int) AT91C_PIO_PA16) //  SSC Transmit Clock
#define AT91C_PA16_TIOB1    ((unsigned int) AT91C_PIO_PA16) //  Timer Counter 1 Multipurpose Timer I/O Pin B
#define AT91C_PIO_PA17       ((unsigned int) 1 << 17) // Pin Controlled by PA17
#define AT91C_PA17_TD       ((unsigned int) AT91C_PIO_PA17) //  SSC Transmit data
#define AT91C_PA17_PCK1     ((unsigned int) AT91C_PIO_PA17) //  PMC Programmable Clock Output 1
#define AT91C_PIO_PA18       ((unsigned int) 1 << 18) // Pin Controlled by PA18
#define AT91C_PA18_RD       ((unsigned int) AT91C_PIO_PA18) //  SSC Receive Data
#define AT91C_PA18_PCK2     ((unsigned int) AT91C_PIO_PA18) //  PMC Programmable Clock Output 2
#define AT91C_PIO_PA19       ((unsigned int) 1 << 19) // Pin Controlled by PA19
#define AT91C_PA19_RK       ((unsigned int) AT91C_PIO_PA19) //  SSC Receive Clock
#define AT91C_PA19_FIQ      ((unsigned int) AT91C_PIO_PA19) //  AIC Fast Interrupt Input
#define AT91C_PIO_PA2        ((unsigned int) 1 <<  2) // Pin Controlled by PA2
#define AT91C_PA2_PWM2     ((unsigned int) AT91C_PIO_PA2) //  PWM Channel 2
#define AT91C_PA2_SCK0     ((unsigned int) AT91C_PIO_PA2) //  USART 0 Serial Clock
#define AT91C_PIO_PA20       ((unsigned int) 1 << 20) // Pin Controlled by PA20
#define AT91C_PA20_RF       ((unsigned int) AT91C_PIO_PA20) //  SSC Receive Frame Sync
#define AT91C_PA20_IRQ0     ((unsigned int) AT91C_PIO_PA20) //  External Interrupt 0
#define AT91C_PIO_PA21       ((unsigned int) 1 << 21) // Pin Controlled by PA21
#define AT91C_PA21_RXD1     ((unsigned int) AT91C_PIO_PA21) //  USART 1 Receive Data
#define AT91C_PA21_PCK1     ((unsigned int) AT91C_PIO_PA21) //  PMC Programmable Clock Output 1
#define AT91C_PIO_PA22       ((unsigned int) 1 << 22) // Pin Controlled by PA22
#define AT91C_PA22_TXD1     ((unsigned int) AT91C_PIO_PA22) //  USART 1 Transmit Data
#define AT91C_PA22_NPCS3    ((unsigned int) AT91C_PIO_PA22) //  SPI Peripheral Chip Select 3
#define AT91C_PIO_PA23       ((unsigned int) 1 << 23) // Pin Controlled by PA23
#define AT91C_PA23_SCK1     ((unsigned int) AT91C_PIO_PA23) //  USART 1 Serial Clock
#define AT91C_PA23_PWM0     ((unsigned int) AT91C_PIO_PA23) //  PWM Channel 0
#define AT91C_PIO_PA24       ((unsigned int) 1 << 24) // Pin Controlled by PA24
#define AT91C_PA24_RTS1     ((unsigned int) AT91C_PIO_PA24) //  USART 1 Ready To Send
#define AT91C_PA24_PWM1     ((unsigned int) AT91C_PIO_PA24) //  PWM Channel 1
#define AT91C_PIO_PA25       ((unsigned int) 1 << 25) // Pin Controlled by PA25
#define AT91C_PA25_CTS1     ((unsigned int) AT91C_PIO_PA25) //  USART 1 Clear To Send
#define AT91C_PA25_PWM2     ((unsigned int) AT91C_PIO_PA25) //  PWM Channel 2
#define AT91C_PIO_PA26       ((unsigned int) 1 << 26) // Pin Controlled by PA26
#define AT91C_PA26_DCD1     ((unsigned int) AT91C_PIO_PA26) //  USART 1 Data Carrier Detect
#define AT91C_PA26_TIOA2    ((unsigned int) AT91C_PIO_PA26) //  Timer Counter 2 Multipurpose Timer I/O Pin A
#define AT91C_PIO_PA27       ((unsigned int) 1 << 27) // Pin Controlled by PA27
#define AT91C_PA27_DTR1     ((unsigned int) AT91C_PIO_PA27) //  USART 1 Data Terminal ready
#define AT91C_PA27_TIOB2    ((unsigned int) AT91C_PIO_PA27) //  Timer Counter 2 Multipurpose Timer I/O Pin B
#define AT91C_PIO_PA28       ((unsigned int) 1 << 28) // Pin Controlled by PA28
#define AT91C_PA28_DSR1     ((unsigned int) AT91C_PIO_PA28) //  USART 1 Data Set ready
#define AT91C_PA28_TCLK1    ((unsigned int) AT91C_PIO_PA28) //  Timer Counter 1 external clock input
#define AT91C_PIO_PA29       ((unsigned int) 1 << 29) // Pin Controlled by PA29
#define AT91C_PA29_RI1      ((unsigned int) AT91C_PIO_PA29) //  USART 1 Ring Indicator
#define AT91C_PA29_TCLK2    ((unsigned int) AT91C_PIO_PA29) //  Timer Counter 2 external clock input
#define AT91C_PIO_PA3        ((unsigned int) 1 <<  3) // Pin Controlled by PA3
#define AT91C_PA3_TWD      ((unsigned int) AT91C_PIO_PA3) //  TWI Two-wire Serial Data
#define AT91C_PA3_NPCS3    ((unsigned int) AT91C_PIO_PA3) //  SPI Peripheral Chip Select 3
#define AT91C_PIO_PA30       ((unsigned int) 1 << 30) // Pin Controlled by PA30
#define AT91C_PA30_IRQ1     ((unsigned int) AT91C_PIO_PA30) //  External Interrupt 1
#define AT91C_PA30_NPCS2    ((unsigned int) AT91C_PIO_PA30) //  SPI Peripheral Chip Select 2
#define AT91C_PIO_PA31       ((unsigned int) 1 << 31) // Pin Controlled by PA31
#define AT91C_PA31_NPCS1    ((unsigned int) AT91C_PIO_PA31) //  SPI Peripheral Chip Select 1
#define AT91C_PA31_PCK2     ((unsigned int) AT91C_PIO_PA31) //  PMC Programmable Clock Output 2
#define AT91C_PIO_PA4        ((unsigned int) 1 <<  4) // Pin Controlled by PA4
#define AT91C_PA4_TWCK     ((unsigned int) AT91C_PIO_PA4) //  TWI Two-wire Serial Clock
#define AT91C_PA4_TCLK0    ((unsigned int) AT91C_PIO_PA4) //  Timer Counter 0 external clock input
#define AT91C_PIO_PA5        ((unsigned int) 1 <<  5) // Pin Controlled by PA5
#define AT91C_PA5_RXD0     ((unsigned int) AT91C_PIO_PA5) //  USART 0 Receive Data
#define AT91C_PA5_NPCS3    ((unsigned int) AT91C_PIO_PA5) //  SPI Peripheral Chip Select 3
#define AT91C_PIO_PA6        ((unsigned int) 1 <<  6) // Pin Controlled by PA6
#define AT91C_PA6_TXD0     ((unsigned int) AT91C_PIO_PA6) //  USART 0 Transmit Data
#define AT91C_PA6_PCK0     ((unsigned int) AT91C_PIO_PA6) //  PMC Programmable Clock Output 0
#define AT91C_PIO_PA7        ((unsigned int) 1 <<  7) // Pin Controlled by PA7
#define AT91C_PA7_RTS0     ((unsigned int) AT91C_PIO_PA7) //  USART 0 Ready To Send
#define AT91C_PA7_PWM3     ((unsigned int) AT91C_PIO_PA7) //  PWM Channel 3
#define AT91C_PIO_PA8        ((unsigned int) 1 <<  8) // Pin Controlled by PA8
#define AT91C_PA8_CTS0     ((unsigned int) AT91C_PIO_PA8) //  USART 0 Clear To Send
#define AT91C_PA8_ADTRG    ((unsigned int) AT91C_PIO_PA8) //  ADC External Trigger
#define AT91C_PIO_PA9        ((unsigned int) 1 <<  9) // Pin Controlled by PA9
#define AT91C_PA9_DRXD     ((unsigned int) AT91C_PIO_PA9) //  DBGU Debug Receive Data
#define AT91C_PA9_NPCS1    ((unsigned int) AT91C_PIO_PA9) //  SPI Peripheral Chip Select 1

// ========== Register definition for PDC_SPI peripheral ========== 
#define AT91C_SPI_PTCR  ((AT91_REG *) 	0xFFFE0120) // (PDC_SPI) PDC Transfer Control Register
#define AT91C_SPI_TNPR  ((AT91_REG *) 	0xFFFE0118) // (PDC_SPI) Transmit Next Pointer Register
#define AT91C_SPI_RNPR  ((AT91_REG *) 	0xFFFE0110) // (PDC_SPI) Receive Next Pointer Register
#define AT91C_SPI_TPR   ((AT91_REG *) 	0xFFFE0108) // (PDC_SPI) Transmit Pointer Register
#define AT91C_SPI_RPR   ((AT91_REG *) 	0xFFFE0100) // (PDC_SPI) Receive Pointer Register
#define AT91C_SPI_PTSR  ((AT91_REG *) 	0xFFFE0124) // (PDC_SPI) PDC Transfer Status Register
#define AT91C_SPI_TNCR  ((AT91_REG *) 	0xFFFE011C) // (PDC_SPI) Transmit Next Counter Register
#define AT91C_SPI_RNCR  ((AT91_REG *) 	0xFFFE0114) // (PDC_SPI) Receive Next Counter Register
#define AT91C_SPI_TCR   ((AT91_REG *) 	0xFFFE010C) // (PDC_SPI) Transmit Counter Register
#define AT91C_SPI_RCR   ((AT91_REG *) 	0xFFFE0104) // (PDC_SPI) Receive Counter Register
// ========== Register definition for SPI peripheral ========== 
#define AT91C_SPI_CSR   ((AT91_REG *) 	0xFFFE0030) // (SPI) Chip Select Register
#define AT91C_SPI_IDR   ((AT91_REG *) 	0xFFFE0018) // (SPI) Interrupt Disable Register
#define AT91C_SPI_SR    ((AT91_REG *) 	0xFFFE0010) // (SPI) Status Register
#define AT91C_SPI_RDR   ((AT91_REG *) 	0xFFFE0008) // (SPI) Receive Data Register
#define AT91C_SPI_CR    ((AT91_REG *) 	0xFFFE0000) // (SPI) Control Register
#define AT91C_SPI_IMR   ((AT91_REG *) 	0xFFFE001C) // (SPI) Interrupt Mask Register
#define AT91C_SPI_IER   ((AT91_REG *) 	0xFFFE0014) // (SPI) Interrupt Enable Register
#define AT91C_SPI_TDR   ((AT91_REG *) 	0xFFFE000C) // (SPI) Transmit Data Register
#define AT91C_SPI_MR    ((AT91_REG *) 	0xFFFE0004) // (SPI) Mode Register

// *****************************************************************************
//               (some) PERIPHERAL ID DEFINITIONS FOR AT91SAM7S256
// *****************************************************************************
#define AT91C_ID_SPI    ((unsigned int)  5) // Serial Peripheral Interface

// *****************************************************************************
//               (some) BASE ADDRESS DEFINITIONS FOR AT91SAM7S256
// *****************************************************************************
#define AT91C_BASE_PIOA      ((AT91PS_PIO) 	0xFFFFF400) // (PIOA) Base Address
#define AT91C_BASE_PMC       ((AT91PS_PMC) 	0xFFFFFC00) // (PMC) Base Address
#define AT91C_BASE_PDC_SPI   ((AT91PS_PDC) 	0xFFFE0100) // (PDC_SPI) Base Address
#define AT91C_BASE_SPI       ((AT91PS_SPI) 	0xFFFE0000) // (SPI) Base Address


#endif
