//* ----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//* ----------------------------------------------------------------------------
//* DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
//* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
//* DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
//* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
//* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//* ----------------------------------------------------------------------------
//* File Name           : lib_AT91SAM7S256.h
//* Object              : AT91SAM7S256 inlined functions
//* Generated           : AT91 SW Application Group  01/16/2006 (16:36:07)
//*
//* CVS Reference       : /lib_MC_SAM7S.h/1.1/Thu Mar 25 15:19:14 2004//
//* CVS Reference       : /lib_pdc.h/1.2/Tue Jul  2 13:29:40 2002//
//* CVS Reference       : /lib_dbgu.h/1.1/Thu Aug 25 12:56:22 2005//
//* CVS Reference       : /lib_VREG_6085B.h/1.1/Tue Feb  1 16:20:47 2005//
//* CVS Reference       : /lib_ssc.h/1.4/Fri Jan 31 12:19:20 2003//
//* CVS Reference       : /lib_spi2.h/1.2/Tue Aug 23 15:37:28 2005//
//* CVS Reference       : /lib_PWM_SAM.h/1.3/Thu Jan 22 10:10:50 2004//
//* CVS Reference       : /lib_tc_1753b.h/1.1/Fri Jan 31 12:20:02 2003//
//* CVS Reference       : /lib_pitc_6079A.h/1.2/Tue Nov  9 14:43:56 2004//
//* CVS Reference       : /lib_pmc_SAM7S.h/1.5/Fri Nov  4 09:37:21 2005//
//* CVS Reference       : /lib_adc.h/1.6/Fri Oct 17 09:12:38 2003//
//* CVS Reference       : /lib_rstc_6098A.h/1.1/Wed Oct  6 10:39:20 2004//
//* CVS Reference       : /lib_rttc_6081A.h/1.1/Wed Oct  6 10:39:38 2004//
//* CVS Reference       : /lib_pio.h/1.3/Fri Jan 31 12:18:56 2003//
//* CVS Reference       : /lib_twi.h/1.3/Mon Jul 19 14:27:58 2004//
//* CVS Reference       : /lib_wdtc_6080A.h/1.1/Wed Oct  6 10:38:30 2004//
//* CVS Reference       : /lib_usart.h/1.5/Thu Nov 21 16:01:54 2002//
//* CVS Reference       : /lib_udp.h/1.5/Tue Aug 30 12:13:47 2005//
//* CVS Reference       : /lib_aic_6075b.h/1.2/Thu Jul  7 07:48:22 2005//
//* ----------------------------------------------------------------------------

#ifndef lib_AT91SAM7S256_H
#define lib_AT91SAM7S256_H

/* *****************************************************************************
                SOFTWARE API FOR AIC
   ***************************************************************************** */
#define AT91C_AIC_BRANCH_OPCODE ((void (*) ()) 0xE51FFF20) // ldr, pc, [pc, #-&F20]

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_ConfigureIt
//* \brief Interrupt Handler Initialization
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_AIC_ConfigureIt (
	AT91PS_AIC pAic,  // \arg pointer to the AIC registers
	unsigned int irq_id,     // \arg interrupt number to initialize
	unsigned int priority,   // \arg priority to give to the interrupt
	unsigned int src_type,   // \arg activation and sense of activation
	void (*newHandler) () ); // \arg address of the interrupt handler


//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_EnableIt
//* \brief Enable corresponding IT number
//*----------------------------------------------------------------------------
__inline void AT91F_AIC_EnableIt (
	AT91PS_AIC pAic,      // \arg pointer to the AIC registers
	unsigned int irq_id ); // \arg interrupt number to initialize


//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_DisableIt
//* \brief Disable corresponding IT number
//*----------------------------------------------------------------------------
__inline void AT91F_AIC_DisableIt (
	AT91PS_AIC pAic,      // \arg pointer to the AIC registers
	unsigned int irq_id ); // \arg interrupt number to initialize


//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_ClearIt
//* \brief Clear corresponding IT number
//*----------------------------------------------------------------------------
__inline void AT91F_AIC_ClearIt (
	AT91PS_AIC pAic,     // \arg pointer to the AIC registers
	unsigned int irq_id); // \arg interrupt number to initialize


//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_AcknowledgeIt
//* \brief Acknowledge corresponding IT number
//*----------------------------------------------------------------------------
__inline void AT91F_AIC_AcknowledgeIt (
	AT91PS_AIC pAic);     // \arg pointer to the AIC registers


//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_SetExceptionVector
//* \brief Configure vector handler
//*----------------------------------------------------------------------------
__inline unsigned int  AT91F_AIC_SetExceptionVector (
	unsigned int *pVector, // \arg pointer to the AIC registers
	void (*Handler) () );   // \arg Interrupt Handler


//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_Trig
//* \brief Trig an IT
//*----------------------------------------------------------------------------
__inline void  AT91F_AIC_Trig (
	AT91PS_AIC pAic,     // \arg pointer to the AIC registers
	unsigned int irq_id); // \arg interrupt number


//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_IsActive
//* \brief Test if an IT is active
//*----------------------------------------------------------------------------
__inline unsigned int  AT91F_AIC_IsActive (
	AT91PS_AIC pAic,     // \arg pointer to the AIC registers
	unsigned int irq_id); // \arg Interrupt Number


//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_IsPending
//* \brief Test if an IT is pending
//*----------------------------------------------------------------------------
__inline unsigned int  AT91F_AIC_IsPending (
	AT91PS_AIC pAic,     // \arg pointer to the AIC registers
	unsigned int irq_id); // \arg Interrupt Number


//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_Open
//* \brief Set exception vectors and AIC registers to default values
//*----------------------------------------------------------------------------
__inline void AT91F_AIC_Open(
	AT91PS_AIC pAic,        // \arg pointer to the AIC registers
	void (*IrqHandler) (),  // \arg Default IRQ vector exception
	void (*FiqHandler) (),  // \arg Default FIQ vector exception
	void (*DefaultHandler)  (), // \arg Default Handler set in ISR
	void (*SpuriousHandler) (), // \arg Default Spurious Handler
	unsigned int protectMode);   // \arg Debug Control Register

