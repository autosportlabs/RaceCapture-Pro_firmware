#include "sampleRecord_test.h"
#include "loggerConfig.h"
#include "sampleRecord.h"
#include "loggerSampleData.h"
#include "sampleRecord_test.h"
#include "loggerHardware.h"
#include "ADC_mock.h"
#include "ADC.h"
#include "imu.h"
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
	CPPUNIT_ASSERT_EQUAL(123 * 0.0048875f, samples->floatValue);

	//accelerometer channels
	samples++;
	CPPUNIT_ASSERT_EQUAL(imu_read_value(0, &lc->ImuConfigs[0]), samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL(imu_read_value(1, &lc->ImuConfigs[1]), samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL(imu_read_value(2, &lc->ImuConfigs[2]), samples->floatValue);

	samples++;
	CPPUNIT_ASSERT_EQUAL(imu_read_value(3, &lc->ImuConfigs[3]), samples->floatValue);

	//GPS / Track channels
        /*
         * !!! BE WARNED!!!  It seems some of these samples should be intValues instead of
         * floatValue.  If you are going to change it to a non-zero value, you will
         * need to double check this.  GDB was the quickest way I found to debug this.
         * Else you will enter a world of pain as I did trying to figure out why you
         * get NaN or something else weird that you didn't expect.
         */
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
        CPPUNIT_ASSERT_EQUAL(-1, samples->intValue);
}

void SampleRecordTest::testInitSampleRecord()
{
	LoggerConfig *lc = getWorkingLoggerConfig();

	size_t expectedEnabledChannels = 14;

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

	for (int i = 0; i < CONFIG_IMU_CHANNELS; i++){
		ImuConfig *ac = &lc->ImuConfigs[i];
		if (ac->cfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL((size_t)i,ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL(ac->cfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(ac->cfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL((void *)get_imu_sample, (void *)ts->get_sample);
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

		if (gpsConfig->sampleRate != SAMPLE_DISABLED){
			if (gpsConfig->positionEnabled){
				CPPUNIT_ASSERT_EQUAL((unsigned short)CHANNEL_Latitude, ts->channelId);
				CPPUNIT_ASSERT_EQUAL(gpsConfig->sampleRate, ts->sampleRate);
				CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
				CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_latitude, ts->channelIndex);
				CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
				ts++;
			}
			{
				CPPUNIT_ASSERT_EQUAL((unsigned short)CHANNEL_Longitude, ts->channelId);
				CPPUNIT_ASSERT_EQUAL(gpsConfig->sampleRate, ts->sampleRate);
				CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
				CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_longitude, ts->channelIndex);
				CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
				ts++;
			}

			if (gpsConfig->speedEnabled){
				CPPUNIT_ASSERT_EQUAL((unsigned short)CHANNEL_Speed, ts->channelId);
				CPPUNIT_ASSERT_EQUAL(gpsConfig->sampleRate, ts->sampleRate);
				CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
				CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_speed, ts->channelIndex);
				CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
				ts++;
			}

			if (gpsConfig->timeEnabled){
				CPPUNIT_ASSERT_EQUAL((unsigned short)CHANNEL_Time, ts->channelId);
				CPPUNIT_ASSERT_EQUAL(gpsConfig->sampleRate, ts->sampleRate);
				CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
				CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_time, ts->channelIndex);
				CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
				ts++;
			}

			if (gpsConfig->satellitesEnabled){
				CPPUNIT_ASSERT_EQUAL((unsigned short)CHANNEL_GPSSats, ts->channelId);
				CPPUNIT_ASSERT_EQUAL(gpsConfig->sampleRate, ts->sampleRate);
				CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
				CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_satellites, ts->channelIndex);
				CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
				ts++;
			}

			if (gpsConfig->distanceEnabled){
				CPPUNIT_ASSERT_EQUAL((unsigned short)CHANNEL_Distance, ts->channelId);
				CPPUNIT_ASSERT_EQUAL(gpsConfig->sampleRate, ts->sampleRate);
				CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
				CPPUNIT_ASSERT_EQUAL((size_t)gps_channel_distance, ts->channelIndex);
				CPPUNIT_ASSERT_EQUAL((void *)get_gps_sample, (void *)ts->get_sample);
				ts++;
			}
		}
	}

	{
		LapConfig *lapConfig = &(lc->LapConfigs);
		if (lapConfig->lapCountCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(lapConfig->lapCountCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(lapConfig->lapCountCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_lapcount, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}

		if (lapConfig->lapTimeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(lapConfig->lapTimeCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(lapConfig->lapTimeCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_laptime, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}

		if (lapConfig->sectorCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(lapConfig->sectorCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(lapConfig->sectorCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_sector, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}

		if (lapConfig->sectorTimeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(lapConfig->sectorTimeCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(lapConfig->sectorTimeCfg.sampleRate, ts->sampleRate);
			CPPUNIT_ASSERT_EQUAL(NIL_SAMPLE, ts->intValue);
			CPPUNIT_ASSERT_EQUAL((size_t)lap_stat_channel_sectortime, ts->channelIndex);
			CPPUNIT_ASSERT_EQUAL((void *)get_lap_stat_sample, (void *)ts->get_sample);
			ts++;
		}

		if (lapConfig->predTimeCfg.sampleRate != SAMPLE_DISABLED){
			CPPUNIT_ASSERT_EQUAL(lapConfig->predTimeCfg.channeId, ts->channelId);
			CPPUNIT_ASSERT_EQUAL(lapConfig->predTimeCfg.sampleRate, ts->sampleRate);
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
