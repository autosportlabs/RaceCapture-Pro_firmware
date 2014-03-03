#include "luaScript.h"
#include "memory.h"
#include "mem_mang.h"
#include "mod_string.h"
#include "magic.h"
#include "printk.h"

#define SCRIPT_LENGTH SCRIPT_PAGES * MEMORY_PAGE_SIZE

static const char g_script[SCRIPT_LENGTH + 1] __attribute__ ((aligned (MEMORY_PAGE_SIZE))) __attribute__((section(".script\n\t#")));

static const char g_defaultScript[] = DEFAULT_SCRIPT;

int flash_default_script(){
	pr_info("flashing default script...");
	int result = memory_flash_region(&g_script, &g_defaultScript, sizeof (DEFAULT_SCRIPT));
	if (result == 0) pr_info("success\r\n"); else pr_info("failed\r\n");
	return result;
}

const char * getScript(){
	return g_script;
}


int flashScriptPage(unsigned int page, const char *data){
	
	int result = -1;
	char * scriptPageAddress = (char *)g_script;
	scriptPageAddress += (page * MEMORY_PAGE_SIZE);
	//if less than the page size, copy it into an expanded buffer
	char * temp = portMalloc(MEMORY_PAGE_SIZE);

	if (temp){
		int size = strlen(data);
		if (size > MEMORY_PAGE_SIZE) size = MEMORY_PAGE_SIZE;
		memset(temp,0,MEMORY_PAGE_SIZE);
		memcpy(temp,data,size);
		result = memory_flash_region((void *)scriptPageAddress,(void *)temp, MEMORY_PAGE_SIZE);
		portFree(temp);
	}
	return result;
}

unsigned int getPageSize(){
	return MEMORY_PAGE_SIZE;
}

unsigned int getScriptPages(){
	return SCRIPT_PAGES;
}
