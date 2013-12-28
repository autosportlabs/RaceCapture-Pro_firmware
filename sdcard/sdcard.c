#include "sdcard.h"
#include "mod_string.h"
#include "modp_numtoa.h"
#include "FreeRTOS.h"
#include "task.h"
#include "loggerHardware.h"
#include "spi.h"

static FATFS Fatfs[1];

void InitFSHardware(void){
	disk_init_hardware();
}

int InitFS(){
	taskENTER_CRITICAL();
	int res = disk_initialize(0);
	if (0 == res){
		res = f_mount(&Fatfs[0], "0", 1);
	}
	taskEXIT_CRITICAL();
	return res;
}

int UnmountFS(){
	return f_mount(NULL, "0", 1);
}

static FIL fatFile;

void TestSDWrite(Serial *serial, int lines, int doFlush, int quiet, int delay)
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
	lock_spi();
	res = InitFS();
	unlock_spi();
	if (res) goto exit;

	if (!quiet){
		put_int(serial, res);
		put_crlf(serial);
		serial->put_s("Opening File... ");
	}
	lock_spi();
	res = f_open(&fatFile,"test1.txt", FA_WRITE | FA_CREATE_ALWAYS);
	unlock_spi();
	if (!quiet){
		put_int(serial, res);
		put_crlf(serial);
	}
	if (res) goto exit;

	if (!quiet) serial->put_s("Writing file..");
	portTickType startTicks = xTaskGetTickCount();
	for (size_t i = 1; i <= lines; i++){
		delayMs(delay);
		lock_spi();
		res = f_puts("The quick brown fox jumped over the lazy dog\n",&fatFile);
		if (doFlush) f_sync(&fatFile);
		unlock_spi();
		if (res == EOF){
			serial->put_s("failed writing at line ");
			put_int(serial, i);
			serial->put_s("(");
			put_int(serial, res);
			serial->put_s(")");
			put_crlf(serial);
			goto exit;
		}
		ResetWatchdog();
	}
	portTickType endTicks = xTaskGetTickCount();

	if (!quiet){
		serial->put_s("Ticks to write: ");
		put_int(serial, endTicks - startTicks);
		put_crlf(serial);
		serial->put_s("Closing... ");
	}

	lock_spi();
	res = f_close(&fatFile);
	unlock_spi();
	if (!quiet){
		put_int(serial, res);
		put_crlf(serial);
	}
	if (res) goto exit;

	if (!quiet){
		serial->put_s("Unmounting... ");
	}
	lock_spi();
	res = UnmountFS();
	unlock_spi();
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
