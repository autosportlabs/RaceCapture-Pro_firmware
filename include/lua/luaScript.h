#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_

#include <stdint.h>
#include "memory.h"

#define SCRIPT_PAGES 40
#define SCRIPT_LENGTH SCRIPT_PAGES * MEMORY_PAGE_SIZE

#define MAGIC_NUMBER_SCRIPT_INIT 0xDECAFBAD

typedef struct _ScriptConfig{
	char script[SCRIPT_LENGTH - 4];
	uint32_t magicInit;
} ScriptConfig;

void initialize_script();

int flash_default_script();

const char * getScript();

int flashScriptPage(unsigned int page, const char *data);

unsigned int getPageSize();

unsigned int getScriptPages();

void unescapeScript(char *data);

#define DEFAULT_SCRIPT "function onTick() end"
	
#endif /*LUASCRIPT_H_*/
