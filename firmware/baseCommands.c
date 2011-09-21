/*
 * baseCommands.c
 *
 *  Created on: Jul 23, 2011
 *      Author: brent
 */
#include "baseCommands.h"
#include "usb_comm.h"
#include "task.h"



void ShowTaskInfo(unsigned int argc, char **argv){

	SendString("Task Info");
	SendCrlf();
	SendString("Status\tPri\tStack\tTask#\tName");
	SendCrlf();
	char taskList[200];
	vTaskList(taskList);
	SendString(taskList);
	SendCrlf();
}
