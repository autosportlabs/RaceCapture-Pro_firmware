/*-----------------------------------------------------------------------------
* Copyright (C) 2010 ARM Limited. All rights reserved.
*
* $Date:        15. July 2011
* $Revision: 	V1.0.10
*
* Project: 	    CMSIS DSP Library
* Title:        arm_iir_lattice_init_q31.c
*
* Description:  Initialization function for the Q31 IIR lattice filter.
*
* Target Processor: Cortex-M4/Cortex-M3/Cortex-M0
*
* Version 1.0.10 2011/7/15
*    Big Endian support added and Merged M0 and M3/M4 Source code.
*
* Version 1.0.3 2010/11/29
*    Re-organized the CMSIS folders and updated documentation.
*
* Version 1.0.2 2010/11/11
*    Documentation updated.
*
* Version 1.0.1 2010/10/05
*    Production release and review comments incorporated.
*
* Version 1.0.0 2010/09/20
*    Production release and review comments incorporated
*
* Version 0.0.7  2010/06/10
*    Misra-C changes done
* ---------------------------------------------------------------------------*/

#include "arm_math.h"

/**
 * @ingroup groupFilters
 */

/**
 * @addtogroup IIR_Lattice
 * @{
 */

/**
 * @brief Initialization function for the Q31 IIR lattice filter.
 * @param[in] *S points to an instance of the Q31 IIR lattice structure.
 * @param[in] numStages number of stages in the filter.
 * @param[in] *pkCoeffs points to the reflection coefficient buffer.  The array is of length numStages.
 * @param[in] *pvCoeffs points to the ladder coefficient buffer.  The array is of length numStages+1.
 * @param[in] *pState points to the state buffer.  The array is of length numStages+blockSize.
 * @param[in] blockSize number of samples to process.
 * @return none.
 */

void arm_iir_lattice_init_q31(
        arm_iir_lattice_instance_q31 * S,
        uint16_t numStages,
        q31_t * pkCoeffs,
        q31_t * pvCoeffs,
        q31_t * pState,
        uint32_t blockSize)
{
        /* Assign filter taps */
        S->numStages = numStages;

        /* Assign reflection coefficient pointer */
        S->pkCoeffs = pkCoeffs;

        /* Assign ladder coefficient pointer */
        S->pvCoeffs = pvCoeffs;

        /* Clear state buffer and size is always blockSize + numStages */
        memset(pState, 0, (numStages + blockSize) * sizeof(q31_t));

        /* Assign state pointer */
        S->pState = pState;


}

/**
 * @} end of IIR_Lattice group
 */
