#include "memory_device.h"
#include "board.h"
#include "memory_device_page_size.h"

#ifndef RCP_TESTING /* groan */
    #define RAMFUNC __attribute__ ((long_call, section (".fastrun")))
#else
    #define RAMFUNC
#endif


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

memory_flash_result_t memory_device_flash_region(const void *address, const void *data, unsigned int length){
	if (length < MEMORY_PAGE_SIZE) length = length + (MEMORY_PAGE_SIZE - length);

	unsigned int pages = length / AT91C_IFLASH_PAGE_SIZE;
	for (unsigned int i = 0; i < pages; i++){
		unsigned int offset = (i * AT91C_IFLASH_PAGE_SIZE);
		if (flash_write((void *)((unsigned int)address + offset),(void *)((unsigned int)data + offset)) != 0 ){
			return MEMORY_FLASH_WRITE_ERROR;
		}
	}
	return 0;
}