/* *****************************************************************************
                SOFTWARE API FOR PDC
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_SetNextRx
//* \brief Set the next receive transfer descriptor
//*----------------------------------------------------------------------------
__inline void AT91F_PDC_SetNextRx (
	AT91PS_PDC pPDC,     // \arg pointer to a PDC controller
	char *address,       // \arg address to the next bloc to be received
	unsigned int bytes);  // \arg number of bytes to be received


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_SetNextTx
//* \brief Set the next transmit transfer descriptor
//*----------------------------------------------------------------------------
__inline void AT91F_PDC_SetNextTx (
	AT91PS_PDC pPDC,       // \arg pointer to a PDC controller
	char *address,         // \arg address to the next bloc to be transmitted
	unsigned int bytes);    // \arg number of bytes to be transmitted


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_SetRx
//* \brief Set the receive transfer descriptor
//*----------------------------------------------------------------------------
__inline void AT91F_PDC_SetRx (
	AT91PS_PDC pPDC,       // \arg pointer to a PDC controller
	char *address,         // \arg address to the next bloc to be received
	unsigned int bytes);    // \arg number of bytes to be received


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_SetTx
//* \brief Set the transmit transfer descriptor
//*----------------------------------------------------------------------------
__inline void AT91F_PDC_SetTx (
	AT91PS_PDC pPDC,       // \arg pointer to a PDC controller
	char *address,         // \arg address to the next bloc to be transmitted
	unsigned int bytes);    // \arg number of bytes to be transmitted


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_EnableTx
//* \brief Enable transmit
//*----------------------------------------------------------------------------
__inline void AT91F_PDC_EnableTx (
	AT91PS_PDC pPDC );       // \arg pointer to a PDC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_EnableRx
//* \brief Enable receive
//*----------------------------------------------------------------------------
__inline void AT91F_PDC_EnableRx (
	AT91PS_PDC pPDC );       // \arg pointer to a PDC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_DisableTx
//* \brief Disable transmit
//*----------------------------------------------------------------------------
__inline void AT91F_PDC_DisableTx (
	AT91PS_PDC pPDC );       // \arg pointer to a PDC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_DisableRx
//* \brief Disable receive
//*----------------------------------------------------------------------------
__inline void AT91F_PDC_DisableRx (
	AT91PS_PDC pPDC );       // \arg pointer to a PDC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_IsTxEmpty
//* \brief Test if the current transfer descriptor has been sent
//*----------------------------------------------------------------------------
__inline int AT91F_PDC_IsTxEmpty ( // \return return 1 if transfer is complete
	AT91PS_PDC pPDC );       // \arg pointer to a PDC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_IsNextTxEmpty
//* \brief Test if the next transfer descriptor has been moved to the current td
//*----------------------------------------------------------------------------
__inline int AT91F_PDC_IsNextTxEmpty ( // \return return 1 if transfer is complete
	AT91PS_PDC pPDC );       // \arg pointer to a PDC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_IsRxEmpty
//* \brief Test if the current transfer descriptor has been filled
//*----------------------------------------------------------------------------
__inline int AT91F_PDC_IsRxEmpty ( // \return return 1 if transfer is complete
	AT91PS_PDC pPDC );       // \arg pointer to a PDC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_IsNextRxEmpty
//* \brief Test if the next transfer descriptor has been moved to the current td
//*----------------------------------------------------------------------------
__inline int AT91F_PDC_IsNextRxEmpty ( // \return return 1 if transfer is complete
	AT91PS_PDC pPDC );       // \arg pointer to a PDC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_Open
//* \brief Open PDC: disable TX and RX reset transfer descriptors, re-enable RX and TX
//*----------------------------------------------------------------------------
__inline void AT91F_PDC_Open (
	AT91PS_PDC pPDC);       // \arg pointer to a PDC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_Close
//* \brief Close PDC: disable TX and RX reset transfer descriptors
//*----------------------------------------------------------------------------
__inline void AT91F_PDC_Close (
	AT91PS_PDC pPDC);       // \arg pointer to a PDC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_SendFrame
//* \brief Close PDC: disable TX and RX reset transfer descriptors
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PDC_SendFrame(
	AT91PS_PDC pPDC,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer );


//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_ReceiveFrame
//* \brief Close PDC: disable TX and RX reset transfer descriptors
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PDC_ReceiveFrame (
	AT91PS_PDC pPDC,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer );

/* *****************************************************************************
                SOFTWARE API FOR DBGU
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_InterruptEnable
//* \brief Enable DBGU Interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_DBGU_InterruptEnable(
        AT91PS_DBGU pDbgu,   // \arg  pointer to a DBGU controller
        unsigned int flag); // \arg  dbgu interrupt to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_InterruptDisable
//* \brief Disable DBGU Interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_DBGU_InterruptDisable(
        AT91PS_DBGU pDbgu,   // \arg  pointer to a DBGU controller
        unsigned int flag); // \arg  dbgu interrupt to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_GetInterruptMaskStatus
//* \brief Return DBGU Interrupt Mask Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_DBGU_GetInterruptMaskStatus( // \return DBGU Interrupt Mask Status
        AT91PS_DBGU pDbgu); // \arg  pointer to a DBGU controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_IsInterruptMasked
//* \brief Test if DBGU Interrupt is Masked 
//*----------------------------------------------------------------------------
__inline int AT91F_DBGU_IsInterruptMasked(
        AT91PS_DBGU pDbgu,   // \arg  pointer to a DBGU controller
        unsigned int flag); // \arg  flag to be tested


/* *****************************************************************************
                SOFTWARE API FOR PIO
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgPeriph
//* \brief Enable pins to be drived by peripheral
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_CfgPeriph(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int periphAEnable,  // \arg PERIPH A to enable
	unsigned int periphBEnable);  // \arg PERIPH B to enable


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgOutput
//* \brief Enable PIO in output mode
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_CfgOutput(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int pioEnable);      // \arg PIO to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgInput
//* \brief Enable PIO in input mode
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_CfgInput(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int inputEnable);      // \arg PIO to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgOpendrain
//* \brief Configure PIO in open drain
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_CfgOpendrain(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int multiDrvEnable); // \arg pio to be configured in open drain


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgPullup
//* \brief Enable pullup on PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_CfgPullup(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int pullupEnable);   // \arg enable pullup on PIO


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgDirectDrive
//* \brief Enable direct drive on PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_CfgDirectDrive(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int directDrive);    // \arg PIO to be configured with direct drive


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgInputFilter
//* \brief Enable input filter on input PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_CfgInputFilter(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int inputFilter);    // \arg PIO to be configured with input filter


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetInput
//* \brief Return PIO input value
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PIO_GetInput( // \return PIO input
	AT91PS_PIO pPio); // \arg  pointer to a PIO controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsInputSet
//* \brief Test if PIO is input flag is active
//*----------------------------------------------------------------------------
__inline int AT91F_PIO_IsInputSet(
	AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
	unsigned int flag); // \arg  flag to be tested



//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_SetOutput
//* \brief Set to 1 output PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_SetOutput(
	AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
	unsigned int flag); // \arg  output to be set


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_ClearOutput
//* \brief Set to 0 output PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_ClearOutput(
	AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
	unsigned int flag); // \arg  output to be cleared


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_ForceOutput
//* \brief Force output when Direct drive option is enabled
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_ForceOutput(
	AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
	unsigned int flag); // \arg  output to be forced


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_Enable
//* \brief Enable PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_Enable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio to be enabled 


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_Disable
//* \brief Disable PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_Disable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio to be disabled 


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetStatus
//* \brief Return PIO Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PIO_GetStatus( // \return PIO Status
        AT91PS_PIO pPio); // \arg  pointer to a PIO controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsSet
//* \brief Test if PIO is Set
//*----------------------------------------------------------------------------
__inline int AT91F_PIO_IsSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_OutputEnable
//* \brief Output Enable PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_OutputEnable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio output to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_OutputDisable
//* \brief Output Enable PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_OutputDisable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio output to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetOutputStatus
//* \brief Return PIO Output Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PIO_GetOutputStatus( // \return PIO Output Status
        AT91PS_PIO pPio); // \arg  pointer to a PIO controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsOuputSet
//* \brief Test if PIO Output is Set
//*----------------------------------------------------------------------------
__inline int AT91F_PIO_IsOutputSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_InputFilterEnable
//* \brief Input Filter Enable PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_InputFilterEnable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio input filter to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_InputFilterDisable
//* \brief Input Filter Disable PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_InputFilterDisable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio input filter to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetInputFilterStatus
//* \brief Return PIO Input Filter Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PIO_GetInputFilterStatus( // \return PIO Input Filter Status
        AT91PS_PIO pPio); // \arg  pointer to a PIO controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsInputFilterSet
//* \brief Test if PIO Input filter is Set
//*----------------------------------------------------------------------------
__inline int AT91F_PIO_IsInputFilterSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetOutputDataStatus
//* \brief Return PIO Output Data Status 
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PIO_GetOutputDataStatus( // \return PIO Output Data Status 
	AT91PS_PIO pPio); // \arg  pointer to a PIO controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_InterruptEnable
//* \brief Enable PIO Interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_InterruptEnable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio interrupt to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_InterruptDisable
//* \brief Disable PIO Interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_InterruptDisable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio interrupt to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetInterruptMaskStatus
//* \brief Return PIO Interrupt Mask Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PIO_GetInterruptMaskStatus( // \return PIO Interrupt Mask Status
        AT91PS_PIO pPio); // \arg  pointer to a PIO controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetInterruptStatus
//* \brief Return PIO Interrupt Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PIO_GetInterruptStatus( // \return PIO Interrupt Status
        AT91PS_PIO pPio); // \arg  pointer to a PIO controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsInterruptMasked
//* \brief Test if PIO Interrupt is Masked 
//*----------------------------------------------------------------------------
__inline int AT91F_PIO_IsInterruptMasked(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsInterruptSet
//* \brief Test if PIO Interrupt is Set
//*----------------------------------------------------------------------------
__inline int AT91F_PIO_IsInterruptSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_MultiDriverEnable
//* \brief Multi Driver Enable PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_MultiDriverEnable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_MultiDriverDisable
//* \brief Multi Driver Disable PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_MultiDriverDisable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetMultiDriverStatus
//* \brief Return PIO Multi Driver Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PIO_GetMultiDriverStatus( // \return PIO Multi Driver Status
        AT91PS_PIO pPio); // \arg  pointer to a PIO controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsMultiDriverSet
//* \brief Test if PIO MultiDriver is Set
//*----------------------------------------------------------------------------
__inline int AT91F_PIO_IsMultiDriverSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_A_RegisterSelection
//* \brief PIO A Register Selection 
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_A_RegisterSelection(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio A register selection


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_B_RegisterSelection
//* \brief PIO B Register Selection 
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_B_RegisterSelection(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio B register selection 


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_Get_AB_RegisterStatus
//* \brief Return PIO Interrupt Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PIO_Get_AB_RegisterStatus( // \return PIO AB Register Status
        AT91PS_PIO pPio); // \arg  pointer to a PIO controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsAB_RegisterSet
//* \brief Test if PIO AB Register is Set
//*----------------------------------------------------------------------------
__inline int AT91F_PIO_IsAB_RegisterSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_OutputWriteEnable
//* \brief Output Write Enable PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_OutputWriteEnable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio output write to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_OutputWriteDisable
//* \brief Output Write Disable PIO
//*----------------------------------------------------------------------------
__inline void AT91F_PIO_OutputWriteDisable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  pio output write to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetOutputWriteStatus
//* \brief Return PIO Output Write Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PIO_GetOutputWriteStatus( // \return PIO Output Write Status
        AT91PS_PIO pPio); // \arg  pointer to a PIO controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsOutputWriteSet
//* \brief Test if PIO OutputWrite is Set
//*----------------------------------------------------------------------------
__inline int AT91F_PIO_IsOutputWriteSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetCfgPullup
//* \brief Return PIO Configuration Pullup
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PIO_GetCfgPullup( // \return PIO Configuration Pullup 
        AT91PS_PIO pPio); // \arg  pointer to a PIO controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsOutputDataStatusSet
//* \brief Test if PIO Output Data Status is Set 
//*----------------------------------------------------------------------------
__inline int AT91F_PIO_IsOutputDataStatusSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsCfgPullupStatusSet
//* \brief Test if PIO Configuration Pullup Status is Set
//*----------------------------------------------------------------------------
__inline int AT91F_PIO_IsCfgPullupStatusSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag); // \arg  flag to be tested


/* *****************************************************************************
                SOFTWARE API FOR PMC
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_CfgSysClkEnableReg
//* \brief Configure the System Clock Enable Register of the PMC controller
//*----------------------------------------------------------------------------
__inline void AT91F_PMC_CfgSysClkEnableReg (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int mode);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_CfgSysClkDisableReg
//* \brief Configure the System Clock Disable Register of the PMC controller
//*----------------------------------------------------------------------------
__inline void AT91F_PMC_CfgSysClkDisableReg (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int mode);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_GetSysClkStatusReg
//* \brief Return the System Clock Status Register of the PMC controller
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PMC_GetSysClkStatusReg (
	AT91PS_PMC pPMC // pointer to a CAN controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_EnablePeriphClock
//* \brief Enable peripheral clock
//*----------------------------------------------------------------------------
__inline void AT91F_PMC_EnablePeriphClock (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int periphIds);  // \arg IDs of peripherals


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_DisablePeriphClock
//* \brief Disable peripheral clock
//*----------------------------------------------------------------------------
__inline void AT91F_PMC_DisablePeriphClock (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int periphIds);  // \arg IDs of peripherals


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_GetPeriphClock
//* \brief Get peripheral clock status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PMC_GetPeriphClock (
	AT91PS_PMC pPMC); // \arg pointer to PMC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_CKGR_CfgMainOscillatorReg
//* \brief Cfg the main oscillator
//*----------------------------------------------------------------------------
__inline void AT91F_CKGR_CfgMainOscillatorReg (
	AT91PS_CKGR pCKGR, // \arg pointer to CKGR controller
	unsigned int mode);


//*----------------------------------------------------------------------------
//* \fn    AT91F_CKGR_GetMainOscillatorReg
//* \brief Cfg the main oscillator
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_CKGR_GetMainOscillatorReg (
	AT91PS_CKGR pCKGR); // \arg pointer to CKGR controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_CKGR_EnableMainOscillator
//* \brief Enable the main oscillator
//*----------------------------------------------------------------------------
__inline void AT91F_CKGR_EnableMainOscillator(
	AT91PS_CKGR pCKGR); // \arg pointer to CKGR controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_CKGR_DisableMainOscillator
//* \brief Disable the main oscillator
//*----------------------------------------------------------------------------
__inline void AT91F_CKGR_DisableMainOscillator (
	AT91PS_CKGR pCKGR); // \arg pointer to CKGR controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_CKGR_CfgMainOscStartUpTime
//* \brief Cfg MOR Register according to the main osc startup time
//*----------------------------------------------------------------------------
__inline void AT91F_CKGR_CfgMainOscStartUpTime (
	AT91PS_CKGR pCKGR, // \arg pointer to CKGR controller
	unsigned int startup_time,  // \arg main osc startup time in microsecond (us)
	unsigned int slowClock);  // \arg slowClock in Hz


//*----------------------------------------------------------------------------
//* \fn    AT91F_CKGR_GetMainClockFreqReg
//* \brief Cfg the main oscillator
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_CKGR_GetMainClockFreqReg (
	AT91PS_CKGR pCKGR); // \arg pointer to CKGR controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_CKGR_GetMainClock
//* \brief Return Main clock in Hz
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_CKGR_GetMainClock (
	AT91PS_CKGR pCKGR, // \arg pointer to CKGR controller
	unsigned int slowClock);  // \arg slowClock in Hz


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_CfgMCKReg
//* \brief Cfg Master Clock Register
//*----------------------------------------------------------------------------
__inline void AT91F_PMC_CfgMCKReg (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int mode);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_GetMCKReg
//* \brief Return Master Clock Register
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PMC_GetMCKReg(
	AT91PS_PMC pPMC); // \arg pointer to PMC controller


//*------------------------------------------------------------------------------
//* \fn    AT91F_PMC_GetMasterClock
//* \brief Return master clock in Hz which correponds to processor clock for ARM7
//*------------------------------------------------------------------------------
__inline unsigned int AT91F_PMC_GetMasterClock (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	AT91PS_CKGR pCKGR, // \arg pointer to CKGR controller
	unsigned int slowClock);  // \arg slowClock in Hz


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_EnablePCK
//* \brief Enable Programmable Clock x Output
//*----------------------------------------------------------------------------
__inline void AT91F_PMC_EnablePCK (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int pck,  // \arg Programmable Clock x Output
	unsigned int mode);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_DisablePCK
//* \brief Disable Programmable Clock x Output
//*----------------------------------------------------------------------------
__inline void AT91F_PMC_DisablePCK (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int pck);  // \arg Programmable Clock x Output


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_EnableIt
//* \brief Enable PMC interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_PMC_EnableIt (
	AT91PS_PMC pPMC,     // pointer to a PMC controller
	unsigned int flag);   // IT to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_DisableIt
//* \brief Disable PMC interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_PMC_DisableIt (
	AT91PS_PMC pPMC, // pointer to a PMC controller
	unsigned int flag); // IT to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_GetStatus
//* \brief Return PMC Interrupt Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PMC_GetStatus( // \return PMC Interrupt Status
	AT91PS_PMC pPMC); // pointer to a PMC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_GetInterruptMaskStatus
//* \brief Return PMC Interrupt Mask Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PMC_GetInterruptMaskStatus( // \return PMC Interrupt Mask Status
	AT91PS_PMC pPMC); // pointer to a PMC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_IsInterruptMasked
//* \brief Test if PMC Interrupt is Masked
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PMC_IsInterruptMasked(
        AT91PS_PMC pPMC,   // \arg  pointer to a PMC controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_IsStatusSet
//* \brief Test if PMC Status is Set
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PMC_IsStatusSet(
        AT91PS_PMC pPMC,   // \arg  pointer to a PMC controller
        unsigned int flag); // \arg  flag to be tested


// ----------------------------------------------------------------------------
//  \fn    AT91F_CKGR_CfgPLLReg
//  \brief Cfg the PLL Register
// ----------------------------------------------------------------------------
__inline void AT91F_CKGR_CfgPLLReg (
	AT91PS_CKGR pCKGR, // \arg pointer to CKGR controller
	unsigned int mode);


// ----------------------------------------------------------------------------
//  \fn    AT91F_CKGR_GetPLLReg
//  \brief Get the PLL Register
// ----------------------------------------------------------------------------
__inline unsigned int AT91F_CKGR_GetPLLReg (
	AT91PS_CKGR pCKGR); // \arg pointer to CKGR controller




/* *****************************************************************************
                SOFTWARE API FOR RSTC
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_RSTSoftReset
//* \brief Start Software Reset
//*----------------------------------------------------------------------------
__inline void AT91F_RSTSoftReset(
        AT91PS_RSTC pRSTC,
        unsigned int reset);


//*----------------------------------------------------------------------------
//* \fn    AT91F_RSTSetMode
//* \brief Set Reset Mode
//*----------------------------------------------------------------------------
__inline void AT91F_RSTSetMode(
        AT91PS_RSTC pRSTC,
        unsigned int mode);


//*----------------------------------------------------------------------------
//* \fn    AT91F_RSTGetMode
//* \brief Get Reset Mode
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_RSTGetMode(
        AT91PS_RSTC pRSTC);


//*----------------------------------------------------------------------------
//* \fn    AT91F_RSTGetStatus
//* \brief Get Reset Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_RSTGetStatus(
        AT91PS_RSTC pRSTC);


//*----------------------------------------------------------------------------
//* \fn    AT91F_RSTIsSoftRstActive
//* \brief Return !=0 if software reset is still not completed
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_RSTIsSoftRstActive(
        AT91PS_RSTC pRSTC);


/* *****************************************************************************
                SOFTWARE API FOR RTTC
   ***************************************************************************** */
