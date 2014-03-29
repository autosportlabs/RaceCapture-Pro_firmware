#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_

#define SCRIPT_PAGES 40

const const char * getScript();

int flashScriptPage(unsigned int page, const char *data);

unsigned int getPageSize();

unsigned int getScriptPages();


//#define DEFAULT_SCRIPT "function onTick() end"

#define DEFAULT_SCRIPT \
"setTickRate(15) \n \
function onTick()  \n \
yaw=getAnalog(3) \n \
if yaw < 0 then yaw = -yaw end \n \
if yaw > 10 then println(\"foo\") end \n \
if yaw > 10 then setGpio(0,1) else setGpio(0,0) end \n \
-if yaw > 20 then setGpio(1,1) else setGpio(1,0) end \n \
if yaw > 30 then setGpio(2,1) else setGpio(2,0) end \n \
end \n "
	
#endif /*LUASCRIPT_H_*/
