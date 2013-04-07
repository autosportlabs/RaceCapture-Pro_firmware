#include "sdcard.h"
#include "mod_string.h"
#include "modp_numtoa.h"
#include "FreeRTOS.h"
#include "task.h"
#include "loggerHardware.h"


static FATFS Fatfs[1];

void InitFSHardware(void){
	disk_init_hardware();
}

int InitFS(){
	taskENTER_CRITICAL();
	int res = disk_initialize(0);
	if (0 == res){
		res = f_mount(0, &Fatfs[0]);
	}
	taskEXIT_CRITICAL();
	return res;
}

int UnmountFS(){
	return f_mount(0,NULL);
}

static FIL fatFile;

void TestSDWrite(Serial *serial, int lines,int doFlush, int quiet)
{

	int res = 0;
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
	res = f_open(&fatFile,"test1.txt", FA_WRITE | FA_CREATE_ALWAYS);
	if (!quiet){
		put_int(serial, res);
		put_crlf(serial);
	}
	if (res) goto exit;

	if (!quiet) serial->put_s("Writing file..");
	portTickType startTicks = xTaskGetTickCount();
	while (lines--){
		res = f_puts("The quick brown fox jumped over the lazy dog\n",&fatFile);
		if (doFlush) res = f_sync(&fatFile);
	}
	portTickType endTicks = xTaskGetTickCount();

	if (!quiet){
		serial->put_s("Ticks to write: ");
		put_int(serial, endTicks - startTicks);
		put_crlf(serial);
		serial->put_s("Closing... ");
	}

	res = f_close(&fatFile);
	if (!quiet){
		put_int(serial, res);
		put_crlf(serial);
	}
	if (res) goto exit;

	if (!quiet){
		serial->put_s("Unmounting... ");
	}
	res = UnmountFS();
	if (!quiet){
		put_int(serial, res);
		put_crlf(serial);
	}
exit:
	if(res == 0){
		serial->put_s("SUCCESS\r\n");
	}
	else{
		serial->put_s("ERROR ");
		put_int(serial, res);
		put_crlf(serial);
	}
}
