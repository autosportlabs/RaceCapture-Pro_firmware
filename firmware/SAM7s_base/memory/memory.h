#ifndef MEMORY_H_
#define MEMORY_H_
#include "AT91SAM7S256.h"
#include "board.h"

#define MEMORY_START_ADDRESS   AT91C_IFLASH
#define MEMORY_SIZE            AT91C_IFLASH_SIZE
#define MEMORY_PAGE_SIZE       AT91C_IFLASH_PAGE_SIZE
#define MEMORY_PAGE_SIZE_32    (MEMORY_PAGE_SIZE / 4)

#ifndef RCP_TESTING /* groan */
    #define RAMFUNC __attribute__ ((long_call, section (".fastrun")))
#else
    #define RAMFUNC
#endif

int flashWriteRegion(void *vAddress, void *vData, unsigned int length);
	
int RAMFUNC flash_write(void * vAddress, void * vData);

#endif /*MEMORY_H_*/
