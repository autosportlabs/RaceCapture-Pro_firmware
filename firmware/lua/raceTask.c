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
#include "accelerometer.h"
#include "math.h"
#include "geometry.h"

#define MIN_SPEED 25
#define STARTING_CORNER_GFORCE 0.1
#define STARTING_ACCEL_GFORCE 0.1
#define STARTING_BRAKING_GFORCE -0.1
#define START_FINISH_TIME_THRESHOLD 30

#define RACE_STACK_SIZE 2000
#define RACE_PRIORITY 2

#define FREQ_30Hz 10
#define FREQ_20Hz 15
#define FREQ_10Hz 30
#define FREQ_5Hz 60
#define FREQ_1Hz 300

#define DRIVER_CMD "driver"

#define HASHTAG "#chumpcar"

#define Y_AXIS 1
#define Y_AXIS_INVERT -1
#define X_AXIS 0
#define X_AXIS_INVERT 1

static float g_startFinishLatitude = 0;
static float g_startFinishLongitude = 0;
static float g_startFinishRadius = 0;

static float g_lastStartFinishTimestamp = 0;
static int g_prevAtStartFinish = 0;

static float g_topSpeed = MIN_SPEED;
static float g_topLapTime = 0;

static float g_maxCornerGforce = STARTING_CORNER_GFORCE;

static float g_maxBrakingGforce = STARTING_BRAKING_GFORCE;
static float g_maxAccelGforce = STARTING_ACCEL_GFORCE;


//driver info
static float g_lastLapTime = 0;
static float g_timeStartSec = 0;
static int g_currentLapCount = 0;

static char g_currentDriver[30] = "My driver";

static float g_personalTopSpeed = MIN_SPEED;
static float g_personalTopLapTime = 0;

static float g_personalMaxCornerGforce = STARTING_CORNER_GFORCE;
static float g_personalMaxBrakingGforce = STARTING_BRAKING_GFORCE;
static float g_personalMaxAccelGforce = STARTING_ACCEL_GFORCE;

static char g_tweet[200];
static char g_twitterNumber[20] = "2068544508";


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
	sendText(g_twitterNumber,tweet);
}

