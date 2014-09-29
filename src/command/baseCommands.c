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
	serial->put_s("== Memory Info ==\r\n");
	serial->put_s("Total memory         : ");
	put_uint(serial, (unsigned int)&_CONFIG_HEAP_SIZE);
	put_crlf(serial);
	serial->put_s("Free memory          : ");
	put_uint(serial, portGetFreeHeapSize());
	put_crlf(serial);
	serial->put_s("== Lua Info ==\r\n");
	lua_State *L = (lua_State *)getLua(); //TODO: this was done for unit testing. fix when Lua runtime is part of unit testing framework
	lua_gc(L,LUA_GCCOLLECT,0);
	serial->put_s("Lua Top              : ");
	put_int(serial, lua_gettop(L));
	put_crlf(serial);
	serial->put_s("Lua memory usage(Kb) : ");
	put_int(serial, lua_gc(L,LUA_GCCOUNT,0));
	put_crlf(serial);
	serial->put_s("== Misc ==\r\n");
	serial->put_s("sizeof LoggerConfig  : ");
	put_int(serial, sizeof(LoggerConfig));
	put_crlf(serial);
	serial->put_s("sizeof ChannelSample : ");
	put_int(serial, sizeof(ChannelSample));
	put_crlf(serial);
}

void ShowTaskInfo(Serial *serial, unsigned int argc, char **argv){
	serial->put_s("Task Info");
	put_crlf(serial);
	serial->put_s("Status\tPri\tStack\tTask#\tName");
	put_crlf(serial);
	char *taskList = (char *)portMalloc(1024);
	if (NULL != taskList){
		vTaskList(taskList);
		serial->put_s(taskList);
		portFree(taskList);
	}
	else{
		serial->put_s("Out of Memory!");
	}
	put_crlf(serial);
}

void GetVersion(Serial *serial, unsigned int argc, char **argv){
	put_nameString(serial, "major", MAJOR_REV_STR);
	put_nameString(serial, "minor", MINOR_REV_STR);
	put_nameString(serial, "bugfix", BUGFIX_REV_STR);
	put_nameString(serial, "serial", cpu_get_serialnumber());
}

void ResetSystem(Serial *serial, unsigned int argc, char **argv){
	cpu_reset();
}

