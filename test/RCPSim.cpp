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

#define LINE_BUFFER_SIZE 2049
int main(int argc, char* argv[])
{

	LoggerConfig *config = getWorkingLoggerConfig();
	initApi();
	initialize_logger_config();
	setupMockSerial();
	imu_init(config);
	resetPredictiveTimer();

	int pt;

	pt = open("/dev/ptmx", O_RDWR | O_NOCTTY);
	if (pt < 0)
	{
		perror("open /dev/ptmx");
		return 1;
	}

	grantpt(pt);
	unlockpt(pt);

	fprintf(stderr, "RaceCapture/Pro simulator on: %s\n", ptsname(pt));

	char line[LINE_BUFFER_SIZE];
	memset(line, 0, LINE_BUFFER_SIZE);
	while(1){
		size_t count = 0;
		line[0] = '\0';
		while (count < LINE_BUFFER_SIZE){
			char c;
			read(pt, &c, 1);
			line[count] = c;
			count++;
			if (c == '\r'){
				line[count] = '\0';
				break;
			}
		}

		printf("rx: (%d): %s\r\n",strlen(line), line);
		mock_resetTxBuffer();
		process_api(getMockSerial(), line, strlen(line));
		char *txBuffer = mock_getTxBuffer();
		printf("tx: (%d) %s\r\n", strlen(txBuffer), txBuffer);
		write(pt, txBuffer, strlen(txBuffer));
	}
	return 0;
}
