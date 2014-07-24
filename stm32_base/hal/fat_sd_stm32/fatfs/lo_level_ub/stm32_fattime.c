/* Martin Thomas 4/2009 */

#include "integer.h"
#include "ff.h"

DWORD get_fattime (void)
{
	DWORD res;

	res = 0;
/* TODO call an RTC to set the timestamp
	RTC_t rtc;
	rtc = 0;

	rtc_gettime( &rtc );
	res =  (((DWORD)rtc.year - 1980) << 25)
			| ((DWORD)rtc.month << 21)
			| ((DWORD)rtc.mday << 16)
			| (WORD)(rtc.hour << 11)
			| (WORD)(rtc.min << 5)
			| (WORD)(rtc.sec >> 1);
*/
	return res;
}

