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
	//TODO - we need to re-init the hardware in cases of
	//write once at SD insertion / fresh reset
	//remove / re-insert SD
	//attempt to write again. (error would occur)
	//the re-init of the hardware seems to solve this.
	//Probably not needing the SPI init, but init around the SD card.
	//Investigate splitting this out into a better SD card init
	disk_init_hardware();
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

void TestSDWrite(int lines,int doFlush)
{

	SendString("sizeof long ");
	SendInt(sizeof(long));
	SendCrlf();
	SendString("Test Write: Lines: ");
	SendInt(lines);
	SendCrlf();
	SendString("Flushing Enabled: " );
	SendInt(doFlush);
	SendCrlf();

	SendString("Card Init...");
	int res = InitFS();
	SendInt(res);
	SendCrlf();

	if (res !=0) return;

	SendString("Drive Mount...");
	SendInt(res);
	SendCrlf();
	if (res !=0) return;

	SendString("Opening File..");
	res = f_open(&fatFile,"test1.txt", FA_WRITE | FA_CREATE_ALWAYS);
	SendInt(res);
	SendCrlf();
	if (res !=0) return;

	SendString("Writing file..");
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

	SendString("Ticks to write: ");
	SendInt(endTicks - startTicks);
	SendCrlf();
	res = f_close(&fatFile);

	SendString("Unmounting...");
	res = UnmountFS();
	SendInt(res);
	SendCrlf();
}
