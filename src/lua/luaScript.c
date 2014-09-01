#include "luaScript.h"
#include "mem_mang.h"
#include "printk.h"
#include "mod_string.h"


#ifndef RCP_TESTING
static const ScriptConfig g_scriptConfig  __attribute__((section(".script\n\t#")));
#else
static ScriptConfig g_scriptConfig = {DEFAULT_SCRIPT, MAGIC_NUMBER_SCRIPT_INIT};
#endif

static ScriptConfig * g_scriptBuffer = NULL;

void initialize_script(){
	if (g_scriptConfig.magicInit != MAGIC_NUMBER_SCRIPT_INIT){
		flash_default_script();
	}
}

int flash_default_script(){
	int result = -1;
	pr_info("flashing default script...");
	ScriptConfig *defaultScriptConfig = (ScriptConfig *)portMalloc(sizeof(ScriptConfig));
	if (defaultScriptConfig != NULL){
		defaultScriptConfig->magicInit = MAGIC_NUMBER_SCRIPT_INIT;
		strncpy(defaultScriptConfig->script, DEFAULT_SCRIPT, sizeof(DEFAULT_SCRIPT));
		result = memory_flash_region(&g_scriptConfig, defaultScriptConfig, sizeof (ScriptConfig));
		portFree(defaultScriptConfig);
	}
	if (result == 0) pr_info("success\r\n"); else pr_info("failed\r\n");
	return result;
}

const char * getScript(){
	return g_scriptConfig.script;
}

//unescapes a string in place
void unescapeScript(char *data){
	char *result = data;
	while (*data){
		if (*data == '\\'){
			switch(*(data + 1)){
				case '_':
					*result = ' ';
					break;
				case 'n':
					*result = '\n';
					break;
				case 'r':
					*result = '\r';
					break;
				case '\\':
					*result = '\\';
					break;
				case '"':
					*result = '\"';
					break;
				case '\0': //this should *NOT* happen
					*result = '\0';
					return;
					break;
				default: // unknown escape char?
					*result = ' ';
					break;
			}
			result++;
			data+=2;
		}
		else{
			*result++ = *data++;
		}
	}
	*result='\0';
}

int flashScriptPage(unsigned int page, const char *data, int mode){

	int result = SCRIPT_ADD_RESULT_OK;

	if (page < MAX_SCRIPT_PAGES){
		if (mode == SCRIPT_ADD_MODE_IN_PROGRESS || mode == SCRIPT_ADD_MODE_COMPLETE){
			if (g_scriptBuffer == NULL){
				pr_info("allocating new script buffer\r\n");
				g_scriptBuffer = (ScriptConfig *)portMalloc(sizeof(ScriptConfig));
				memcpy(g_scriptBuffer, &g_scriptConfig, sizeof(ScriptConfig));
			}

			if (g_scriptBuffer != NULL){
				page = page * SCRIPT_PAGE_SIZE;
				char *pageToAdd = g_scriptBuffer->script + page;
				strncpy(pageToAdd, data, SCRIPT_PAGE_SIZE);

				if (mode == SCRIPT_ADD_MODE_COMPLETE){
					pr_info("completed updating script, flashing: ");
					if (memory_flash_region(&g_scriptConfig, g_scriptBuffer, sizeof(ScriptConfig)) == 0){
						pr_info("success\r\n");
					}
					else{
						pr_error("error\r\n");
						result = SCRIPT_ADD_RESULT_FAIL;
					}
					portFree(g_scriptBuffer);
					g_scriptBuffer = NULL;
				}
			}
			else{
				pr_error("could not allocate buffer for script\r\n");
				result = SCRIPT_ADD_RESULT_FAIL;
			}
		}
	}
	else{
		pr_error("invalid track index\r\n");
		result = SCRIPT_ADD_RESULT_FAIL;
	}
	return result;
}






