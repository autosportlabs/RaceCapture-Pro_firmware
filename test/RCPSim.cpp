#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "imu.h"
#include "loggerApi.h"
#include "mock_serial.h"
#include "predictive_timer_2.h"
#include "loggerConfig.h"
#include "mod_string.h"
#include "printk.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "fileWriter.h"
#include "connectivityTask.h"
#include "loggerTaskEx.h"0

#define LINE_BUFFER_SIZE 2049

#define OBD2_TASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define GPS_TASK_PRIORITY 			( tskIDLE_PRIORITY + 5 )
#define CONNECTIVITY_TASK_PRIORITY 	( tskIDLE_PRIORITY + 4 )
#define LOGGER_TASK_PRIORITY		( tskIDLE_PRIORITY + 6 )
#define FILE_WRITER_TASK_PRIORITY	( tskIDLE_PRIORITY + 4 )
#define LUA_TASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define USB_COMM_TASK_PRIORITY		( tskIDLE_PRIORITY + 6 )
#define GPIO_TASK_PRIORITY 			( tskIDLE_PRIORITY + 4 )

int main(int argc, char* argv[])
{

	LoggerConfig *config = getWorkingLoggerConfig();
	initApi();
	initialize_logger_config();
	init_serial();
	//	setupMockSerial();
	imu_init(config);
	resetPredictiveTimer();
   startFileWriterTask		( FILE_WRITER_TASK_PRIORITY );
   startLoggerTaskEx		( LOGGER_TASK_PRIORITY );
   startConnectivityTask	( CONNECTIVITY_TASK_PRIORITY );
   
   /* Set the scheduler running.  This function will not return unless a task calls vTaskEndScheduler(). */
   vTaskStartScheduler();

	return 0;
}
