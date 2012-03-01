#include "luaLoggerBinding.h"

#include "loggerPinDefs.h"
#include "loggerHardware.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerConfig.h"
#include "gps.h"
#include "accelerometer.h"
#include "usb_comm.h"
#include "luaScript.h"
#include "luaTask.h"
#include <string.h>
#include "usart.h"

extern xSemaphoreHandle g_xLoggerStart;
extern int g_loggingShouldRun;

#define TEMP_BUFFER_LEN 200

char g_tempBuffer[TEMP_BUFFER_LEN];

void registerLuaLoggerBindings(){

	lua_State *L = getLua();
	lockLua();
	//Read / control inputs and outputs
	lua_register(L,"getGpio",Lua_GetGPIO);
	lua_register(L,"getButton",Lua_GetButton);
	lua_register(L,"setGpio",Lua_SetGPIO);

	lua_register(L,"isSDCardPresent", Lua_IsSDCardPresent);
	lua_register(L,"isSDCardWritable", Lua_IsSDCardWritable);
	lua_register(L,"setPWMDutyCycle",Lua_SetPWMDutyCycle);
	lua_register(L,"setPWMDutyCycleRaw",Lua_SetPWMDutyCycleRaw);

	lua_register(L,"setPwmPeriod",Lua_SetPWMPeriod);
	lua_register(L,"setPwmPeriodRaw",Lua_SetPWMPeriodRaw);
	lua_register(L,"setAnalogOut",Lua_SetAnalogOut);

	lua_register(L,"getTimerRpm",Lua_GetRPM);
	lua_register(L,"getTimerPeriodMs",Lua_GetPeriodMs);
	lua_register(L,"getTimerPeriodUsec",Lua_GetPeriodUsec);
	lua_register(L,"getTimerFrequency",Lua_GetFrequency);
	lua_register(L,"getTimerRaw",Lua_GetTimerRaw);


	lua_register(L,"resetTimerCount",Lua_ResetTimerCount);
	lua_register(L,"getTimerCount",Lua_GetTimerCount);
	
	lua_register(L,"getAnalog",Lua_GetAnalog);
	lua_register(L,"getAnalogRaw",Lua_GetAnalogRaw);


	lua_register(L,"readSerial", Lua_ReadSerialLine);
	lua_register(L,"writeSerial", Lua_WriteSerial);

	lua_register(L,"getGpsLatitude",Lua_GetGPSLatitude);
	lua_register(L,"getGpsLongitude", Lua_GetGPSLongitude);
	lua_register(L,"getGpsVelocity",Lua_GetGPSVelocity);
	lua_register(L,"getGpsQuality", Lua_GetGPSQuality);
	lua_register(L,"getGpsTime", Lua_GetGPSTime);
	lua_register(L,"getGpsSecondsSinceMidnight", Lua_GetGPSSecondsSinceMidnight);
	lua_register(L,"getTimeDiff", Lua_GetTimeDiff);
	lua_register(L,"getTimeSince", Lua_GetTimeSince);
					
	lua_register(L,"readAccel",Lua_ReadAccelerometer);
	lua_register(L,"readAccelRaw",Lua_ReadAccelerometerRaw);
	
	lua_register(L,"startLogging",Lua_StartLogging);
	lua_register(L,"stopLogging",Lua_StopLogging);	

	lua_register(L,"setLed",Lua_SetLED);

	//Logger configuration editing
	lua_register(L,"flashLoggerConfig", Lua_FlashLoggerConfig);

	lua_register(L,"setAnalogLabel", Lua_SetAnalogChannelLabel);
	lua_register(L,"getAnalogLabel", Lua_GetAnalogChannelLabel);
	
	lua_register(L,"setAnalogSampleRate", Lua_SetAnalogChannelSampleRate);
	lua_register(L,"getAnalogSampleRate", Lua_GetAnalogChannelSampleRate);
	
	lua_register(L,"setAnalogScaling", Lua_SetAnalogChannelScaling);
	lua_register(L,"getAnalogScaling", Lua_GetAnalogChannelScaling);
	
	lua_register(L,"setPwmClockFrequency",Lua_SetPWMClockFrequency);
	lua_register(L,"getPwmClockFrequency",Lua_GetPWMClockFrequency);
	
	lua_register(L,"setPwmLabel", Lua_SetPWMLabel);
	lua_register(L,"getPwmLabel", Lua_GetPWMLabel);
	
	lua_register(L,"setPwmSampleRate", Lua_SetPWMSampleRate);
	lua_register(L,"getPwmSampleRate", Lua_GetPWMSampleRate);
	
	lua_register(L,"setPwmOutputConfig", Lua_SetPWMOutputConfig);
	lua_register(L,"getPwmOutputConfig", Lua_GetPWMOutputConfig);
	
	lua_register(L,"setPwmLoggingConfig", Lua_SetPWMLoggingConfig);
	lua_register(L,"getPwmLoggingConfig", Lua_GetPWMLoggingConfig);
	
	lua_register(L,"setPwmStartupDutyCycle", Lua_SetPWMStartupDutyCycle);
	lua_register(L,"getPwmStartupDutyCycle", Lua_GetPWMStartupDutyCycle);
		
	lua_register(L,"setPwmStartupPeriod", Lua_SetPWMStartupPeriod);
	lua_register(L,"getPwmStartupPeriod", Lua_GetPWMStartupPeriod);
	
	lua_register(L,"setPwmVoltageScaling", Lua_SetPWMVoltageScaling);
	lua_register(L,"getPwmVoltageScaling", Lua_GetPWMVoltageScaling);
	
	lua_register(L,"setGpsInstalled", Lua_SetGPSInstalled);
	lua_register(L,"getGpsInstalled", Lua_GetGPSInstalled);
	
	lua_register(L,"setGpsQualityLabel", Lua_SetGPSQualityLabel);
	lua_register(L,"getGpsQualityLabel", Lua_GetGPSQualityLabel);
	
	lua_register(L,"setGpsSatsLabel", Lua_SetGPSSatsLabel);
	lua_register(L,"getGpsSatsLabel", Lua_GetGPSSatsLabel);
	
	lua_register(L,"setGpsLatitudeLabel", Lua_SetGPSLatitudeLabel);
	lua_register(L,"getGpsLatitudeLabel", Lua_GetGPSLatitudeLabel);
	
	lua_register(L,"setGpsLongitudeLabel", Lua_SetGPSLongitudeLabel);
	lua_register(L,"getGpsLongitudeLabel", Lua_GetGPSLongitudeLabel);
	
	lua_register(L,"setGpsTimeLabel", Lua_SetGPSTimeLabel);
	lua_register(L,"getGpsTimeLabel", Lua_GetGPSTimeLabel);
	
	lua_register(L,"setGpsVelocityLabel", Lua_SetGPSVelocityLabel);
	lua_register(L,"getGpsVelocityLabel", Lua_GetGPSVelocityLabel);
	
	lua_register(L,"setGpsPositionSampleRate", Lua_SetGPSPositionSampleRate);
	lua_register(L,"getGpsPositionSampleRate", Lua_GetGPSPositionSampleRate);
	
	lua_register(L,"setGpsVelocitySampleRate", Lua_SetGPSVelocitySampleRate);
	lua_register(L,"getGpsVelocitySampleRate", Lua_GetGPSVelocitySampleRate);
	
	lua_register(L,"setGpsTimeSampleRate", Lua_SetGPSTimeSampleRate);
	lua_register(L,"getGpsTimeSampleRate", Lua_GetGPSTimeSampleRate);
	
	lua_register(L,"setGpioLabel", Lua_SetGPIOLabel);
	lua_register(L,"getGpioLabel", Lua_GetGPIOLabel);
	
	lua_register(L,"setGpioSampleRate", Lua_SetGPIOSampleRate);
	lua_register(L,"getGpioSampleRate", Lua_GetGPIOSampleRate);
	
	lua_register(L,"setGpioConfig", Lua_SetGPIOConfig);
	lua_register(L,"getGpioConfig", Lua_GetGPIOConfig);
	
	lua_register(L,"setTimerLabel", Lua_SetTimerLabel);
	lua_register(L,"getTimerLabel", Lua_GetTimerLabel);
	
	lua_register(L,"setTimerSampleRate", Lua_SetTimerSampleRate);
	lua_register(L,"getTimerSampleRate", Lua_GetTimerSampleRate);
	
	lua_register(L,"setTimerConfig", Lua_SetTimerConfig);
	lua_register(L,"getTimerConfig", Lua_GetTimerConfig);
	
	lua_register(L,"setTimerPulsePerRevolution",Lua_SetTimerPulsePerRevolution);
	lua_register(L,"getTimerPulsePerRevolution",Lua_GetTimerPulsePerRevolution);
	
	lua_register(L,"setTimerDivider", Lua_SetTimerDivider);
	lua_register(L,"getTimerDivider", Lua_GetTimerDivider);
	
	lua_register(L,"calculateTimerScaling", Lua_CalculateTimerScaling);
	lua_register(L,"getTimerScaling", Lua_GetTimerScaling);
	
	lua_register(L,"setAccelInstalled",Lua_SetAccelInstalled);
	lua_register(L,"getAccelInstalled",Lua_GetAccelInstalled);
	
	lua_register(L,"setAccelLabel",Lua_SetAccelLabel);
	lua_register(L,"getAccelLabel",Lua_GetAccelLabel);
	
	lua_register(L,"setAccelSampleRate",Lua_SetAccelSampleRate);
	lua_register(L,"getAccelSampleRate",Lua_GetAccelSampleRate);
	
	lua_register(L,"setAccelIdleSampleRate",Lua_SetAccelIdleSampleRate);
	lua_register(L,"getAccelIdleSampleRate",Lua_GetAccelIdleSampleRate);
	
	lua_register(L,"setAccelConfig",Lua_SetAccelConfig);
	lua_register(L,"getAccelConfig",Lua_GetAccelConfig);
	
	lua_register(L,"setAccelChannel",Lua_SetAccelChannel);
	lua_register(L,"getAccelChannel",Lua_GetAccelChannel);
	
	lua_register(L,"setAccelZeroValue",Lua_SetAccelZeroValue);
	lua_register(L,"getAccelZeroValue",Lua_GetAccelZeroValue);

	lua_register(L,"calibrateAccelZero",Lua_CalibrateAccelZero);
	
	unlockLua();
}

