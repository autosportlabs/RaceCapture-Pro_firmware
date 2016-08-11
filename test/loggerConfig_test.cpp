/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
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

#include "channel_config.h"
#include "cpu.h"
#include "loggerConfig.h"
#include "loggerConfig_test.h"
#include "units.h"
#include <string.h>
#include <string>

using std::string;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LoggerConfigTest );

#define FIELD_LENGTH 200

void LoggerConfigTest::setUp() {
	initialize_logger_config();
}


void LoggerConfigTest::tearDown() {}

void LoggerConfigTest::testLoggerInitVersionInfo() {
   LoggerConfig *lc = getWorkingLoggerConfig();

   VersionInfo *vi = &lc->RcpVersionInfo;
   CPPUNIT_ASSERT_EQUAL((unsigned int) MAJOR_REV, vi->major);
   CPPUNIT_ASSERT_EQUAL((unsigned int) MINOR_REV, vi->minor);
   CPPUNIT_ASSERT_EQUAL((unsigned int) BUGFIX_REV, vi->bugfix);
}

void LoggerConfigTest::testLoggerInitPwmClock() {
   LoggerConfig *lc = getWorkingLoggerConfig();

   CPPUNIT_ASSERT_EQUAL(DEFAULT_PWM_CLOCK_FREQUENCY, (int) lc->PWMClockFrequency);
}

void LoggerConfigTest::testLoggerInitTimeConfig() {
   LoggerConfig *lc = getWorkingLoggerConfig();

   TimeConfig *tc = lc->TimeConfigs;
   CPPUNIT_ASSERT_EQUAL(string("Interval"), string(tc[0].cfg.label));
   CPPUNIT_ASSERT_EQUAL(string("ms"), string(tc[0].cfg.units));
   CPPUNIT_ASSERT(tc[0].cfg.sampleRate != SAMPLE_DISABLED);

   CPPUNIT_ASSERT_EQUAL(string("Utc"), string(tc[1].cfg.label));
   CPPUNIT_ASSERT_EQUAL(string("ms"), string(tc[1].cfg.units));
   CPPUNIT_ASSERT(tc[1].cfg.sampleRate != SAMPLE_DISABLED);
}

void LoggerConfigTest::testLoggerInitAdcConfig() {
   LoggerConfig *lc = getWorkingLoggerConfig();

   ADCConfig *c = lc->ADCConfigs;
   for (int i = 0; i < 7; ++i) {
      CPPUNIT_ASSERT(strlen(c[i].cfg.label));
      CPPUNIT_ASSERT(c[i].cfg.sampleRate == SAMPLE_DISABLED);
   }

   CPPUNIT_ASSERT_EQUAL(string("Battery"), string(c[7].cfg.label));
   CPPUNIT_ASSERT_EQUAL(string("Volts"), string(c[7].cfg.units));
   CPPUNIT_ASSERT(c[7].cfg.sampleRate != 0);
}

void LoggerConfigTest::testLoggerInitPwmConfig() {
   LoggerConfig *lc = getWorkingLoggerConfig();
   PWMConfig *c = lc->PWMConfigs;

   for (size_t i = 0; i < CONFIG_PWM_CHANNELS; ++i) {
      CPPUNIT_ASSERT(strlen(c[i].cfg.label));
      CPPUNIT_ASSERT(c[i].cfg.sampleRate == SAMPLE_DISABLED);
   }
}

void LoggerConfigTest::testLoggerInitGpioConfig() {
   LoggerConfig *lc = getWorkingLoggerConfig();

   GPIOConfig *c = lc->GPIOConfigs;
   for (size_t i = 0; i < CONFIG_GPIO_CHANNELS; ++i) {
      CPPUNIT_ASSERT(strlen(c[i].cfg.label));
      CPPUNIT_ASSERT(c[i].cfg.sampleRate == SAMPLE_DISABLED);
   }
}

void LoggerConfigTest::testLoggerInitTimerConfig() {
	LoggerConfig *lc = getWorkingLoggerConfig();
	set_default_timer_config(lc->TimerConfigs, CONFIG_TIMER_CHANNELS);

	for (size_t i = 0; i < CONFIG_TIMER_CHANNELS; ++i) {
		TimerConfig *tc = lc->TimerConfigs + i;
		char label[8];
		if (i)
			snprintf(label, 8, "RPM%d", i + 1);
		else
			strcpy(label, "RPM");


		CPPUNIT_ASSERT_EQUAL(string(label), string(tc->cfg.label));
		CPPUNIT_ASSERT_EQUAL(string("rpm"), string(tc->cfg.units));
		CPPUNIT_ASSERT_EQUAL((float) 0, tc->cfg.min);
		CPPUNIT_ASSERT_EQUAL((float) 8000, tc->cfg.max);
		CPPUNIT_ASSERT(tc->cfg.sampleRate == SAMPLE_DISABLED);
	}
}

