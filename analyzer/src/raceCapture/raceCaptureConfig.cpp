/*
 * raceCaptureConfig.cpp
 *
 *  Created on: Apr 28, 2012
 *      Author: brent
 */
#include "raceCapture/raceCaptureConfig.h"

SampleRates ChannelConfig::sampleRates;

ChannelConfig::ChannelConfig(){
}

SampleRates ChannelConfig::GetSampleRates(){

//	const wxString SAMPLE_RATES[] = {"Disabled","1","5","10","20","30","50","100"};
	//TODO make threadsafe or change design
	if (ChannelConfig::sampleRates.Count() == 0){
		ChannelConfig::sampleRates.Add(sample_disabled);
		ChannelConfig::sampleRates.Add(sample_1Hz);
		ChannelConfig::sampleRates.Add(sample_5Hz);
		ChannelConfig::sampleRates.Add(sample_10Hz);
		ChannelConfig::sampleRates.Add(sample_20Hz);
		ChannelConfig::sampleRates.Add(sample_30Hz);
		ChannelConfig::sampleRates.Add(sample_50Hz);
		ChannelConfig::sampleRates.Add(sample_100Hz);
	}
	return sampleRates;
}

void RaceCaptureConfig::SetDefaults(){

}

void RaceCaptureConfig::ChannelConfigFromJson(ChannelConfig &channelConfig, const Object &channelConfigJson){

	channelConfig.label = ((String)channelConfigJson["label"]).Value();
	channelConfig.units = ((String)channelConfigJson["units"]).Value();
	channelConfig.sampleRate = (sample_rate_t)((Number)channelConfigJson["sampleRate"]).Value();
}


void RaceCaptureConfig::PopulateGpsConfig(Object &gpsRoot){
	gpsConfig.gpsInstalled = ((Number)gpsRoot["gpsInstalled"]).Value();
	gpsConfig.startFinishLatitude = ((Number)gpsRoot["startFinishLatitude"]).Value();
	gpsConfig.startFinishLongitude = ((Number)gpsRoot["startFinishLongitude"]).Value();
	gpsConfig.startFinishRadius = ((Number)gpsRoot["startFinishRadius"]).Value();
	ChannelConfigFromJson(gpsConfig.latitudeCfg,gpsRoot["latitude"]);
	ChannelConfigFromJson(gpsConfig.longitudeCfg,gpsRoot["longitude"]);
	ChannelConfigFromJson(gpsConfig.velocityCfg,gpsRoot["velocity"]);
	ChannelConfigFromJson(gpsConfig.timeCfg,gpsRoot["time"]);
	ChannelConfigFromJson(gpsConfig.qualityCfg,gpsRoot["quality"]);
	ChannelConfigFromJson(gpsConfig.satellitesCfg,gpsRoot["satellites"]);
	ChannelConfigFromJson(gpsConfig.lapCountCfg,gpsRoot["lapCount"]);
	ChannelConfigFromJson(gpsConfig.lapTimeCfg,gpsRoot["lapTime"]);

}

void RaceCaptureConfig::PopulateAnalogConfig(Array &analogRoot){

	Array::const_iterator itAnalog(analogRoot.Begin());
	int i=0;
	for (; itAnalog != analogRoot.End(); ++itAnalog){
		const Object &analog = *itAnalog;
		AnalogConfig &analogConfig = analogConfigs[i++];
		ChannelConfigFromJson(analogConfig.channelConfig,analog["channelConfig"]);
		analogConfig.loggingPrecision = ((Number)analog["loggingPrecision"]);
		analogConfig.linearScaling = ((Number)analog["linearScaling"]);
		analogConfig.scalingMode = (scaling_mode_t)((int)((Number)analog["scalingMode"]));

		Array scalingMapJson = analog["scalingMap"];
		ScalingMap &scalingMap = analogConfig.scalingMap;

		Array::const_iterator itScalingMap(scalingMapJson.Begin());

		int ii=0;
		for (; itScalingMap != scalingMapJson.End();++itScalingMap){
			const Object &scalingMapItem = *itScalingMap;
			scalingMap.rawValues[ii] = ((Number)scalingMapItem["raw"]);
			scalingMap.scaledValue[ii] = ((Number)scalingMapItem["scaled"]);
			ii++;
		}
	}
}

