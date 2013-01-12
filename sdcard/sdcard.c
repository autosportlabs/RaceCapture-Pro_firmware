#include "sdcard.h"
#include "usb_comm.h"
#include "string.h"
#include "modp_numtoa.h"
#include "FreeRTOS.h"
#include "task.h"
#include "loggerHardware.h"
#include "usb_comm.h"


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

void TestSDWrite(Serial *serial, int lines,int doFlush)
{

	serial->put_s("sizeof long ");
	put_int(serial, sizeof(long));
	put_crlf(serial);
	serial->put_s("Test Write: Lines: ");
	put_int(serial, lines);
	put_crlf(serial);
	serial->put_s("Flushing Enabled: " );
	put_int(serial, doFlush);
	put_crlf(serial);

	serial->put_s("Card Init...");
	int res = InitFS();
	put_int(serial, res);
	put_crlf(serial);

	if (res !=0) return;

	serial->put_s("Drive Mount...");
	put_int(serial, res);
	put_crlf(serial);
	if (res !=0) return;

	serial->put_s("Opening File..");
	res = f_open(&fatFile,"test1.txt", FA_WRITE | FA_CREATE_ALWAYS);
	put_int(serial, res);
	put_crlf(serial);
	if (res !=0) return;

	serial->put_s("Writing file..");
	portTickType startTicks = xTaskGetTickCount();
	while (lines--){
//		SendString("chars: ");
		res = f_puts("The quick brown fox jumped over the lazy dog\n",&fatFile);
//		SendInt(res);
//		SendString("; sync: " );
		if (doFlush) res = f_sync(&fatFile);
//		SendInt(res);
//		SendCrlf();
	}
	portTickType endTicks = xTaskGetTickCount();

	serial->put_s("Ticks to write: ");
	put_int(serial, endTicks - startTicks);
	put_crlf(serial);
	res = f_close(&fatFile);

	serial->put_s("Unmounting...");
	res = UnmountFS();
	put_int(serial, res);
	put_crlf(serial);
}