void LoggerConfigTest::testLoggerInitImuConfig() {
   const char *names[] = {"AccelX", "AccelY", "AccelZ"};
   LoggerConfig *lc = getWorkingLoggerConfig();

   ImuConfig *c = lc->ImuConfigs;
   for (size_t i = 0; i < 3; ++i) {
      CPPUNIT_ASSERT_EQUAL(string(names[i]), string(c[i].cfg.label));
      CPPUNIT_ASSERT_EQUAL(string("G"), string(c[i].cfg.units));
      CPPUNIT_ASSERT(c[i].cfg.sampleRate == SAMPLE_25Hz);
   }

   CPPUNIT_ASSERT_EQUAL(string("Yaw"), string(c[3].cfg.label));
   CPPUNIT_ASSERT_EQUAL(string("Deg/Sec"), string(c[3].cfg.units));
   CPPUNIT_ASSERT(c[3].cfg.sampleRate == SAMPLE_25Hz);
}

void LoggerConfigTest::testLoggerInitObd2Config() {
   LoggerConfig *lc = getWorkingLoggerConfig();

   OBD2Config *c = &lc->OBD2Configs;
   CPPUNIT_ASSERT(c->enabled == 0);

   for (int i = 0; i < OBD2_CHANNELS; ++i) {
      CPPUNIT_ASSERT(strlen(c->pids[i].cfg.label));
      CPPUNIT_ASSERT(c->pids[i].cfg.sampleRate == 0);
   }
}

void LoggerConfigTest::testLoggerInitGpsConfig() {
	LoggerConfig *lc = getWorkingLoggerConfig();
	ChannelConfig *cc;

	cc = &lc->GPSConfigs.latitude;
	CPPUNIT_ASSERT_EQUAL(string("Latitude"), string(cc->label));
	CPPUNIT_ASSERT_EQUAL(string("Degrees"), string(cc->units));
	CPPUNIT_ASSERT(cc->sampleRate == DEFAULT_GPS_SAMPLE_RATE);

	cc = &lc->GPSConfigs.longitude;
	CPPUNIT_ASSERT_EQUAL(string("Longitude"), string(cc->label));
	CPPUNIT_ASSERT_EQUAL(string("Degrees"), string(cc->units));
	CPPUNIT_ASSERT(cc->sampleRate == DEFAULT_GPS_SAMPLE_RATE);

	cc = &lc->GPSConfigs.speed;
	CPPUNIT_ASSERT_EQUAL(string("Speed"), string(cc->label));
	CPPUNIT_ASSERT_EQUAL(string(units_get_label(UNIT_SPEED_MILES_HOUR)),
			     string(cc->units));
	CPPUNIT_ASSERT(cc->sampleRate == DEFAULT_GPS_SAMPLE_RATE);

	cc = &lc->GPSConfigs.distance;
	CPPUNIT_ASSERT_EQUAL(string("Distance"), string(cc->label));
	CPPUNIT_ASSERT_EQUAL(string(units_get_label(UNIT_LENGTH_MILES)),
			     string(cc->units));
	CPPUNIT_ASSERT(cc->sampleRate == DEFAULT_GPS_SAMPLE_RATE);

	cc = &lc->GPSConfigs.altitude;
	CPPUNIT_ASSERT_EQUAL(string("Altitude"), string(cc->label));
	CPPUNIT_ASSERT_EQUAL(string(units_get_label(UNIT_LENGTH_FEET)),
			     string(cc->units));
	CPPUNIT_ASSERT(cc->sampleRate == DEFAULT_GPS_SAMPLE_RATE);

	cc = &lc->GPSConfigs.satellites;
	CPPUNIT_ASSERT_EQUAL(string("GPSSats"), string(cc->label));
	CPPUNIT_ASSERT_EQUAL(string(""), string(cc->units));
	CPPUNIT_ASSERT(cc->sampleRate == DEFAULT_GPS_SAMPLE_RATE);

	cc = &lc->GPSConfigs.quality;
	CPPUNIT_ASSERT_EQUAL(string("GPSQual"), string(cc->label));
	CPPUNIT_ASSERT_EQUAL(string(""), string(cc->units));
	CPPUNIT_ASSERT(cc->sampleRate == DEFAULT_GPS_SAMPLE_RATE);

	cc = &lc->GPSConfigs.DOP;
	CPPUNIT_ASSERT_EQUAL(string("GPSDOP"), string(cc->label));
	CPPUNIT_ASSERT_EQUAL(string(""), string(cc->units));
	CPPUNIT_ASSERT(cc->sampleRate == DEFAULT_GPS_SAMPLE_RATE);
}

