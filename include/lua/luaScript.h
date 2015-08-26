#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_

#include <stdint.h>
#include "memory.h"
#include "capabilities.h"

#if LUA_SUPPORT == 1
#define SCRIPT_ADD_RESULT_OK  		1
#define SCRIPT_ADD_RESULT_FAIL  	0

#define SCRIPT_ADD_MODE_IN_PROGRESS	1
#define SCRIPT_ADD_MODE_COMPLETE 	2

#define MAGIC_NUMBER_SCRIPT_INIT 0xDECAFBAD

#define SCRIPT_PAGE_SIZE 256
#define MAX_SCRIPT_PAGES SCRIPT_MEMORY_LENGTH / SCRIPT_PAGE_SIZE

typedef struct _ScriptConfig {
    char script[SCRIPT_MEMORY_LENGTH - 4];
    uint32_t magicInit;
} ScriptConfig;

void initialize_script();

int flash_default_script();

const char * getScript();

int flashScriptPage(unsigned int page, const char *data, int mode);

void unescapeScript(char *data);

#define DEFAULT_SCRIPT "function onTick() end"

#endif /*LUA_SUPPORT == 1 */
#endif /*LUASCRIPT_H_*/
