#ifndef RACETASK_H_
#define RACETASK_H_


void startRaceTask(void);
void raceTask(void *params);
void textSenderTask(void *params);
void setStartFinishPoint(float latitude, float longitude, float radius);
float getStartFinishLatitude(void);
float getStartFinishLongitude(void);
float getStartFinishRadius(void);
void setTweetNumber(const char *number);

#endif /*LUATASK_H_*/
