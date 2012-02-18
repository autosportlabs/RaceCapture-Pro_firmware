#include "sdcard.h"
#include "usb_comm.h"
#include "string.h"
#include "modp_numtoa.h"
#include "FreeRTOS.h"
#include "task.h"
#include "loggerHardware.h"

#define MAX_LOG_FILE_INDEX 99999
#define FREQ_100Hz 3
#define DISK_TIMER_PRIORITY ( tskIDLE_PRIORITY + 4 )
#define DISK_TIMER_STACK_SIZE 20
static FATFS Fatfs[1];

void createDiskTimerTask(){
	xTaskCreate( diskTimerTask,( signed portCHAR * ) "diskTask",DISK_TIMER_STACK_SIZE, NULL, DISK_TIMER_PRIORITY, NULL );
}

void diskTimerTask(void *params){
	while(1){
		portTickType xLastWakeTime = xTaskGetTickCount();
		disk_timerproc();
		vTaskDelayUntil( &xLastWakeTime, FREQ_100Hz );
	}
}

int InitEFS(){
	int res = disk_initialize(0);
	if (0 != res) return res;
	res = f_mount(0, &Fatfs[0]);
	return res;
}

int UnmountEFS(){
	return f_mount(0,NULL);
}

int OpenNextLogFile(FIL *f){

	char filename[13];
	int i = 0;
	int rc;
	for (; i < MAX_LOG_FILE_INDEX; i++){
		strcpy(filename,"rc_");
		char numBuf[12];
		modp_itoa10(i,numBuf);
		strcat(filename,numBuf);
		strcat(filename,".log");
		rc = f_open(f,filename, FA_WRITE | FA_CREATE_NEW);
		if ( rc == 0 ) break;
		f_close(f);
	}
	if (i >= MAX_LOG_FILE_INDEX) return -2;
	return rc;
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
	int res = InitEFS();
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
	res = UnmountEFS();
	SendInt(res);
	SendCrlf();
}