int Lua_IsSDCardPresent(lua_State *L){
	lua_pushinteger(L,isCardPresent());
	return 1;
}

int Lua_IsSDCardWritable(lua_State *L){
	lua_pushinteger(L,isCardWritable());
	return 1;
}

int Lua_SetAccelInstalled(lua_State *L){
	if (lua_gettop(L) >= 1){
		getWorkingLoggerConfig()->AccelInstalled = lua_toboolean(L,1);	
	}
	return 0;	
}

int Lua_GetAccelInstalled(lua_State *L){
	lua_pushboolean(L,getWorkingLoggerConfig()->AccelInstalled);
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

int Lua_SetAccelIdleSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->idleSampleRate = encodeSampleRate(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetAccelIdleSampleRate(lua_State *L){
	if (lua_gettop(L) >=1 ){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c){	
			lua_pushnumber(L, c->idleSampleRate);
			return 1;
		}
	}
	return 0;	
}

int Lua_SetAccelConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->config = filterAccelConfig(lua_tointeger(L,2));	
	}
	return 0;
}

int Lua_GetAccelConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushnumber(L, c->config);
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
		if (NULL != c) c->config = filterTimerConfig(lua_tointeger(L,2));
	}	
	return 0;
}

int Lua_GetTimerConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L, c->config);
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
		if (NULL != c) calculateTimerScaling(c);
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
		int channel = lua_tointeger(L,1) - 1 ;
		if (channel >= 0 && channel <= 2){//1 based
			GPIOConfig *c = getGPIOConfigChannel(channel);
			//0= configure as input, 1=configure as output
			if (NULL != c) c->config = filterGPIOConfig(lua_tointeger(L,2));
			InitGPIO(getWorkingLoggerConfig()); //reload configuration
		}
	}
	return 0;
}

