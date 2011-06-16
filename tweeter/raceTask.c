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

#define MIN_SPEED 50
#define STARTING_CORNER_GFORCE 0.1
#define STARTING_ACCEL_GFORCE 0.1
#define STARTING_BRAKING_GFORCE -0.1
#define START_FINISH_TIME_THRESHOLD 30

#define RACE_STACK_SIZE 1000
#define RACE_PRIORITY 2

#define TEXT_SENDER_STACK_SIZE 100
#define TEXT_SENDER_PRIORITY 2

#define FREQ_30Hz 10
#define FREQ_20Hz 15
#define FREQ_10Hz 30
#define FREQ_5Hz 60
#define FREQ_1Hz 300

#define NOTIFY_INTERVAL 600

#define DRIVER_CMD "driver"
#define WHOAMI "@asl_labrat1"
#define HASHTAG "#chumpcar"
#define COLOR_CMD "color"

#define Y_AXIS 1
#define Y_AXIS_INVERT -1
#define X_AXIS 0
#define X_AXIS_INVERT 1

static float g_startFinishLatitude = 0;
static float g_startFinishLongitude = 0;
static float g_startFinishRadius = 0;

static int g_prevAtStartFinish = 0;
static float g_lastStartFinishTimestamp = 0;

typedef struct{
	int updated;
	float value;
} trackablePoint;


static trackablePoint g_topSpeed = { .updated=0, .value=MIN_SPEED};
static trackablePoint g_topLapTime = { .updated=0, .value=0};
static trackablePoint g_maxCornerGforce = { .updated=0, .value=STARTING_CORNER_GFORCE};
static trackablePoint g_maxBrakingGforce = { .updated=0, .value=STARTING_BRAKING_GFORCE};
static trackablePoint g_maxAccelGforce = { .updated=0, .value=STARTING_ACCEL_GFORCE};

//driver info
static char g_currentDriver[30] = "My driver";

static float g_lastLapTime = 0;
static float g_timeStartSec = 0;
static float g_currentLapCount = 0;

static trackablePoint g_personalTopSpeed = { .updated=0, .value=MIN_SPEED};
static trackablePoint g_personalTopLapTime = { .updated=0, .value=0};

static trackablePoint g_personalMaxCornerGforce = { .updated=0, .value=STARTING_CORNER_GFORCE};
static trackablePoint g_personalMaxBrakingGforce = { .updated=0, .value=STARTING_BRAKING_GFORCE};
static trackablePoint g_personalMaxAccelGforce = { .updated=0, .value=STARTING_ACCEL_GFORCE};

#define TEXT_MSG_LEN 200
#define TEXT_MSG_QUEUE_LEN 10

static xQueueHandle textMsgTxQueue;
static xQueueHandle textMsgRxQueue;

static char g_txtSendBuffer[TEXT_MSG_LEN];

static char g_tweet[TEXT_MSG_LEN];
static char g_receivedText[TEXT_MSG_LEN];

static char g_twitterNumber[20] = "40404";

static float g_lastNotifiedTimestamp = 0;

void startRaceTask(void){

	textMsgTxQueue = xQueueCreate(TEXT_MSG_QUEUE_LEN,sizeof(signed char[TEXT_MSG_LEN]));
	if (NULL == textMsgTxQueue) return;

	textMsgRxQueue = xQueueCreate(TEXT_MSG_QUEUE_LEN,sizeof(signed char[TEXT_MSG_LEN]));
	if (NULL == textMsgRxQueue) return;

	xTaskCreate( raceTask,
					( signed portCHAR * ) "raceTask",
					RACE_STACK_SIZE,
					NULL,
					RACE_PRIORITY,
					NULL);

	xTaskCreate( textSenderTask,
			(signed portCHAR *) "textSender",
			TEXT_SENDER_STACK_SIZE,
			NULL,
			TEXT_SENDER_PRIORITY,
			NULL);
}


static void lower(const char *pstr){
	for(char *p = pstr;*p;++p) *p=*p>0x40&&*p<0x5b?*p|0x60:*p;
}

static void tweet(const char *tweet){
	xQueueSend( textMsgTxQueue, tweet, portMAX_DELAY );
}

