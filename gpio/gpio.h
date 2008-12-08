#ifndef GPIO_H_
#define GPIO_H_

#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

void setGPO(int gpo, int state);
int readGPI(int gpi);

void pushbutton_irq_handler ( void );
void onGPO1Task(void *pvParameters);
void onGPO2Task(void *pvParameters);
void onGPO3Task(void *pvParameters);
void onGPI1Task(void *pvParameters);
void onGPI2Task(void *pvParameters);
void onGPI3Task(void *pvParameters);
void onPushbuttonTask(void *pvParameters);

#endif /*GPIO_H_*/
