#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_

#include <stdint.h>
#include "memory.h"

#define SCRIPT_PAGES 40
#define SCRIPT_LENGTH SCRIPT_PAGES * MEMORY_PAGE_SIZE

#define MAGIC_NUMBER_SCRIPT_INIT 0xDECAFBAD

typedef struct _ScriptConfig{
	uint32_t magicInit;
	char script[SCRIPT_LENGTH - 4];
} ScriptConfig;

typedef struct _DefaultScriptConfig{
	uint32_t magicInit;
	char script[100];
} DefaultScriptConfig;

void initialize_script();

int flash_default_script();

const char * getScript();

int flashScriptPage(unsigned int page, const char *data);

unsigned int getPageSize();

unsigned int getScriptPages();

void unescapeScript(char *data);

#define DEFAULT_SCRIPT_CONFIG {MAGIC_NUMBER_SCRIPT_INIT, "function onTick() end"}
	
#endif /*LUASCRIPT_H_*/
