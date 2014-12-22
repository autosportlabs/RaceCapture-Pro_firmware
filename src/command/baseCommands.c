/*
 * baseCommands.c
 *
 *  Created on: Jul 23, 2011
 *      Author: brent
 */
#include "baseCommands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mem_mang.h"
#include "luaScript.h"
#include "lua.h"
#include "luaTask.h"
#include "memory.h"
#include "loggerConfig.h"
#include "cpu.h"

extern unsigned int _CONFIG_HEAP_SIZE;

void ShowStats(Serial *serial, unsigned int argc, char **argv){
   serial_printf(serial, "== Memory Info ==\r\n");
   serial_printf(serial, "Total memory         : %u\r\n", (unsigned int)&_CONFIG_HEAP_SIZE);
   serial_printf(serial, "Free memory          : %u\r\n", portGetFreeHeapSize());
	serial_printf(serial, "== Lua Info ==\r\n");
	lua_State *L = (lua_State *)getLua(); //TODO: this was done for unit testing. fix when Lua runtime is part of unit testing framework
	lua_gc(L,LUA_GCCOLLECT,0);
	serial_printf(serial, "Lua Top              : %d\r\n", lua_gettop(L));
	serial_printf(serial, "Lua memory usage(Kb) : %d\r\n", lua_gc(L,LUA_GCCOUNT,0));
	serial_printf(serial, "== Misc ==\r\n");
	serial_printf(serial, "sizeof LoggerConfig  : %d\r\n", sizeof(LoggerConfig));
	serial_printf(serial, "sizeof ChannelSample : %d\r\n", sizeof(ChannelSample));
}

void ShowTaskInfo(Serial *serial, unsigned int argc, char **argv){
	char *taskList = (char *)portMalloc(1024);
	if (NULL != taskList){
		vTaskList(taskList);
		serial_printf(serial, "Task Info\r\nStatus\tPri\tStack\tTask#\tName\r\n\%s\r\n", taskList);
		portFree(taskList);
	}
	else{
		serial_printf(serial, "Task Info\r\nOut Of Memory!\r\n");
	}
}

void GetVersion(Serial *serial, unsigned int argc, char **argv){
	serial_printf(serial, "major=\"%s\";minor=\"%s\";bugfix=\"%s\";serial=\"%s\"\r\n",
      MAJOR_REV_STR,
      MINOR_REV_STR,
      BUGFIX_REV_STR,
      cpu_get_serialnumber()
      );
}

void ResetSystem(Serial *serial, unsigned int argc, char **argv){
	cpu_reset(0);
}
