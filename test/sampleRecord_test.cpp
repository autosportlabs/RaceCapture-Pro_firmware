#include "sampleRecord_test.h"
#include "GPIO.h"
#include "loggerConfig.h"
#include "predictive_timer_2.h"
#include "sampleRecord.h"
#include "loggerSampleData.h"
#include "sampleRecord_test.h"
#include "loggerHardware.h"
#include "ADC_mock.h"
#include "ADC.h"
#include "imu.h"
#include "gps.h"
#include "task.h"
#include "capabilities.h"

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
   initGPS();

	//mock up some values to test later
	lc->ADCConfigs[7].scalingMode = SCALING_MODE_RAW;
	ADC_mock_set_value(7, 123);
	ADC_sample_all();

	size_t channelCount = get_enabled_channel_count(lc);
	ChannelSample * samples = create_channel_sample_buffer(lc, channelCount);
	init_channel_sample_buffer(lc, samples, channelCount);

   // Set it so we have 1 tick.
   resetTicks();
   incrementTick();
   CPPUNIT_ASSERT_EQUAL(1, (int) (xTaskGetTickCount()));

	const unsigned short highSampleRate =
     (unsigned short) populate_sample_buffer(samples, channelCount, 0);

   // Interval Channel
   CPPUNIT_ASSERT_EQUAL(highSampleRate, samples->cfg->sampleRate);
   CPPUNIT_ASSERT_EQUAL((int) (xTaskGetTickCount() * MS_PER_TICK), samples->valueInt);


   // UtC Channel.  Just test that its 0 for now
   samples++;
   CPPUNIT_ASSERT_EQUAL(highSampleRate, samples->cfg->sampleRate);
   CPPUNIT_ASSERT_EQUAL(0ll, (long long) getMillisSinceEpoch());
   CPPUNIT_ASSERT_EQUAL(0ll, samples->valueLongLong);


	//analog channel
   samples++;
	CPPUNIT_ASSERT_EQUAL(123 * 0.0048828125f, samples->valueFloat);

	//accelerometer channels
	samples++;
	CPPUNIT_ASSERT_EQUAL(imu_read_value(0, &lc->ImuConfigs[0]), samples->valueFloat);

	samples++;
	CPPUNIT_ASSERT_EQUAL(imu_read_value(1, &lc->ImuConfigs[1]), samples->valueFloat);

	samples++;
	CPPUNIT_ASSERT_EQUAL(imu_read_value(2, &lc->ImuConfigs[2]), samples->valueFloat);

	samples++;
	CPPUNIT_ASSERT_EQUAL(imu_read_value(3, &lc->ImuConfigs[3]), samples->valueFloat);

	//GPS / Track channels
        /*
         * !!! BE WARNED!!!  It seems some of these samples should be valueInts instead of
         * valueFloat.  If you are going to change it to a non-zero value, you will
         * need to double check this.  GDB was the quickest way I found to debug this.
         * Else you will enter a world of pain as I did trying to figure out why you
         * get NaN or something else weird that you didn't expect.
         */
	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->valueFloat);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->valueFloat);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->valueFloat);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->valueFloat);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->valueFloat);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->valueFloat);

	samples++;
	CPPUNIT_ASSERT_EQUAL((float)0, samples->valueFloat);

        samples++;
        CPPUNIT_ASSERT_EQUAL(-1, samples->valueInt);
}