int Lua_GetGPIOConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		int channel = lua_tointeger(L,1) - 1;
		if (channel >= 0 && channel <= 2){
			GPIOConfig *c = getGPIOConfigChannel(channel);
			if (NULL !=c){
				//0=configure as input, 1=configure as output
				lua_pushinteger(L,c->config);
				return 1;
			}
		}
	}	
	return 0;
}

int Lua_SetGPSInstalled(lua_State *L){
	if (lua_gettop(L) >=1 ){
		getWorkingLoggerConfig()->GPSInstalled = lua_toboolean(L,1);	
	}
	return 0;	
}

int Lua_GetGPSInstalled(lua_State *L){
	lua_pushboolean(L,getWorkingLoggerConfig()->GPSInstalled);
	return 1;	
}


int Lua_SetGPSQualityLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.qualityCfg.label,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSQualityLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.qualityCfg.label);
	return 1;	
}

int Lua_SetGPSSatsLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.satellitesCfg.label,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSSatsLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.satellitesCfg.label);
	return 1;	
}

int Lua_SetGPSLatitudeLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.latitudeCfg.label,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSLatitudeLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.latitudeCfg.label);
	return 1;
}

int Lua_SetGPSLongitudeLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.longitudeCfg.label,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSLongitudeLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.longitudeCfg.label);
	return 1;	
}

