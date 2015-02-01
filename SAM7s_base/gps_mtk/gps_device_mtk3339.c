#include "gps_device.h"

#define LATITUDE_DATA_LEN 12
#define LONGITUDE_DATA_LEN 13
#define UTC_TIME_BUFFER_LEN 11
#define UTC_SPEED_BUFFER_LEN 10

#define GPS_DATA_LINE_BUFFER_LEN 	200
static char g_GPSdataLine[GPS_DATA_LINE_BUFFER_LEN];

int checksumValid(const char *gpsData, size_t len) {
   int valid = 0;
   unsigned char checksum = 0;
   size_t i = 0;
   for (; i < len - 1; i++) {
      char c = *(gpsData + i);
      if (c == '*' || c == '\0')
         break;
      else if (c == '$')
         continue;
      else
         checksum ^= c;
   }
   if (len > i + 2) {
      unsigned char dataChecksum = modp_xtoc(gpsData + i + 1);
      if (checksum == dataChecksum)
         valid = 1;
   }
   return valid;
}

/**
 * Performs a full update of the g_dtLastFix value.  Also update the g_dtFirstFix value if it hasn't
 * already been set along with updating the Milliseconds as well
 * @param fixDateTime The DateTime of the GPS fix.
 */
static void updateFullDateTime(DateTime fixDateTime) {
   updateUptimeAtSample();
   updateMillisSinceEpoch(fixDateTime);

   g_dtLastFix = fixDateTime;

   if (g_dtFirstFix.year == 0)
      g_dtFirstFix = fixDateTime;
}

/**
 * Like atoi, but is non-destructive to the string passed in and provides an offset and length
 * functionality.  Max Len is 3.
 * @param str The start of the String to parse.
 * @param offset How far in to start reading the string.
 * @param len The number of characters to read.
 */
static int atoiOffsetLenSafe(const char *str, size_t offset, size_t len) {
   char buff[4] = { 0 };

   // Bounds check.  Don't want any bleeding hearts in here...
   if (len > (sizeof(buff) - 1))
         len = sizeof(buff) - 1;

   memcpy(buff, str + offset, len);
   return modp_atoi(buff);
}

static float parseLatitude(char *data){
   unsigned int len = strlen(data);

   if (len <= 0 && len > LATITUDE_DATA_LEN)
      return 0.0;

   //Raw GPS Format is ddmm.mmmmmm
   int degrees = atoiOffsetLenSafe(data, 0, 2);
   float minutes = modp_atof(data + 2);

   return degrees + (minutes / 60);
}

static float parseLongitude(char *data){
   unsigned int len = strlen(data);

   if (len <= 0 && len > LONGITUDE_DATA_LEN)
      return 0.0;

   //Raw GPS Format is dddmm.mmmmmm
   int degrees = atoiOffsetLenSafe(data, 0, 3);
   float minutes = modp_atof(data + 3);

   return degrees + (minutes / 60);
}

//Parse Global Positioning System Fix Data.
static void parseGGA(char *data) {
   char * delim = strchr(data, ',');
   int param = 0;
   int keepParsing = 1;

   while (delim != NULL && keepParsing) {
      *delim = '\0';
      switch (param) {
         /*
      case 1: {
         unsigned int len = strlen(data);
         if (len > 0 && len <= LATITUDE_DATA_LEN) {
            //Raw GPS Format is ddmm.mmmmmm
            char degreesStr[3] = { 0 };
            memcpy(degreesStr, data, 2);
            degreesStr[2] = 0;
            float minutes = modp_atof(data + 2);
            minutes = minutes / 60.0;
            latitude = modp_atoi(degreesStr) + minutes;
         } else {
            latitude = 0;
            //TODO log error
         }
      }
         break;
      case 2: {
         if (data[0] == 'S') {
            latitude = -latitude;
         }
      }
         break;
      case 3: {
         unsigned int len = strlen(data);
         if (len > 0 && len <= LONGITUDE_DATA_LEN) {
            //Raw GPS Format is dddmm.mmmmmm
            char degreesStr[4] = { 0 };
            memcpy(degreesStr, data, 3);
            degreesStr[3] = 0;
            float minutes = modp_atof(data + 3);
            minutes = minutes / 60.0;
            longitude = modp_atoi(degreesStr) + minutes;
         } else {
            longitude = 0;
            //TODO log error
         }
      }
         break;
      case 4: {
         if (data[0] == 'W') {
            longitude = -longitude;
         }
      }
         break;
      case 5:
         g_gpsQuality = (enum GpsSignalQuality) modp_atoi(data);
         break;
      case 6:
         g_satellitesUsedForPosition = modp_atoi(data);
         keepParsing = 0;
         break;
         */ // Kept until Full sanity checks pass.

      case 5:
         // FIXME: Better suport fo GPS quality here?
         g_gpsQuality = modp_atoi(data) > 0 ? GPS_QUALITY_FIX : GPS_QUALITY_NO_FIX;
         break;

      case 6:
         g_satellitesUsedForPosition = modp_atoi(data);
         keepParsing = 0;
         break;

      }

      param++;
      data = delim + 1;
      delim = strchr(data, ',');
   }
}

//Parse GNSS DOP and Active Satellites
static void parseGSA(char *data) {}

