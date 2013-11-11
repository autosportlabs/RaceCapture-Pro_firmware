#include "luaTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "portable.h"
#include "luaScript.h"
#include "luaBaseBinding.h"
#include "mem_mang.h"

#define LUA_30Hz 10
#define LUA_20Hz 15
#define LUA_10Hz 30
#define LUA_5Hz 60
#define LUA_1Hz 300

#define LUA_STACK_SIZE 1000
#define LUA_PRIORITY 2

#define LUA_PERIODIC_FUNCTION "onTick"


static lua_State *g_lua;
static xSemaphoreHandle xLuaLock;
static unsigned int lastPointer;
static int g_shouldReloadScript;

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

int getShouldReloadScript(void){
	return g_shouldReloadScript;
}

void setShouldReloadScript(int reload){
	g_shouldReloadScript = reload;
}

void startLuaTask(){


	g_shouldReloadScript = 0;

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
	luaL_dostring(g_lua,getScript());
	unlockLua();
}

void luaTask(void *params){

	doScript();

	while(1){
		portTickType xLastWakeTime, startTickTime;
		const portTickType xFrequency = LUA_1Hz;
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

		vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}

lua_State * getLua(){
	return g_lua;
}

