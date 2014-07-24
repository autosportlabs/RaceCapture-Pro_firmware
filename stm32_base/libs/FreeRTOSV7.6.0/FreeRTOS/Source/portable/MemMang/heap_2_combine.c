/*
    FreeRTOS V6.0.5 - Copyright (C) 2010 Real Time Engineers Ltd.
    modified by Antonio Barbalace (C) 2010 - firstfit search and coaelescing
*/

/*
 * An enhancement to heap_2.c that attempts simple coalescing of adjacent blocks.
 */
#include <stdlib.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"
#include "mod_string.h"

/* Setup the correct byte alignment mask for the defined byte alignment. */
#if portBYTE_ALIGNMENT == 8
	#define heapBYTE_ALIGNMENT_MASK	( ( size_t ) 0x0007 )
#endif

#if portBYTE_ALIGNMENT == 4
	#define heapBYTE_ALIGNMENT_MASK	( ( size_t ) 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
	#define heapBYTE_ALIGNMENT_MASK	( ( size_t ) 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1
	#define heapBYTE_ALIGNMENT_MASK	( ( size_t ) 0x0000 )
#endif

#ifndef heapBYTE_ALIGNMENT_MASK
	#error "Invalid portBYTE_ALIGNMENT definition"
#endif

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/* the following undef statement let you use the original order by size of the
 * malloc blocks in the linked list */
#undef USE_ORDER_BY_SIZE

//the following is defined in the linker script
extern unsigned int _CONFIG_HEAP_SIZE;
#define configTOTAL_HEAP_SIZE ((unsigned int)(&_CONFIG_HEAP_SIZE))

//the following is defined in the linker script
extern unsigned portCHAR  _heap_address[];

/* Allocate the memory for the heap.  The struct is used to force byte
alignment without using any non-portable code. */
static union xRTOS_HEAP
{
	#if portBYTE_ALIGNMENT == 8
		volatile portDOUBLE dDummy;
	#else
		volatile unsigned long ulDummy;
	#endif
	unsigned char *ucHeap;
} xHeap;

/* Define the linked list structure.  This is used to link free blocks in order
of their size. */
typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} xBlockLink;


static const unsigned short  heapSTRUCT_SIZE	= ( sizeof( xBlockLink ) + portBYTE_ALIGNMENT - ( sizeof( xBlockLink ) % portBYTE_ALIGNMENT ) );
#define heapMINIMUM_BLOCK_SIZE	( ( size_t ) ( heapSTRUCT_SIZE * 2 ) )

/* Create a couple of list links to mark the start and end of the list. */
static xBlockLink xStart, xEnd;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining = configTOTAL_HEAP_SIZE;

/* STATIC FUNCTIONS ARE DEFINED AS MACROS TO MINIMIZE THE FUNCTION CALL DEPTH. */

/*
 * Insert a block into the list of free blocks - which is ordered by size of
 * the block.  Small blocks at the start of the list and large blocks at the end
 * of the list.
 */
#define prvInsertBlockIntoFreeList( pxBlockToInsert )								\
{																					\
xBlockLink *pxIterator;																\
size_t xBlockSize;																	\
																					\
	xBlockSize = pxBlockToInsert->xBlockSize;										\
																					\
	/* Iterate through the list until a block is found that has a larger size */	\
	/* than the block we are inserting. */											\
	for( pxIterator = &xStart; pxIterator->pxNextFreeBlock->xBlockSize < xBlockSize; pxIterator = pxIterator->pxNextFreeBlock )	\
	{																				\
		/* There is nothing to do here - just iterate to the correct position. */	\
	}																				\
																					\
	/* Update the list to include the block being inserted in the correct */		\
	/* position. */																	\
	pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;					\
	pxIterator->pxNextFreeBlock = pxBlockToInsert;									\
}
/*-----------------------------------------------------------*/

/*
 * This order enable first fit search on the list 
 * the list is organized by address
 */
