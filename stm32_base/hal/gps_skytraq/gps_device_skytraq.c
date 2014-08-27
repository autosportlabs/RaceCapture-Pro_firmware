#include "gps_device.h"
#include <stdint.h>
#include <stddef.h>
#include "printk.h"
#include "mem_mang.h"
#include "taskUtil.h"
#include "printk.h"
#include "FreeRTOS.h"
#include "task.h"

#define MSG_ID_QUERY_GPS_SW_VER	0x02


#define MAX_PAYLOAD_LEN			256
#define GPS_MSG_RX_WAIT_MS		1000
#define GPS_MESSAGE_BUFFER_LEN	1024
#define TARGET_UPDATE_RATE 		50
#define TARGET_BAUD_RATE 		921600

#define BAUD_RATE_COUNT 		2
#define BAUD_RATES 				{{9600, 1}, {921600, 8}}

typedef struct _BaudRateCodes{
	uint32_t baud;
	uint8_t code;
} BaudRateCodes;

#define UPDATE_RATE_COUNT 		10
#define UPDATE_RATES 			{1, 2, 4, 5, 8, 10, 20, 25, 40, 50}

#define MSG_ID_QUERY_SW_VERSION 0x02
#define MSG_ID_ACK				0x83
#define MSG_ID_NACK 			0x84

typedef struct _QuerySwVersion{
	uint8_t softwareType;
} QuerySwVersion;

typedef struct _Ack{
	uint8_t ackId;
} Ack;

typedef struct _Nack{
	uint8_t ackId;
} Nack;

typedef struct _GpsMessagePayload{
	uint8_t messageId;
	union{
		uint8_t body[MAX_PAYLOAD_LEN - 1];
		QuerySwVersion querySoftwareVersionMsg;
		Ack ackMsg;
		Nack nackMsg;
	};
} GpsMessagePayload;

typedef struct _GpsMessage{
	uint16_t payloadLength;
	union{
		uint8_t rawPayload[MAX_PAYLOAD_LEN];
		GpsMessagePayload payload;
	};
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
			uint8_t *payload = msg->rawPayload;
			for (size_t i = 0; i < len; i++){
				checksum ^= payload[i];
			}
		}
	}
	return checksum;
}

static void txGpsMessage(GpsMessage *msg, Serial *serial){
	serial->put_c(0xA0);
	serial->put_c(0xA1);

	uint16_t payloadLength = msg->payloadLength;
	serial->put_c((uint8_t)payloadLength >> 8);
	serial->put_c((uint8_t)payloadLength & 0xFF);

	uint8_t *payload = msg->rawPayload;
	while(payloadLength--){
		serial->put_c(*(payload++));
	}

	serial->put_c(calculateChecksum(msg));

	serial->put_c(0x0D);
	serial->put_c(0x0A);
}

static gps_msg_result_t rxGpsMessage(GpsMessage *msg, Serial *serial){

	gps_msg_result_t result = GPS_MSG_FAIL;
	size_t timeoutLen = msToTicks(GPS_MSG_RX_WAIT_MS);
	size_t isTimeout = 0;

	while (!isTimeout){

		size_t timeoutStart = xTaskGetTickCount();

		isTimeout = 1;
		while(! isTimeoutMs(timeoutStart, GPS_MSG_RX_WAIT_MS)){
			uint8_t h1 = 0, h2 = 0;
			size_t h1b = serial_read_byte(serial, &h1, timeoutLen);
			size_t h2b = serial_read_byte(serial, &h2, timeoutLen);

			if (h1b && h2b){
				pr_error("gps rx header: ");
				pr_error_int(h1);
				pr_error(" ");
				pr_error_int(h2);
				pr_error("\r\n");
				if (h1 == 0xA0 && h2 == 0xA1){
					uint8_t len_h = 0, len_l = 0;
					size_t len_hb = serial_read_byte(serial, &len_h, timeoutLen);
					size_t len_lb = serial_read_byte(serial, &len_l, timeoutLen);
					if (!(len_hb && len_lb)){
						isTimeout = 1;
						break;
					}
					uint16_t len = (len_h << 8) + len_l;

					if (len <= MAX_PAYLOAD_LEN){
						uint8_t c = 0;
						pr_error("payload: (");
						pr_error_int(len);
						pr_error(") ");
						for (size_t i = 0; i < len; i++){
							if (! serial_read_byte(serial, &c, timeoutLen)){
								isTimeout = 1;
								break;
							}
							msg->rawPayload[i] = c;
							pr_error_int(c);
							pr_error(" ");
						}
						pr_error(" end of payload\r\n");
					}
					uint8_t checksum = 0;
					if (! serial_read_byte(serial, &checksum, timeoutLen)){
						isTimeout = 1;
						break;
					}
					uint8_t calculatedChecksum = calculateChecksum(msg);

					if (calculatedChecksum == checksum){
						uint8_t eos1 = 0, eos2 = 0;
						if (! (serial_read_byte(serial, &eos1, timeoutLen) && serial_read_byte(serial, &eos2, timeoutLen))){
							isTimeout = 1;
							break;
						}
						if (eos1 == 0x0D && eos2 == 0x0A){
							result = GPS_MSG_SUCCESS;
						}
					}
				}
			}
		}
	}
	if (isTimeout){
		result = GPS_MSG_TIMEOUT;
	}
	return result;
}

static void sendQuerySwVersion(GpsMessage * gpsMsg, Serial * serial){
	gpsMsg->payload.messageId = MSG_ID_QUERY_SW_VERSION;
	gpsMsg->payload.querySoftwareVersionMsg.softwareType = 0x00;
	gpsMsg->payloadLength = sizeof(QuerySwVersion) + 1;
	gpsMsg->checksum = calculateChecksum(gpsMsg);
	txGpsMessage(gpsMsg, serial);
}


uint32_t detectGpsBaudRate(GpsMessage *gpsMsg, Serial *serial){
	BaudRateCodes baud_rates[BAUD_RATE_COUNT] = BAUD_RATES;

	while(1){
	for (size_t i = 0; i < BAUD_RATE_COUNT; i++){
		uint32_t baudRate = baud_rates[i].baud;
		pr_info("detecting baud ");
		pr_info_int(baudRate);
		pr_info("\r\n");
		//	configure_serial(SERIAL_GPS, 8, 0, 1, baudRate);
		sendQuerySwVersion(gpsMsg, serial);
		if (rxGpsMessage(gpsMsg, serial) == GPS_MSG_SUCCESS){
			if (gpsMsg->payload.messageId == MSG_ID_ACK){
				return baudRate;
			}
		}
	}
	}
	return 0;
}

int GPS_device_provision(Serial *serial){
	GpsMessage *gpsMsg = portMalloc(sizeof(GpsMessage));
	if (gpsMsg == NULL){
		pr_error("Could not create buffer for GPS message");
		return 0;
	}
	uint32_t baudRate = detectGpsBaudRate(gpsMsg, serial);
	if (baudRate){
		pr_info("GPS module detected at ");
		pr_info_int(baudRate);
		pr_info("\r\n");
		return 1;
	}
	else{
		pr_error("Error provisioning: could not detect GPS module on known baud rates\r\n");
		return 0;
	}
}