void RaceCaptureConfig::PopulatePulseInputConfig(Array &pulseInputRoot){
	Array::const_iterator itPulseInput(pulseInputRoot.Begin());
	int i=0;
	for (; itPulseInput != pulseInputRoot.End(); ++itPulseInput){
		const Object &pulseInput = *itPulseInput;
		TimerConfig &pulseInputConfig = timerConfigs[i++];
		ChannelConfigFromJson(pulseInputConfig.channelConfig,pulseInput["channelConfig"]);
		pulseInputConfig.loggingPrecision = ((Number)pulseInput["loggingPrecision"]);
		pulseInputConfig.slowTimerEnabled = ((Number)pulseInput["slowTimer"]);
		pulseInputConfig.mode = (timer_mode_t)((int)((Number)pulseInput["mode"]));
		pulseInputConfig.pulsePerRev = ((Number)pulseInput["pulsePerRev"]);
		pulseInputConfig.timerDivider = ((Number)pulseInput["timerDivider"]);
		pulseInputConfig.scaling = ((Number)pulseInput["scaling"]);
	}
}

void RaceCaptureConfig::PopulateAccelConfig(Array &accelRoot){
	Array::const_iterator itAccel(accelRoot.Begin());
	int i=0;
	for (; itAccel!= accelRoot.End(); ++itAccel){
		const Object &accel = *itAccel;
		AccelConfig &accelConfig = accelConfigs[i++];
		ChannelConfigFromJson(accelConfig.channelConfig,accel["channelConfig"]);
		accelConfig.mode = (accel_mode_t)((int)((Number)accel["mode"]));
		accelConfig.channel = (accel_channel_t)(int)((Number)accel["mapping"]);
		accelConfig.zeroValue = (int)((Number)accel["zeroValue"]);
	}
}

void RaceCaptureConfig::PopulatePulseOutputConfig(Array &pulseOutputRoot){
	Array::const_iterator itPulseOutput(pulseOutputRoot.Begin());
	int i=0;
	for (; itPulseOutput != pulseOutputRoot.End(); ++itPulseOutput){
		const Object &pulseOutput = *itPulseOutput;
		PwmConfig &pulseOutputConfig = pwmConfigs[i++];
		ChannelConfigFromJson(pulseOutputConfig.channelConfig, pulseOutput["channelConfig"]);
		pulseOutputConfig.loggingPrecision = Number(pulseOutput["loggingPrecision"]);
		pulseOutputConfig.outputMode =  (pwm_output_mode_t)(int)Number(pulseOutput["outputMode"]);
		pulseOutputConfig.loggingMode = (pwm_logging_mode_t)(int)Number(pulseOutput["loggingMode"]);
		pulseOutputConfig.startupDutyCycle = Number(pulseOutput["startupDutyCycle"]);
		pulseOutputConfig.startupPeriod = Number(pulseOutput["startupPeriod"]);
		pulseOutputConfig.voltageScaling = Number(pulseOutput["voltageScaling"]);
	}
}

void RaceCaptureConfig::PopulateGpioConfig(Array &gpioConfigRoot){
	Array::const_iterator itGpio(gpioConfigRoot.Begin());
	int i=0;
	for (; itGpio != gpioConfigRoot.End(); ++itGpio){
		const Object &gpio = *itGpio;
		GpioConfig &gpioConfig = gpioConfigs[i++];
		ChannelConfigFromJson(gpioConfig.channelConfig,gpio["channelConfig"]);
		gpioConfig.mode = (gpio_mode_t)(int)Number(gpio["mode"]);
	}
}