int Lua_SetGPSTimeLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.timeCfg.label,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSTimeLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.timeCfg.label);
	return 1;	
}

int Lua_SetGPSVelocityLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.velocityCfg.label,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSVelocityLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.velocityCfg.label);
	return 1;	
}

int Lua_SetGPSPositionSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		LoggerConfig *cfg = getWorkingLoggerConfig();
		cfg->GPSConfig.latitudeCfg.sampleRate = cfg->GPSConfig.longitudeCfg.sampleRate = encodeSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetGPSPositionSampleRate(lua_State *L){
	//TODO we pull one for all... is there a better way? individual settable sample rates?
	lua_pushinteger(L,getWorkingLoggerConfig()->GPSConfig.latitudeCfg.sampleRate);
	return 1;		
}

int Lua_SetGPSVelocitySampleRate(lua_State *L){
	if (lua_gettop(L) >= 1 ){
		getWorkingLoggerConfig()->GPSConfig.velocityCfg.sampleRate = encodeSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetGPSVelocitySampleRate(lua_State *L){
	lua_pushinteger(L,getWorkingLoggerConfig()->GPSConfig.velocityCfg.sampleRate);
	return 1;
}

int Lua_SetGPSTimeSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1 ){
		getWorkingLoggerConfig()->GPSConfig.timeCfg.sampleRate = encodeSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetGPSTimeSampleRate(lua_State *L){
	lua_pushinteger(L,getWorkingLoggerConfig()->GPSConfig.timeCfg.sampleRate);
	return 1;
}

int Lua_SetPWMClockFrequency(lua_State *L){
	if (lua_gettop(L) >= 1){
		getWorkingLoggerConfig()->PWMClockFrequency = filterPWMClockFrequency(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetPWMClockFrequency(lua_State *L){
	lua_pushinteger(L,getWorkingLoggerConfig()->PWMClockFrequency);
	return 1;	
}

int Lua_SetPWMLabel(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c) setLabelGeneric(c->cfg.label,lua_tostring(L,2));
	}
	return 0;
}

int Lua_GetPWMLabel(lua_State *L){
	if (lua_gettop(L) >=1){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushstring(L,c->cfg.label);
		}	
	}	
	return 0;
}

int Lua_SetPWMSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->cfg.sampleRate = encodeSampleRate(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetPWMSampleRate(lua_State *L){
	if (lua_gettop(L) >=1){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->cfg.sampleRate);
			return 1;
		}	
	}	
	return 0;
}

int Lua_SetPWMOutputConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->outputConfig = filterPWMOutputConfig(lua_tointeger(L,2));	
	}
	return 0;
}

