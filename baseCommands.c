/*
 * baseCommands.c
 *
 *  Created on: Jul 23, 2011
 *      Author: brent
 */
#include "baseCommands.h"
#include "FreeRTOS.h"
#include "task.h"

void ShowTaskInfo(Serial *serial, unsigned int argc, char **argv){

	serial->put_s("Task Info");
	put_crlf(serial);
	serial->put_s("Status\tPri\tStack\tTask#\tName");
	put_crlf(serial);
	char *taskList = (char *)pvPortMalloc(1024);
	if (NULL != taskList){
		vTaskList(taskList);
		serial->put_s(taskList);
		vPortFree(taskList);
	}
	else{
		serial->put_s("Out of Memory!");
	}
	put_crlf(serial);
}

void GetVersion(Serial *serial, unsigned int argc, char **argv){
	put_nameString(serial, "major", MAJOR_REV);
	put_nameString(serial, "minor", MINOR_REV);
	put_nameString(serial, "bugfix", BUGFIX_REV);
}