#define prvInsertBlockIntoFreeListFF( pxBlockToInsert )								\
{																					\
xBlockLink *pxBlock, *pxPreviousBlock;												\
																					\
	/* Iterate through the list until the correct position is found, ordered by */	\
	/* address, the first it fit is good */											\
	pxPreviousBlock = &xStart;														\
	pxBlock = xStart.pxNextFreeBlock;												\
	/* in this case there are no blocks in the list no coalescence is needed */		\
	if (pxBlock == &xEnd) {															\
		pxPreviousBlock->pxNextFreeBlock = pxBlockToInsert;							\
		pxBlockToInsert->pxNextFreeBlock = pxBlock;									\
	}																				\
	else {																			\
		/* First of all find the correct position */								\
		while( (pxBlock < pxBlockToInsert) && (pxBlock->pxNextFreeBlock) ) {		\
			pxPreviousBlock = pxBlock;												\
			pxBlock = pxBlock->pxNextFreeBlock;										\
		}																			\
		/* link the block in the list */											\
		pxPreviousBlock->pxNextFreeBlock = pxBlockToInsert;							\
		pxBlockToInsert->pxNextFreeBlock = pxBlock;									\
		/* try to COALESCE with the previous (if it is not the xStart) */			\
		if (pxPreviousBlock != &xStart)												\
			if ( ((char*)pxPreviousBlock + (pxPreviousBlock->xBlockSize)) == ((char*) pxBlockToInsert) ) {	\
				pxPreviousBlock->xBlockSize += pxBlockToInsert->xBlockSize;			\
				pxPreviousBlock->pxNextFreeBlock = pxBlockToInsert->pxNextFreeBlock;	\
				pxBlockToInsert = pxPreviousBlock;									\
			}																		\
		/* try to COALESCE with the next (if it is not the xEnd) */					\
		if (pxBlock != &xEnd)														\
			if ( ((char*)pxBlockToInsert + (pxBlockToInsert->xBlockSize)) == ((char*) pxBlock) ) {	\
				pxBlockToInsert->xBlockSize += pxBlock->xBlockSize;					\
				pxBlockToInsert->pxNextFreeBlock = pxBlock->pxNextFreeBlock;		\
			}																		\
	}																				\
}
/*-----------------------------------------------------------*/

#define prvHeapInit()																\
{																					\
	xBlockLink *pxFirstFreeBlock;													\
	xHeap.ucHeap =  _heap_address;													\
																					\
	/* xStart is used to hold a pointer to the first item in the list of free */	\
	/* blocks.  The void cast is used to prevent compiler warnings. */				\
	xStart.pxNextFreeBlock = ( void * ) xHeap.ucHeap;								\
	xStart.xBlockSize = ( size_t ) 0;												\
																					\
	/* xEnd is used to mark the end of the list of free blocks. */					\
	xEnd.xBlockSize = configTOTAL_HEAP_SIZE;										\
	xEnd.pxNextFreeBlock = NULL;													\
																					\
	/* To start with there is a single free block that is sized to take up the		\
	entire heap space. */															\
	pxFirstFreeBlock = ( void * ) xHeap.ucHeap;										\
	pxFirstFreeBlock->xBlockSize = configTOTAL_HEAP_SIZE;							\
	pxFirstFreeBlock->pxNextFreeBlock = &xEnd;										\
}
/*-----------------------------------------------------------*/