//*--------------------------------------------------------------------------------------
//* \fn     AT91F_SetRTT_TimeBase()
//* \brief  Set the RTT prescaler according to the TimeBase in ms
//*--------------------------------------------------------------------------------------
__inline unsigned int AT91F_RTTSetTimeBase(
        AT91PS_RTTC pRTTC, 
        unsigned int ms);


//*--------------------------------------------------------------------------------------
//* \fn     AT91F_RTTSetPrescaler()
//* \brief  Set the new prescaler value
//*--------------------------------------------------------------------------------------
__inline unsigned int AT91F_RTTSetPrescaler(
        AT91PS_RTTC pRTTC, 
        unsigned int rtpres);


//*--------------------------------------------------------------------------------------
//* \fn     AT91F_RTTRestart()
//* \brief  Restart the RTT prescaler
//*--------------------------------------------------------------------------------------
__inline void AT91F_RTTRestart(
        AT91PS_RTTC pRTTC);



//*--------------------------------------------------------------------------------------
//* \fn     AT91F_RTT_SetAlarmINT()
//* \brief  Enable RTT Alarm Interrupt
//*--------------------------------------------------------------------------------------
__inline void AT91F_RTTSetAlarmINT(
        AT91PS_RTTC pRTTC);


//*--------------------------------------------------------------------------------------
//* \fn     AT91F_RTT_ClearAlarmINT()
//* \brief  Disable RTT Alarm Interrupt
//*--------------------------------------------------------------------------------------
__inline void AT91F_RTTClearAlarmINT(
        AT91PS_RTTC pRTTC);


