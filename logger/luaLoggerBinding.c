#include "luaLoggerBinding.h"

#include "loggerPinDefs.h"
#include "loggerHardware.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerConfig.h"
#include "loggerData.h"
#include "gps.h"
#include "accelerometer.h"
#include "ADC.h"
#include "timer.h"
#include "CAN.h"
#include "PWM.h"
#include "LED.h"
#include "GPIO.h"
#include "luaScript.h"
#include "luaTask.h"
#include "mod_string.h"
#include "usart.h"
#include "printk.h"
#include "modp_numtoa.h"
#include "loggerTaskEx.h"

extern xSemaphoreHandle g_xLoggerStart;
extern int g_loggingShouldRun;

#define TEMP_BUFFER_LEN 200

char g_tempBuffer[TEMP_BUFFER_LEN];

void registerLuaLoggerBindings(){

	lua_State *L = getLua();
	lockLua();
	//Read / control inputs and outputs
	lua_registerlight(L,"getGpio",Lua_GetGPIO);
	lua_registerlight(L,"getButton",Lua_GetButton);
	lua_registerlight(L,"setGpio",Lua_SetGPIO);

	lua_registerlight(L,"isSDCardPresent", Lua_IsSDCardPresent);
	lua_registerlight(L,"isSDCardWritable", Lua_IsSDCardWritable);

	lua_registerlight(L,"setPWMDutyCycle",Lua_SetPWMDutyCycle);
	lua_registerlight(L,"setPWMDutyCycleRaw",Lua_SetPWMDutyCycleRaw);
	lua_registerlight(L,"setPwmPeriod",Lua_SetPWMPeriod);
	lua_registerlight(L,"setPwmPeriodRaw",Lua_SetPWMPeriodRaw);
	lua_registerlight(L,"setAnalogOut",Lua_SetAnalogOut);

	lua_registerlight(L,"getTimerRpm",Lua_GetRPM);
	lua_registerlight(L,"getTimerPeriodMs",Lua_GetPeriodMs);
	lua_registerlight(L,"getTimerPeriodUsec",Lua_GetPeriodUsec);
	lua_registerlight(L,"getTimerFrequency",Lua_GetFrequency);
	lua_registerlight(L,"getTimerRaw",Lua_GetTimerRaw);

	lua_registerlight(L,"resetTimerCount",Lua_ResetTimerCount);
	lua_registerlight(L,"getTimerCount",Lua_GetTimerCount);
	
	lua_registerlight(L,"getAnalog",Lua_GetAnalog);
	lua_registerlight(L,"getAnalogRaw",Lua_GetAnalogRaw);

	lua_registerlight(L,"readSerial", Lua_ReadSerialLine);
	lua_registerlight(L,"writeSerial", Lua_WriteSerial);

	lua_registerlight(L,"getGpsLatitude",Lua_GetGPSLatitude);
	lua_registerlight(L,"getGpsLongitude", Lua_GetGPSLongitude);
	lua_registerlight(L,"getGpsSpeed",Lua_GetGPSSpeed);
	lua_registerlight(L,"getGpsQuality", Lua_GetGPSQuality);
	lua_registerlight(L,"getGpsTime", Lua_GetGPSTime);
	lua_registerlight(L,"getLapCount", Lua_GetLapCount);
	lua_registerlight(L,"getLapTime", Lua_GetLapTime);
	lua_registerlight(L,"getGpsSec", Lua_GetGPSSecondsSinceMidnight);
	lua_registerlight(L,"getAtStartFinish",Lua_GetGPSAtStartFinish);

	lua_registerlight(L, "initCAN", Lua_InitCAN);
	lua_registerlight(L, "txCAN", Lua_SendCANMessage);
	lua_registerlight(L, "rxCAN", Lua_ReceiveCANMessage);
	lua_registerlight(L, "queryOBD", Lua_QueryOBD);

	lua_registerlight(L,"getTimeDiff", Lua_GetTimeDiff);
	lua_registerlight(L,"getTimeSince", Lua_GetTimeSince);
					
	lua_registerlight(L,"getAccel",Lua_ReadAccelerometer);
	lua_registerlight(L,"getAccelRaw",Lua_ReadAccelerometerRaw);
	
	lua_registerlight(L,"startLogging",Lua_StartLogging);
	lua_registerlight(L,"stopLogging",Lua_StopLogging);

	lua_registerlight(L,"setLed",Lua_SetLED);

	//Logger configuration editing
	lua_registerlight(L,"flashLoggerConfig", Lua_FlashLoggerConfig);

	lua_registerlight(L,"setAnalogLabel", Lua_SetAnalogChannelLabel);
	lua_registerlight(L,"getAnalogLabel", Lua_GetAnalogChannelLabel);
	
	lua_registerlight(L,"setAnalogSampleRate", Lua_SetAnalogChannelSampleRate);
	lua_registerlight(L,"getAnalogSampleRate", Lua_GetAnalogChannelSampleRate);
	
	lua_registerlight(L,"setAnalogScaling", Lua_SetAnalogChannelScaling);
	lua_registerlight(L,"getAnalogScaling", Lua_GetAnalogChannelScaling);
	
	lua_registerlight(L,"setPwmClockFrequency",Lua_SetPWMClockFrequency);
	lua_registerlight(L,"getPwmClockFrequency",Lua_GetPWMClockFrequency);
	
	lua_registerlight(L,"setPwmLabel", Lua_SetPWMLabel);
	lua_registerlight(L,"getPwmLabel", Lua_GetPWMLabel);
	
	lua_registerlight(L,"setPwmSampleRate", Lua_SetPWMSampleRate);
	lua_registerlight(L,"getPwmSampleRate", Lua_GetPWMSampleRate);
	
	lua_registerlight(L,"setPwmOutputConfig", Lua_SetPWMOutputConfig);
	lua_registerlight(L,"getPwmOutputConfig", Lua_GetPWMOutputConfig);
	
	lua_registerlight(L,"setPwmLoggingConfig", Lua_SetPWMLoggingConfig);
	lua_registerlight(L,"getPwmLoggingConfig", Lua_GetPWMLoggingConfig);
	
	lua_registerlight(L,"setPwmStartupDutyCycle", Lua_SetPWMStartupDutyCycle);
	lua_registerlight(L,"getPwmStartupDutyCycle", Lua_GetPWMStartupDutyCycle);
		
	lua_registerlight(L,"setPwmStartupPeriod", Lua_SetPWMStartupPeriod);
	lua_registerlight(L,"getPwmStartupPeriod", Lua_GetPWMStartupPeriod);
	
	lua_registerlight(L,"setPwmVoltageScaling", Lua_SetPWMVoltageScaling);
	lua_registerlight(L,"getPwmVoltageScaling", Lua_GetPWMVoltageScaling);
	
	lua_registerlight(L,"setGpsInstalled", Lua_SetGPSInstalled);
	lua_registerlight(L,"getGpsInstalled", Lua_GetGPSInstalled);
	
	lua_registerlight(L,"setGpsStartFinish",Lua_SetGPSStartFinish);
	lua_registerlight(L,"getGpsStartFinish",Lua_GetGPSStartFinish);
	
	lua_registerlight(L,"setGpsSatsLabel", Lua_SetGPSSatsLabel);
	lua_registerlight(L,"getGpsSatsLabel", Lua_GetGPSSatsLabel);
	
	lua_registerlight(L,"setGpsLatitudeLabel", Lua_SetGPSLatitudeLabel);
	lua_registerlight(L,"getGpsLatitudeLabel", Lua_GetGPSLatitudeLabel);
	
	lua_registerlight(L,"setGpsLongitudeLabel", Lua_SetGPSLongitudeLabel);
	lua_registerlight(L,"getGpsLongitudeLabel", Lua_GetGPSLongitudeLabel);
	
	lua_registerlight(L,"setGpsTimeLabel", Lua_SetGPSTimeLabel);
	lua_registerlight(L,"getGpsTimeLabel", Lua_GetGPSTimeLabel);
	
	lua_registerlight(L,"setGpsSpeedLabel", Lua_SetGPSSpeedLabel);
	lua_registerlight(L,"getGpsSpeedLabel", Lua_GetGPSSpeedLabel);
	
	lua_registerlight(L,"setGpsPositionSampleRate", Lua_SetGPSPositionSampleRate);
	lua_registerlight(L,"getGpsPositionSampleRate", Lua_GetGPSPositionSampleRate);
	
	lua_registerlight(L,"setGpsSpeedSampleRate", Lua_SetGPSSpeedSampleRate);
	lua_registerlight(L,"getGpsSpeedSampleRate", Lua_GetGPSSpeedSampleRate);
	
	lua_registerlight(L,"setGpsTimeSampleRate", Lua_SetGPSTimeSampleRate);
	lua_registerlight(L,"getGpsTimeSampleRate", Lua_GetGPSTimeSampleRate);

	lua_registerlight(L,"setLapCountSampleRate", Lua_SetLapCountSampleRate);
	lua_registerlight(L,"getLapCountSampleRate", Lua_GetLapCountSampleRate);

	lua_registerlight(L,"setLapTimeSampleRate", Lua_SetLapTimeSampleRate);
	lua_registerlight(L,"getLapTimeSampleRate", Lua_GetLapTimeSampleRate);

	lua_registerlight(L,"setLapCountLabel", Lua_SetLapCountLabel);
	lua_registerlight(L,"getLapCountLabel", Lua_GetLapCountLabel);

	lua_registerlight(L,"setLapTimeLabel", Lua_SetLapTimeLabel);
	lua_registerlight(L,"getLapTimeLabel", Lua_GetLapTimeLabel);
	
	lua_registerlight(L,"setGpioLabel", Lua_SetGPIOLabel);
	lua_registerlight(L,"getGpioLabel", Lua_GetGPIOLabel);
	
	lua_registerlight(L,"setGpioSampleRate", Lua_SetGPIOSampleRate);
	lua_registerlight(L,"getGpioSampleRate", Lua_GetGPIOSampleRate);
	
	lua_registerlight(L,"setGpioConfig", Lua_SetGPIOConfig);
	lua_registerlight(L,"getGpioConfig", Lua_GetGPIOConfig);
	
	lua_registerlight(L,"setTimerLabel", Lua_SetTimerLabel);
	lua_registerlight(L,"getTimerLabel", Lua_GetTimerLabel);
	
	lua_registerlight(L,"setTimerSampleRate", Lua_SetTimerSampleRate);
	lua_registerlight(L,"getTimerSampleRate", Lua_GetTimerSampleRate);
	
	lua_registerlight(L,"setTimerConfig", Lua_SetTimerConfig);
	lua_registerlight(L,"getTimerConfig", Lua_GetTimerConfig);
	
	lua_registerlight(L,"setTimerPulsePerRevolution",Lua_SetTimerPulsePerRevolution);
	lua_registerlight(L,"getTimerPulsePerRevolution",Lua_GetTimerPulsePerRevolution);
	
	lua_registerlight(L,"setTimerDivider", Lua_SetTimerDivider);
	lua_registerlight(L,"getTimerDivider", Lua_GetTimerDivider);
	
	lua_registerlight(L,"calculateTimerScaling", Lua_CalculateTimerScaling);
	lua_registerlight(L,"getTimerScaling", Lua_GetTimerScaling);
	
	lua_registerlight(L,"setAccelLabel",Lua_SetAccelLabel);
	lua_registerlight(L,"getAccelLabel",Lua_GetAccelLabel);
	
	lua_registerlight(L,"setAccelSampleRate",Lua_SetAccelSampleRate);
	lua_registerlight(L,"getAccelSampleRate",Lua_GetAccelSampleRate);
	
	lua_registerlight(L,"setAccelConfig",Lua_SetAccelConfig);
	lua_registerlight(L,"getAccelConfig",Lua_GetAccelConfig);
	
	lua_registerlight(L,"setAccelChannel",Lua_SetAccelChannel);
	lua_registerlight(L,"getAccelChannel",Lua_GetAccelChannel);
	
	lua_registerlight(L,"setAccelZeroValue",Lua_SetAccelZeroValue);
	lua_registerlight(L,"getAccelZeroValue",Lua_GetAccelZeroValue);

	lua_registerlight(L,"calibrateAccelZero",Lua_CalibrateAccelZero);
	
	lua_registerlight(L,"setBgStream", Lua_SetBackgroundStreaming);
	lua_registerlight(L,"getBgStream", Lua_GetBackgroundStreaming);

	unlockLua();
}

