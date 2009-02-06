#include "luaTask.h"
#include "FreeRTOS.h"
#include "task.h"

#define LUA_30Hz 10
#define LUA_20Hz 15
#define LUA_10Hz 30

#define LUA_STACK_SIZE 100
#define LUA_PRIORITY 2

#define LUA_PERIODIC_FUNCTION "onTick"

static const luaL_Reg lualibs[] = {
  {"", luaopen_base},
  {LUA_TABLIBNAME, luaopen_table},
//  {LUA_STRLIBNAME, luaopen_string},
//  {LUA_MATHLIBNAME, luaopen_math},
//  {LUA_DBLIBNAME, luaopen_debug},
  {NULL, NULL}
};


void startLuaTask(){

	xTaskCreate( luaTask,
					( signed portCHAR * ) "luaTask",
					LUA_STACK_SIZE,
					NULL,
					LUA_PRIORITY,
					NULL);
}

int doIt(lua_State *L){

	return 0;	
}

void luaTask(void *params){

	lua_State *L=lua_open();

	const luaL_Reg *lib = lualibs;
  	for (; lib->func; lib++) {
	    lua_pushcfunction(L, lib->func);
	    lua_pushstring(L, lib->name);
	    lua_call(L, 1, 0);
	}

	lua_register(L,LUA_PERIODIC_FUNCTION,doIt);
	
	while(1){

		portTickType xLastWakeTime, startTickTime;
		const portTickType xFrequency = LUA_10Hz;
		startTickTime = xLastWakeTime = xTaskGetTickCount();
		
 		lua_getglobal(L, LUA_PERIODIC_FUNCTION);
    	if (! lua_isnil(L,-1)){
        	if (lua_pcall(L, 0, LUA_MULTRET, 0)){
        		//error
	          	//  const char *error = lua_tostring (g_luaVM, -1);
    	      	//  stringstream errorMsg;
        	  	//  errorMsg << "Error calling OnStart(): " << error;
            	//	ERROR(errorMsg);
            	//  WriteConsole(errorMsg.str());
        	}
    	} else{
	       // ERROR("OnStart: No OnStart() script function defined");
	        lua_pop(L,1);
	    }

        if (luaL_loadstring(L,"onTick()") || lua_pcall(L, 0, LUA_MULTRET, 0)){
/*            //handle error
            const char *error = lua_tostring (lua, -1);
            const char *errorMsg = "resultCode=\"unrecognizedCommand\";\r\n";
            Writeline(*connectionHandle,errorMsg,strlen(errorMsg));
            ERROR("SocketThread API command error: " << error);
            */
        }
    		
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}