//*--------------------------------------------------------------------------------------
//* \fn     AT91F_RTT_SetRttIncINT()
//* \brief  Enable RTT INC Interrupt
//*--------------------------------------------------------------------------------------
__inline void AT91F_RTTSetRttIncINT(
        AT91PS_RTTC pRTTC);


//*--------------------------------------------------------------------------------------
//* \fn     AT91F_RTT_ClearRttIncINT()
//* \brief  Disable RTT INC Interrupt
//*--------------------------------------------------------------------------------------
__inline void AT91F_RTTClearRttIncINT(
        AT91PS_RTTC pRTTC);


//*--------------------------------------------------------------------------------------
//* \fn     AT91F_RTT_SetAlarmValue()
//* \brief  Set RTT Alarm Value
//*--------------------------------------------------------------------------------------
__inline void AT91F_RTTSetAlarmValue(
        AT91PS_RTTC pRTTC, unsigned int alarm);


//*--------------------------------------------------------------------------------------
//* \fn     AT91F_RTT_GetAlarmValue()
//* \brief  Get RTT Alarm Value
//*--------------------------------------------------------------------------------------
__inline unsigned int AT91F_RTTGetAlarmValue(
        AT91PS_RTTC pRTTC);


//*--------------------------------------------------------------------------------------
//* \fn     AT91F_RTTGetStatus()
//* \brief  Read the RTT status
//*--------------------------------------------------------------------------------------
__inline unsigned int AT91F_RTTGetStatus(
        AT91PS_RTTC pRTTC);


//*--------------------------------------------------------------------------------------
//* \fn     AT91F_RTT_ReadValue()
//* \brief  Read the RTT value
//*--------------------------------------------------------------------------------------
__inline unsigned int AT91F_RTTReadValue(
        AT91PS_RTTC pRTTC);


/* *****************************************************************************
                SOFTWARE API FOR PITC
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_PITInit
//* \brief System timer init : period in µsecond, system clock freq in MHz
//*----------------------------------------------------------------------------
__inline void AT91F_PITInit(
        AT91PS_PITC pPITC,
        unsigned int period,
        unsigned int pit_frequency);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PITSetPIV
//* \brief Set the PIT Periodic Interval Value 
//*----------------------------------------------------------------------------
__inline void AT91F_PITSetPIV(
        AT91PS_PITC pPITC,
        unsigned int piv);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PITEnableInt
//* \brief Enable PIT periodic interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_PITEnableInt(
        AT91PS_PITC pPITC);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PITDisableInt
//* \brief Disable PIT periodic interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_PITDisableInt(
        AT91PS_PITC pPITC);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PITGetMode
//* \brief Read PIT mode register
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PITGetMode(
        AT91PS_PITC pPITC);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PITGetStatus
//* \brief Read PIT status register
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PITGetStatus(
        AT91PS_PITC pPITC);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PITGetPIIR
//* \brief Read PIT CPIV and PICNT without ressetting the counters
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PITGetPIIR(
        AT91PS_PITC pPITC);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PITGetPIVR
//* \brief Read System timer CPIV and PICNT without ressetting the counters
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PITGetPIVR(
        AT91PS_PITC pPITC);


/* *****************************************************************************
                SOFTWARE API FOR WDTC
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_WDTSetMode
//* \brief Set Watchdog Mode Register
//*----------------------------------------------------------------------------
__inline void AT91F_WDTSetMode(
        AT91PS_WDTC pWDTC,
        unsigned int Mode);


//*----------------------------------------------------------------------------
//* \fn    AT91F_WDTRestart
//* \brief Restart Watchdog
//*----------------------------------------------------------------------------
__inline void AT91F_WDTRestart(
        AT91PS_WDTC pWDTC);


//*----------------------------------------------------------------------------
//* \fn    AT91F_WDTSGettatus
//* \brief Get Watchdog Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_WDTSGettatus(
        AT91PS_WDTC pWDTC);


//*----------------------------------------------------------------------------
//* \fn    AT91F_WDTGetPeriod
//* \brief Translate ms into Watchdog Compatible value
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_WDTGetPeriod(unsigned int ms);


/* *****************************************************************************
                SOFTWARE API FOR VREG
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_VREG_Enable_LowPowerMode
//* \brief Enable VREG Low Power Mode
//*----------------------------------------------------------------------------
__inline void AT91F_VREG_Enable_LowPowerMode(
        AT91PS_VREG pVREG);


//*----------------------------------------------------------------------------
//* \fn    AT91F_VREG_Disable_LowPowerMode
//* \brief Disable VREG Low Power Mode
//*----------------------------------------------------------------------------
__inline void AT91F_VREG_Disable_LowPowerMode(
        AT91PS_VREG pVREG);


/* *****************************************************************************
                SOFTWARE API FOR MC
   ***************************************************************************** */

#define AT91C_MC_CORRECT_KEY  ((unsigned int) 0x5A << 24) // (MC) Correct Protect Key

//*----------------------------------------------------------------------------
//* \fn    AT91F_MC_Remap
//* \brief Make Remap
//*----------------------------------------------------------------------------
__inline void AT91F_MC_Remap (void);     //  


//*----------------------------------------------------------------------------
//* \fn    AT91F_MC_EFC_CfgModeReg
//* \brief Configure the EFC Mode Register of the MC controller
//*----------------------------------------------------------------------------
__inline void AT91F_MC_EFC_CfgModeReg (
	AT91PS_MC pMC, // pointer to a MC controller
	unsigned int mode);        // mode register 


//*----------------------------------------------------------------------------
//* \fn    AT91F_MC_EFC_GetModeReg
//* \brief Return MC EFC Mode Regsiter
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_MC_EFC_GetModeReg(
	AT91PS_MC pMC); // pointer to a MC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_MC_EFC_ComputeFMCN
//* \brief Return MC EFC Mode Regsiter
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_MC_EFC_ComputeFMCN(
	int master_clock); // master clock in Hz


//*----------------------------------------------------------------------------
//* \fn    AT91F_MC_EFC_PerformCmd
//* \brief Perform EFC Command
//*----------------------------------------------------------------------------
__inline void AT91F_MC_EFC_PerformCmd (
	AT91PS_MC pMC, // pointer to a MC controller
    unsigned int transfer_cmd);


//*----------------------------------------------------------------------------
//* \fn    AT91F_MC_EFC_GetStatus
//* \brief Return MC EFC Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_MC_EFC_GetStatus(
	AT91PS_MC pMC); // pointer to a MC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_MC_EFC_IsInterruptMasked