void RaceCaptureConfig::PopulateOutputConfig(Object &outputConfig){
	loggerOutputConfig.loggingMode = (logging_mode_t)(int)Number(outputConfig["sdLoggingMode"]);
	loggerOutputConfig.telemetryMode = (telemetry_mode_t)(int)Number(outputConfig["telemetryMode"]);
	loggerOutputConfig.p2pDestinationAddrHigh = (unsigned int)Number(outputConfig["p2pDestAddrHigh"]);
	loggerOutputConfig.p2pDestinationAddrLow = (unsigned int)Number(outputConfig["p2pDestinationAddrLow"]);
}

void RaceCaptureConfig::FromJson(Object root){
	PopulateGpsConfig(root["gpsConfig"]);
	PopulateAnalogConfig(root["analogConfigs"]);
	PopulatePulseInputConfig(root["pulseInputConfigs"]);
	PopulateAccelConfig(root["accelConfigs"]);
	PopulatePulseOutputConfig(root["pulseOutputConfigs"]);
	PopulateGpioConfig(root["gpioConfigs"]);
	PopulateOutputConfig(root["outputConfig"]);
}

Object RaceCaptureConfig::ChannelConfigToJson(ChannelConfig &channelConfig){
	Object cfg;
	cfg["label"] = String(channelConfig.label.ToAscii());
	cfg["units"] = String(channelConfig.units.ToAscii());
	cfg["sampleRate"] = Number(channelConfig.sampleRate);
	return cfg;
}

Object RaceCaptureConfig::GpsConfigToJson(){
	Object cfg;
	cfg["gpsInstalled"] = Number(gpsConfig.gpsInstalled);
	cfg["startFinishLatitude"] = Number(gpsConfig.startFinishLatitude);
	cfg["startFinishLongitude"] = Number(gpsConfig.startFinishLongitude);
	cfg["startFinishRadius"] = Number(gpsConfig.startFinishRadius);
	cfg["latitude"] = ChannelConfigToJson(gpsConfig.latitudeCfg);
	cfg["longitude"] = ChannelConfigToJson(gpsConfig.longitudeCfg);
	cfg["velocity"] = ChannelConfigToJson(gpsConfig.velocityCfg);
	cfg["time"]  = ChannelConfigToJson(gpsConfig.timeCfg);
	cfg["quality"] = ChannelConfigToJson(gpsConfig.qualityCfg);
	cfg["satellites"] = ChannelConfigToJson(gpsConfig.satellitesCfg);
	cfg["lapCount"] = ChannelConfigToJson(gpsConfig.lapCountCfg);
	cfg["lapTime"] = ChannelConfigToJson(gpsConfig.lapTimeCfg);
	return cfg;
}

Array RaceCaptureConfig::AnalogConfigToJson(){
	Array analogConfigs;
	for (int i=0; i < CONFIG_ANALOG_CHANNELS; i++){
		Object cfg;
		AnalogConfig &analogConfig = this->analogConfigs[i];
		cfg["channelConfig"] = ChannelConfigToJson(analogConfig.channelConfig);
		cfg["loggingPrecision"] = Number(analogConfig.loggingPrecision);
		cfg["linearScaling"] = Number(analogConfig.linearScaling);
		cfg["scalingMode"] = Number(analogConfig.scalingMode);

		Array scalingMap;
		for (int m = 0; m < CONFIG_ANALOG_SCALING_BINS; m++ ){
			Object mapItem;
			mapItem["raw"] = Number(analogConfig.scalingMap.rawValues[m]);
			mapItem["scaled"] = Number(analogConfig.scalingMap.scaledValue[m]);
			scalingMap.Insert(mapItem);
		}
		cfg["scalingMap"] = scalingMap;
		analogConfigs.Insert(cfg);
	}
	return analogConfigs;
}