////////////////////////////////////////////////////
// common functions
////////////////////////////////////////////////////

static TrackConfig * getTrackConfig(){
	return &(getWorkingLoggerConfig()->TrackConfigs);
}

static int setLuaSampleRate(lua_State *L, int *sampleRate){
	if (lua_gettop(L) >= 1 ){
		*sampleRate = encodeSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

static int getLuaSampleRate(lua_State *L, int sampleRate){
	lua_pushinteger(L,sampleRate);
	return 1;
}

static int setLuaChannelLabel(lua_State *L, char *label){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(label,lua_tostring(L,1));
	}
	return 0;
}

static int getLuaChannelLabel(lua_State *L, char *label){
	lua_pushstring(L,label);
	return 0;
}

////////////////////////////////////////////////////

int Lua_SetBackgroundStreaming(lua_State *L){
	if (lua_gettop(L) >= 1){
		getWorkingLoggerConfig()->ConnectivityConfigs.backgroundStreaming = (lua_tointeger(L,1) == 1);
	}
	return 0;
}

int Lua_GetBackgroundStreaming(lua_State *L){
	lua_pushinteger(L, getWorkingLoggerConfig()->ConnectivityConfigs.backgroundStreaming == 1);
	return 1;
}

int Lua_IsSDCardPresent(lua_State *L){
	lua_pushinteger(L,GPIO_is_SD_card_present());
	return 1;
}

int Lua_IsSDCardWritable(lua_State *L){
	lua_pushinteger(L,GPIO_is_SD_card_writable());
	return 1;
}

int Lua_SetAccelLabel(lua_State *L){
	if (lua_gettop(L) >= 2){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) setLabelGeneric(c->cfg.label,lua_tostring(L,2));
	}
	return 0;	
}

