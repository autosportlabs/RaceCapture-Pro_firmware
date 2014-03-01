#include "OBD2.h"
#include "CAN.h"
#include "loggerConfig.h"

#define STANDARD_PID_RESPONSE 			0x7e8
#define CUSTOM_MODE_SHOW_CURRENT_DATA 	0x41

static int OBD2_current_values[OBD2_CHANNELS];

void OBD2_set_current_PID_value(size_t index, int value){
	if (index < OBD2_CHANNELS){
		OBD2_current_values[index] = value;
	}
}

int OBD2_get_current_PID_value(size_t index){
	return OBD2_current_values[index];
}

static int decode_pid(unsigned char pid, CAN_msg *msg, int *result){
	if (	msg->addressValue == STANDARD_PID_RESPONSE &&
			msg->data[0] >= 3 &&
			msg->data[1] == CUSTOM_MODE_SHOW_CURRENT_DATA &&
			msg->data[2] == pid ){

		int A = msg->data[3];
		int B = msg->data[4];
		//int C = msg->data[5]; //save for later
		//int D = msg->data[6];

		switch(pid){
			case 0x04: //calculated engine load
				*result = A * 100 / 255;
				break;
			case 0x05: //engine coolant temperature (C)
				*result = A - 40;
				break;
			case 0x06: //short term fuel % trim - Bank 1
			case 0x07: //short term fuel % trim - Bank 1
			case 0x08: //short term fuel % trim - Bank 2
			case 0x09: //short term fuel % trim - Bank 2
				*result = (A - 128) * 100 / 128;
				break;
			case 0x0A: //fuel pressure (KPa (gauge))
				*result  = A * 3;
				break;
			case 0x0B: //intake manifold pressure (KPa absolute)
				*result = A;
				break;
			case 0x0C: //RPM
				*result = ((A * 256) + B) / 4;
				break;
			case 0x0D: //vehicle speed (km/ h)
				*result = A;
				break;
			case 0x0E: //timing advance (degrees)
				*result = (A - 128) / 2;
				break;
			case 0x0F: //Intake air temperature (C)
				*result = A - 40;
				break;
			case 0x10: //MAF airflow rate (grams / sec)
				*result = ((A * 256) + B) / 100;
				break;
			case 0x11: //throttle position %
				*result = A * 100 / 255;
				break;
			case 0x2F: //fuel level input %
				*result = A * 100 / 255;
				break;
			case 0x5C: //Engine oil temp (C)
				*result = A - 40;
				break;
		}
	}
}

int OBD2_request_PID(unsigned char pid, int *value, size_t timeout){
	CAN_msg msg;
	msg.addressValue = 0x7df;
	msg.data[0] = 2;
	msg.data[1] = 1;
	msg.data[2] = pid;
	msg.dataLength = 3;

	int result = CAN_tx_msg(&msg, timeout);
	if (result) result = CAN_rx_msg(&msg, timeout);
	if (result) result = decode_pid(pid, &msg, value);
	return result;
}
