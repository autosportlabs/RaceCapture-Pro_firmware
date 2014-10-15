#include "sdcard.h"
#include "mod_string.h"
#include "modp_numtoa.h"
#include "FreeRTOS.h"
#include "task.h"
#include "taskUtil.h"
#include "loggerHardware.h"
#include "watchdog.h"
#include "diskio.h"
#include "sdcard_device.h"
#include "mem_mang.h"

static FATFS *FatFs;

void InitFSHardware(void){
	disk_init_hardware();
}

int InitFS(){
	int res = -1;
	FatFs = pvPortMalloc(sizeof(FATFS));
	if (FatFs){
		taskENTER_CRITICAL();
		res = disk_initialize(0);
		taskEXIT_CRITICAL();
		if (0 == res) {
			res = f_mount(FatFs, "0", 1);
		}
	}
	return res;
}

int UnmountFS(){
	return f_mount(NULL, "0", 1);
}

void TestSDWrite(Serial *serial, int lines, int doFlush, int quiet)
{
	int res = 0;
	FIL *fatFile = NULL;

	fatFile = pvPortMalloc(sizeof(FIL));
	if (NULL == fatFile){
		if (!quiet) serial->put_s("could not allocate file object\r\n");
		goto exit;
	}

	if (!quiet){
		serial->put_s("Test Write: Lines: ");
		put_int(serial, lines);
		put_crlf(serial);
		serial->put_s("Flushing Enabled: " );
		put_int(serial, doFlush);
		put_crlf(serial);
		serial->put_s("Card Init... ");
	}
	res = InitFS();
	if (res) goto exit;

	if (!quiet){
		put_int(serial, res);
		put_crlf(serial);
		serial->put_s("Opening File... ");
	}
	res = f_open(fatFile,"test1.txt", FA_WRITE | FA_CREATE_ALWAYS);
	if (!quiet){
		put_int(serial, res);
		put_crlf(serial);
	}
	if (res) goto exit;

	if (!quiet) serial->put_s("Writing file..");
	portTickType startTicks = xTaskGetTickCount();
	for (int i = 1; i <= lines; i++){
		res = f_puts("The quick brown fox jumped over the lazy dog\n",fatFile);
		if (doFlush) f_sync(fatFile);
		if (res == EOF){
			if (!quiet) serial->put_s("failed writing at line ");
			put_int(serial, i);
			serial->put_s("(");
			put_int(serial, res);
			serial->put_s(")");
			put_crlf(serial);
			goto exit;
		}
		watchdog_reset();
	}
	portTickType endTicks = xTaskGetTickCount();

	if (!quiet){
		serial->put_s("Ticks to write: ");
		put_int(serial, endTicks - startTicks);
		put_crlf(serial);
		serial->put_s("Closing... ");
	}

	res = f_close(fatFile);
	if (!quiet){
		put_int(serial, res);
		put_crlf(serial);
	}
	if (res) goto exit;

	if (!quiet)		serial->put_s("Unmounting... ");
	res = UnmountFS();
	if (!quiet){
		put_int(serial, res);
		put_crlf(serial);
	}
exit:
	if(res == 0){
		if (!quiet) serial->put_s("SUCCESS\r\n");
	}
	else{
		if (!quiet){
			serial->put_s("ERROR ");
			put_int(serial, res);
			put_crlf(serial);
		}
	}
	if (fatFile != NULL) vPortFree(fatFile);
}