int Lua_GetAccelLabel(lua_State *L){
	if (lua_gettop(L) >= 1 ){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushstring(L,c->cfg.label);
			return 1;
		}
	}
	return 0;	
}

int Lua_SetAccelSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2 ){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->cfg.sampleRate = encodeSampleRate(lua_tointeger(L,2));
	}	
	return 0;
}

int Lua_GetAccelSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushnumber(L,c->cfg.sampleRate);
			return 1;	
		}
	}
	return 0;	
}


int Lua_SetAccelConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->mode = filterAccelMode(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetAccelConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushnumber(L, c->mode);
			return 1;
		}
	}
	return 0;	
}

int Lua_SetAccelChannel(lua_State *L){
	if (lua_gettop(L) >= 2){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->accelChannel = filterAccelChannel(lua_tointeger(L,2));	
	}
	return 0;
}

int Lua_GetAccelChannel(lua_State *L){
	if (lua_gettop(L) >= 1){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushnumber(L, c->accelChannel);
			return 1;	
		}
	}
	return 0;
}

int Lua_SetAccelZeroValue(lua_State *L){
	if (lua_gettop(L) >= 2){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->zeroValue = filterAccelRawValue(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetAccelZeroValue(lua_State *L){
	if (lua_gettop(L) >=2 ){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->zeroValue);
			return 1;
		}
	}
	return 0;	
}

int Lua_CalibrateAccelZero(lua_State *L){
	calibrateAccelZero();
	return 0;
}

int Lua_SetTimerLabel(lua_State *L){
	if (lua_gettop(L) >= 3){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			setLabelGeneric(c->cfg.label,lua_tostring(L,2));
			setLabelGeneric(c->cfg.units,lua_tostring(L,3));
		}		
	}
	return 0;
}

