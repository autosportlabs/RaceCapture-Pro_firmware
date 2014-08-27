#include "gps_device.h"
#include <stdint.h>
#include <stddef.h>
#include "printk.h"
#include "mem_mang.h"
#include "taskUtil.h"

#define MAX_PAYLOAD_LEN			100
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
	uint8_t payload[MAX_PAYLOAD_LEN];
	uint8_t checksum;
} GpsMessage;

typedef enum {
	GPS_MSG_SUCCESS = 0,
	GPS_MSG_FAIL,
	GPS_MSG_TIMEOUT
} gps_msg_result_t;


static uint8_t calculateChecksum(GpsMessage *msg){
	uint8_t checksum = 0;
	if (msg){
		uint16_t len = msg->payloadLength;
		if (len <= MAX_PAYLOAD_LEN){
			uint8_t *payload = msg->payload;
			for (size_t i = 0; i < len; i++){
				checksum ^= payload[i];
			}
		}
	}
	return checksum;
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

static gps_msg_result_t rxGpsMessage(uint8_t *buffer, GpsMessage *msg, Serial *serial){

	gps_msg_result_t result = GPS_MSG_FAIL;
	size_t timeoutLen = msToTicks(GPS_MSG_RX_WAIT_MS);
	size_t messageReceived = 0;
	size_t isTimeout = 0;

	while (!messageReceived && !isTimeout){
		uint8_t h1 = 0, h2 = 0;

		if (!(serial->get_c_wait(&h1, timeoutLen) && serial->get_c_wait(&h2, timeoutLen))){
			isTimeout = 1;
			break;
		}

		if (h1 == 0xA0 && h2 == 0xA1){
			uint8_t len_h = 0, len_l = 0;
			if (!(serial->get_c_wait(&len_h, timeoutLen) && serial->get_c_wait(&len_l, timeoutLen))){
				isTimeout = 1;
				break;
			}
			uint16_t len = (len_h << 8) + len_l;

			if (len <= MAX_PAYLOAD_LEN){
				uint8_t c = 0;
				for (size_t i = 0; i < len; i++){
					if (!serial->get_c_wait(&c, timeoutLen)){
						isTimeout = 1;
						break;
					}
					msg->payload[i] = c;
				}
			}
			uint8_t checksum = 0;
			if (!serial->get_c_wait(&checksum, timeoutLen)){
				isTimeout = 1;
				break;
			}
			uint8_t calculatedChecksum = calculateChecksum(msg);

			if (calculatedChecksum == checksum){
				uint8_t eos1 = 0, eos2 = 0;
				if (! (serial->get_c_wait(&eos1, timeoutLen) && serial->get_c_wait(&eos2, timeoutLen))){
					isTimeout = 1;
					break;
				}
				if (eos1 == 0x0D && eos2 == 0x0A){
					result = GPS_MSG_SUCCESS;
				}
			}
		}
	}
	if (isTimeout){
		result = GPS_MSG_TIMEOUT;
	}
	return result;
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