static void strcati(char *dest, int i){
	char tmp[20];
	modp_itoa10(i,tmp);
	strcat(dest,tmp);
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
	strcat(g_tweet," finished his driving stint! Total driving time: ");
	strcatf(g_tweet,elapsed / 60.0);
	strcat(g_tweet," mins. ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);

	strcpy(g_tweet,"Based on my calcs " );
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet,"'s top speed was ");
	strcatf(g_tweet, g_personalTopSpeed);
	strcat(g_tweet," kph and best lap time was ");
	strcatf(g_tweet, g_personalTopLapTime);
	strcat(g_tweet," minutes ");
	strcat(g_tweet," and did ");
	strcati(g_tweet,g_currentLapCount);
	strcat(g_tweet," laps ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static char * getRandomWish(){
	return "Good Luck!";
}

static void handleNewDriver(const char *txt){

	g_timeStartSec = getSecondsSinceMidnight();
	g_personalTopSpeed = MIN_SPEED;
	g_lastLapTime = 0;
	g_personalTopLapTime = 0;
	g_lastStartFinishTimestamp = 0;
	g_currentLapCount = 0;
	g_personalMaxCornerGforce = STARTING_CORNER_GFORCE;
	g_personalMaxBrakingGforce = STARTING_BRAKING_GFORCE;
	g_personalMaxAccelGforce = STARTING_ACCEL_GFORCE;


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

	if (strncmp(p,DRIVER_CMD,6) == 0){
		handleOldDriver();
		handleNewDriver(p+7);
	}
}


static char * getRandomExclamation(){

	return "Sweet!";
}

static void tweetPersonalTopSpeed(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just bested his personal top speed! ");
	strcatf(g_tweet,g_personalTopSpeed);
	strcat(g_tweet," kph ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetOverallTopSpeed(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet,"!!! ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just claimed overall top speed! ");
	strcatf(g_tweet,g_topSpeed);
	strcat(g_tweet," kph ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetTopPersonalLapTime(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just bested his personal lap time! ");
	strcatf(g_tweet,g_personalTopLapTime);
	strcat(g_tweet," mins ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetTopLapTime(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just grabbed overall best lap time! ");
	strcatf(g_tweet,g_topLapTime);
	strcat(g_tweet," mins ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetMaxCorneringForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," snagged highest overall cornering GForce of ");
	strcatf(g_tweet,g_maxCornerGforce);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetMaxBrakingForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," snagged highest overall braking GForce of ");
	strcatf(g_tweet,g_maxBrakingGforce);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetMaxAccelForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," snagged highest overall acceleration GForce of ");
	strcatf(g_tweet,g_maxAccelGforce);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}


static void tweetMaxPersonalCorneringForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," got highest personal cornering GForce of ");
	strcatf(g_tweet,g_personalMaxCornerGforce);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetMaxPersonalBrakingForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," got highest personal GForce Braking of ");
	strcatf(g_tweet,g_personalMaxBrakingGforce);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetMaxPersonalAccelForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," got highest personal GForce Acceleration of ");
	strcatf(g_tweet,g_personalMaxAccelGforce);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetCurrentLapTime(void){
	strcpy(g_tweet,g_currentDriver);
	strcat(g_tweet,"'s lap #");
	strcati(g_tweet,g_currentLapCount);
	strcat(g_tweet," time is: ");
	strcatf(g_tweet,g_lastLapTime);
	strcat(g_tweet," mins ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}


static void doSpeedChecks(void){

	float speed = getGPSVelocity();
	if (speed > g_personalTopSpeed){
		g_personalTopSpeed = speed;
		tweetPersonalTopSpeed();
	}
	if (speed > g_topSpeed){
		g_topSpeed = speed;
		tweetOverallTopSpeed();
	}
}


static void doGForceChecks(void){


	//handle Accelerometer
	float xGforce = convertAccelRawToG(readAccelAxis(X_AXIS),DEFAULT_ACCEL_ZERO);
	float yGforce = convertAccelRawToG(readAccelAxis(Y_AXIS),DEFAULT_ACCEL_ZERO);

	xGforce = xGforce * X_AXIS_INVERT;
	yGforce = yGforce * Y_AXIS_INVERT;

	xGforce = fabsf(xGforce);

	//overall
	if (xGforce > g_maxCornerGforce){
		g_maxCornerGforce = xGforce;
		tweetMaxCorneringForce();
	}

	if (yGforce < g_maxBrakingGforce){
		g_maxBrakingGforce = yGforce;
		tweetMaxBrakingForce();
	}

	if (yGforce > g_maxAccelGforce){
		g_maxAccelGforce = yGforce;
		tweetMaxAccelForce();
	}

	//personal

	if (xGforce > g_personalMaxCornerGforce){
		g_personalMaxCornerGforce = xGforce;
		tweetMaxPersonalCorneringForce();
	}

	if (yGforce < g_personalMaxBrakingGforce){
		g_personalMaxBrakingGforce = yGforce;
		tweetMaxPersonalBrakingForce();
	}

	if (yGforce > g_personalMaxAccelGforce){
		g_personalMaxAccelGforce = yGforce;
		tweetMaxPersonalAccelForce();
	}
}


static int atStartFinish(){

	struct circ_area area;
	struct point p;
	p.x = g_startFinishLongitude;
	p.y = g_startFinishLatitude;

	struct point currentP;
	currentP.x = getLongitude();
	currentP.y = getLatitude();

	create_circ(&area,&p,g_startFinishRadius);
	int within =  within_circ(&area,&currentP);
/*	SendFloat(p.x,10);
	SendString(" ");
	SendFloat(p.y,10);
	SendString(" within: ");
	SendInt(within);
	SendString(" ");
	SendFloat(g_startFinishRadius,10);
	SendCrlf();
	*/
	g_startFinishRadius = g_startFinishRadius * 1.1;
	return within;
}

static void doNewLap(float elapsed){

	float lapTime = elapsed / 60.0;

	if (g_topLapTime == 0){
		g_topLapTime = lapTime;
		tweetTopLapTime();
	}
	else{
		if (lapTime < g_topLapTime){
			g_topLapTime = lapTime;
			tweetTopLapTime();
		}
	}

	if (g_personalTopLapTime == 0){
		g_personalTopLapTime = lapTime;
		tweetTopPersonalLapTime();
	}
	else{
		if (lapTime < g_personalTopLapTime){
			g_personalTopLapTime = lapTime;
			tweetTopPersonalLapTime();
		}
	}
	g_lastLapTime = lapTime;
	tweetCurrentLapTime();
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
			if (rc == 0){
				cellModemInited = 1;
				g_timeStartSec = getSecondsSinceMidnight();
			}
		}
		else{
			ToggleLED(LED2);
			const char *txt = receiveText(1);
			if (NULL != txt){
				handleTxt(txt);
				deleteInbox();
			}

			doSpeedChecks();

			doGForceChecks();

			if (atStartFinish()){
				if (g_prevAtStartFinish == 0){
					if (g_lastStartFinishTimestamp == 0){
						g_lastStartFinishTimestamp = getSecondsSinceMidnight();
					}
					else{
						float currentTimestamp = getSecondsSinceMidnight();
						float elapsed = getTimeDiff(g_lastStartFinishTimestamp,currentTimestamp);
						if (elapsed > START_FINISH_TIME_THRESHOLD){
							doNewLap(elapsed);
							g_lastStartFinishTimestamp = currentTimestamp;
						}
					}
				}
				g_prevAtStartFinish = 1;
			}
			else{
				g_prevAtStartFinish = 0;
			}

			deleteSent();
		}

		vTaskDelayUntil( &xLastWakeTime, xFrequency );

	}

}


void setStartFinishPoint(float latitude,float longitude,float radius){
	g_startFinishLatitude = latitude;
	g_startFinishLongitude = longitude;
	g_startFinishRadius = radius;
}

float getStartFinishLatitude(void){
	return g_startFinishLatitude;
}

float getStartFinishLongitude(void){
	return g_startFinishLongitude;
}

float getStartFinishRadius(void){
	return g_startFinishRadius;
}

void setTweetNumber(const char *number){
	strcpy(g_twitterNumber,number);
}