void LoggerConfigTest::testLoggerInitLapConfig() {
   LoggerConfig *lc = getWorkingLoggerConfig();
   ChannelConfig *cc;

   cc = &lc->LapConfigs.lapCountCfg;
   CPPUNIT_ASSERT_EQUAL(string("LapCount"), string(cc->label));
   CPPUNIT_ASSERT_EQUAL(string(""), string(cc->units));
   CPPUNIT_ASSERT(cc->sampleRate == SAMPLE_10Hz);

   cc = &lc->LapConfigs.current_lap_cfg;
   CPPUNIT_ASSERT_EQUAL(string("CurrentLap"), string(cc->label));
   CPPUNIT_ASSERT_EQUAL(string(""), string(cc->units));
   CPPUNIT_ASSERT(cc->sampleRate == SAMPLE_10Hz);

   cc = &lc->LapConfigs.lapTimeCfg;
   CPPUNIT_ASSERT_EQUAL(string("LapTime"), string(cc->label));
   CPPUNIT_ASSERT_EQUAL(string("Min"), string(cc->units));
   CPPUNIT_ASSERT(cc->sampleRate == SAMPLE_10Hz);

   cc = &lc->LapConfigs.sectorCfg;
   CPPUNIT_ASSERT_EQUAL(string("Sector"), string(cc->label));
   CPPUNIT_ASSERT_EQUAL(string(""), string(cc->units));
   CPPUNIT_ASSERT(cc->sampleRate == SAMPLE_10Hz);

   cc = &lc->LapConfigs.sectorTimeCfg;
   CPPUNIT_ASSERT_EQUAL(string("SectorTime"), string(cc->label));
   CPPUNIT_ASSERT_EQUAL(string("Min"), string(cc->units));
   CPPUNIT_ASSERT(cc->sampleRate == SAMPLE_10Hz);

   cc = &lc->LapConfigs.predTimeCfg;
   CPPUNIT_ASSERT_EQUAL(string("PredTime"), string(cc->label));
   CPPUNIT_ASSERT_EQUAL(string("Min"), string(cc->units));
   CPPUNIT_ASSERT(cc->sampleRate == SAMPLE_5Hz);

   cc = &lc->LapConfigs.elapsed_time_cfg;
   CPPUNIT_ASSERT_EQUAL(string("ElapsedTime"), string(cc->label));
   CPPUNIT_ASSERT_EQUAL(string("Min"), string(cc->units));
   CPPUNIT_ASSERT(cc->sampleRate == SAMPLE_10Hz);

}


void LoggerConfigTest::testLoggerInitConnectivityConfig() {
   LoggerConfig *lc = getWorkingLoggerConfig();

   BluetoothConfig *btc = &lc->ConnectivityConfigs.bluetoothConfig;

   CPPUNIT_ASSERT_EQUAL(string(""), string(btc->new_name));
   CPPUNIT_ASSERT_EQUAL(string(""), string(btc->new_pin));
   CPPUNIT_ASSERT_EQUAL(DEFAULT_BT_ENABLED, (int) btc->btEnabled);

   CellularConfig *cc = &lc->ConnectivityConfigs.cellularConfig;
   CPPUNIT_ASSERT_EQUAL(DEFAULT_CELL_ENABLED, (int) cc->cellEnabled);
   CPPUNIT_ASSERT_EQUAL(string(DEFAULT_APN_HOST), string(cc->apnHost));
   CPPUNIT_ASSERT_EQUAL(string(DEFAULT_APN_USER), string(cc->apnUser));
   CPPUNIT_ASSERT_EQUAL(string(DEFAULT_APN_PASS), string(cc->apnPass));

   TelemetryConfig *tc = &lc->ConnectivityConfigs.telemetryConfig;
   CPPUNIT_ASSERT_EQUAL(true, (bool) tc->backgroundStreaming);
   CPPUNIT_ASSERT_EQUAL((size_t) 0, strlen(tc->telemetryDeviceId));
   CPPUNIT_ASSERT_EQUAL(string(DEFAULT_TELEMETRY_SERVER_HOST),
                        string(tc->telemetryServerHost));
}