int Lua_GetPWMOutputConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->outputConfig);
			return 1;	
		}		
	}
	return 0;	
}

int Lua_SetPWMLoggingConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->loggingConfig = filterPWMLoggingConfig(lua_tointeger(L,2));	
	}
	return 0;	
}

int Lua_GetPWMLoggingConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->loggingConfig);
			return 1;	
		}
	}
	return 0;
}

int Lua_SetPWMStartupDutyCycle(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->startupDutyCycle = filterPWMDutyCycle(lua_tointeger(L,2));
		}	
	}
	return 0;	
}

int Lua_GetPWMStartupDutyCycle(lua_State *L){
	if (lua_gettop(L) >= 1){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->startupDutyCycle);
			return 1;	
		}		
	}
	return 0;
}

int Lua_SetPWMStartupPeriod(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->startupPeriod = filterPWMPeriod(lua_tointeger(L,2));
		}	
	}
	return 0;	
}

int Lua_GetPWMStartupPeriod(lua_State *L){
	if (lua_gettop(L) >= 1){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->startupPeriod);
			return 1;	
		}		
	}
	return 0;	
}

int Lua_SetPWMVoltageScaling(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->voltageScaling = lua_tonumber(L,2);
		}	
	}
	return 0;	
}

int Lua_GetPWMVoltageScaling(lua_State *L){
	if (lua_gettop(L) >= 1){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
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
			c->scaling = lua_tonumber(L,2);
		}
	}
	return 0;	
}

int Lua_GetAnalogChannelScaling(lua_State *L){
	if (lua_gettop(L) >= 1){
		ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushnumber(L,c->scaling);			
		}		
	}
	return 0;
}


int Lua_GetAnalog(lua_State *L){
	float result = -1;
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			result = c->scaling * ReadADC(channel);
		}
	}
	lua_pushnumber(L,result);
	return 1;
}

int Lua_GetAnalogRaw(lua_State *L){
	int result = -1;
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		if (channel >= 0 && channel < CONFIG_ADC_CHANNELS){
			result = (int)ReadADC(channel);
		}
	}
	lua_pushnumber(L,result);
	return 1;
}

int calculateXTimerValue(lua_State *L, unsigned int (*scaler)(unsigned int,unsigned int)){
	int result = -1;
	if (lua_gettop(L) >=  1){
		int channel = lua_tointeger(L,1);
		TimerConfig *c = getTimerConfigChannel(channel);
		if (c != NULL) result = (*scaler)(getTimerPeriod(channel),c->calculatedScaling);
	}
	return result;
}


int Lua_GetRPM(lua_State *L){
	lua_pushinteger(L,calculateXTimerValue(L,calculateRPM));
	return 1;
}

int Lua_GetPeriodMs(lua_State *L){
	lua_pushinteger(L,calculateXTimerValue(L,calculatePeriodMs));
	return 1;
}

int Lua_GetPeriodUsec(lua_State *L){
	lua_pushinteger(L,calculateXTimerValue(L,calculatePeriodUsec));
	return 1;
}

int Lua_GetFrequency(lua_State *L){
	lua_pushinteger(L,calculateXTimerValue(L,calculateFrequencyHz));
	return 1;
}