int Lua_GetTimerLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushstring(L,c->cfg.label);
			lua_pushstring(L,c->cfg.units);
			return 2;			
		}
	}
	return 0;	
}


int Lua_SetTimerSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->cfg.sampleRate = encodeSampleRate(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetTimerSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->cfg.sampleRate);
			return 1;
		}
	}
	return 0;
}

int Lua_SetTimerConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->mode = filterTimerMode(lua_tointeger(L,2));
	}	
	return 0;
}

int Lua_GetTimerConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L, c->mode);
			return 1;
		}
	}
	return 0;	
}

int Lua_SetTimerPulsePerRevolution(lua_State *L){
	if (lua_gettop(L) >= 2){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->pulsePerRevolution = lua_tointeger(L,2);
	}
	return 0;
}

int Lua_GetTimerPulsePerRevolution(lua_State *L){
	if (lua_gettop(L) >=1 ){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->pulsePerRevolution);
			return 1;	
		}
	}	
	return 0;
}

int Lua_SetTimerDivider(lua_State *L){
	if (lua_gettop(L) >= 2){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->timerDivider = filterTimerDivider(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetTimerDivider(lua_State *L){
	if (lua_gettop(L) >=1 ){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->timerDivider);
			return 1;
		}	
	}
	return 0;
}

int Lua_CalculateTimerScaling(lua_State *L){
	if (lua_gettop(L) >=1 ){
		TimerConfig * c= getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c) calculateTimerScaling(BOARD_MCK, c);
	}
	return 0;
}

int Lua_GetTimerScaling(lua_State *L){
	if (lua_gettop(L) >=1 ){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushinteger(L,c->calculatedScaling);
			return 1;			
		}
	}
	return 0;	
}

int Lua_SetGPIOLabel(lua_State *L){
	if (lua_gettop(L) >= 2){
		GPIOConfig *c = getGPIOConfigChannel(lua_tointeger(L,1));
		if (NULL != c) setLabelGeneric(c->cfg.label,lua_tostring(L,2));
	}
	return 0;	
}

int Lua_GetGPIOLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		GPIOConfig *c = getGPIOConfigChannel(lua_tointeger(L,1));
		if (NULL !=c){
			lua_pushstring(L,c->cfg.label);
			return 1;	
		}
	}	
	return 0;
}

int Lua_SetGPIOSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2){
		GPIOConfig *c = getGPIOConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->cfg.sampleRate = encodeSampleRate(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetGPIOSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		GPIOConfig *c = getGPIOConfigChannel(lua_tointeger(L,1));
		if (NULL !=c){
			lua_pushinteger(L,c->cfg.sampleRate);
			return 1;	
		}
	}	
	return 0;
}

int Lua_SetGPIOConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		int channel = lua_tointeger(L,1);
		if (channel >= 0 && channel <= 2){
			GPIOConfig *c = getGPIOConfigChannel(channel);
			//0= configure as input, 1=configure as output
			if (NULL != c) c->mode = filterGpioMode(lua_tointeger(L,2));
			GPIO_init(getWorkingLoggerConfig()); //reload configuration
		}
	}
	return 0;
}

int Lua_GetGPIOConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		int channel = lua_tointeger(L,1);
		if (channel >= 0 && channel <= 2){
			GPIOConfig *c = getGPIOConfigChannel(channel);
			if (NULL !=c){
				//0=configure as input, 1=configure as output
				lua_pushinteger(L,c->mode);
				return 1;
			}
		}
	}	
	return 0;
}

int Lua_SetGPSInstalled(lua_State *L){
	if (lua_gettop(L) >=1 ){
		getWorkingLoggerConfig()->GPSConfigs.GPSInstalled = lua_toboolean(L,1);
	}
	return 0;	
}

int Lua_GetGPSInstalled(lua_State *L){
	lua_pushboolean(L,getWorkingLoggerConfig()->GPSConfigs.GPSInstalled);
	return 1;	
}

