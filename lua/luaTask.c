#include "luaTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "heap.h"
#include "usb_comm.h"
#include "luaLoggerBinding.h"

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

int doIt(lua_State *L){

	int val = (int)lua_tointeger(L,1);
	SendString("in doIt() ");
	SendInt(val);
	SendCrlf();
	return 0;	
}


void luaTask(void *params){

	g_lua=lua_newstate( myAlloc, NULL);
	
	//open optional libraries
	//luaopen_base(g_lua);
	//luaopen_table(g_lua);
	//luaopen_string(g_lua);

	RegisterLuaRaceCaptureFunctions(g_lua);
	lua_register(g_lua,"doIt",doIt);

	luaL_dostring(g_lua,"function onTick() \n if getAccelerometerRaw(1) > 2200 then setLED(2,1) end\n end");
	
	while(1){
		portTickType xLastWakeTime, startTickTime;
		const portTickType xFrequency = LUA_1Hz;
		startTickTime = xLastWakeTime = xTaskGetTickCount();
		
 		lua_getglobal(g_lua, LUA_PERIODIC_FUNCTION);
    	if (! lua_isnil(g_lua,-1)){
        	if (lua_pcall(g_lua, 0, LUA_MULTRET, 0) != 0){
        		SendString("Error Calling onTick(): ");
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

void testLua(){
	
	SendString("Testing Lua- ");
	int status = luaL_dostring(g_lua,"getAccelerometerRaw(1);");
	SendString("Status: ");
	SendInt(status);
	SendCrlf();
}
