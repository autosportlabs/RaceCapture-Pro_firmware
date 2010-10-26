#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_

#define SCRIPT_PAGES 40

const const char * getScript();

int flashScriptPage(unsigned int page, const char *data);

unsigned int getPageSize();

unsigned int getScriptPages();

#define DEFAULT_SCRIPT \
		"function xonTick()\n" \
		"end\n"; 
		
#define TEST_SCRIPT \
		"tol = 100\n" \
		"idle = 0\n" \
		"thresh = 20\n" \
		"logging = 0\n" \
		"lastX = 0\n" \
		"function onTick()\n" \
		"if lastX == 0 then lastX = getAccelerometerRaw(0) end\n" \
		"x = getAccelerometerRaw(0)\n" \
		"if logging == 1 and  x < lastX + tol and x > lastX - tol then idle = idle + 1 else idle = 0 end\n" \
		"if logging == 1 and idle >= thresh then stopLogging();logging = 0 end\n" \
		"if logging == 0 and (x > lastX + tol or x < lastX - tol) then startLogging();logging = 1;idle = 0 end\n" \
		"lastX = x\n" \
		"end\n";
		
		//"print(x .." ");print(lastX .." ");println(idle);\n" 
		
#define DEFAULT_SCRIPT_TEST \
		"function onTick()\n" \
		"setLED(2,1)\n" \
		"for i = 1,1000 do end\n" \
		"setLED(2,0)\n" \
		"for i = 1,1000 do end\n" \
		"end\n";
	
#endif /*LUASCRIPT_H_*/