int Lua_SetGPSStartFinish(lua_State *L){
	if (lua_gettop(L) >= 2){
		TrackConfig *c = &(getWorkingLoggerConfig()->TrackConfigs);
		c->startFinishConfig.latitude = lua_tonumber(L,1);
		c->startFinishConfig.longitude = lua_tonumber(L,2);
		if (lua_gettop(L) >=3) c->startFinishConfig.targetRadius = lua_tonumber(L,3);
	}
	return 0;
}

int Lua_GetGPSStartFinish(lua_State *L){
	TrackConfig *c = &(getWorkingLoggerConfig()->TrackConfigs);
	lua_pushnumber(L,c->startFinishConfig.latitude);
	lua_pushnumber(L,c->startFinishConfig.longitude);
	lua_pushnumber(L,c->startFinishConfig.targetRadius);
	return 3;
}

int Lua_GetGPSAtStartFinish(lua_State *L){
	lua_pushinteger(L,getAtStartFinish());
	return 1;
}

int Lua_SetSplit(lua_State *L){
	if (lua_gettop(L) >= 2){
		TrackConfig *c = &(getWorkingLoggerConfig()->TrackConfigs);
		c->splitConfig.latitude = lua_tonumber(L,1);
		c->splitConfig.longitude = lua_tonumber(L,2);
		if (lua_gettop(L) >=3) c->splitConfig.targetRadius = lua_tonumber(L,3);
	}
	return 0;
}

int Lua_GetSplit(lua_State *L){
	TrackConfig *c = &(getWorkingLoggerConfig()->TrackConfigs);
	lua_pushnumber(L,c->splitConfig.latitude);
	lua_pushnumber(L,c->splitConfig.longitude);
	lua_pushnumber(L,c->splitConfig.targetRadius);
	return 3;
}

int Lua_GetAtSplit(lua_State *L){
	lua_pushinteger(L,getAtSplit());
	return 1;
}

int Lua_SetGPSSatsLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfigs.satellitesCfg.label,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSSatsLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfigs.satellitesCfg.label);
	return 1;	
}

int Lua_SetGPSLatitudeLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfigs.latitudeCfg.label,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSLatitudeLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfigs.latitudeCfg.label);
	return 1;
}

int Lua_SetGPSLongitudeLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfigs.longitudeCfg.label,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSLongitudeLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfigs.longitudeCfg.label);
	return 1;	
}

int Lua_SetGPSTimeLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfigs.timeCfg.label,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSTimeLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfigs.timeCfg.label);
	return 1;	
}

int Lua_SetGPSSpeedLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfigs.speedCfg.label,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSSpeedLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfigs.speedCfg.label);
	return 1;	
}

int Lua_SetGPSPositionSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		LoggerConfig *cfg = getWorkingLoggerConfig();
		cfg->GPSConfigs.latitudeCfg.sampleRate = cfg->GPSConfigs.longitudeCfg.sampleRate = encodeSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetGPSPositionSampleRate(lua_State *L){
	//TODO we pull one for all... is there a better way? individual settable sample rates?
	lua_pushinteger(L,getWorkingLoggerConfig()->GPSConfigs.latitudeCfg.sampleRate);
	return 1;		
}

int Lua_SetGPSSpeedSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1 ){
		getWorkingLoggerConfig()->GPSConfigs.speedCfg.sampleRate = encodeSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetGPSSpeedSampleRate(lua_State *L){
	lua_pushinteger(L,getWorkingLoggerConfig()->GPSConfigs.speedCfg.sampleRate);
	return 1;
}

int Lua_SetGPSTimeSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1 ){
		getWorkingLoggerConfig()->GPSConfigs.timeCfg.sampleRate = encodeSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetGPSTimeSampleRate(lua_State *L){
	lua_pushinteger(L,getWorkingLoggerConfig()->GPSConfigs.timeCfg.sampleRate);
	return 1;
}

int Lua_SetLapCountSampleRate(lua_State *L){
	return setLuaSampleRate(L, &getTrackConfig()->lapCountCfg.sampleRate);
}

int Lua_GetLapCountSampleRate(lua_State *L){
	return getLuaSampleRate(L, getTrackConfig()->lapCountCfg.sampleRate);
}

int Lua_SetLapTimeSampleRate(lua_State *L){
	return setLuaSampleRate(L, &getTrackConfig()->lapTimeCfg.sampleRate);
}

int Lua_GetLapTimeSampleRate(lua_State *L){
	return getLuaSampleRate(L, getTrackConfig()->lapTimeCfg.sampleRate);
}

int Lua_SetLapCountLabel(lua_State *L){
	return setLuaChannelLabel(L, getTrackConfig()->lapCountCfg.label);
}

int Lua_GetLapCountLabel(lua_State *L){
	return getLuaChannelLabel(L, getTrackConfig()->lapCountCfg.label);
}

int Lua_SetLapTimeLabel(lua_State *L){
	return setLuaChannelLabel(L,getTrackConfig()->lapTimeCfg.label);
}

int Lua_GetLapTimeLabel(lua_State *L){
	return getLuaChannelLabel(L,getTrackConfig()->lapTimeCfg.label);
}


int Lua_SetPWMClockFrequency(lua_State *L){
	if (lua_gettop(L) >= 1){
		getWorkingLoggerConfig()->PWMClockFrequency = filterPwmClockFrequency(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetPWMClockFrequency(lua_State *L){
	lua_pushinteger(L,getWorkingLoggerConfig()->PWMClockFrequency);
	return 1;	
}

int Lua_SetPWMLabel(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c) setLabelGeneric(c->cfg.label,lua_tostring(L,2));
	}
	return 0;
}

int Lua_GetPWMLabel(lua_State *L){
	if (lua_gettop(L) >=1){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushstring(L,c->cfg.label);
		}	
	}	
	return 0;
}

int Lua_SetPWMSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->cfg.sampleRate = encodeSampleRate(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetPWMSampleRate(lua_State *L){
	if (lua_gettop(L) >=1){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->cfg.sampleRate);
			return 1;
		}	
	}	
	return 0;
}

