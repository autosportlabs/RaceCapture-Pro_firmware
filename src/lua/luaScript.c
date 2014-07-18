#include "luaScript.h"
#include "mem_mang.h"
#include "mod_string.h"
#include "printk.h"

#ifndef RCP_TESTING
static const ScriptConfig g_scriptConfig __attribute__ ((aligned (MEMORY_PAGE_SIZE))) __attribute__((section(".script\n\t#")));
#else
static ScriptConfig g_scriptConfig = DEFAULT_SCRIPT_CONFIG;
#endif

void initialize_script(){
	if (g_scriptConfig.magicInit != MAGIC_NUMBER_SCRIPT_INIT){
		//flash_default_script();
	}
}

int flash_default_script(){
	int result = -1;
	pr_info("flashing default script...");
	ScriptConfig *defaultScriptConfig = pvPortMalloc(sizeof(ScriptConfig));
	if (defaultScriptConfig != NULL){
		defaultScriptConfig->magicInit = MAGIC_NUMBER_SCRIPT_INIT;
		strncpy(defaultScriptConfig->script, DEFAULT_SCRIPT, sizeof(DEFAULT_SCRIPT));

		result = memory_flash_region(&g_scriptConfig, &defaultScriptConfig, sizeof (ScriptConfig));
		pr_info_int(sizeof(ScriptConfig));
		pr_info(" ");
		pr_info_int(g_scriptConfig.magicInit);
		pr_info(" ");
		pr_info_int(defaultScriptConfig->magicInit);
		pr_info("\r\n");
		vPortFree(defaultScriptConfig);
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

int flashScriptPage(unsigned int page, const char *data){
	int result = -1;
	char * scriptPageAddress = (char *)g_scriptConfig.script;
	scriptPageAddress += (page * MEMORY_PAGE_SIZE);
	//if less than the page size, copy it into an expanded buffer
	char * temp = (char *)portMalloc(MEMORY_PAGE_SIZE);

	if (temp){
		size_t size = strlen(data);
		if (size > MEMORY_PAGE_SIZE) size = MEMORY_PAGE_SIZE;
		memset(temp, 0, MEMORY_PAGE_SIZE);
		memcpy(temp, data, size);
		result = memory_flash_region((void *)scriptPageAddress, (void *)temp, MEMORY_PAGE_SIZE);
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