void SampleRecordTest::testInitSampleRecord()
{
   LoggerConfig *lc = getWorkingLoggerConfig();

   size_t expectedEnabledChannels = 17;

   size_t channelCount = get_enabled_channel_count(lc);
   CPPUNIT_ASSERT_EQUAL(expectedEnabledChannels, channelCount);

   ChannelSample * samples = create_channel_sample_buffer(lc, channelCount);
   init_channel_sample_buffer(lc, samples, channelCount);

   ChannelSample * ts = samples;

   const struct TimeConfig *tc = lc->TimeConfigs;

   // Check what should be Uptime (Interval)
   CPPUNIT_ASSERT_EQUAL(string("Interval"), string(tc->cfg.label));
   CPPUNIT_ASSERT_EQUAL(string("ms"), string(tc->cfg.units));
   CPPUNIT_ASSERT_EQUAL(TimeType_Uptime, tc->tt);
   CPPUNIT_ASSERT_EQUAL(SampleData_Int_Noarg, ts->sampleData);
   ++ts;
   ++tc;

   // Check what should be the Utc
   CPPUNIT_ASSERT_EQUAL(string("Utc"), string(tc->cfg.label));
   CPPUNIT_ASSERT_EQUAL(string("ms"), string(tc->cfg.units));
   CPPUNIT_ASSERT_EQUAL(TimeType_UtcMillis, tc->tt);
   CPPUNIT_ASSERT_EQUAL(SampleData_LongLong_Noarg, ts->sampleData);
   ++ts;

   for (int i = 0; i < CONFIG_ADC_CHANNELS; i++){
      ADCConfig *ac = &lc->ADCConfigs[i];
      if (ac->cfg.sampleRate != SAMPLE_DISABLED){
         CPPUNIT_ASSERT_EQUAL((size_t)i, ts->channelIndex);
         CPPUNIT_ASSERT_EQUAL((void *) &ac->cfg, (void *) ts->cfg);
         CPPUNIT_ASSERT_EQUAL((void *) get_analog_sample, (void *)ts->get_float_sample);
         CPPUNIT_ASSERT_EQUAL(SampleData_Float, ts->sampleData);
         ts++;
      }
   }

   for (int i = 0; i < CONFIG_IMU_CHANNELS; i++){
      ImuConfig *ac = &lc->ImuConfigs[i];
      if (ac->cfg.sampleRate != SAMPLE_DISABLED){
         CPPUNIT_ASSERT_EQUAL((size_t)i,ts->channelIndex);
         CPPUNIT_ASSERT_EQUAL((void *) &ac->cfg, (void *) ts->cfg);
         CPPUNIT_ASSERT_EQUAL((void *)get_imu_sample, (void *)ts->get_float_sample);
         CPPUNIT_ASSERT_EQUAL(SampleData_Float, ts->sampleData);
         ts++;
      }
   }

   for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
      TimerConfig *tc = &lc->TimerConfigs[i];
      if (tc->cfg.sampleRate != SAMPLE_DISABLED){
         CPPUNIT_ASSERT_EQUAL((size_t)i, ts->channelIndex);
         CPPUNIT_ASSERT_EQUAL((void *) &tc->cfg, (void *) ts->cfg);
         CPPUNIT_ASSERT_EQUAL((void *)get_timer_sample, (void *)ts->get_float_sample);
         CPPUNIT_ASSERT_EQUAL(SampleData_Float, ts->sampleData);
         ts++;
      }
   }

   for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
      GPIOConfig *gc = &lc->GPIOConfigs[i];
      if (gc->cfg.sampleRate != SAMPLE_DISABLED){
         CPPUNIT_ASSERT_EQUAL((size_t)i, ts->channelIndex);
         CPPUNIT_ASSERT_EQUAL((void *) &gc->cfg, (void *) ts->cfg);
         CPPUNIT_ASSERT_EQUAL((void *) GPIO_get, (void *) ts->get_int_sample);
         CPPUNIT_ASSERT_EQUAL(SampleData_Int, ts->sampleData);
         ts++;
      }
   }

   for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
      PWMConfig *pc = &lc->PWMConfigs[i];
      if (pc->cfg.sampleRate != SAMPLE_DISABLED){
         CPPUNIT_ASSERT_EQUAL((size_t)i, ts->channelIndex);
         CPPUNIT_ASSERT_EQUAL((void *) &pc->cfg, (void *) ts->cfg);
         CPPUNIT_ASSERT_EQUAL((void *)get_pwm_sample, (void *)ts->get_int_sample);
         CPPUNIT_ASSERT_EQUAL(SampleData_Int, ts->sampleData);
         ts++;
      }
   }


   GPSConfig *gpsConfig = &(lc->GPSConfigs);

   if (gpsConfig->latitude.sampleRate != SAMPLE_DISABLED){
      CPPUNIT_ASSERT_EQUAL((void *) &gpsConfig->latitude, (void *) ts->cfg);
      CPPUNIT_ASSERT_EQUAL((void *) getLatitude, (void *) ts->get_float_sample);
      CPPUNIT_ASSERT_EQUAL(SampleData_Float_Noarg, ts->sampleData);
      ts++;
   }

   if (gpsConfig->longitude.sampleRate != SAMPLE_DISABLED){
      CPPUNIT_ASSERT_EQUAL((void *) &gpsConfig->longitude, (void *) ts->cfg);
      CPPUNIT_ASSERT_EQUAL((void *) getLongitude, (void *) ts->get_float_sample);
      CPPUNIT_ASSERT_EQUAL(SampleData_Float_Noarg, ts->sampleData);
      ts++;
   }

   if (gpsConfig->speed.sampleRate != SAMPLE_DISABLED){
      CPPUNIT_ASSERT_EQUAL((void *) &gpsConfig->speed, (void *) ts->cfg);
      CPPUNIT_ASSERT_EQUAL((void *) getGpsSpeedInMph, (void *) ts->get_float_sample);
      CPPUNIT_ASSERT_EQUAL(SampleData_Float_Noarg, ts->sampleData);
      ts++;
   }

   if (gpsConfig->distance.sampleRate != SAMPLE_DISABLED){
      CPPUNIT_ASSERT_EQUAL((void *) &gpsConfig->distance, (void *) ts->cfg);
      CPPUNIT_ASSERT_EQUAL((void *) getGpsDistance, (void *) ts->get_float_sample);
      CPPUNIT_ASSERT_EQUAL(SampleData_Float_Noarg, ts->sampleData);
      ts++;
   }

   if (gpsConfig->satellites.sampleRate != SAMPLE_DISABLED){
      CPPUNIT_ASSERT_EQUAL((void *) &gpsConfig->satellites, (void *) ts->cfg);
      CPPUNIT_ASSERT_EQUAL((void *) getSatellitesUsedForPosition, (void *) ts->get_int_sample);
      CPPUNIT_ASSERT_EQUAL(SampleData_Int_Noarg, ts->sampleData);
      ts++;
   }

   LapConfig *lapConfig = &(lc->LapConfigs);
   if (lapConfig->lapCountCfg.sampleRate != SAMPLE_DISABLED){
      CPPUNIT_ASSERT_EQUAL((void *) &lapConfig->lapCountCfg, (void *) ts->cfg);
      CPPUNIT_ASSERT_EQUAL(SampleData_Int_Noarg, ts->sampleData);
      CPPUNIT_ASSERT_EQUAL((void *) getLapCount, (void *) ts->get_int_sample);
      ts++;
   }

   if (lapConfig->lapTimeCfg.sampleRate != SAMPLE_DISABLED){
      CPPUNIT_ASSERT_EQUAL((void *) &lapConfig->lapTimeCfg, (void *) ts->cfg);
      CPPUNIT_ASSERT_EQUAL(SampleData_Float_Noarg, ts->sampleData);
      CPPUNIT_ASSERT_EQUAL((void *) getLastLapTimeInMinutes, (void *) ts->get_float_sample);
      ts++;
   }

   if (lapConfig->sectorCfg.sampleRate != SAMPLE_DISABLED){
      CPPUNIT_ASSERT_EQUAL((void *) &lapConfig->sectorCfg, (void *) ts->cfg);
      CPPUNIT_ASSERT_EQUAL(SampleData_Int_Noarg, ts->sampleData);
      CPPUNIT_ASSERT_EQUAL((void *) getLastSector, (void *) ts->get_int_sample);
      ts++;
   }

   if (lapConfig->sectorTimeCfg.sampleRate != SAMPLE_DISABLED){
      CPPUNIT_ASSERT_EQUAL((void *) &lapConfig->sectorTimeCfg, (void *) ts->cfg);
      CPPUNIT_ASSERT_EQUAL(SampleData_Float_Noarg, ts->sampleData);
      CPPUNIT_ASSERT_EQUAL((void *) getLastSectorTimeInMinutes, (void *) ts->get_float_sample);
      ts++;
   }

   if (lapConfig->predTimeCfg.sampleRate != SAMPLE_DISABLED){
      CPPUNIT_ASSERT_EQUAL((void *) &lapConfig->predTimeCfg, (void *) ts->cfg);
      CPPUNIT_ASSERT_EQUAL(SampleData_Float_Noarg, ts->sampleData);
      CPPUNIT_ASSERT_EQUAL((void *) getPredictedTimeInMinutes, (void *) ts->get_float_sample);
      ts++;
   }

   //amount shoud match
   unsigned int size = (unsigned int)ts - (unsigned int)samples;
   CPPUNIT_ASSERT_EQUAL(expectedEnabledChannels * sizeof(ChannelSample), size);
}