Array RaceCaptureConfig::PulseInputConfigToJson(){
	Array pulseInputConfigs;
	for (int i=0; i < CONFIG_TIMER_CHANNELS; i++){
		Object cfg;
		TimerConfig &timerConfig = this->timerConfigs[i];
		cfg["channelConfig"] = ChannelConfigToJson(timerConfig.channelConfig);
		cfg["loggingPrecision"] = Number(timerConfig.loggingPrecision);
		cfg["slowTimer"] = Number(timerConfig.slowTimerEnabled);
		cfg["mode"] = Number(timerConfig.mode);
		cfg["pulsePerRev"] = Number(timerConfig.pulsePerRev);
		cfg["timerDivider"] = Number(timerConfig.timerDivider);
		cfg["scaling"] = Number(timerConfig.scaling);
		pulseInputConfigs.Insert(cfg);
	}
	return pulseInputConfigs;
}

Array RaceCaptureConfig::AccelConfigToJson(){
	Array accelConfigs;
	for (int i=0; i < CONFIG_ACCEL_CHANNELS; i++){
		Object cfg;
		AccelConfig &accelConfig = this->accelConfigs[i];
		cfg["channelConfig"] = ChannelConfigToJson(accelConfig.channelConfig);
		cfg["mode"] = Number(accelConfig.mode);
		cfg["mapping"] = Number(accelConfig.channel);
		cfg["zeroValue"] = Number(accelConfig.zeroValue);
		accelConfigs.Insert(cfg);
	}
	return accelConfigs;
}

Array RaceCaptureConfig::PulseOutputConfigToJson(){
	Array pulseOutputConfigs;
	for (int i=0; i < CONFIG_ANALOG_PULSE_CHANNELS; i++){
		Object cfg;
		PwmConfig &pwmConfig = this->pwmConfigs[i];
		cfg["channelConfig"] = ChannelConfigToJson(pwmConfig.channelConfig);
		cfg["loggingPrecision"] = Number(pwmConfig.loggingPrecision);
		cfg["outputMode"] = Number(pwmConfig.outputMode);
		cfg["loggingMode"] = Number(pwmConfig.loggingMode);
		cfg["startupDutyCycle"] = Number(pwmConfig.startupDutyCycle);
		cfg["startupPeriod"] = Number(pwmConfig.startupPeriod);
		cfg["voltageScaling"] = Number(pwmConfig.voltageScaling);
		pulseOutputConfigs.Insert(cfg);
	}
	return pulseOutputConfigs;
}

Array RaceCaptureConfig::GpioConfigToJson(){
	Array gpioConfigs;
	for (int i=0; i < CONFIG_GPIO_CHANNELS; i++){
		Object cfg;
		GpioConfig &gpioConfig = this->gpioConfigs[i];
		cfg["channelConfig"] = ChannelConfigToJson(gpioConfig.channelConfig);
		cfg["mode"] = Number(gpioConfig.mode);
		gpioConfigs.Insert(cfg);
	}
	return gpioConfigs;
}

Object RaceCaptureConfig::OutputConfigToJson(){
	Object cfg;
	cfg["sdLoggingMode"] = Number(loggerOutputConfig.loggingMode);
	cfg["telemetryMode"] = Number(loggerOutputConfig.telemetryMode);
	cfg["p2pDestAddrHigh"] = Number(loggerOutputConfig.p2pDestinationAddrHigh);
	cfg["p2pDestAddrLow"] = Number(loggerOutputConfig.p2pDestinationAddrLow);
	return cfg;
}


Object RaceCaptureConfig::ToJson(void){

	Object objRoot;
	objRoot["gpsConfig"] = GpsConfigToJson();
	objRoot["analogConfigs"] = AnalogConfigToJson();
	objRoot["pulseInputConfigs"] = PulseInputConfigToJson();
	objRoot["accelConfigs"] = AccelConfigToJson();
	objRoot["pulseOutputConfigs"] = PulseOutputConfigToJson();
	objRoot["gpioConfigs"] = GpioConfigToJson();
	objRoot["outputConfig"] = OutputConfigToJson();
	return objRoot;
}