//Parse Course Over Ground and Ground Speed
static void parseVTG(char *data) {

   char * delim = strchr(data, ',');
   int param = 0;

   int keepParsing = 1;
   while (delim != NULL && keepParsing) {
      *delim = '\0';
      switch (param) {
      case 6: //Speed over ground
      {
         if (strlen(data) >= 1) {
            setGPSSpeed(modp_atof(data)); //convert to MPH
         }
         keepParsing = 0;
      }
         break;
      default:
         break;
      }
      param++;
      data = delim + 1;
      delim = strchr(data, ',');
   }
}

//Parse Geographic Position - Latitude / Longitude
static void parseGLL(char *data) {

}

//Parse Time & Date
static void parseZDA(char *data) {
   /*
    $GPZDA

    Date & Time

    UTC, day, month, year, and local time zone.

    $--ZDA,hhmmss.ss,xx,xx,xxxx,xx,xx
    hhmmss.ss = UTC
    xx = Day, 01 to 31
    xx = Month, 01 to 12
    xxxx = Year
    xx = Local zone description, 00 to +/- 13 hours
    xx = Local zone minutes description (same sign as hours)
    */
   return;
}

//Parse GNSS Satellites in View
static void parseGSV(char *data) {

}

//Parse Recommended Minimum Navigation Information
static void parseRMC(char *data) {
   /*
    * $GPRMC,053740.000,A,2503.6319,N,12136.0099,E,2.69,79.65,100106,,,A*53
    * Message ID $GPRMC RMC protocol header
    * UTC Time 053740.000 hhmmss.sss
    * Status A A=data valid or V=data not valid
    * Latitude 2503.6319 ddmm.mmmm
    * N/S Indicator N N=north or S=south
    * Longitude 12136.0099 dddmm.mmmm
    * E/W Indicator E E=east or W=west
    * Speed over ground 2.69 knots True
    * Course over ground 79.65 degrees
    * Date 100106 Ddmmyy
    * Magnetic variation degrees Not shown
    * Variation sense E=east or W=west (Not shown)
    * Mode A
    * A=autonomous, D=DGPS, E=DR, N=Data not valid,
    * R=Coarse Position, S=Simulator
    * Checksum *53
    * <CR> <LF> End of message termination
    */

   char *delim = strchr(data, ',');
   int param = 0;
   DateTime dt = { 0 };
   float latitude = 0.0;
   float longitude = 0.0;

   while (delim) {
      *delim = '\0';
      switch (param) {
      case 0: //UTC Time (HHMMSS.SSS)
         dt.hour = (int8_t) atoiOffsetLenSafe(data, 0, 2);
         dt.minute = (int8_t) atoiOffsetLenSafe(data, 2, 2);
         dt.second = (int8_t) atoiOffsetLenSafe(data, 4, 2);
         dt.millisecond = (int16_t) atoiOffsetLenSafe(data, 7, 3);
         break;

      case 2:
    	 latitude = parseLatitude(data);
         break;

      case 3:
         if (data[0] == 'S') latitude = -latitude;
         break;

      case 4:
    	 longitude = parseLongitude(data);
         break;

      case 5:
         if (data[0] == 'W') longitude = -longitude;
         break;

      case 6: //Speed over ground
         if (strlen(data) >= 1) setGPSSpeed(modp_atof(data) * KNOTS_TO_KPH);
         break;

      case 8: //Date (DDMMYY)
         dt.day = (int8_t) atoiOffsetLenSafe(data, 0, 2);
         dt.month = (int8_t) atoiOffsetLenSafe(data, 2, 2);
         dt.year = (int16_t) atoiOffsetLenSafe(data, 4, 2) + 2000;
         break;
      }

      ++param;
      data = ++delim;
      delim = strchr(delim, ',');
   }

   updateFullDateTime(dt);
}

static void processGPSData(char *gpsData, size_t len) {
   if (len <= 4 || !checksumValid(gpsData, len) || strstr(gpsData, "$GP") != gpsData) {
      pr_trace("GPS: corrupt frame ");
      pr_trace(gpsData);
      pr_trace("\r\n");
      return;
   }
   size_t positionUpdated = 0;

   // Advance the pointer 3 spaces since we know it begins with "$GP"
   gpsData += 3;
   if (strstr(gpsData, "GGA,")) {
      parseGGA(gpsData + 4);
   } else if (strstr(gpsData, "VTG,")) { //Course Over Ground and Ground Speed
      parseVTG(gpsData + 4);
   } else if (strstr(gpsData, "GSA,")) { //GPS Fix gpsData
      parseGSA(gpsData + 4);
   } else if (strstr(gpsData, "GSV,")) { //Satellites in view
      parseGSV(gpsData + 4);
   } else if (strstr(gpsData, "RMC,")) { //Recommended Minimum Specific GNSS Data
      parseRMC(gpsData + 4);
      positionUpdated = 1;
   } else if (strstr(gpsData, "GLL,")) { //Geographic Position - Latitude/Longitude
      parseGLL(gpsData + 4);
   } else if (strstr(gpsData, "ZDA,")) { //Time & Date
      parseZDA(gpsData + 4);
   }

   if (positionUpdated && !isGpsDataCold()) {
      onLocationUpdated();
      flashGpsStatusLed();
   }
}

int GPS_device_provision(Serial *serial){
	//nothing to provision, factory defaults are good
	return 1;
}

int GPS_device_get_update(GpsSamp *gpsSample, Serial *serial){
	int len = serial->get_line(g_GPSdataLine, GPS_DATA_LINE_BUFFER_LEN - 1);
	g_GPSdataLine[len] = '\0';
	return GPS_MSG_SUCCESS;
	//      processGPSData(g_GPSdataLine, len);

}
