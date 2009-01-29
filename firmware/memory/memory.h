#ifndef MEMORY_H_
#define MEMORY_H_

 
#define RAMFUNC __attribute__ ((long_call, section (".fastrun")))

unsigned int getMemoryPageNumber(void *vAddress);
	
unsigned int RAMFUNC flash_write(void * vAddress, void * vData);



#endif /*MEMORY_H_*/