int Lua_SetPWMOutputConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->outputMode = filterPwmOutputMode(lua_tointeger(L,2));	
	}
	return 0;
}

int Lua_GetPWMOutputConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->outputMode);
			return 1;	
		}		
	}
	return 0;	
}

int Lua_SetPWMLoggingConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->loggingMode = filterPwmLoggingMode(lua_tointeger(L,2));	
	}
	return 0;	
}

int Lua_GetPWMLoggingConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->loggingMode);
			return 1;	
		}
	}
	return 0;
}

int Lua_SetPWMStartupDutyCycle(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->startupDutyCycle = filterPwmDutyCycle(lua_tointeger(L,2));
		}	
	}
	return 0;	
}

int Lua_GetPWMStartupDutyCycle(lua_State *L){
	if (lua_gettop(L) >= 1){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->startupDutyCycle);
			return 1;	
		}		
	}
	return 0;
}

int Lua_SetPWMStartupPeriod(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->startupPeriod = filterPwmPeriod(lua_tointeger(L,2));
		}	
	}
	return 0;	
}

int Lua_GetPWMStartupPeriod(lua_State *L){
	if (lua_gettop(L) >= 1){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->startupPeriod);
			return 1;	
		}		
	}
	return 0;	
}

int Lua_SetPWMVoltageScaling(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->voltageScaling = lua_tonumber(L,2);
		}	
	}
	return 0;	
}

int Lua_GetPWMVoltageScaling(lua_State *L){
	if (lua_gettop(L) >= 1){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushnumber(L,c->voltageScaling);
			return 1;			
		}		
	}
	return 0;	
}


int Lua_SetAnalogChannelLabel(lua_State *L){
	if (lua_gettop(L) >= 3){
		ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			setLabelGeneric(c->cfg.label,lua_tostring(L,2));
			setLabelGeneric(c->cfg.units,lua_tostring(L,3));
		}
	}
	return 0;
}

int Lua_GetAnalogChannelLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushstring(L,c->cfg.label);
			lua_pushstring(L,c->cfg.units);
			return 2;			
		}	
	}	
	return 0;
}

int Lua_SetAnalogChannelSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2){
		ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->cfg.sampleRate = encodeSampleRate(lua_tointeger(L,2));
		}
	}
	return 0;	
}

int Lua_GetAnalogChannelSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushinteger(L,c->cfg.sampleRate);
			return 1;	
		}	
	}	
	return 0;
}

int Lua_SetAnalogChannelScaling(lua_State *L){
	if (lua_gettop(L) >= 2){
		ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			c->linearScaling = lua_tonumber(L,2);
		}
	}
	return 0;	
}

int Lua_GetAnalogChannelScaling(lua_State *L){
	if (lua_gettop(L) >= 1){
		ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushnumber(L,c->linearScaling);			
		}		
	}
	return 0;
}


int Lua_GetAnalog(lua_State *L){
	float analogValue = -1;
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		ADCConfig *ac = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL != ac){
			unsigned int adcRaw = ADC_read(channel);
			switch(ac->scalingMode){
			case SCALING_MODE_RAW:
				analogValue = adcRaw;
				break;
			case SCALING_MODE_LINEAR:
				analogValue = (ac->linearScaling * (float)adcRaw);
				break;
			case SCALING_MODE_MAP:
				analogValue = GetMappedValue((float)adcRaw,&(ac->scalingMap));
				break;
			}
		}
	}
	lua_pushnumber(L, analogValue);
	return 1;
}

int Lua_GetAnalogRaw(lua_State *L){
	int result = -1;
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		if (channel >= 0 && channel < CONFIG_ADC_CHANNELS){
			result = (int)ADC_read(channel);
		}
	}
	lua_pushnumber(L,result);
	return 1;
}

