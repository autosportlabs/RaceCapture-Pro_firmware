#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_

#define SCRIPT_PAGES 40

const const char * getScript();

int flashScriptPage(unsigned int page, const char *data);

unsigned int getPageSize();

unsigned int getScriptPages();

#define DEFAULT_SCRIPT \
		"setStartFinishPoint(47.806953,-122.346572,0.0001)\n" \
		"setTweetNumber(\"2068544508\")\n" \
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
