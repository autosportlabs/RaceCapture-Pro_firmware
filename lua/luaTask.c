#include "luaTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "portable.h"
#include "usb_comm.h"
#include "luaLoggerBinding.h"
#include "luaScript.h"

#define LUA_30Hz 10
#define LUA_20Hz 15
#define LUA_10Hz 30
#define LUA_5Hz 60
#define LUA_1Hz 300

#define LUA_STACK_SIZE 3000
#define LUA_PRIORITY 2

#define LUA_PERIODIC_FUNCTION "onTick"




//#define ALLOC_DEBUG

void * myAlloc (void *ud, void *ptr, size_t osize,size_t nsize) {
	
#ifdef ALLOC_DEBUG
	SendString("myAlloc ");
	SendUint((unsigned int)ptr);
	SendString(" ");
	SendInt(osize);
	SendString(" ");
	SendInt(nsize);
#endif

   //(void)ud;  (void)osize;

   if (nsize == 0) {
     vPortFree(ptr);
#ifdef ALLOC_DEBUG
	SendCrlf();
#endif
     return NULL;
   }
   else{
   	 void *newPtr = pvPortRealloc(ptr, nsize);
#ifdef ALLOC_DEBUG
   	 SendString(" ");
   	 SendUint((unsigned int)newPtr);
   	 SendCrlf();
#endif
   	 return newPtr;
   }
}


lua_State *g_lua;
static xSemaphoreHandle xLuaLock;


void lockLua(void){
	xSemaphoreTake(xLuaLock, portMAX_DELAY);
}

void unlockLua(void){
	xSemaphoreGive(xLuaLock);
}

void startLuaTask(){

	vSemaphoreCreateBinary(xLuaLock);

	lockLua();

	g_lua=lua_newstate( myAlloc, NULL);
	
	//open optional libraries
	luaopen_base(g_lua);
	luaopen_table(g_lua);
	luaopen_string(g_lua);

	RegisterLuaRaceCaptureFunctions(g_lua);
	unlockLua();

	xTaskCreate( luaTask,
					( signed portCHAR * ) "luaTask",
					LUA_STACK_SIZE,
					NULL,
					LUA_PRIORITY,
					NULL);
}

void luaTask(void *params){

	lockLua();
	luaL_dostring(g_lua,getScript());
	unlockLua();


	while(1){
		portTickType xLastWakeTime, startTickTime;
		const portTickType xFrequency = LUA_1Hz;
		startTickTime = xLastWakeTime = xTaskGetTickCount();

		lockLua();
 		lua_getglobal(g_lua, LUA_PERIODIC_FUNCTION);
    	if (! lua_isnil(g_lua,-1)){
        	if (lua_pcall(g_lua, 0, 0, 0) != 0){
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

