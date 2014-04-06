#include "sampleRecord_test.h"
#include "loggerConfig.h"
#include "sampleRecord.h"
#include "loggerSampleData.h"
#include "sampleRecord_test.h"
#include "loggerHardware.h"
#include "ADC_mock.h"
#include "ADC.h"
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
	initialize_logger_config();
}


void SampleRecordTest::tearDown()
{
}


void SampleRecordTest::testPopulateSampleRecord(){
	LoggerConfig *lc = getWorkingLoggerConfig();

	//mock up some values to test later
	lc->ADCConfigs[7].scalingMode = SCALING_MODE_RAW;
	ADC_mock_set_value(7, 123);
	ADC_sample_all();

	size_t channelCount = get_enabled_channel_count(lc);
	ChannelSample * samples = create_channel_sample_buffer(lc, channelCount);
	init_channel_sample_buffer(lc, samples, channelCount);

	populate_sample_buffer(samples, channelCount, 0);

	//analog channel
	CPPUNIT_ASSERT_EQUAL(123.0F, samples->floatValue);

	//accelerometer channels
	samples++;
	CPPUNIT_ASSERT_EQUAL(accelerometer_read_value(0, &lc->AccelConfigs[0]), samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL(accelerometer_read_value(1, &lc->AccelConfigs[1]), samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL(accelerometer_read_value(2, &lc->AccelConfigs[2]), samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL(accelerometer_read_value(3, &lc->AccelConfigs[3]), samples->floatValue);

	//GPS / Track channels
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
			CPPUNIT_ASSERT_EQUAL(ac->cfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(ac->cfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL((void *)get_analog_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		AccelConfig *ac = &lc->AccelConfigs[i];
		if (ac->cfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((size_t)i,ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL(ac->cfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(ac->cfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL((void *)get_accel_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *tc = &lc->TimerConfigs[i];
		if (tc->cfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((size_t)i, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL(tc->cfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(tc->cfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL((void *)get_timer_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIOConfig *gc = &lc->GPIOConfigs[i];
		if (gc->cfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((size_t)i, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL(gc->cfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(gc->cfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL((void *)get_gpio_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *pc = &lc->PWMConfigs[i];
		if (pc->cfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((size_t)i, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL(pc->cfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(pc->cfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL((void *)get_timer_sample, (void *)ts->get_sample);
			ts++;
		}
	}

	{
		GPSConfig *gpsConfig = &(lc->GPSConfigs);
		if (gpsConfig->latitudeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(gpsConfig->latitudeCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(gpsConfig->latitudeCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_latitude, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
			ts++;
		}

		if (gpsConfig->longitudeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(gpsConfig->longitudeCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(gpsConfig->longitudeCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_longitude, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
			ts++;
		}

		if (gpsConfig->speedCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(gpsConfig->speedCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(gpsConfig->speedCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_speed, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
			ts++;
		}

		if (gpsConfig->timeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(gpsConfig->timeCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(gpsConfig->timeCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_time, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
			ts++;
		}

		if (gpsConfig->satellitesCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(gpsConfig->satellitesCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(gpsConfig->satellitesCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_satellites, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
			ts++;
		}

		if (gpsConfig->distanceCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(gpsConfig->distanceCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(gpsConfig->distanceCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_distance, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
			ts++;
		}

	}

	{
		LapConfig *trackConfig = &(lc->LapConfigs);
		if (trackConfig->lapCountCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(trackConfig->lapCountCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(trackConfig->lapCountCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_lapcount, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}

		if (trackConfig->lapTimeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(trackConfig->lapTimeCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(trackConfig->lapTimeCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_laptime, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}

		if (trackConfig->splitTimeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(trackConfig->splitTimeCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(trackConfig->splitTimeCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_splittime, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}

		if (trackConfig->predTimeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(trackConfig->predTimeCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(trackConfig->predTimeCfg.sampleRate, ts->sampleRate);
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
