#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_

const const char * getScript();

int flashScriptPage(unsigned int page, const char *data);

unsigned int getPageSize();

unsigned int getScriptPages();

#define DEFAULT_SCRIPT \
		"function onTick()\n" \
		"end\n"; 
		
#define TEST_SCRIPT \
		"function onTick()\n" \
		"if getAccelerometerRaw(0) > 2200 then setLED(2,1) else setLED(2,0) end\n" \
		"end\n";
		
#define TEST_SCRIPT2 \
		"function onTick()\n" \
		"setLED(2,1)\n" \
		"for i = 1,1000000 do end\n" \
		"setLED(2,0)\n" \
		"end\n";
	
#endif /*LUASCRIPT_H_*/