int Lua_GetTimerRaw(lua_State *L){
	int result = -1;
	if (lua_gettop(L) >= 1){
		int channel = lua_tointeger(L,1);
		result = getTimerPeriod(channel);
	}
	lua_pushinteger(L,result);
	return 1;
}

int Lua_ResetTimerCount(lua_State *L){
	if (lua_gettop(L) >= 1){
		resetTimerCount(lua_tointeger(L,1));
	}
	return 0;
}


int Lua_GetTimerCount(lua_State *L){
	int result = -1;
	if (lua_gettop(L) >= 1){
		int channel = lua_tointeger(L,1);
		result = getTimerCount(channel);		
	}
	lua_pushinteger(L,result);
	return 1;
}

int Lua_GetButton(lua_State *L){
	unsigned int pushbutton = GetGPIOBits() & PIO_PUSHBUTTON_SWITCH;
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
	unsigned int result = 0;
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		unsigned int gpioBits = GetGPIOBits();
		switch (channel){
			case 0:
				result = gpioBits & GPIO_1;
				break;
			case 1:
				result = gpioBits & GPIO_2;
				break;
			case 2:
				result = gpioBits & GPIO_3;
				break;
		}
	}
	lua_pushinteger(L,(result != 0 ));
	return 1;
}

int Lua_SetGPIO(lua_State *L){
	if (lua_gettop(L) >=2){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		unsigned int state = (unsigned int)lua_tointeger(L,2);
		unsigned int gpioBits = 0;
		switch (channel){
			case 0:
				gpioBits = GPIO_1;
				break;
			case 1:
				gpioBits = GPIO_2;
				break;
			case 2:
				gpioBits = GPIO_3;
				break;
		}
		if (state){
			SetGPIOBits(gpioBits);
		} else{
			ClearGPIOBits(gpioBits);
		}
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

int Lua_GetGPSVelocity(lua_State *L){
	lua_pushnumber(L,getGPSVelocity());
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
		if (channel >= ACCELEROMETER_CHANNEL_MIN && channel <= ACCELEROMETER_CHANNEL_MAX){
			float g = convertAccelRawToG(readAccelChannel(channel),DEFAULT_ACCEL_ZERO);
			lua_pushnumber(L,g);
			return 1;
		}
	}
	return 0;
}


int Lua_ReadAccelerometerRaw(lua_State *L){
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		if (channel >= ACCELEROMETER_CHANNEL_MIN && channel <= ACCELEROMETER_CHANNEL_MAX){
			int accelValue =  readAccelChannel(channel);
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
			PWM_SetDutyCycle( channel, (unsigned short)dutyCycleRaw);
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
			PWM_SetPeriod( channel, (unsigned short)periodRaw);
		}
	}	
	return 0;
}

int Lua_SetAnalogOut(lua_State *L){
	if (lua_gettop(L) >= 2){
		PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			unsigned int channel = (unsigned int)lua_tointeger(L,1);
			if (channel >= 0 && channel < CONFIG_PWM_CHANNELS){
				float dutyCycle = (float)lua_tonumber(L,2) / c->voltageScaling;
				PWM_SetDutyCycle(channel,(unsigned short)dutyCycle);
			}
		}
	}
	return 0;
}


int Lua_StartLogging(lua_State *L){
	if (! g_loggingShouldRun) xSemaphoreGive(g_xLoggerStart);
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
		unsigned int mask = 0;
		switch (LED){
			case 1:
				mask = LED1;
				break;
			case 2:
				mask = LED2;
				break;
			case 3:
				mask = LED3;
				break;
		}
		if (state){
			EnableLED(mask);
		}
		else{
			DisableLED(mask);	
		}
	}
	return 0;
}


int Lua_FlashLoggerConfig(lua_State *L){
	int result = flashLoggerConfig();
	lua_pushinteger(L,result);
	return 1;	
}

