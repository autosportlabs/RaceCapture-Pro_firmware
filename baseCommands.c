/*
 * baseCommands.c
 *
 *  Created on: Jul 23, 2011
 *      Author: brent
 */
#include "baseCommands.h"
#include "usb_comm.h"

static struct cmd_t testTaskCmd = {"testCommand","Help Info",TestCommand,NULL};
static struct cmd_t taskInfoCmd ={"showTasks", "Show status of running tasks", ShowTaskInfo, NULL};

void InitBaseCommands(){

	InitCommandParser(COMMAND_PROMPT,WELCOME_MSG);
	AppendCommand(&taskInfoCmd);
	AppendCommand(&testTaskCmd);
}

void TestCommand(unsigned char argc, char **argv){

	SendString("Test!");
	SendCrlf();
}

void ShowTaskInfo(unsigned char argc, char **argv){

	SendString("Task Info");
	SendCrlf();
	SendString("Status\tPri\tStack\tTask#\tName");
	SendCrlf();
	char taskList[200];
	vTaskList(taskList);
	SendString(taskList);
	SendCrlf();
}