//* \brief Test if EFC MC Interrupt is Masked 
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_MC_EFC_IsInterruptMasked(
        AT91PS_MC pMC,   // \arg  pointer to a MC controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_MC_EFC_IsInterruptSet
//* \brief Test if EFC MC Interrupt is Set
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_MC_EFC_IsInterruptSet(
        AT91PS_MC pMC,   // \arg  pointer to a MC controller
        unsigned int flag); // \arg  flag to be tested


/* *****************************************************************************
                SOFTWARE API FOR SPI
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_CfgCs
//* \brief Configure SPI chip select register
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_CfgCs (
	AT91PS_SPI pSPI,     // pointer to a SPI controller
	int cs,     // SPI cs number (0 to 3)
 	int val);   //  chip select register


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_EnableIt
//* \brief Enable SPI interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_EnableIt (
	AT91PS_SPI pSPI,     // pointer to a SPI controller
	unsigned int flag);   // IT to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_DisableIt
//* \brief Disable SPI interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_DisableIt (
	AT91PS_SPI pSPI, // pointer to a SPI controller
	unsigned int flag); // IT to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_Reset
//* \brief Reset the SPI controller
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_Reset (
	AT91PS_SPI pSPI // pointer to a SPI controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_Enable
//* \brief Enable the SPI controller
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_Enable (
	AT91PS_SPI pSPI // pointer to a SPI controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_Disable
//* \brief Disable the SPI controller
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_Disable (
	AT91PS_SPI pSPI // pointer to a SPI controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_CfgMode
//* \brief Enable the SPI controller
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_CfgMode (
	AT91PS_SPI pSPI, // pointer to a SPI controller
	int mode);        // mode register 


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_CfgPCS
//* \brief Switch to the correct PCS of SPI Mode Register : Fixed Peripheral Selected
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_CfgPCS (
	AT91PS_SPI pSPI, // pointer to a SPI controller
	char PCS_Device); // PCS of the Device


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_ReceiveFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initializaed with Next Buffer, 0 if PDC is busy
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_SPI_ReceiveFrame (
	AT91PS_SPI pSPI,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer );


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_SendFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initializaed with Next Buffer, 0 if PDC is bSPIy
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_SPI_SendFrame(
	AT91PS_SPI pSPI,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer );


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_Close
//* \brief Close SPI: disable IT disable transfert, close PDC
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_Close (
	AT91PS_SPI pSPI);     // \arg pointer to a SPI controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_PutChar
//* \brief Send a character,does not check if ready to send
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_PutChar (
	AT91PS_SPI pSPI,
	unsigned int character,
             unsigned int cs_number );


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_GetChar
//* \brief Receive a character,does not check if a character is available
//*----------------------------------------------------------------------------
__inline int AT91F_SPI_GetChar (
	const AT91PS_SPI pSPI);


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_GetInterruptMaskStatus
//* \brief Return SPI Interrupt Mask Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_SPI_GetInterruptMaskStatus( // \return SPI Interrupt Mask Status
        AT91PS_SPI pSpi); // \arg  pointer to a SPI controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_IsInterruptMasked
//* \brief Test if SPI Interrupt is Masked 
//*----------------------------------------------------------------------------
__inline int AT91F_SPI_IsInterruptMasked(
        AT91PS_SPI pSpi,   // \arg  pointer to a SPI controller
        unsigned int flag); // \arg  flag to be tested


/* *****************************************************************************
                SOFTWARE API FOR ADC
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_EnableIt
//* \brief Enable ADC interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_ADC_EnableIt (
	AT91PS_ADC pADC,     // pointer to a ADC controller
	unsigned int flag);   // IT to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_DisableIt
//* \brief Disable ADC interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_ADC_DisableIt (
	AT91PS_ADC pADC, // pointer to a ADC controller
	unsigned int flag); // IT to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetStatus
//* \brief Return ADC Interrupt Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetStatus( // \return ADC Interrupt Status
	AT91PS_ADC pADC); // pointer to a ADC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetInterruptMaskStatus
//* \brief Return ADC Interrupt Mask Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetInterruptMaskStatus( // \return ADC Interrupt Mask Status
	AT91PS_ADC pADC); // pointer to a ADC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_IsInterruptMasked
//* \brief Test if ADC Interrupt is Masked 
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_IsInterruptMasked(
        AT91PS_ADC pADC,   // \arg  pointer to a ADC controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_IsStatusSet
//* \brief Test if ADC Status is Set
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_IsStatusSet(
        AT91PS_ADC pADC,   // \arg  pointer to a ADC controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_CfgModeReg
//* \brief Configure the Mode Register of the ADC controller
//*----------------------------------------------------------------------------
__inline void AT91F_ADC_CfgModeReg (
	AT91PS_ADC pADC, // pointer to a ADC controller
	unsigned int mode);        // mode register 


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetModeReg
//* \brief Return the Mode Register of the ADC controller value
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetModeReg (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_CfgTimings
//* \brief Configure the different necessary timings of the ADC controller
//*----------------------------------------------------------------------------
__inline void AT91F_ADC_CfgTimings (
	AT91PS_ADC pADC, // pointer to a ADC controller
	unsigned int mck_clock, // in MHz 
	unsigned int adc_clock, // in MHz 
	unsigned int startup_time, // in us 
	unsigned int sample_and_hold_time);	// in ns  


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_EnableChannel
//* \brief Return ADC Timer Register Value
//*----------------------------------------------------------------------------
__inline void AT91F_ADC_EnableChannel (
	AT91PS_ADC pADC, // pointer to a ADC controller
	unsigned int channel);        // mode register 


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_DisableChannel
//* \brief Return ADC Timer Register Value
//*----------------------------------------------------------------------------
__inline void AT91F_ADC_DisableChannel (
	AT91PS_ADC pADC, // pointer to a ADC controller
	unsigned int channel);        // mode register 


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetChannelStatus
//* \brief Return ADC Timer Register Value
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetChannelStatus (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_StartConversion
//* \brief Software request for a analog to digital conversion 
//*----------------------------------------------------------------------------
__inline void AT91F_ADC_StartConversion (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_SoftReset
//* \brief Software reset
//*----------------------------------------------------------------------------
__inline void AT91F_ADC_SoftReset (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetLastConvertedData
//* \brief Return the Last Converted Data
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetLastConvertedData (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetConvertedDataCH0
//* \brief Return the Channel 0 Converted Data
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetConvertedDataCH0 (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetConvertedDataCH1
//* \brief Return the Channel 1 Converted Data
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetConvertedDataCH1 (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetConvertedDataCH2
//* \brief Return the Channel 2 Converted Data
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetConvertedDataCH2 (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetConvertedDataCH3
//* \brief Return the Channel 3 Converted Data
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetConvertedDataCH3 (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetConvertedDataCH4
//* \brief Return the Channel 4 Converted Data
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetConvertedDataCH4 (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetConvertedDataCH5
//* \brief Return the Channel 5 Converted Data
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetConvertedDataCH5 (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetConvertedDataCH6
//* \brief Return the Channel 6 Converted Data
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetConvertedDataCH6 (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_GetConvertedDataCH7
//* \brief Return the Channel 7 Converted Data
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_ADC_GetConvertedDataCH7 (
	AT91PS_ADC pADC // pointer to a ADC controller
	);


/* *****************************************************************************
                SOFTWARE API FOR SSC
   ***************************************************************************** */
//* Define the standard I2S mode configuration

//* Configuration to set in the SSC Transmit Clock Mode Register
//* Parameters :  nb_bit_by_slot : 8, 16 or 32 bits
//* 			  nb_slot_by_frame : number of channels
#define AT91C_I2S_ASY_MASTER_TX_SETTING(nb_bit_by_slot, nb_slot_by_frame)( +\
									   AT91C_SSC_CKS_DIV   +\
                            		   AT91C_SSC_CKO_CONTINOUS      +\
                            		   AT91C_SSC_CKG_NONE    +\
                                       AT91C_SSC_START_FALL_RF +\
                           			   AT91C_SSC_STTOUT  +\
                            		   ((1<<16) & AT91C_SSC_STTDLY) +\
                            		   ((((nb_bit_by_slot*nb_slot_by_frame)/2)-1) <<24))