static void strcati(char *dest, int i){
	char tmp[20];
	modp_itoa10(i,tmp);
	strcat(dest,tmp);
}

static float toMPH(float kph){
	return kph *  0.621371192;
}

static void strcatMinutes(char *dest, float fractionalMinutes){
    int whole = (int)fractionalMinutes;
    strcati(dest,whole);
    strcat(dest,":");
    fractionalMinutes = fractionalMinutes - whole;
    int mins = 60 * fractionalMinutes;
    if (mins < 10) strcat(dest,"0");
    strcati(dest,mins);
}

static void strcatf(char *dest, float f){

	char tmp[20];
	modp_ftoa(f,tmp,2);
	strcat(dest,tmp);
}

static char * getRandomExclamation(){

	return "Cool!";
}

static char * getRandomWish(){
	return "Drive Like the wind!";
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
	strcatf(g_tweet, g_personalTopSpeed.value);
	strcat(g_tweet," mph and best lap time was ");
	strcatf(g_tweet, g_personalTopLapTime.value);
	strcat(g_tweet," minutes ");
	strcat(g_tweet," and did ");
	strcati(g_tweet,g_currentLapCount);
	strcat(g_tweet," laps ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}



static void handleNewDriver(const char *txt){

	g_timeStartSec = getSecondsSinceMidnight();
	g_personalTopSpeed.value = MIN_SPEED;
	g_lastLapTime = 0;
	g_personalTopLapTime.value = 0;
	g_lastStartFinishTimestamp = 0;
	g_currentLapCount = 0;
	g_personalMaxCornerGforce.value = STARTING_CORNER_GFORCE;
	g_personalMaxBrakingGforce.value = STARTING_BRAKING_GFORCE;
	g_personalMaxAccelGforce.value = STARTING_ACCEL_GFORCE;


	strcpy(g_tweet,"Looks like ");
	strcat(g_tweet,txt);
	strcat(g_tweet," is driving me now! ");
	strcat(g_tweet,getRandomWish());
	strcat(g_tweet," ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
	strcpy(g_currentDriver,txt);
}

#define RED "red"
#define GREEN "green"
#define BLUE "blue"
#define WHITE "white"
#define OFF	"off"

static void setColor(const char *c){

	if (strlen(c) < 6) return;

	unsigned int red = 1;
	unsigned int green = 1;
	unsigned int blue = 1;

	if (strncmp(c,RED,strlen(RED))==0){
		red = 99;
		green = 1;
		blue = 1;
	}
	else if (strncmp(c,GREEN,strlen(GREEN))==0){
		red = 1;
		green = 99;
		blue = 1;
	}
	else if (strncmp(c,BLUE,strlen(BLUE))==0){
		red = 1;
		green = 1;
		blue = 99;
	}
	else if (strncmp(c,OFF,strlen(OFF)) == 0){
		red = 1;
		green = 1;
		blue = 1;
	}
	else if (strncmp(c,WHITE,strlen(WHITE)) == 0){
		red = 99;
		green = 99;
		blue = 99;
	}
	else{

		char r[3];
		char g[3];
		char b[3];
		r[2] = '\0';
		g[2] = '\0';
		b[2] = '\0';
		memcpy(r,c,2);
		memcpy(g,c+2,2);
		memcpy(b,c+4,2);

		red = strtol(r,NULL,16);
		green = strtol(g,NULL,16);
		blue = strtol(b,NULL,16);
		red = (red * 100) / 256;
		green = (green * 100) / 256;
		blue = (blue * 100) / 256;
		if (red == 0) red = 1;
		if (green == 0) green = 1;
		if (blue == 0) blue = 1;
	}

	PWM_SetDutyCycle(1,(unsigned short)green);
	PWM_SetDutyCycle(2,(unsigned short)red);
	PWM_SetDutyCycle(3,(unsigned short)blue);
}


static void handleMention(const char *msg){

	if (strlen(msg)==0) return;
	if (strncmp(msg,COLOR_CMD,strlen(COLOR_CMD)) == 0){
		setColor(msg + strlen(COLOR_CMD) + 1);
	}
}

#define LZERO "lzero"
#define LONE "lone"

static void handleTxt(const char *txt){

	const char *p = txt;
	lower(p);

	if (strncmp(p,DRIVER_CMD,strlen(DRIVER_CMD)) == 0){
		handleOldDriver();
		handleNewDriver(p+7);
	}
	if (strncmp(p,LZERO,strlen(LZERO)) == 0){
		PWM_SetDutyCycle(1,0);
		PWM_SetDutyCycle(2,0);
		PWM_SetDutyCycle(3,0);
	}
	if (strncmp(p,LONE,strlen(LONE)) == 0){
		PWM_SetDutyCycle(1,99);
		PWM_SetDutyCycle(2,99);
		PWM_SetDutyCycle(3,99);
	}
	char * whoami = strstr(txt,WHOAMI);
	if (NULL != whoami){
		handleMention(whoami+strlen(WHOAMI)+ 1);
	}
}


static void tweetPersonalTopSpeed(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just bested his personal top speed! ");
	strcatf(g_tweet,g_personalTopSpeed.value);
	strcat(g_tweet," mph ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetOverallTopSpeed(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just claimed team top speed! ");
	strcatf(g_tweet,g_topSpeed.value);
	strcat(g_tweet," mph ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetTopPersonalLapTime(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just bested his personal lap time! ");
	strcatMinutes(g_tweet,g_personalTopLapTime.value);
	strcat(g_tweet," mins ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetTopLapTime(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," just grabbed team best lap time! ");
	strcatMinutes(g_tweet,g_topLapTime.value);
	strcat(g_tweet," mins ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetMaxCorneringForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," snagged highest team cornering GForce of ");
	strcatf(g_tweet,g_maxCornerGforce.value);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetMaxBrakingForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," snagged highest team braking GForce of ");
	strcatf(g_tweet,g_maxBrakingGforce.value);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetMaxAccelForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," snagged highest team acceleration GForce of ");
	strcatf(g_tweet,g_maxAccelGforce.value);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}


static void tweetMaxPersonalCorneringForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," got highest personal cornering GForce of ");
	strcatf(g_tweet,g_personalMaxCornerGforce.value);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetMaxPersonalBrakingForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," got highest personal GForce Braking of ");
	strcatf(g_tweet,g_personalMaxBrakingGforce.value);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetMaxPersonalAccelForce(void){
	strcpy(g_tweet,getRandomExclamation());
	strcat(g_tweet," ");
	strcat(g_tweet,g_currentDriver);
	strcat(g_tweet," got highest personal GForce Acceleration of ");
	strcatf(g_tweet,g_personalMaxAccelGforce.value);
	strcat(g_tweet,"G ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void tweetCurrentLapTime(void){
	strcpy(g_tweet,g_currentDriver);
	strcat(g_tweet,"'s lap #");
	strcati(g_tweet,g_currentLapCount);
	strcat(g_tweet," time is: ");
	strcatMinutes(g_tweet,g_lastLapTime);
	strcat(g_tweet," mins ");
	strcat(g_tweet,HASHTAG);
	tweet(g_tweet);
}

static void notifyPending(void){

	if (g_topLapTime.updated){
		tweetTopLapTime();
		g_topLapTime.updated = 0;
	}
	if (g_personalTopLapTime.updated){
		tweetTopPersonalLapTime();
		g_personalTopLapTime.updated = 0;
	}
	if (g_maxCornerGforce.updated){
		tweetMaxCorneringForce();
		g_maxCornerGforce.updated = 0;
	}
	if (g_maxBrakingGforce.updated){
		tweetMaxBrakingForce();
		g_maxBrakingGforce.updated = 0;
	}
	if (g_maxAccelGforce.updated){
		tweetMaxAccelForce();
		g_maxAccelGforce.updated = 0;
	}
	if (g_personalMaxCornerGforce.updated){
		tweetMaxPersonalCorneringForce();
		g_personalMaxCornerGforce.updated = 0;
	}
	if (g_personalMaxBrakingGforce.updated){
		tweetMaxPersonalBrakingForce();
		g_personalMaxBrakingGforce.updated = 0;
	}
	if (g_personalMaxAccelGforce.updated){
		tweetMaxPersonalAccelForce();
		g_personalMaxAccelGforce.updated = 0;
	}
	if (g_personalTopSpeed.updated){
		tweetPersonalTopSpeed();
		g_personalTopSpeed.updated = 0;
	}
	if (g_topSpeed.updated){
		tweetOverallTopSpeed();
		g_topSpeed.updated = 0;
	}
}

static void doSpeedChecks(void){

	float speed = toMPH(getGPSVelocity());
	if (speed > g_personalTopSpeed.value){
		g_personalTopSpeed.value = speed;
		g_personalTopSpeed.updated = 1;
	}
	if (speed > g_topSpeed.value){
		g_topSpeed.value = speed;
		g_topSpeed.updated = 1;
	}
}


static void doGForceChecks(void){


	//handle Accelerometer
	float xGforce = convertAccelRawToG(readAccelChannel(X_AXIS),DEFAULT_ACCEL_ZERO);
	float yGforce = convertAccelRawToG(readAccelChannel(Y_AXIS),DEFAULT_ACCEL_ZERO);

	xGforce = xGforce * X_AXIS_INVERT;
	yGforce = yGforce * Y_AXIS_INVERT;

	xGforce = fabsf(xGforce);

	//overall
	if (xGforce > g_maxCornerGforce.value){
		g_maxCornerGforce.value = xGforce;
		g_maxCornerGforce.updated = 1;
	}

	if (yGforce < g_maxBrakingGforce.value){
		g_maxBrakingGforce.value = yGforce;
		g_maxBrakingGforce.updated = 1;
	}

	if (yGforce > g_maxAccelGforce.value){
		g_maxAccelGforce.value = yGforce;
		g_maxAccelGforce.updated = 1;
	}

	//personal

	if (xGforce > g_personalMaxCornerGforce.value){
		g_personalMaxCornerGforce.value = xGforce;
		g_personalMaxCornerGforce.updated = 1;
	}

	if (yGforce < g_personalMaxBrakingGforce.value){
		g_personalMaxBrakingGforce.value = yGforce;
		g_personalMaxBrakingGforce.updated = 1;
	}

	if (yGforce > g_personalMaxAccelGforce.value){
		g_personalMaxAccelGforce.value = yGforce;
		g_personalMaxAccelGforce.updated = 1;
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
	return within;
}


static void doNewLap(float elapsed){

	float lapTime = elapsed / 60.0;

	if (g_topLapTime.value == 0){
		g_topLapTime.value = lapTime;
		g_topLapTime.updated = 1;
	}
	else{
		if (lapTime < g_topLapTime.value){
			g_topLapTime.value = lapTime;
			g_topLapTime.updated = 1;
		}
	}

	if (g_personalTopLapTime.value == 0){
		g_personalTopLapTime.value = lapTime;
		g_personalTopLapTime.updated = 1;
	}
	else{
		if (lapTime < g_personalTopLapTime.value){
			g_personalTopLapTime.value = lapTime;
			g_personalTopLapTime.updated = 1;
		}
	}
	g_currentLapCount++;
	g_lastLapTime = lapTime;
	tweetCurrentLapTime();
}

void raceTask(void *params){

	PWM_SetDutyCycle(1,(unsigned short)0);
	PWM_SetDutyCycle(2,(unsigned short)0);
	PWM_SetDutyCycle(3,(unsigned short)0);

	PWM_SetDutyCycle(1,(unsigned short)99);
	PWM_SetDutyCycle(2,(unsigned short)99);
	PWM_SetDutyCycle(3,(unsigned short)99);

	int cellModemInited = 0;

	memset(g_receivedText,0,200);

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
			if (xQueueReceive(textMsgRxQueue,g_receivedText,0)){
				handleTxt(g_receivedText);
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

		}

		if (getTimeSince(g_lastNotifiedTimestamp)>NOTIFY_INTERVAL){
			notifyPending();
			g_lastNotifiedTimestamp = getSecondsSinceMidnight();
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

void textSenderTask(void *params){

	while(1){
		while((xQueueReceive( textMsgTxQueue, g_txtSendBuffer, 0))){
			sendText(g_twitterNumber,g_txtSendBuffer);
			deleteSent();
		}

		const char *txt = receiveText(1);
		if (NULL != txt){
			xQueueSend(textMsgRxQueue,txt,portMAX_DELAY);
			deleteInbox();
		}

		vTaskDelay(100);
	}
}
