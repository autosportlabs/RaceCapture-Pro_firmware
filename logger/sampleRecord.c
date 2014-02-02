/*
 * sampleRecord.c
 *
 *  Created on: Feb 29, 2012
 *      Author: brent
 */
#include "sampleRecord.h"
#include "loggerConfig.h"
#include "mem_mang.h"

ChannelSample ** create_channel_sample_buffer(LoggerConfig *loggerConfig){
	size_t channelCount = get_enabled_channel_count(loggerConfig);
	ChannelSample ** samples = portMalloc(sizeof(ChannelSample[channelCount]));
	return samples;
}

void init_channel_sample_buffer(ChannelSample ** samples, size_t count){
	ChannelSample *sample = samples[0];

	//todo WIP!!

	for (int i=0; i < CONFIG_ACCEL_CHANNELS; i++){
		if (loggerConfig->AccelConfigs[i].cfg.sampleRate != SAMPLE_DISABLED) channels++;
	}

	for (int i=0; i < CONFIG_ADC_CHANNELS; i++){
		if (loggerConfig->ADCConfigs[i].cfg.sampleRate != SAMPLE_DISABLED) channels++;
	}

	for (int i=0; i < CONFIG_TIMER_CHANNELS; i++){
		if (loggerConfig->TimerConfigs[i].cfg.sampleRate != SAMPLE_DISABLED) channels++;
	}

	for (int i=0; i < CONFIG_GPIO_CHANNELS; i++){
		if (loggerConfig->GPIOConfigs[i].cfg.cfg.sampleRate != SAMPLE_DISABLED) channels++;
	}

	for (int i=0; i < CONFIG_PWM_CHANNELS; i++){
		if (loggerConfig->PWMConfigs[i].cfg.sampleRate != SAMPLE_DISABLED) channels++;
	}

	if (loggerConfig->GPSConfigs.latitudeCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (loggerConfig->GPSConfigs.longitudeCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (loggerConfig->GPSConfigs.speedCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (loggerConfig->GPSConfigs.timeCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (loggerConfig->GPSConfigs.satellitesCfg.sampleRate != SAMPLE_DISABLED) channels++;

	if (loggerConfig->TrackConfigs.lapCountCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (loggerConfig->TrackConfigs.lapTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (loggerConfig->TrackConfigs.splitTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (loggerConfig->TrackConfigs.distanceCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (loggerConfig->TrackConfigs.predTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;


}
void initSampleRecord(LoggerConfig *loggerConfig,SampleRecord *sr){

	for (int i=0; i < CONFIG_ADC_CHANNELS; i++){
		ChannelSample *s = &(sr->ADCSamples[i]);
		ADCConfig *adcCfg = &(loggerConfig->ADCConfigs[i]);
		s->precision = adcCfg->loggingPrecision;
		s->channelConfig = &(adcCfg->cfg);
		s->intValue = NIL_SAMPLE;
	}
	for (int i=0; i < CONFIG_PWM_CHANNELS; i++){
		ChannelSample *s = &(sr->PWMSamples[i]);
		PWMConfig *pwmCfg = &(loggerConfig->PWMConfigs[i]);
		s->precision = pwmCfg->loggingPrecision;
		s->channelConfig = &(pwmCfg->cfg);
		s->intValue = NIL_SAMPLE;
	}
	for (int i=0; i < CONFIG_GPIO_CHANNELS; i++){
		ChannelSample *s = &(sr->GPIOSamples[i]);
		s->precision = DEFAULT_GPIO_LOGGING_PRECISION;
		s->channelConfig = &(loggerConfig->GPIOConfigs[i].cfg);
		s->intValue = NIL_SAMPLE;
	}
	for (int i=0; i < CONFIG_TIMER_CHANNELS; i++){
		ChannelSample *s = &(sr->TimerSamples[i]);
		TimerConfig *timerCfg = &(loggerConfig->TimerConfigs[i]);
		s->precision = timerCfg->loggingPrecision;
		s->channelConfig = &(timerCfg->cfg);
		s->intValue = NIL_SAMPLE;
	}
	for (int i=0; i < CONFIG_ACCEL_CHANNELS; i++){
		ChannelSample *s = &(sr->AccelSamples[i]);
		s->precision = DEFAULT_ACCEL_LOGGING_PRECISION;
		s->channelConfig = &(loggerConfig->AccelConfigs[i].cfg);
		s->intValue = NIL_SAMPLE;
	}
	{
		ChannelSample *s = &(sr->GPS_LatitueSample);
		s->precision = DEFAULT_GPS_POSITION_LOGGING_PRECISION;
		s->channelConfig = &(loggerConfig->GPSConfigs.latitudeCfg);
		s->intValue = NIL_SAMPLE;
	}
	{
		ChannelSample *s = &(sr->GPS_LongitudeSample);
		s->precision = DEFAULT_GPS_POSITION_LOGGING_PRECISION;
		s->channelConfig = &(loggerConfig->GPSConfigs.longitudeCfg);
		s->intValue = NIL_SAMPLE;
	}
	{
		ChannelSample *s = &(sr->GPS_SpeedSample);
		s->precision = DEFAULT_GPS_SPEED_LOGGING_PRECISION;
		s->channelConfig = &(loggerConfig->GPSConfigs.speedCfg);
		s->intValue = NIL_SAMPLE;
	}
	{
		ChannelSample *s = &(sr->GPS_TimeSample);
		s->precision = DEFAULT_GPS_TIME_LOGGING_PRECISION;
		s->channelConfig = &(loggerConfig->GPSConfigs.timeCfg);
		s->intValue = NIL_SAMPLE;
	}
	{
		ChannelSample *s = &(sr->Track_LapCountSample);
		s->precision = DEFAULT_LAP_COUNT_LOGGING_PRECISION;
		s->channelConfig = &(loggerConfig->TrackConfigs.lapCountCfg);
		s->intValue = NIL_SAMPLE;
	}
	{
		ChannelSample *s = &(sr->Track_LapTimeSample);
		s->precision = DEFAULT_LAP_TIME_LOGGING_PRECISION;
		s->channelConfig = &(loggerConfig->TrackConfigs.lapTimeCfg);
		s->intValue = NIL_SAMPLE;
	}
	{
		ChannelSample *s = &(sr->Track_SplitTimeSample);
		s->precision = DEFAULT_LAP_TIME_LOGGING_PRECISION;
		s->channelConfig =  &(loggerConfig->TrackConfigs.splitTimeCfg);
		s->intValue = NIL_SAMPLE;
	}
	{
		ChannelSample *s = &(sr->Track_DistanceSample);
		s->precision = DEFAULT_DISTANCE_LOGGING_PRECISION;
		s->channelConfig =  &(loggerConfig->TrackConfigs.distanceCfg);
		s->intValue = NIL_SAMPLE;
	}
	{
		ChannelSample *s = &(sr->Track_PredTimeSample);
		s->precision = DEFAULT_LAP_TIME_LOGGING_PRECISION;
		s->channelConfig =  &(loggerConfig->TrackConfigs.predTimeCfg);
		s->intValue = NIL_SAMPLE;
	}
	{
		ChannelSample *s = &(sr->GPS_SatellitesSample);
		s->precision = DEFAULT_GPS_SATELLITES_LOGGING_PRECISION;
		s->channelConfig = &(loggerConfig->GPSConfigs.satellitesCfg);
		s->intValue = NIL_SAMPLE;
	}
}

void clearSampleRecordBuffer(SampleRecord **srBuff, int size){
	for (int i=0;i < size; i++){
		SampleRecord *sr = srBuff[i];
		clearSampleRecord(sr);
	}
}

void clearSampleRecord(SampleRecord* sr){
	for (int s=0; s < SAMPLE_RECORD_CHANNELS; s++){
		sr->Samples[s].intValue = NIL_SAMPLE;
	}
}

SampleRecord ** createSampleRecordBuffer(LoggerConfig *loggerConfig, int size){
	SampleRecord ** srBuff = (SampleRecord **)portMalloc(sizeof(SampleRecord[size]));
	initSampleRecordBuffer(loggerConfig, srBuff, size);
	return srBuff;
}

void initSampleRecordBuffer(LoggerConfig *loggerConfig, SampleRecord ** srBuff,int size){
	for (int i=0; i < size; i++) initSampleRecord(loggerConfig,srBuff[i]);
}

void freeSampleRecordBuffer(SampleRecord ** sampleRecordBuffer){
	portFree(sampleRecordBuffer);
}
