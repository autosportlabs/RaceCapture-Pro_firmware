#include "OBD2.h"
#include "CAN.h"


static int decode_pid(unsigned char pid, CAN_msg *msg, int *result){


	int A = msg[0];
	int B = msg[1];
	int C = msg[2];
	int D = msg[3];

	if (	msg[0] == 0x7e8 &&
			pid == msg[1] &&
			pid == msg[2]){

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
			*result = ((A*256)+B)/4;
			break;
		case 0x0D: //vehicle speed (km/ h)
			*result = A;
			break;
		case 0x0E: //timing advance (degrees)
			*result = (A-128)/2;
			break;
		case 0x0F: //Intake air temperature (C)
			*result = A - 40;
			break;
		case 0x10: //MAF airflow rate (grams / sec)
			*result = ((A*256)+B) / 100;
			break;
		case 0x11: //throttle position %
			*result = A*100/255;
			break;
		case 0x2F: //fuel level input %
			*result = A*100/255;
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

	return 	CAN_tx_msg(&msg, timeout) &&
			CAN_rx_msg(msg, timeout) &&
			decode_pid(pid, &msg, *value);
}