static int luaToTimerValues(lua_State *L, unsigned int *timerPeriod, unsigned int *scaling){
	int result = 0;
	if (lua_gettop(L) >=  1){
		int channel = lua_tointeger(L,1);
		TimerConfig *c = getTimerConfigChannel(channel);
		if (NULL != c){
			*timerPeriod = timer_get_period(channel);
			*scaling = c->calculatedScaling;
			result = 1;
		}
	}
	return result;
}

int Lua_GetRPM(lua_State *L){
	unsigned int timerPeriod, scaling;
	int result = 0;
	if (luaToTimerValues(L, &timerPeriod, &scaling)){
		int rpm = TIMER_PERIOD_TO_RPM(timerPeriod, scaling);
		lua_pushinteger(L, rpm);
		result = 1;
	}
	return result;
}

int Lua_GetPeriodMs(lua_State *L){
	unsigned int timerPeriod, scaling;
	int result = 0;
	if (luaToTimerValues(L, &timerPeriod, &scaling)){
		lua_pushinteger(L, TIMER_PERIOD_TO_MS(timerPeriod, scaling));
		result = 1;
	}
	return result;
}

int Lua_GetPeriodUsec(lua_State *L){
	unsigned int timerPeriod, scaling;
	int result = 0;
	if (luaToTimerValues(L, &timerPeriod, &scaling)){
		lua_pushinteger(L, TIMER_PERIOD_TO_USEC(timerPeriod, scaling));
		result = 1;
	}
	return result;
}

int Lua_GetFrequency(lua_State *L){
	unsigned int timerPeriod, scaling;
	int result = 0;
	if (luaToTimerValues(L, &timerPeriod, &scaling)){
		lua_pushinteger(L, TIMER_PERIOD_TO_HZ(timerPeriod, scaling));
		result = 1;
	}
	return result;
}

int Lua_GetTimerRaw(lua_State *L){
	int result = -1;
	if (lua_gettop(L) >= 1){
		int channel = lua_tointeger(L,1);
		result = timer_get_period(channel);
	}
	lua_pushinteger(L,result);
	return 1;
}

int Lua_ResetTimerCount(lua_State *L){
	if (lua_gettop(L) >= 1){
		timer_reset_count(lua_tointeger(L,1));
	}
	return 0;
}


int Lua_GetTimerCount(lua_State *L){
	int result = -1;
	if (lua_gettop(L) >= 1){
		int channel = lua_tointeger(L,1);
		result = timer_get_count(channel);
	}
	lua_pushinteger(L,result);
	return 1;
}

int Lua_GetButton(lua_State *L){
	unsigned int pushbutton = GPIO_is_button_pressed();
	lua_pushinteger(L,(pushbutton == 0));
	return 1;	
}

int Lua_WriteSerial(lua_State *L){

	if (lua_gettop(L) >= 2){
		int uart = lua_tointeger(L,1);
		const char * data= lua_tostring(L,2);
		switch (uart){
		case 0:
			usart0_puts(data);
			break;
		case 1:
			usart1_puts(data);
			break;
		}
	}
	return 0;
}

int Lua_ReadSerialLine(lua_State *L){

	if (lua_gettop(L) >= 1){
		int uart = lua_tointeger(L,1);
		switch (uart){
		case 0:
			usart0_readLine(g_tempBuffer, TEMP_BUFFER_LEN);
			break;
		case 1:
			usart1_readLine(g_tempBuffer, TEMP_BUFFER_LEN);
			break;
		default:
			return 0; //no result, return nil
		}
		lua_pushstring(L,g_tempBuffer);
		return 1;
	}
	return 0; //missing parameter
}

int Lua_GetGPIO(lua_State *L){

	unsigned int state = 0;
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		state = GPIO_get(channel);
	}
	lua_pushinteger(L, state);
	return 1;
}

int Lua_SetGPIO(lua_State *L){
	if (lua_gettop(L) >=2){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		unsigned int state = ((unsigned int)lua_tointeger(L,2) == 1);
		GPIO_set(channel, state);
	}
	return 0;
}

int Lua_GetGPSLongitude(lua_State *L){
	lua_pushnumber(L,getLongitude());
	return 1;
}


int Lua_GetGPSLatitude(lua_State *L){
	lua_pushnumber(L,getLatitude());
	return 1;
}

int Lua_GetGPSSpeed(lua_State *L){
	lua_pushnumber(L,getGPSSpeed());
	return 1;
}

int Lua_GetGPSQuality(lua_State *L){
	lua_pushnumber(L,getGPSQuality());
	return 1;
}

int Lua_GetGPSTime(lua_State *L){
	lua_pushnumber(L,getUTCTime());
	return 1;
}

int Lua_GetLapTime(lua_State *L){
	lua_pushnumber(L,getLastLapTime());
	return 1;
}

int Lua_GetLapCount(lua_State *L){
	lua_pushinteger(L,getLapCount());
	return 1;
}

int Lua_GetGPSSecondsSinceMidnight(lua_State *L){
	float s = getSecondsSinceMidnight();
	lua_pushnumber(L,s);
	return 1;
}


