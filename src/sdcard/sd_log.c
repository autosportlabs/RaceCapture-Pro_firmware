/*
 * Race Capture Firmware
 *
 * Copyright (C) 2019 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */


#include "sdcard.h"
#include "sd_log.h"
#include "printk.h"
#include "taskUtil.h"
#include "modp_numtoa.h"
#include <string.h>

#ifdef SDCARD_SUPPORT
#define SD_PR_FILE
#endif //  SDCARD_SUPPORT

#define PR_FILEBASE "sd_log"
#define PR_FILEEXT ".txt"
#define MAX_LOG_FILE_INDEX 9999
#define SYNC_INTERVAL_MS 1000

#ifdef SD_PR_FILE
static FIL* pr_file = NULL;
static bool pr_file_open = false;
static uint32_t last_sync = 0;

bool sd_log_init_file()
{
        bool ret = false;
	if ( sdcard_write_ready( true ) )
	{
		if( pr_file == NULL )
		{
			pr_file = pvPortMalloc(sizeof(FIL));
			if ( pr_file == NULL )
			{
				pr_error( "Failed to allocate FIL for pr_log\r\n" );
			}
		}

		// open for writing, the next file in a sequence.
		// The first name that does not already exits is opened for write.
		if(  pr_file != NULL )
		{
			char name_buf[16];
			if ( sdcard_open_next( pr_file, name_buf, PR_FILEBASE, PR_FILEEXT, MAX_LOG_FILE_INDEX ) )
			{
				pr_file_open = true;
			}
			else
			{
				pr_info( "close sd" );
				pr_file_open = false;
				sd_close( pr_file );
			}
		}
	}
	return ret;
}
void sd_log_sync()
{
	if ( pr_file_open && isTimeoutMs( last_sync, SYNC_INTERVAL_MS ) )
	{
		last_sync = getCurrentTicks();
		sd_sync( pr_file );
	}
}

bool _sd_log_write( const char* str )
{
	if ( ! pr_file_open )
	{
		sd_log_init_file();
	}
	if ( pr_file_open )
	{
		unsigned int written = 0;
		if ( FR_OK == sd_write( pr_file, str, strlen(str), &written ) )
                {
                        sd_log_sync();
                        pr_trace( "sd_log_write\r\n" );
                }
                else
		{
			pr_file_open = false;
			sd_close( pr_file );
		}
	}
        return pr_file_open;
}

bool sd_log_uint(const unsigned int value)
{
        char buf[12];
        modp_uitoa10(value, buf);
        return sd_log(buf);
}

bool sd_log_int(const int value)
{
        char buf[12];
        modp_itoa10(value, buf);
        return sd_log(buf);
}

bool sd_log_float(const float value)
{
        char buf[20];
	if ( value != value )
	{
		strcpy( buf, "nan" );
	}
	else
	{
		modp_ftoa(value, buf, 6);
	}
        return sd_log(buf);
}

#endif
