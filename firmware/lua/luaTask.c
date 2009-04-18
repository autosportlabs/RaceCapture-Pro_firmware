#include "luaTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
#include "usb_comm.h"
#include "luaLoggerBinding.h"
#include "luaScript.h"

#define LUA_30Hz 10
#define LUA_20Hz 15
#define LUA_10Hz 30
#define LUA_5Hz 60
#define LUA_1Hz 300

#define LUA_STACK_SIZE 2000
#define LUA_PRIORITY 2

#define LUA_PERIODIC_FUNCTION "onTick"


void * myAlloc (void *ud, void *ptr, size_t osize,size_t nsize) {
	
//	SendString("myAlloc ");
//	SendInt(nsize);
   (void)ud;  (void)osize;
   if (nsize == 0) {
     vPortFree(ptr);
//	SendCrlf();
     return NULL;
   }
   else{
   	 void *newPtr = pvPortRealloc(ptr, nsize);
//   	 SendString(" ");
//   	 SendUint((unsigned int)newPtr);
//   	 SendCrlf();
   	 return newPtr;
   }
}


lua_State *g_lua;

void startLuaTask(){

	xTaskCreate( luaTask,
					( signed portCHAR * ) "luaTask",
					LUA_STACK_SIZE,
					NULL,
					LUA_PRIORITY,
					NULL);
}

void luaTask(void *params){

	g_lua=lua_newstate( myAlloc, NULL);
	
	//open optional libraries
	//luaopen_base(g_lua);
	//luaopen_table(g_lua);
	//luaopen_string(g_lua);

	RegisterLuaRaceCaptureFunctions(g_lua);
	portENTER_CRITICAL();
	luaL_dostring(g_lua,getScript());
	portEXIT_CRITICAL();
	while(1){
		portTickType xLastWakeTime, startTickTime;
		const portTickType xFrequency = LUA_5Hz;
		startTickTime = xLastWakeTime = xTaskGetTickCount();

 		lua_getglobal(g_lua, LUA_PERIODIC_FUNCTION);
    	if (! lua_isnil(g_lua,-1)){
        	if (lua_pcall(g_lua, 0, 0, 0) != 0){
        		SendString("Error calling ");
        		SendString(LUA_PERIODIC_FUNCTION);
        		SendString("(): ");
        		SendString( lua_tostring(g_lua,-1));
        		SendCrlf();
        		lua_pop(g_lua,1);
        	}
    	} else{
	       // //handle missing function error
	        lua_pop(g_lua,1);
	    }
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}

lua_State * getLua(){
	return g_lua;
}

