#include "memory.h"


/**
 * Name:      flash_write
 * Purpose:   Writes a page in the flash memory
 * Inputs:
 *  - Destination address in the flash
 *  - Source data buffer
 * Output: OK if write is successful, ERROR otherwise
 */
 
int flashWriteRegion(void *vAddress, void *vData, unsigned int length){

	unsigned int pages = length / AT91C_IFLASH_PAGE_SIZE;
	for (unsigned int i = 0; i < pages; i++){
		unsigned int offset = (i * AT91C_IFLASH_PAGE_SIZE);
		if (flash_write((void *)((unsigned int)vAddress + offset),(void *)((unsigned int)vData + offset)) != 0 ){
			return -1;	
		}
	}
	return 0;
}

int RAMFUNC flash_write(void * vAddress, void * vData){

  // Local variables
  unsigned int page;
  unsigned int mask;
  unsigned int status;
  unsigned int * pAddress = (unsigned int *) vAddress;
  unsigned int * pData = (unsigned int *) vData;

  // Program FMCN field in Flash Mode Register
  AT91C_BASE_MC->MC_FMR = ((BOARD_MCK / 666666 << 16) & AT91C_MC_FMCN) | AT91C_MC_FWS_1FWS;
 
  // Calculate page number and flash address
  page = ((((unsigned int) pAddress - (unsigned int) MEMORY_START_ADDRESS) / AT91C_IFLASH_PAGE_SIZE) << 8) & AT91C_MC_PAGEN;

  // Copy page in write buffer
  for (unsigned int i=0; i < MEMORY_PAGE_SIZE_32; i++) {
    pAddress[i] = pData[i];
  }

  // Start flash write operation and wait for completion (IT disabled to avoid flash access)
  while (!(AT91C_BASE_MC->MC_FSR & AT91C_MC_FRDY));
  mask = AT91C_BASE_AIC->AIC_IMR;
  AT91C_BASE_AIC->AIC_IDCR = 0xFFFFFFFF;

  AT91C_BASE_MC->MC_FCR = page | AT91C_MC_FCMD_START_PROG | AT91C_MC_CORRECT_KEY;

  do {

    status = AT91C_BASE_MC->MC_FSR;

  } while (!(status & AT91C_MC_FRDY));
  AT91C_BASE_AIC->AIC_IECR = mask;

  if ((status & (AT91C_MC_LOCKE | AT91C_MC_PROGE)) != 0) {
    return -1;
  }
  else {
    return 0;
  }
}
