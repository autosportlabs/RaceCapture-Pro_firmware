#include "sampleRecord_test.h"
#include "loggerConfig.h"
#include "sampleRecord.h"
#include "loggerSampleData.h"
#include "sampleRecord_test.h"
#include "loggerHardware.h"
#include "ADC_mock.h"
#include "accelerometer.h"
#include "gps.h"

#include <string>

using std::string;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( SampleRecordTest );

void SampleRecordTest::setUp()
{
	InitLoggerHardware();
	initGPS();
	updateActiveLoggerConfig();
}


void SampleRecordTest::tearDown()
{
}


void SampleRecordTest::testPopulateSampleRecord(){
	LoggerConfig *lc = getWorkingLoggerConfig();

	//mock up some values to test later
	lc->ADCConfigs[7].scalingMode = SCALING_MODE_RAW;
	lc->ADCConfigs[7].loggingPrecision = 0;
	ADC_mock_set_value(7, 123);

	size_t channelCount = get_enabled_channel_count(lc);
	ChannelSample * samples = create_channel_sample_buffer(lc, channelCount);
	init_channel_sample_buffer(lc, samples, channelCount);

	populate_sample_buffer(samples, channelCount, 0);

	CPPUNIT_ASSERT_EQUAL(accelerometer_read_value(0, &lc->AccelConfigs[0]), samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL(accelerometer_read_value(1, &lc->AccelConfigs[1]), samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL(accelerometer_read_value(2, &lc->AccelConfigs[2]), samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL(accelerometer_read_value(3, &lc->AccelConfigs[3]), samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL(123, samples->intValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->floatValue);

}

void SampleRecordTest::testInitSampleRecord()
{
	LoggerConfig *lc = getWorkingLoggerConfig();

	size_t expectedEnabledChannels = 13;

	size_t channelCount = get_enabled_channel_count(lc);
	CPPUNIT_ASSERT_EQUAL(channelCount, expectedEnabledChannels); //current logger config enables 13 channels

	ChannelSample * samples = create_channel_sample_buffer(lc, channelCount);
	init_channel_sample_buffer(lc, samples, channelCount);

	ChannelSample * ts = samples;

	for (int i = 0; i < CONFIG_ADC_CHANNELS; i++){
		ADCConfig *ac = &lc->ADCConfigs[i];
		if (ac->cfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((size_t)i, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL(ac->loggingPrecision, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&ac->cfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL((void *)get_analog_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		AccelConfig *ac = &lc->AccelConfigs[i];
		if (ac->cfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((size_t)i,ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_ACCEL_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&ac->cfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL((void *)get_accel_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *tc = &lc->TimerConfigs[i];
		if (tc->cfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((size_t)i, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL(tc->loggingPrecision, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&tc->cfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL((void *)get_timer_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIOConfig *gc = &lc->GPIOConfigs[i];
		if (gc->cfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((size_t)i, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_GPIO_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&gc->cfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL((void *)get_gpio_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *pc = &lc->PWMConfigs[i];
		if (pc->cfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((size_t)i, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL(pc->loggingPrecision, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&pc->cfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL((void *)get_timer_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	{
		GPSConfig *gpsConfig = &(lc->GPSConfigs);
		if (gpsConfig->latitudeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_GPS_POSITION_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&gpsConfig->latitudeCfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_latitude, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
			ts++;
		}

		if (gpsConfig->longitudeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_GPS_POSITION_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&gpsConfig->longitudeCfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_longitude, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
			ts++;
		}

		if (gpsConfig->speedCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_GPS_SPEED_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&gpsConfig->speedCfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_speed, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
			ts++;
		}

		if (gpsConfig->timeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_GPS_TIME_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&gpsConfig->timeCfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_time, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
			ts++;
		}

		if (gpsConfig->satellitesCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_GPS_SATELLITES_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&gpsConfig->satellitesCfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_satellites, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	{
		TrackConfig *trackConfig = &(lc->TrackConfigs);
		if (trackConfig->lapCountCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_LAP_COUNT_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&trackConfig->lapCountCfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_lapcount, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}

		if (trackConfig->lapTimeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_LAP_TIME_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&trackConfig->lapTimeCfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_laptime, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}

		if (trackConfig->splitTimeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_LAP_TIME_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&trackConfig->splitTimeCfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_splittime, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}

		if (trackConfig->distanceCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_DISTANCE_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&trackConfig->distanceCfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_distance, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}

		if (trackConfig->predTimeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((unsigned int)DEFAULT_LAP_COUNT_LOGGING_PRECISION, ts->precision);
			CPPUNIT_ASSERT_EQUAL((void *)&trackConfig->predTimeCfg, (void *)ts->channelConfig);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_predtime, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	//amount shoud match
	unsigned int size = (unsigned int)ts - (unsigned int)samples;
	CPPUNIT_ASSERT_EQUAL(expectedEnabledChannels * sizeof(ChannelSample), size);
}
