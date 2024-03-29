/* ----------------------------------------------------------------------
* Copyright (C) 2010 ARM Limited. All rights reserved.
*
* $Date:        15. July 2011
* $Revision: 	V1.0.10
*
* Project: 	    CMSIS DSP Library
* Title:		arm_min_q7.c
*
* Description:	Minimum value of a Q7 vector.
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
*    Production release and review comments incorporated.
* ---------------------------------------------------------------------------- */

#include "arm_math.h"

/**
 * @ingroup groupStats
 */

/**
 * @addtogroup Min
 * @{
 */


/**
 * @brief Minimum value of a Q7 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult minimum value returned here
 * @param[out]      *pIndex index of minimum value returned here
 * @return none.
 *
 */

void arm_min_q7(
        q7_t * pSrc,
        uint32_t blockSize,
        q7_t * pResult,
        uint32_t * pIndex)
{

#ifndef ARM_MATH_CM0

        /* Run the below code for Cortex-M4 and Cortex-M3 */

        q7_t minVal, minVal1, minVal2, res, x0, x1;    /* Temporary variables to store the output value. */
        uint32_t blkCnt, indx, index1, index2, index3, indxMod;       /* loop counter */

        /* Initialise the index value to zero. */
        indx = 0u;

        /* Load first input value that act as reference value for comparision */
        res = *pSrc++;

        /* Loop over blockSize number of values */
        blkCnt = (blockSize - 1u) >> 2u;

        while(blkCnt > 0u) {
                indxMod = blockSize - (blkCnt * 4u);

                /* Load two input values for comparision */
                x0 = *pSrc++;
                x1 = *pSrc++;

                if(x0 > x1) {
                        /* Update the minimum value and its index */
                        minVal1 = x1;
                        index1 = indxMod + 1u;
                } else {
                        /* Update the minimum value and its index */
                        minVal1 = x0;
                        index1 = indxMod;
                }

                /* Load two input values for comparision */
                x0 = *pSrc++;
                x1 = *pSrc++;

                if(x0 > x1) {
                        /* Update the minimum value and its index */
                        minVal2 = x1;
                        index2 = indxMod + 3u;
                } else {
                        /* Update the minimum value and its index */
                        minVal2 = x0;
                        index2 = indxMod + 2u;
                }

                if(minVal1 > minVal2) {
                        /* Update the minimum value and its index */
                        minVal = minVal2;
                        index3 = index2;
                } else {
                        /* Update the minimum value and its index */
                        minVal = minVal1;
                        index3 = index1;
                }

                if(res > minVal) {
                        /* Update the minimum value and its index */
                        res = minVal;
                        indx = index3;
                }

                /* Decrement the loop counter */
                blkCnt--;

        }

        blkCnt = (blockSize - 1u) % 0x04u;

        while(blkCnt > 0u) {
                /* Initialize minVal to the next consecutive values one by one */
                minVal = *pSrc++;

                /* compare for the minimum value */
                if(res > minVal) {
                        /* Update the minimum value and its index */
                        res = minVal;
                        indx = blockSize - blkCnt;
                }

                /* Decrement the loop counter */
                blkCnt--;
        }

        /* Store the minimum value and its index into destination pointers */
        *pResult = res;
        *pIndex = indx;

#else

        /* Run the below code for Cortex-M0 */

        q7_t minVal, out;                              /* Temporary variables to store the output value. */
        uint32_t blkCnt, outIndex;                     /* loop counter */

        /* Initialise the index value to zero. */
        outIndex = 0u;

        /* Load first input value that act as reference value for comparision */
        out = *pSrc++;

        /* Loop over blockSize - 1 number of values */
        blkCnt = (blockSize - 1u);

        while(blkCnt > 0u) {
                /* Initialize minVal to the next consecutive values one by one */
                minVal = *pSrc++;

                /* compare for the minimum value */
                if(out > minVal) {
                        /* Update the minimum value and its index */
                        out = minVal;
                        outIndex = blockSize - blkCnt;
                }

                /* Decrement the loop counter */
                blkCnt--;
        }

        /* Store the minimum value and its index into destination pointers */
        *pResult = out;
        *pIndex = outIndex;

#endif /* #ifndef ARM_MATH_CM0 */

}

/**
 * @} end of Min group
 */