int Lua_GetTimeDiff(lua_State *L){

	if (lua_gettop(L) >= 2){
		float t1 = (float)lua_tonumber(L,1);
		float t2 = (float)lua_tonumber(L,2);
		lua_pushnumber(L,getTimeDiff(t1,t2));
		return 1;
	}
	return 0;
}

int Lua_GetTimeSince(lua_State *L){

	if (lua_gettop(L) >= 1){
		float t1 = (float)lua_tonumber(L,1);
		lua_pushnumber(L,getTimeSince(t1));
		return 1;
	}
	return 0;
}

int Lua_ReadAccelerometer(lua_State *L){
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		if (channel >= 0 && channel < CONFIG_ACCEL_CHANNELS){
			AccelConfig *ac = &getWorkingLoggerConfig()->AccelConfigs[channel];
			float accelG = accelerometer_read_value(channel,ac);
			lua_pushnumber(L,accelG);
			return 1;
		}
	}
	return 0;
}

int Lua_ReadAccelerometerRaw(lua_State *L){
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		if (channel >= 0 && channel <= CONFIG_ACCEL_CHANNELS){
			int accelValue =  accelerometer_read(channel);
			lua_pushinteger(L,accelValue);
			return 1;
		}
	}
	return 0;
}

int Lua_SetPWMDutyCycle(lua_State *L){
	return Lua_SetPWMDutyCycleRaw(L);
}

int Lua_SetPWMDutyCycleRaw(lua_State *L){
	if (lua_gettop(L) >= 2){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		unsigned int dutyCycleRaw = (unsigned int)lua_tointeger(L,2);
		if (channel >= 0 && channel < CONFIG_PWM_CHANNELS){
			PWM_set_duty_cycle( channel, (unsigned short)dutyCycleRaw);
		}
	}
	return 0;
}

int Lua_SetPWMPeriod(lua_State *L){
	return Lua_SetPWMPeriodRaw(L);
}

int Lua_SetPWMPeriodRaw(lua_State *L){
	if (lua_gettop(L) >= 2){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		unsigned int periodRaw = (unsigned int)lua_tointeger(L,2);
		if (channel >= 0 && channel < CONFIG_PWM_CHANNELS){
			PWM_channel_set_period(channel, (unsigned short)periodRaw);
		}
	}	
	return 0;
}

int Lua_SetAnalogOut(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPwmConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			unsigned int channel = (unsigned int)lua_tointeger(L,1);
			if (channel >= 0 && channel < CONFIG_PWM_CHANNELS){
				float dutyCycle = (float)lua_tonumber(L,2) / c->voltageScaling;
				PWM_set_duty_cycle(channel,(unsigned short)dutyCycle);
			}
		}
	}
	return 0;
}

int Lua_InitCAN(lua_State *L){
	if (lua_gettop(L) >= 1){
		int baud = lua_tointeger(L, 1);
		int rc = CAN_init(baud);
		lua_pushinteger(L, rc);
		return 1;
	}
	else{
		return 0;
	}
}

int Lua_SendCANMessage(lua_State *L){
	size_t timeout = 1000;
	int rc = -1;
	if (lua_gettop(L) >= 3){
		CAN_msg msg;
		msg.addressValue = (unsigned int)lua_tointeger(L, 1);
		msg.isExtendedAddress = lua_tointeger(L, 2);

		int size = luaL_getn(L, 3);
		if (size <= CAN_MSG_SIZE){
			for (int i = 1; i <= size; i++){
				lua_pushnumber(L,i);
				lua_gettable(L, 3);
				int val = lua_tonumber(L, -1);
				msg.data[i - 1] = val;
				lua_pop(L, 1);
			}
		}
		msg.dataLength = size;
		rc = CAN_tx_msg(&msg, timeout);
	}
	lua_pushinteger(L, rc);
	return rc;
}

int Lua_ReceiveCANMessage(lua_State *L){
	size_t timeout = 1000;
	CAN_msg msg;
	int rc = CAN_rx_msg(&msg,timeout);
	if (rc == 1){
		lua_pushinteger(L, msg.addressValue);
		lua_pushinteger(L, msg.isExtendedAddress);
		lua_newtable(L);
		for (int i = 1; i <= msg.dataLength; i++){
			lua_pushnumber(L, i);
			lua_pushnumber(L, msg.data[i - 1]);
			lua_rawset(L, -3);
		}
		return 3;
	}
	else{
		return 0;
	}
}

int Lua_QueryOBD(lua_State *L){
 return 0;
}

int Lua_StartLogging(lua_State *L){
	startLogging();
	return 0;
}

int Lua_StopLogging(lua_State *L){
	g_loggingShouldRun = 0;
	return 0;
}

int Lua_SetLED(lua_State *L){
	if (lua_gettop(L) >= 2){
		unsigned int LED = lua_tointeger(L,1);
		unsigned int state = lua_tointeger(L,2);
		if (state){
			LED_enable(LED);
		}
		else{
			LED_disable(LED);
		}
	}
	return 0;
}


int Lua_FlashLoggerConfig(lua_State *L){
	int result = flashLoggerConfig();
	lua_pushinteger(L,result);
	return 1;	
}

