#ifndef SDCARD_H_
#define SDCARD_H_
#include "serial.h"
#include "ff.h"

void TestSDWrite(Serial *serial, int lines,int doFlush, int quiet, int delay);
void InitFSHardware(void);
int InitFS();
int UnmountFS();
int OpenNextLogFile(FIL *f);

#endif /*SDCARD_H_*/