//* Configuration to set in the SSC Transmit Frame Mode Register
//* Parameters : nb_bit_by_slot : 8, 16 or 32 bits
//* 			 nb_slot_by_frame : number of channels
#define AT91C_I2S_ASY_TX_FRAME_SETTING(nb_bit_by_slot, nb_slot_by_frame)( +\
									(nb_bit_by_slot-1)  +\
                            		AT91C_SSC_MSBF   +\
                            		(((nb_slot_by_frame-1)<<8) & AT91C_SSC_DATNB)  +\
                            		(((nb_bit_by_slot-1)<<16) & AT91C_SSC_FSLEN) +\
                            		AT91C_SSC_FSOS_NEGATIVE)


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_SetBaudrate
//* \brief Set the baudrate according to the CPU clock
//*----------------------------------------------------------------------------
__inline void AT91F_SSC_SetBaudrate (
        AT91PS_SSC pSSC,        // \arg pointer to a SSC controller
        unsigned int mainClock, // \arg peripheral clock
        unsigned int speed);     // \arg SSC baudrate


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_Configure
//* \brief Configure SSC
//*----------------------------------------------------------------------------
__inline void AT91F_SSC_Configure (
             AT91PS_SSC pSSC,          // \arg pointer to a SSC controller
             unsigned int syst_clock,  // \arg System Clock Frequency
             unsigned int baud_rate,   // \arg Expected Baud Rate Frequency
             unsigned int clock_rx,    // \arg Receiver Clock Parameters
             unsigned int mode_rx,     // \arg mode Register to be programmed
             unsigned int clock_tx,    // \arg Transmitter Clock Parameters
             unsigned int mode_tx);     // \arg mode Register to be programmed


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_EnableRx
//* \brief Enable receiving datas
//*----------------------------------------------------------------------------
__inline void AT91F_SSC_EnableRx (
	AT91PS_SSC pSSC);     // \arg pointer to a SSC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_DisableRx
//* \brief Disable receiving datas
//*----------------------------------------------------------------------------
__inline void AT91F_SSC_DisableRx (
	AT91PS_SSC pSSC);     // \arg pointer to a SSC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_EnableTx
//* \brief Enable sending datas
//*----------------------------------------------------------------------------
__inline void AT91F_SSC_EnableTx (
	AT91PS_SSC pSSC);     // \arg pointer to a SSC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_DisableTx
//* \brief Disable sending datas
//*----------------------------------------------------------------------------
__inline void AT91F_SSC_DisableTx (
	AT91PS_SSC pSSC);     // \arg pointer to a SSC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_EnableIt
//* \brief Enable SSC IT
//*----------------------------------------------------------------------------
__inline void AT91F_SSC_EnableIt (
	AT91PS_SSC pSSC, // \arg pointer to a SSC controller
	unsigned int flag);   // \arg IT to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_DisableIt
//* \brief Disable SSC IT
//*----------------------------------------------------------------------------
__inline void AT91F_SSC_DisableIt (
	AT91PS_SSC pSSC, // \arg pointer to a SSC controller
	unsigned int flag);   // \arg IT to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_ReceiveFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initialized with Next Buffer, 0 if PDC is busy
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_SSC_ReceiveFrame (
	AT91PS_SSC pSSC,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer );


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_SendFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initialized with Next Buffer, 0 if PDC is busy
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_SSC_SendFrame(
	AT91PS_SSC pSSC,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer );


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_GetInterruptMaskStatus
//* \brief Return SSC Interrupt Mask Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_SSC_GetInterruptMaskStatus( // \return SSC Interrupt Mask Status
        AT91PS_SSC pSsc); // \arg  pointer to a SSC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_IsInterruptMasked
//* \brief Test if SSC Interrupt is Masked 
//*----------------------------------------------------------------------------
__inline int AT91F_SSC_IsInterruptMasked(
        AT91PS_SSC pSsc,   // \arg  pointer to a SSC controller
        unsigned int flag); // \arg  flag to be tested


/* *****************************************************************************
                SOFTWARE API FOR USART
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Baudrate
//* \brief Calculate the baudrate
//* Standard Asynchronous Mode : 8 bits , 1 stop , no parity
#define AT91C_US_ASYNC_MODE ( AT91C_US_USMODE_NORMAL + \
                        AT91C_US_NBSTOP_1_BIT + \
                        AT91C_US_PAR_NONE + \
                        AT91C_US_CHRL_8_BITS + \
                        AT91C_US_CLKS_CLOCK )

//* Standard External Asynchronous Mode : 8 bits , 1 stop , no parity
#define AT91C_US_ASYNC_SCK_MODE ( AT91C_US_USMODE_NORMAL + \
                            AT91C_US_NBSTOP_1_BIT + \
                            AT91C_US_PAR_NONE + \
                            AT91C_US_CHRL_8_BITS + \
                            AT91C_US_CLKS_EXT )

//* Standard Synchronous Mode : 8 bits , 1 stop , no parity
#define AT91C_US_SYNC_MODE ( AT91C_US_SYNC + \
                       AT91C_US_USMODE_NORMAL + \
                       AT91C_US_NBSTOP_1_BIT + \
                       AT91C_US_PAR_NONE + \
                       AT91C_US_CHRL_8_BITS + \
                       AT91C_US_CLKS_CLOCK )

//* SCK used Label
#define AT91C_US_SCK_USED (AT91C_US_CKLO | AT91C_US_CLKS_EXT)

//* Standard ISO T=0 Mode : 8 bits , 1 stop , parity
#define AT91C_US_ISO_READER_MODE ( AT91C_US_USMODE_ISO7816_0 + \
					   		 AT91C_US_CLKS_CLOCK +\
                       		 AT91C_US_NBSTOP_1_BIT + \
                       		 AT91C_US_PAR_EVEN + \
                       		 AT91C_US_CHRL_8_BITS + \
                       		 AT91C_US_CKLO +\
                       		 AT91C_US_OVER)

//* Standard IRDA mode
#define AT91C_US_ASYNC_IRDA_MODE (  AT91C_US_USMODE_IRDA + \
                            AT91C_US_NBSTOP_1_BIT + \
                            AT91C_US_PAR_NONE + \
                            AT91C_US_CHRL_8_BITS + \
                            AT91C_US_CLKS_CLOCK )

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Baudrate
//* \brief Caluculate baud_value according to the main clock and the baud rate
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_US_Baudrate (
	const unsigned int main_clock, // \arg peripheral clock
	const unsigned int baud_rate);  // \arg UART baudrate


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_SetBaudrate
//* \brief Set the baudrate according to the CPU clock
//*----------------------------------------------------------------------------
__inline void AT91F_US_SetBaudrate (
	AT91PS_USART pUSART,    // \arg pointer to a USART controller
	unsigned int mainClock, // \arg peripheral clock
	unsigned int speed);     // \arg UART baudrate


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_SetTimeguard
//* \brief Set USART timeguard
//*----------------------------------------------------------------------------
__inline void AT91F_US_SetTimeguard (
	AT91PS_USART pUSART,    // \arg pointer to a USART controller
	unsigned int timeguard); // \arg timeguard value


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_EnableIt
//* \brief Enable USART IT
//*----------------------------------------------------------------------------
__inline void AT91F_US_EnableIt (
	AT91PS_USART pUSART, // \arg pointer to a USART controller
	unsigned int flag);   // \arg IT to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_DisableIt
//* \brief Disable USART IT
//*----------------------------------------------------------------------------
__inline void AT91F_US_DisableIt (
	AT91PS_USART pUSART, // \arg pointer to a USART controller
	unsigned int flag);   // \arg IT to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Configure
//* \brief Configure USART
//*----------------------------------------------------------------------------
__inline void AT91F_US_Configure (
	AT91PS_USART pUSART,     // \arg pointer to a USART controller
	unsigned int mainClock,  // \arg peripheral clock
	unsigned int mode ,      // \arg mode Register to be programmed
	unsigned int baudRate ,  // \arg baudrate to be programmed
	unsigned int timeguard ); // \arg timeguard to be programmed


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_EnableRx
//* \brief Enable receiving characters
//*----------------------------------------------------------------------------
__inline void AT91F_US_EnableRx (
	AT91PS_USART pUSART);     // \arg pointer to a USART controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_EnableTx
//* \brief Enable sending characters
//*----------------------------------------------------------------------------
__inline void AT91F_US_EnableTx (
	AT91PS_USART pUSART);     // \arg pointer to a USART controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_ResetRx
//* \brief Reset Receiver and re-enable it
//*----------------------------------------------------------------------------
__inline void AT91F_US_ResetRx (
	AT91PS_USART pUSART);     // \arg pointer to a USART controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_ResetTx
//* \brief Reset Transmitter and re-enable it
//*----------------------------------------------------------------------------
__inline void AT91F_US_ResetTx (
	AT91PS_USART pUSART);     // \arg pointer to a USART controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_DisableRx
//* \brief Disable Receiver
//*----------------------------------------------------------------------------
__inline void AT91F_US_DisableRx (
	AT91PS_USART pUSART);     // \arg pointer to a USART controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_DisableTx
//* \brief Disable Transmitter
//*----------------------------------------------------------------------------
__inline void AT91F_US_DisableTx (
	AT91PS_USART pUSART);     // \arg pointer to a USART controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Close
//* \brief Close USART: disable IT disable receiver and transmitter, close PDC
//*----------------------------------------------------------------------------
__inline void AT91F_US_Close (
	AT91PS_USART pUSART);     // \arg pointer to a USART controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_TxReady
//* \brief Return 1 if a character can be written in US_THR
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_US_TxReady (
	AT91PS_USART pUSART );     // \arg pointer to a USART controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_RxReady
//* \brief Return 1 if a character can be read in US_RHR
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_US_RxReady (
	AT91PS_USART pUSART );     // \arg pointer to a USART controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Error
//* \brief Return the error flag
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_US_Error (
	AT91PS_USART pUSART );     // \arg pointer to a USART controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_PutChar
//* \brief Send a character,does not check if ready to send
//*----------------------------------------------------------------------------
__inline void AT91F_US_PutChar (
	AT91PS_USART pUSART,
	int character );


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_GetChar
//* \brief Receive a character,does not check if a character is available
//*----------------------------------------------------------------------------
__inline int AT91F_US_GetChar (
	const AT91PS_USART pUSART);


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_SendFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initializaed with Next Buffer, 0 if PDC is busy
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_US_SendFrame(
	AT91PS_USART pUSART,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer );


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_ReceiveFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initializaed with Next Buffer, 0 if PDC is busy
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_US_ReceiveFrame (
	AT91PS_USART pUSART,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer );


//*----------------------------------------------------------------------------
//* \fn    AT91F_US_SetIrdaFilter
//* \brief Set the value of IrDa filter tregister
//*----------------------------------------------------------------------------
__inline void AT91F_US_SetIrdaFilter (
	AT91PS_USART pUSART,
	unsigned char value
);


/* *****************************************************************************
                SOFTWARE API FOR TWI
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_EnableIt
//* \brief Enable TWI IT
//*----------------------------------------------------------------------------
__inline void AT91F_TWI_EnableIt (
	AT91PS_TWI pTWI, // \arg pointer to a TWI controller
	unsigned int flag);   // \arg IT to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_DisableIt
//* \brief Disable TWI IT
//*----------------------------------------------------------------------------
__inline void AT91F_TWI_DisableIt (
	AT91PS_TWI pTWI, // \arg pointer to a TWI controller
	unsigned int flag);   // \arg IT to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_Configure
//* \brief Configure TWI in master mode
//*----------------------------------------------------------------------------
__inline void AT91F_TWI_Configure ( AT91PS_TWI pTWI );          // \arg pointer to a TWI controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_GetInterruptMaskStatus
//* \brief Return TWI Interrupt Mask Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_TWI_GetInterruptMaskStatus( // \return TWI Interrupt Mask Status
        AT91PS_TWI pTwi); // \arg  pointer to a TWI controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_IsInterruptMasked
//* \brief Test if TWI Interrupt is Masked 
//*----------------------------------------------------------------------------
__inline int AT91F_TWI_IsInterruptMasked(
        AT91PS_TWI pTwi,   // \arg  pointer to a TWI controller
        unsigned int flag); // \arg  flag to be tested


/* *****************************************************************************
                SOFTWARE API FOR TC
   ***************************************************************************** */
