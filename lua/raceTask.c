#include "raceTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "portable.h"
#include "cellModem.h"
#include "loggerHardware.h"
#include "gps.h"
#include "modp_numtoa.h"
#include "usb_comm.h"
#include "string.h"


#define MIN_SPEED 25

#define RACE_STACK_SIZE 2000
#define RACE_PRIORITY 2

#define FREQ_30Hz 10
#define FREQ_20Hz 15
#define FREQ_10Hz 30
#define FREQ_5Hz 60
#define FREQ_1Hz 300

#define DRIVER_CMD "driver"

#define HASHTAG "#chumpcar"



static float g_topSpeed = MIN_SPEED;
static float g_timeStartSec = 0;
static float g_lastLapTime = 0;
static float g_topLapTime = 0;
static char g_currentDriver[30] = "driver";

static float g_personalTopSpeed = MIN_SPEED;
static float g_personalTopLapTime = 0;

static char g_tweet[200];

void startRaceTask(void){

	xTaskCreate( raceTask,
					( signed portCHAR * ) "raceTask",
					RACE_STACK_SIZE,
					NULL,
					RACE_PRIORITY,
					NULL);
}

static void lower(const char *pstr){
	for(char *p = pstr;*p;++p) *p=*p>0x40&&*p<0x5b?*p|0x60:*p;
}

static void tweet(const char *tweet){
	sendText("40404",tweet);
}

static void strcatf(char *dest, float f){

	char tmp[20];
	modp_ftoa(f,tmp,2);
	strcat(dest,tmp);
}

static void handleOldDriver(){

	float elapsed = getTimeSince(g_timeStartSec);
	strcpy(g_tweet,"Hey! ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," finished their driving stint! Total driving time: ");
	strcatf(g_tweet,elapsed / 60.0);
	strcat(g_tweet," mins. ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);

	strcpy(g_tweet,"Based on my calculations " );
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet,"'s top speed was ");
	strcatf(g_tweet, g_topSpeed);
	strcat(g_tweet," kph and best lap time was ");
	strcatf(g_tweet, g_topLapTime);
	strcat(g_tweet," minutes ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static char * getRandomWish(){
	return "Good Luck!";
}

static void handleNewDriver(const char *txt){

	g_timeStartSec = getSecondsSinceMidnight();
	g_personalTopSpeed = -10 ; //MIN_SPEED;
	g_lastLapTime = 0;
	g_personalTopLapTime = 0;

	strcpy(g_tweet,"Looks like ");
	strcat(g_tweet,txt);
	strcat(g_tweet," is driving me now! ");
	strcat(g_tweet,getRandomWish());
	strcat(g_tweet," ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
	strcpy(g_currentDriver,txt);
}


static void handleTxt(const char *txt){

	const char *p = txt;
	lower(p);
//	SendString(txt);
//	SendCrlf();

	if (strncmp(p,DRIVER_CMD,6) == 0){
		handleOldDriver();
		handleNewDriver(p+7);
	}
}


static char * getRandomExclamation(){

	return "Sweet!";
}

static void tweetPersonalTopSpeed(){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just bested his personal top speed! ");
	strcatf(g_tweet,g_personalTopSpeed);
	strcat(g_tweet," kph ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetOverallTopSpeed(){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet,"!!! ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just claimed overall top speed! ");
	strcatf(g_tweet,g_topSpeed);
	strcat(g_tweet," kph ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetPersonalTopLapTime(){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just bested his personal lap time! ");
	strcatf(g_tweet,g_personalTopLapTime);
	strcat(g_tweet," mins ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetOverallTopLapTime(){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just grabbed overall best lap time! ");
	strcatf(g_tweet,g_topLapTime);
	strcat(g_tweet," mins ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}


float calculateCurrentLapTime(){

	return -10.5;
}

void raceTask(void *params){


	int cellModemInited = 0;

	EnableLED(LED2);
	while(1){
		portTickType xLastWakeTime, startTickTime;
		const portTickType xFrequency = FREQ_5Hz;
		startTickTime = xLastWakeTime = xTaskGetTickCount();

		if (!cellModemInited){
			int rc = initCellModem();
			if (rc == 0) cellModemInited = 1;
		}
		else{
			ToggleLED(LED2);
			const char *txt = receiveText(1);
			if (NULL != txt){
				handleTxt(txt);
				deleteAllTexts();
			}
		}
		float speed = getGPSVelocity();
		if (speed > g_personalTopSpeed){
			g_personalTopSpeed = speed;
			tweetPersonalTopSpeed();
		}
		if (speed > g_topSpeed){
			g_topSpeed = speed;
			tweetOverallTopSpeed();
		}


		float lapTime = calculateCurrentLapTime();

		if (lapTime < g_personalTopLapTime){
			g_personalTopLapTime = lapTime;
			tweetPersonalTopLapTime();
		}

		if (lapTime < g_topLapTime){
			g_topLapTime = lapTime;
			tweetOverallTopLapTime();
		}


		vTaskDelayUntil( &xLastWakeTime, xFrequency );

	}

}