void *pvPortMalloc( size_t xWantedSize )
{
xBlockLink *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
static portBASE_TYPE xHeapHasBeenInitialised = pdFALSE;
void *pvReturn = NULL;

	vTaskSuspendAll();
	{
		/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		if( xHeapHasBeenInitialised == pdFALSE )
		{
			prvHeapInit();
			xHeapHasBeenInitialised = pdTRUE;
		}

		/* The wanted size is increased so it can contain a xBlockLink
		structure in addition to the requested amount of bytes. */
		if( xWantedSize > 0 )
		{
			xWantedSize += heapSTRUCT_SIZE;

			/* Ensure that blocks are always aligned to the required number of bytes. */
			if( xWantedSize & heapBYTE_ALIGNMENT_MASK )
			{
				/* Byte alignment required. */
				xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & heapBYTE_ALIGNMENT_MASK ) );
			}
		}

		if( ( xWantedSize > 0 ) && ( xWantedSize < configTOTAL_HEAP_SIZE ) )
		{
			/* Blocks are stored in byte order - traverse the list from the start
			(smallest) block until one of adequate size is found. */
			pxPreviousBlock = &xStart;
			pxBlock = xStart.pxNextFreeBlock;
			while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock ) )
			{
				pxPreviousBlock = pxBlock;
				pxBlock = pxBlock->pxNextFreeBlock;
			}

			/* If we found the end marker then a block of adequate size was not found. */
			if( pxBlock != &xEnd )
			{
				/* Return the memory space - jumping over the xBlockLink structure
				at its start. */
				pvReturn = ( void * ) ( ( ( unsigned char * ) pxPreviousBlock->pxNextFreeBlock ) + heapSTRUCT_SIZE );

				/* If the block is larger than required it can be split into two. */
				if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
				{
					/* This block is to be split into two.  Create a new block
					following the number of bytes requested. The void cast is
					used to prevent byte alignment warnings from the compiler. */
					pxNewBlockLink = ( void * ) ( ( ( unsigned char * ) pxBlock ) + xWantedSize );

					/* Calculate the sizes of two blocks split from the single
					block. */
					pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
					pxBlock->xBlockSize = xWantedSize;

					/* Insert the new block into the list of free blocks. */
#ifdef USE_ORDER_BY_SIZE
					pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;
					prvInsertBlockIntoFreeList( ( pxNewBlockLink ) );
				}
#else					
					pxPreviousBlock->pxNextFreeBlock = pxNewBlockLink;
					pxNewBlockLink->pxNextFreeBlock = pxBlock->pxNextFreeBlock;
					/* list is ok, try to COALESCE with the next (if it is not the xEnd)
					coalescing with previous has no sense (it is not adjacent for sure */
					pxBlock = pxNewBlockLink->pxNextFreeBlock;  			
					if (pxBlock != &xEnd)														
						if ( ((char*)pxNewBlockLink + (pxNewBlockLink->xBlockSize)) == ((char*) pxBlock) ) {
							pxNewBlockLink->xBlockSize += pxBlock->xBlockSize;					
							pxNewBlockLink->pxNextFreeBlock = pxBlock->pxNextFreeBlock;		
						}																		
				}
				/* If the block can't be split into two slice simply take it out of the list */
				else 
					pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;
#endif
				xFreeBytesRemaining -= xWantedSize;
			}
		}
	}
	xTaskResumeAll();

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if( pvReturn == NULL )
		{
			extern void vApplicationMallocFailedHook( void );
			vApplicationMallocFailedHook();
		}
	}
	#endif

	return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree( void *pv )
{
unsigned char *puc = ( unsigned char * ) pv;
xBlockLink *pxLink;

	if( pv )
	{
		/* The memory being freed will have an xBlockLink structure immediately
		before it. */
		puc -= heapSTRUCT_SIZE;

		/* This casting is to keep the compiler from issuing warnings. */
		pxLink = ( void * ) puc;

		vTaskSuspendAll();
		{
			xFreeBytesRemaining += pxLink->xBlockSize;
			
			/* Add this block to the list of free blocks. */
#ifdef USE_ORDER_BY_SIZE
			prvInsertBlockIntoFreeList( ( ( xBlockLink * ) pxLink ) );
#else
			prvInsertBlockIntoFreeListFF( ( pxLink ) );
#endif
		}
		xTaskResumeAll();
	}
}

void * pvPortRealloc( void *pv, size_t xWantedSize){

	if (! pv){
		return pvPortMalloc(xWantedSize);
	}else{
		unsigned portCHAR *puc = ( unsigned portCHAR * ) pv;
		xBlockLink *pxLink;
		puc -= heapSTRUCT_SIZE;

		pxLink = (void *)puc;
		size_t origSize = pxLink->xBlockSize;
		if (origSize == xWantedSize){
			return pv;
		}
		else{
			void *newPv = pvPortMalloc(xWantedSize);
			memcpy(newPv, pv, xWantedSize < origSize ? xWantedSize : origSize);
			vPortFree(pv);
			return newPv;
		}
	}
}


/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize( void )
{
	return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void )
{
	// TODO
	// memset to ZERO
	
	/* This just exists to keep the linker quiet. */
}