//*------------------------- Internal Function --------------------------------
//*----------------------------------------------------------------------------
//* Function Name       : AT91F_TC_Open
//* Object              : Initialize Timer Counter Channel and enable is clock
//* Input Parameters    : <tc_pt> = TC Channel Descriptor Pointer
//*                       <mode> = Timer Counter Mode
//*                     : <TimerId> = Timer peripheral ID definitions
//* Output Parameters   : None
//*----------------------------------------------------------------------------

__inline void AT91F_TC_Open ( AT91PS_TC TC_pt, unsigned int Mode, unsigned int TimerId);



//*----------------------------------------------------------------------------
//* \fn    AT91F_TC_InterruptEnable
//* \brief Enable TC Interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_TC_InterruptEnable(
        AT91PS_TC pTc,   // \arg  pointer to a TC controller
        unsigned int flag); // \arg  TC interrupt to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_TC_InterruptDisable
//* \brief Disable TC Interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_TC_InterruptDisable(
        AT91PS_TC pTc,   // \arg  pointer to a TC controller
        unsigned int flag); // \arg  TC interrupt to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_TC_GetInterruptMaskStatus
//* \brief Return TC Interrupt Mask Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_TC_GetInterruptMaskStatus( // \return TC Interrupt Mask Status
        AT91PS_TC pTc); // \arg  pointer to a TC controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_TC_IsInterruptMasked
//* \brief Test if TC Interrupt is Masked 
//*----------------------------------------------------------------------------
__inline int AT91F_TC_IsInterruptMasked(
        AT91PS_TC pTc,   // \arg  pointer to a TC controller
        unsigned int flag); // \arg  flag to be tested


