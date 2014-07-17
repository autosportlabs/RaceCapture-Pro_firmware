#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_

#include <stdint.h>
#include "memory.h"

#define SCRIPT_ADD_RESULT_OK  		1
#define SCRIPT_ADD_RESULT_FAIL  	0

#define SCRIPT_ADD_MODE_IN_PROGRESS	1
#define SCRIPT_ADD_MODE_COMPLETE 	2

#define MAGIC_NUMBER_SCRIPT_INIT 0xDECAFBAD

#define SCRIPT_PAGE_SIZE 256
#define MAX_SCRIPT_PAGES SCRIPT_MEMORY_LENGTH / SCRIPT_PAGE_SIZE

typedef struct _ScriptConfig{
	uint32_t magicInit;
	char script[SCRIPT_MEMORY_LENGTH - sizeof(uint32_t)];
} ScriptConfig;

typedef struct _DefaultScriptConfig{
	uint32_t magicInit;
	char script[100];
} DefaultScriptConfig;

void initialize_script();

int flash_default_script();

const char * getScript();

int flashScriptPage(unsigned int page, const char *data, int mode);

void unescapeScript(char *data);

#define DEFAULT_SCRIPT_CONFIG {MAGIC_NUMBER_SCRIPT_INIT, "function onTick() end"}
	
#endif /*LUASCRIPT_H_*/
