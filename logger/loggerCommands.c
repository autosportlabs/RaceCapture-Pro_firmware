/*
 * loggerCommands.c
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */
#include <stddef.h>
#include "mod_string.h"
#include "loggerCommands.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "loggerConfig.h"
#include "loggerHardware.h"
#include "accelerometer.h"
#include "printk.h"
#include "sdcard.h"
#include "loggerData.h"
#include "sampleRecord.h"
#include "loggerSampleData.h"
#include "usart.h"
#include "board.h"
#include "mem_mang.h"
#include "accelerometer.h"
#include "GPIO.h"


void TestSD(Serial *serial, unsigned int argc, char **argv){
	int lines = 1;
	int doFlush = 0;
	int quiet = 0;
	int delay = 0;
	if (argc > 1) lines = modp_atoi(argv[1]);
	if (argc > 2) doFlush = modp_atoi(argv[2]);
	if (argc > 3) quiet = modp_atoi(argv[3]);
	if (argc > 4) delay = modp_atoi(argv[4]);
	TestSDWrite(serial, lines, doFlush, quiet, delay);
}



static void StartTerminalSession(Serial *fromSerial, Serial *toSerial){

	while (1){
		char c = fromSerial->get_c_wait(0);
		if (c == 27) break;
		if (c){
			fromSerial->put_c(c);
			if (c == '\r') fromSerial->put_c('\n');
			toSerial->put_c(c);
			if (c == '\r') toSerial->put_c('\n');
		}
		c = toSerial->get_c_wait(0);
		if (c){
			fromSerial->put_c(c);
			if (c == '\r') fromSerial->put_c('\n');
		}
	}
}

void StartTerminal(Serial *serial, unsigned int argc, char **argv){
	if (argc < 3){
		put_commandError(serial, ERROR_CODE_MISSING_PARAMS);
		return;
	}

	serial->put_s("Entering Terminal. Press ESC to exit\r\n");

	unsigned int port = modp_atoui(argv[1]);
	unsigned int baud = modp_atoui(argv[2]);

	switch(port){
		case 0:
			initUsart0(8, 0, 1, baud);
			StartTerminalSession(serial, get_serial_usart0());
			break;
		case 1:
			initUsart1(8, 0, 1, baud);
			StartTerminalSession(serial, get_serial_usart1());
			break;
		default:
			put_commandError(serial, ERROR_CODE_INVALID_PARAM);
	}
}

void ViewLog(Serial *serial, unsigned int argc, char **argv)
{
        serial->put_s("Starting logging mode.  Hit \"q\" to exit\r\n");

        while(1) {
                // Write log to serial
                read_log_to_serial(serial);

                // Look for 'q' to exit.
                char c = serial->get_c_wait(300 / 5); //TODO refactor this when we go to millisecond based delays
                if (c == 'q') break;
        }

        // Give a little space when we finish up with log watching.
        serial->put_s("\r\n\r\n");
        serial->flush();
}

void SetLogLevel(Serial *serial, unsigned int argc, char **argv)
{
        // XXX make this more robust maybe.
        if (argc < 1 || argv[1][0] < '0' || argv[1][0] > '8') {
        		put_commandError(serial, ERROR_CODE_INVALID_PARAM);
                return;
        }

        enum log_level level = (enum log_level) modp_atoui(argv[1]);
        set_log_level(level);
        put_commandOK(serial);
}


