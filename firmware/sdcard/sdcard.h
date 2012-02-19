#ifndef SDCARD_H_
#define SDCARD_H_

#include "ff.h"
#include "diskio.h"

void createDiskTimerTask();
void diskTimerTask(void *params);
void TestSDWrite(int lines,int doFlush);
int InitFS();
int UnmountFS();
int OpenNextLogFile(FIL *f);

#endif /*SDCARD_H_*/
