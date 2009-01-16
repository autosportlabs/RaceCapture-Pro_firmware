#ifndef SDCARD_H_
#define SDCARD_H_

#include "efs.h"
#include "ls.h"
#include "mkfs.h"
#include "interfaces/efsl_dbg_printf_arm.h"

void ListRootDir(void);
int InitEFS();
int UnmountEFS();
int OpenNextLogFile(EmbeddedFile *f);

#endif /*SDCARD_H_*/