/* *****************************************************************************
                SOFTWARE API FOR PWMC
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_PWM_GetStatus
//* \brief Return PWM Interrupt Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PWMC_GetStatus( // \return PWM Interrupt Status
	AT91PS_PWMC pPWM); // pointer to a PWM controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWM_InterruptEnable
//* \brief Enable PWM Interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_PWMC_InterruptEnable(
        AT91PS_PWMC pPwm,   // \arg  pointer to a PWM controller
        unsigned int flag); // \arg  PWM interrupt to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWM_InterruptDisable
//* \brief Disable PWM Interrupt
//*----------------------------------------------------------------------------
__inline void AT91F_PWMC_InterruptDisable(
        AT91PS_PWMC pPwm,   // \arg  pointer to a PWM controller
        unsigned int flag); // \arg  PWM interrupt to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWM_GetInterruptMaskStatus
//* \brief Return PWM Interrupt Mask Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PWMC_GetInterruptMaskStatus( // \return PWM Interrupt Mask Status
        AT91PS_PWMC pPwm); // \arg  pointer to a PWM controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWM_IsInterruptMasked
//* \brief Test if PWM Interrupt is Masked
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PWMC_IsInterruptMasked(
        AT91PS_PWMC pPWM,   // \arg  pointer to a PWM controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWM_IsStatusSet
//* \brief Test if PWM Interrupt is Set
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_PWMC_IsStatusSet(
        AT91PS_PWMC pPWM,   // \arg  pointer to a PWM controller
        unsigned int flag); // \arg  flag to be tested


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWM_CfgChannel
//* \brief Test if PWM Interrupt is Set
//*----------------------------------------------------------------------------
__inline void AT91F_PWMC_CfgChannel(
        AT91PS_PWMC pPWM,   // \arg  pointer to a PWM controller
        unsigned int channelId, // \arg PWM channel ID
        unsigned int mode, // \arg  PWM mode
        unsigned int period, // \arg PWM period
        unsigned int duty); // \arg PWM duty cycle


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWM_StartChannel
//* \brief Enable channel
//*----------------------------------------------------------------------------
__inline void AT91F_PWMC_StartChannel(
        AT91PS_PWMC pPWM,   // \arg  pointer to a PWM controller
        unsigned int flag); // \arg  Channels IDs to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWM_StopChannel
//* \brief Disable channel
//*----------------------------------------------------------------------------
__inline void AT91F_PWMC_StopChannel(
        AT91PS_PWMC pPWM,   // \arg  pointer to a PWM controller
        unsigned int flag); // \arg  Channels IDs to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWM_UpdateChannel
//* \brief Update Period or Duty Cycle
//*----------------------------------------------------------------------------
__inline void AT91F_PWMC_UpdateChannel(
        AT91PS_PWMC pPWM,   // \arg  pointer to a PWM controller
        unsigned int channelId, // \arg PWM channel ID
        unsigned int update); // \arg  Channels IDs to be enabled


/* *****************************************************************************
                SOFTWARE API FOR UDP
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EnableIt
//* \brief Enable UDP IT
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_EnableIt (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned int flag);   // \arg IT to be enabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_DisableIt
//* \brief Disable UDP IT
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_DisableIt (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned int flag);   // \arg IT to be disabled


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_SetAddress
//* \brief Set UDP functional address
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_SetAddress (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned char address);   // \arg new UDP address


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EnableEp
//* \brief Enable Endpoint
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_EnableEp (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned char endpoint);   // \arg endpoint number


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_DisableEp
//* \brief Enable Endpoint
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_DisableEp (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned char endpoint);   // \arg endpoint number


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_SetState
//* \brief Set UDP Device state
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_SetState (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned int flag);   // \arg new UDP address


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_GetState
//* \brief return UDP Device state
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_UDP_GetState ( // \return the UDP device state
	AT91PS_UDP pUDP);     // \arg pointer to a UDP controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_ResetEp
//* \brief Reset UDP endpoint
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_ResetEp ( // \return the UDP device state
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned int flag);   // \arg Endpoints to be reset


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpStall
//* \brief Endpoint will STALL requests
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_EpStall(
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned char endpoint);   // \arg endpoint number


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpWrite
//* \brief Write value in the DPR
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_EpWrite(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint,  // \arg endpoint number
	unsigned char value);     // \arg value to be written in the DPR


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpRead
//* \brief Return value from the DPR
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_UDP_EpRead(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint);  // \arg endpoint number


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpEndOfWr
//* \brief Notify the UDP that values in DPR are ready to be sent
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_EpEndOfWr(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint);  // \arg endpoint number


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpClear
//* \brief Clear flag in the endpoint CSR register
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_EpClear(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint,  // \arg endpoint number
	unsigned int flag);       // \arg flag to be cleared


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpSet
//* \brief Set flag in the endpoint CSR register
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_EpSet(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint,  // \arg endpoint number
	unsigned int flag);       // \arg flag to be cleared


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpStatus
//* \brief Return the endpoint CSR register
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_UDP_EpStatus(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint);  // \arg endpoint number


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_GetInterruptMaskStatus
//* \brief Return UDP Interrupt Mask Status
//*----------------------------------------------------------------------------
__inline unsigned int AT91F_UDP_GetInterruptMaskStatus(
  AT91PS_UDP pUdp);        // \arg  pointer to a UDP controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_IsInterruptMasked
//* \brief Test if UDP Interrupt is Masked 
//*----------------------------------------------------------------------------
__inline int AT91F_UDP_IsInterruptMasked(
  AT91PS_UDP pUdp,       // \arg  pointer to a UDP controller
  unsigned int flag);     // \arg  flag to be tested


// ----------------------------------------------------------------------------
//  \fn    AT91F_UDP_InterruptStatusRegister
//  \brief Return the Interrupt Status Register
// ----------------------------------------------------------------------------
__inline unsigned int AT91F_UDP_InterruptStatusRegister( 
  AT91PS_UDP pUDP );      // \arg  pointer to a UDP controller


// ----------------------------------------------------------------------------
//  \fn    AT91F_UDP_InterruptClearRegister
//  \brief Clear Interrupt Register
// ----------------------------------------------------------------------------
__inline void AT91F_UDP_InterruptClearRegister (
  AT91PS_UDP pUDP,       // \arg pointer to UDP controller
  unsigned int flag);     // \arg IT to be cleat


// ----------------------------------------------------------------------------
//  \fn    AT91F_UDP_EnableTransceiver
//  \brief Enable transceiver
// ----------------------------------------------------------------------------
__inline void AT91F_UDP_EnableTransceiver( 
  AT91PS_UDP pUDP );      // \arg  pointer to a UDP controller


// ----------------------------------------------------------------------------
//  \fn    AT91F_UDP_DisableTransceiver
//  \brief Disable transceiver
// ----------------------------------------------------------------------------
__inline void AT91F_UDP_DisableTransceiver( 
  AT91PS_UDP pUDP );      // \arg  pointer to a UDP controller


//*----------------------------------------------------------------------------
//* \fn    AT91F_MC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  MC
//*----------------------------------------------------------------------------
__inline void AT91F_MC_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_CfgPMC
//* \brief Enable Peripheral clock in PMC for  DBGU
//*----------------------------------------------------------------------------
__inline void AT91F_DBGU_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_CfgPIO
//* \brief Configure PIO controllers to drive DBGU signals
//*----------------------------------------------------------------------------
__inline void AT91F_DBGU_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWMC_CH3_CfgPIO
//* \brief Configure PIO controllers to drive PWMC_CH3 signals
//*----------------------------------------------------------------------------
__inline void AT91F_PWMC_CH3_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWMC_CH2_CfgPIO
//* \brief Configure PIO controllers to drive PWMC_CH2 signals
//*----------------------------------------------------------------------------
__inline void AT91F_PWMC_CH2_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWMC_CH1_CfgPIO
//* \brief Configure PIO controllers to drive PWMC_CH1 signals
//*----------------------------------------------------------------------------
__inline void AT91F_PWMC_CH1_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWMC_CH0_CfgPIO
//* \brief Configure PIO controllers to drive PWMC_CH0 signals
//*----------------------------------------------------------------------------
__inline void AT91F_PWMC_CH0_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_VREG_CfgPMC
//* \brief Enable Peripheral clock in PMC for  VREG
//*----------------------------------------------------------------------------
__inline void AT91F_VREG_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  SSC
//*----------------------------------------------------------------------------
__inline void AT91F_SSC_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_CfgPIO
//* \brief Configure PIO controllers to drive SSC signals
//*----------------------------------------------------------------------------
__inline void AT91F_SSC_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_CfgPMC
//* \brief Enable Peripheral clock in PMC for  SPI
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_CfgPIO
//* \brief Configure PIO controllers to drive SPI signals
//*----------------------------------------------------------------------------
__inline void AT91F_SPI_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PWMC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  PWMC
//*----------------------------------------------------------------------------
__inline void AT91F_PWMC_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_TC0_CfgPMC
//* \brief Enable Peripheral clock in PMC for  TC0
//*----------------------------------------------------------------------------
__inline void AT91F_TC0_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_TC0_CfgPIO
//* \brief Configure PIO controllers to drive TC0 signals
//*----------------------------------------------------------------------------
__inline void AT91F_TC0_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_TC1_CfgPMC
//* \brief Enable Peripheral clock in PMC for  TC1
//*----------------------------------------------------------------------------
__inline void AT91F_TC1_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_TC1_CfgPIO
//* \brief Configure PIO controllers to drive TC1 signals
//*----------------------------------------------------------------------------
__inline void AT91F_TC1_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_TC2_CfgPMC
//* \brief Enable Peripheral clock in PMC for  TC2
//*----------------------------------------------------------------------------
__inline void AT91F_TC2_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_TC2_CfgPIO
//* \brief Configure PIO controllers to drive TC2 signals
//*----------------------------------------------------------------------------
__inline void AT91F_TC2_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PITC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  PITC
//*----------------------------------------------------------------------------
__inline void AT91F_PITC_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  PMC
//*----------------------------------------------------------------------------
__inline void AT91F_PMC_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_CfgPIO
//* \brief Configure PIO controllers to drive PMC signals
//*----------------------------------------------------------------------------
__inline void AT91F_PMC_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  ADC
//*----------------------------------------------------------------------------
__inline void AT91F_ADC_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_ADC_CfgPIO
//* \brief Configure PIO controllers to drive ADC signals
//*----------------------------------------------------------------------------
__inline void AT91F_ADC_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_RSTC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  RSTC
//*----------------------------------------------------------------------------
__inline void AT91F_RSTC_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_RTTC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  RTTC
//*----------------------------------------------------------------------------
__inline void AT91F_RTTC_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIOA_CfgPMC
//* \brief Enable Peripheral clock in PMC for  PIOA
//*----------------------------------------------------------------------------
__inline void AT91F_PIOA_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_CfgPMC
//* \brief Enable Peripheral clock in PMC for  TWI
//*----------------------------------------------------------------------------
__inline void AT91F_TWI_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_CfgPIO
//* \brief Configure PIO controllers to drive TWI signals
//*----------------------------------------------------------------------------
__inline void AT91F_TWI_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_WDTC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  WDTC
//*----------------------------------------------------------------------------
__inline void AT91F_WDTC_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_US1_CfgPMC
//* \brief Enable Peripheral clock in PMC for  US1
//*----------------------------------------------------------------------------
__inline void AT91F_US1_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_US1_CfgPIO
//* \brief Configure PIO controllers to drive US1 signals
//*----------------------------------------------------------------------------
__inline void AT91F_US1_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_US0_CfgPMC
//* \brief Enable Peripheral clock in PMC for  US0
//*----------------------------------------------------------------------------
__inline void AT91F_US0_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_US0_CfgPIO
//* \brief Configure PIO controllers to drive US0 signals
//*----------------------------------------------------------------------------
__inline void AT91F_US0_CfgPIO (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_CfgPMC
//* \brief Enable Peripheral clock in PMC for  UDP
//*----------------------------------------------------------------------------
__inline void AT91F_UDP_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  AIC
//*----------------------------------------------------------------------------
__inline void AT91F_AIC_CfgPMC (void);


//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_CfgPIO
//* \brief Configure PIO controllers to drive AIC signals
//*----------------------------------------------------------------------------
__inline void AT91F_AIC_CfgPIO (void);

#endif // lib_AT91SAM7S256_H
