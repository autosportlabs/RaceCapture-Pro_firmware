#ifndef SDCARD_H_
#define SDCARD_H_

#include "ff.h"
#include "diskio.h"
#include "serial.h"

void createDiskTimerTask();
void diskTimerTask(void *params);
void TestSDWrite(Serial *serial, int lines,int doFlush, int quiet);
void InitFSHardware(void);
int InitFS();
int UnmountFS();
int OpenNextLogFile(FIL *f);

#endif /*SDCARD_H_*/
