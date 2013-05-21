#include "luaScript.h"
#include "memory.h"
#include "heap.h"
#include "mod_string.h"

#define SCRIPT_LENGTH SCRIPT_PAGES * MEMORY_PAGE_SIZE

static const char g_script[SCRIPT_LENGTH + 1] __attribute__ ((aligned (MEMORY_PAGE_SIZE))) __attribute__((section(".text\n\t#"))) = DEFAULT_SCRIPT;

const char * getScript(){
	return g_script;
}

int flashScriptPage(unsigned int page, const char *data){
	
	int result = -1;
	char * scriptPageAddress = (char *)g_script;
	scriptPageAddress += (page * MEMORY_PAGE_SIZE);
	//if less than the page size, copy it into an expanded buffer
	char * temp = pvPortMalloc(MEMORY_PAGE_SIZE);

	if (temp){
		int size = strlen(data);
		if (size > MEMORY_PAGE_SIZE) size = MEMORY_PAGE_SIZE;
		memset(temp,0,MEMORY_PAGE_SIZE);
		memcpy(temp,data,size);
		result = flashWriteRegion((void *)scriptPageAddress,(void *)temp, MEMORY_PAGE_SIZE);
		vPortFree(temp);
	}
	return result;
}

unsigned int getPageSize(){
	return MEMORY_PAGE_SIZE;
}

unsigned int getScriptPages(){
	return SCRIPT_PAGES;
}
