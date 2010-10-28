#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_

#define SCRIPT_PAGES 40

const const char * getScript();

int flashScriptPage(unsigned int page, const char *data);

unsigned int getPageSize();

unsigned int getScriptPages();

#define DEFAULT_SCRIPTxx \
		"function xonTick()\n" \
		"end\n";

#define DEFAULT_SCRIPT \
		"--5 corners, Edmonds\n" \
		"--setStartFinishPoint(47.806948,-122.346578,0.001)\n" \
		"setStartFinishPoint(45.596536,-122.698436,0.001)\n" \
		"setTweetNumber(\"40404\")\n" \
		"function xonTick()\n" \
		"end\n"; 

		
#define DEFAULT_SCRIPT_TEST \
		"function onTick()\n" \
		"setLED(2,1)\n" \
		"for i = 1,1000 do end\n" \
		"setLED(2,0)\n" \
		"for i = 1,1000 do end\n" \
		"end\n";
	
#endif /*LUASCRIPT_H_*/
