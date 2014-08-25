#include "gps_device.h"
#include <stdint.h>
#include "printk.h"
#include "mem_mang.h"
#include "taskUtil.h"

#define GPS_MSG_RX_WAIT_MS		1000
#define GPS_MESSAGE_BUFFER_LEN	1024
#define TARGET_UPDATE_RATE 		50
#define TARGET_BAUD_RATE 		921600

#define BAUD_RATE_COUNT 		3
#define BAUD_RATES 				{{9600, 1}, {115200, 5}, {921600, 8} }

typedef struct _BaudRateCodes{
	uint32_t baud;
	uint8_t code;
} BaudRateCodes;

#define UPDATE_RATE_COUNT 		10
#define UPDATE_RATES 			{1, 2, 4, 5, 8, 10, 20, 25, 40, 50}

typedef struct _GpsMessage{
	uint16_t payloadLength;
	uint8_t * payload;
	uint8_t checksum;
} GpsMessage;

typedef enum {
	GPS_MSG_SUCCESS = 0,
	GPS_MSG_FAIL
} gps_msg_result_t;


static uint8_t calculateChecksum(uint8_t *msg){
	return 0;
}

static void sendGpsMessage(GpsMessage *msg, Serial *serial){
	serial->put_c(0xA0);
	serial->put_c(0xA1);

	uint16_t payloadLength = msg->payloadLength;
	serial->put_c((uint8_t)payloadLength >> 8);
	serial->put_c((uint8_t)payloadLength & 0xFF);

	uint8_t *payload = msg->payload;
	while(payloadLength--){
		serial->put_c(*(payload++));
	}
	serial->put_c(calculateChecksum(msg));

	serial->put_c(0x0D);
	serial->put_c(0x0A);
}

static gps_msg_result_t rxGpsMessage(char *line, GpsMessage *msg, Serial *serial){
	size_t count = serial->get_line_wait(line, GPS_MESSAGE_BUFFER_LEN, msToTicks(GPS_MSG_RX_WAIT_MS));
	if (count){

	}
	else{
		return GPS_MSG_FAIL;
	}


}


int GPS_device_provision(Serial *serial){
	BaudRateCodes baud_rates[BAUD_RATE_COUNT] = BAUD_RATES;
	char *line = portMalloc(GPS_MESSAGE_BUFFER_LEN);
	if (line == NULL){
		pr_error("Could not create buffer for GPS message");
		return 0;
	}


	return 1;
}
