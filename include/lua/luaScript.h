#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_

#define SCRIPT_PAGES 40

int flash_default_script();

const char * getScript();

int flashScriptPage(unsigned int page, const char *data);

unsigned int getPageSize();

unsigned int getScriptPages();


#define DEFAULT_SCRIPT "function onTick() end"
	
#endif /*LUASCRIPT_H_*/
