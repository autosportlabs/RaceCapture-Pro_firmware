/*
 * loggerCommands.c
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */
#include <stddef.h>
#include "gpsTask.h"
#include "mod_string.h"
#include "loggerCommands.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "loggerConfig.h"
#include "tracks.h"
#include "luaScript.h"
#include "loggerHardware.h"
#include "printk.h"
#include "sdcard.h"
#include "loggerData.h"
#include "sampleRecord.h"
#include "loggerSampleData.h"
#include "usart.h"
#include "mem_mang.h"
#include "loggerTaskEx.h"
#include "taskUtil.h"
#include "GPIO.h"


void TestSD(Serial *serial, unsigned int argc, char **argv){
	int lines = 1;
	int doFlush = 0;
	int quiet = 0;
	if (argc > 1) lines = modp_atoi(argv[1]);
	if (argc > 2) doFlush = modp_atoi(argv[2]);
	if (argc > 3) quiet = modp_atoi(argv[3]);
	TestSDWrite(serial, lines, doFlush, quiet);
}


void ResetConfig(Serial *serial, unsigned int argc, char **argv){
	if (flash_default_logger_config() == 0 && flash_default_script() == 0 && flash_default_tracks() == 0) {
		put_commandOK(serial);
		configChanged();
	}
	else{
		put_commandError(serial, ERROR_CODE_CRITICAL_ERROR);
	}
}

static void StartTerminalSession(Serial *fromSerial, Serial *toSerial){

	while (1){
		char c = 0;
		if (fromSerial->get_c_wait(&c, 0)){
			if (c == 27) break;
			fromSerial->put_c(c);
			if (c == '\r') fromSerial->put_c('\n');
			toSerial->put_c(c);
			if (c == '\r') toSerial->put_c('\n');
		}
		if (toSerial->get_c_wait(&c, 0)){
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

	Serial *targetSerial = get_serial(port);
	if (targetSerial){
		configure_serial(port, 8, 0, 1, baud);
		StartTerminalSession(serial, targetSerial);
	}
	else{
		put_commandError(serial, ERROR_CODE_INVALID_PARAM);
	}
}

void ViewLog(Serial *serial, unsigned int argc, char **argv)
{
        serial->put_s("Starting logging mode.  Hit \"q\" to exit\r\n");

        while(1) {
                // Write log to serial
                read_log_to_serial(serial, 0);

                // Look for 'q' to exit.
                char c = 0;
                serial->get_c_wait(&c, msToTicks(5));
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

void LogGpsData(Serial *serial, unsigned int argc, char **argv) {
   if (argc != 2) {
      serial->put_s("Must pas one argument only.  Enter 0 to disable, or non-zero to enable\r\n");
      put_commandError(serial, ERROR_CODE_INVALID_PARAM);
   } else {
      const bool enable = (argv[1][0] != '0');
      setGpsDataLogging(enable);
      serial->put_s(enable ? "Enabling" : "Disabling");
      serial->put_s(" the printing of raw GPS data to the log.\r\n");
      put_commandOK(serial);
   }

   serial->flush();
}
