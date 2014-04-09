#include "sector_test.h"
#include "gps.h"
#include "loggerConfig.h"
#include <stdlib.h>
#include <fstream>
#include <streambuf>
#include "mod_string.h"
#include "modp_atonum.h"

#include <cppunit/extensions/HelperMacros.h>
using std::ifstream;
using std::ios;
using std::istreambuf_iterator;

#define CPPUNIT_ASSERT_CLOSE_ENOUGH(ACTUAL, EXPECTED) CPPUNIT_ASSERT((abs((ACTUAL - EXPECTED)) < 0.00001))

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( SectorTest );

#define FILE_PREFIX string("test/")

void SectorTest::setUp()
{
	initGPS();
}


void SectorTest::tearDown()
{
}

vector<string> & SectorTest::split(string &s, char delim, vector<string> &elems) {
    std::stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> SectorTest::split(string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

string SectorTest::readFile(string filename){
	ifstream t(filename.c_str());
	if (!t.is_open()){
		t.open(string(FILE_PREFIX + filename).c_str());

	}
	if (!t.is_open()){
		throw ("Can not find file " + filename);
	}
	string str;

	t.seekg(0, ios::end);
	int length = t.tellg();
	str.reserve(length);
	t.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(t)),
				istreambuf_iterator<char>());
	return str;
}

#define Test_Track { \
	0.0001, \
	{47.806934,-122.341150}, \
	{ \
		{47.806875,-122.335818}, \
		{47.79974,-122.335704}, \
		{47.799719,-122.346416}, \
		{47.806886,-122.346494}, \
		{0, 0}, \
		{0, 0}, \
		{0, 0}, \
		{0, 0}, \
		{0, 0}, \
		{0, 0} \
	} \
	}

void SectorTest::outputSectorTimes(vector<float> & sectorTimes, int lap){
	for (size_t i = 0; i < sectorTimes.size(); i++){
		printf("lap %d | sector %d | %f\r", lap, i, sectorTimes[i]);
	}


}
void SectorTest::testSectorTimes(){
	printf("\rSector Times:\r");
	string log = readFile("predictive_time_test_lap.log");

	std::istringstream iss(log);

	Track *trackCfg  = 	&(getWorkingLoggerConfig()->TrackConfigs.track);

	Track testTrack = Test_Track;
	memcpy(trackCfg, &testTrack, sizeof(Track));

	setGPSQuality(GPS_QUALITY_DIFFERENTIAL);

	vector<float> sectorTimes;
	int currentSector = 0;
	int currentLap = 0;
	int lineNo = 0;
	string line;
	while (std::getline(iss, line)){
		lineNo++;
		vector<string> values = split(line, ',');

		string latitudeRaw = values[5];
		string longitudeRaw = values[6];
		string speedRaw = values[7];
		string timeRaw = values[8];

		timeRaw = "0" + timeRaw;

		if (values[0][0] != '#' && latitudeRaw.size() > 0 && longitudeRaw.size() > 0 && speedRaw.size() > 0 && timeRaw.size() > 0){
			//printf("%s", line.c_str());
			float lat = modp_atof(latitudeRaw.c_str());
			float lon = modp_atof(longitudeRaw.c_str());
			float speed = modp_atof(speedRaw.c_str());
			float utcTime = modp_atof(timeRaw.c_str());

			setGPSSpeed(speed);
			setUTCTime(utcTime);
			updatePosition(lat, lon);
			double secondsSinceMidnight = calculateSecondsSinceMidnight(timeRaw.c_str());
			updateSecondsSinceMidnight(secondsSinceMidnight);
			onLocationUpdated();

			int sector = getLastSector();
			if (sector != currentSector){
				if (sector < currentSector) {
					sectorTimes.clear();
					currentSector = 0;
				}
				currentSector = sector;
				sectorTimes.push_back(getLastSectorTime());
			}
			int lap = getLapCount();
			//only 3 laps to test in this data, lap 4 is the last and never crosses any sector points.
			size_t sectorCounts = sectorTimes.size();
			if (lap > currentLap ){
				switch (lap){
				case 1:
					outputSectorTimes(sectorTimes, currentLap);
					CPPUNIT_ASSERT_EQUAL(4, (int)sectorCounts);
					break;
				case 2:
					outputSectorTimes(sectorTimes, currentLap);
					CPPUNIT_ASSERT_EQUAL(4, (int)sectorCounts);
					break;
				case 3:
					outputSectorTimes(sectorTimes, currentLap);
					CPPUNIT_ASSERT_EQUAL(4, (int)sectorCounts);
				case 4:
					outputSectorTimes(sectorTimes, currentLap);
					CPPUNIT_ASSERT_EQUAL(4, (int)sectorCounts);
					break;
				}
				currentLap = lap;
			}
			//printf("%.7f,%.7f | lapTime (%d) %f | sectorTime: (%d) %f\r\n", lat, lon, getLapCount(), getLastLapTime(), getLastSector(), getLastSectorTime());
		}
	}
}
