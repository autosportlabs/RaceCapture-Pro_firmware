#include "luaTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "portable.h"
#include "luaScript.h"
#include "luaBaseBinding.h"
#include "mem_mang.h"
#include "taskUtil.h"
#include "printk.h"
#include "mod_string.h"

#define DEFAULT_ONTICK_HZ 1
#define MAX_ONTICK_HZ 30
#define LUA_STACK_SIZE 1000
#define LUA_PRIORITY 2

#define LUA_PERIODIC_FUNCTION "onTick"


static lua_State *g_lua;
static xSemaphoreHandle xLuaLock;
static unsigned int lastPointer;
static int g_shouldReloadScript;
static size_t onTickSleepInterval;

//#define ALLOC_DEBUG

#ifdef ALLOC_DEBUG
static int g_allocDebug = 0;
#endif
void * myAlloc (void *ud, void *ptr, size_t osize,size_t nsize) {
	
#ifdef ALLOC_DEBUG
	if (g_allocDebug){
		SendString("myAlloc- ptr:");
		SendUint((unsigned int)ptr);
		SendString(" osize:");
		SendInt(osize);
		SendString(" nsize:");
		SendInt(nsize);
	}
#endif

   if (nsize == 0) {
     portFree(ptr);
#ifdef ALLOC_DEBUG
     if (g_allocDebug){
    	 SendString(" (free)");
    	 SendCrlf();
     }
#endif
     return NULL;
   }
   else{
	 void *newPtr;
	 if (osize != nsize){
		 newPtr = portRealloc(ptr, nsize);
	 }
	 else{
		 newPtr = ptr;
	 }
#ifdef ALLOC_DEBUG
   	 if (g_allocDebug){
   		 if (ptr != newPtr){
   			 SendString(" newPtr:");
   			 SendUint((unsigned int)newPtr);
   		 }
   		 else{
   			 SendString(" (same)");
   		 }
		 SendCrlf();
   	 }
#endif
   	 lastPointer = (unsigned int)newPtr;
   	 return newPtr;
   }
}

unsigned int getLastPointer(){
	return lastPointer;
}

void setAllocDebug(int enableDebug){
#ifdef ALLOC_DEBUG
	g_allocDebug = enableDebug;
#endif

}

int getAllocDebug(){
#ifdef ALLOC_DEBUG
	return g_allocDebug;
#else
	return 0;
#endif
}

void lockLua(void){
	xSemaphoreTake(xLuaLock, portMAX_DELAY);
}

void unlockLua(void){
	xSemaphoreGive(xLuaLock);
}

void set_ontick_freq(size_t freq){
	if (freq <= MAX_ONTICK_HZ) onTickSleepInterval = msToTicks(1000 / freq);
}

size_t get_ontick_freq(){
	return 1000 / ticksToMs(onTickSleepInterval);
}

inline int getShouldReloadScript(void){
	return g_shouldReloadScript;
}

inline void setShouldReloadScript(int reload){
	g_shouldReloadScript = reload;
}

void startLuaTask(){
	setShouldReloadScript(0);
	set_ontick_freq(DEFAULT_ONTICK_HZ);

	vSemaphoreCreateBinary(xLuaLock);

	lockLua();
	g_lua=lua_newstate( myAlloc, NULL);
	//open optional libraries
	luaopen_base(g_lua);
	luaopen_table(g_lua);
	luaopen_string(g_lua);
	luaopen_math(g_lua);
	registerBaseLuaFunctions(g_lua);
	unlockLua();

	xTaskCreate( luaTask,
					( signed portCHAR * ) "luaTask",
					LUA_STACK_SIZE,
					NULL,
					LUA_PRIORITY,
					NULL);
}

static void doScript(void){
	lockLua();
	const char *script = getScript();
	size_t len = strlen(script);
	pr_info("running lua script len(");
	pr_info_int(len);
	pr_info(")...");

	lua_gc(g_lua, LUA_GCCOLLECT,0);
	int result = 0;
	//int result = (luaL_loadbuffer(g_lua, script, len, "startup"));// || lua_pcall(g_lua, 0, LUA_MULTRET, 0));
	if (0 != result){
		pr_error("startup script error: (");
		pr_error(lua_tostring(g_lua,-1));
		pr_error(")\r\n");
		lua_pop(g_lua,1);
	}
	pr_info("done\r\n");
	lua_gc(g_lua, LUA_GCCOLLECT,0);
	pr_info("lua memory usage: ");
	pr_info_int(lua_gc(g_lua, LUA_GCCOUNT,0));
	pr_info("Kb\r\n");
	unlockLua();
}

void luaTask(void *params){
	doScript();
	while(1){
		portTickType xLastWakeTime, startTickTime;
		startTickTime = xLastWakeTime = xTaskGetTickCount();
		if (getShouldReloadScript()){
			doScript();
			setShouldReloadScript(0);
		}
		lockLua();
 		lua_getglobal(g_lua, LUA_PERIODIC_FUNCTION);
    	if (! lua_isnil(g_lua,-1)){
        	if (lua_pcall(g_lua, 0, 0, 0) != 0){
// TODO log or indicate error. store this in a "Last Error"
//        		SendString("Error calling ");
//        		SendString(LUA_PERIODIC_FUNCTION);
//        		SendString("(): ");
//        		SendString( lua_tostring(g_lua,-1));
//        		SendCrlf();
        		lua_pop(g_lua,1);
        	}
    	} else{
	       // //handle missing function error
	        lua_pop(g_lua,1);
	    }
    	unlockLua();
		vTaskDelayUntil( &xLastWakeTime, onTickSleepInterval );
	}
}

lua_State * getLua(){
	return g_lua;
}
