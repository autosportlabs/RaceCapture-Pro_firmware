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
#include "memory.h"
#include <string.h>
#include "base64.h"

extern xSemaphoreHandle g_xLoggerStart;
extern int g_loggingShouldRun;


void RegisterLuaRaceCaptureFunctions(lua_State *L){

	//Read / control inputs and outputs
	lua_register(L,"getInput",Lua_GetInput);
	lua_register(L,"getButton",Lua_GetButton);
	lua_register(L,"setOutput",Lua_SetOutput);

	lua_register(L,"setPWMDutyCycle",Lua_SetPWMDutyCycle);
	lua_register(L,"setPWMDutyCycleRaw",Lua_SetPWMDutyCycleRaw);

	lua_register(L,"setPWMFrequency",Lua_SetPWMFrequency);
	lua_register(L,"setPWMPeriodRaw",Lua_SetPWMPeriodRaw);
	lua_register(L,"setAnalogOut",Lua_SetAnalogOut);

	lua_register(L,"getRPM",Lua_GetRPM);
	lua_register(L,"getPeriodMs",Lua_GetPeriodMs);
	lua_register(L,"getPeriodUsec",Lua_GetPeriodUsec);
	lua_register(L,"getFrequency",Lua_GetFrequency);
	lua_register(L,"getTimerRaw",Lua_GetTimerRaw);
	lua_register(L,"getTimerCount",Lua_GetTimerCount);
	
	lua_register(L,"getAnalog",Lua_GetAnalog);
	lua_register(L,"getAnalogRaw",Lua_GetAnalogRaw);

	lua_register(L,"getGPS",Lua_GetGPS);
					
	lua_register(L,"getAccelerometer",Lua_GetAccelerometer);
	lua_register(L,"getAccelerometerRaw",Lua_GetAccelerometerRaw);
	
	lua_register(L,"startLogging",Lua_StartLogging);
	lua_register(L,"stopLogging",Lua_StopLogging);	

	lua_register(L,"setLED",Lua_SetLED);

	//Script management	
	lua_register(L,"updateScriptPage",Lua_UpdateScriptPage);
	lua_register(L,"getScriptPage",Lua_GetScriptPage);
	lua_register(L,"reloadScript",Lua_ReloadScript);
	
	//Utility
	lua_register(L,"print",Lua_Print);
	lua_register(L,"println", Lua_Println);
	lua_register(L,"getStackSize", Lua_GetStackSize);
	lua_register(L,"base64encode", Lua_Base64encode);
	lua_register(L,"base64decode", Lua_Base64decode);
	
	//Logger configuration editing
	lua_register(L,"flashLoggerConfig", Lua_FlashLoggerConfig);

	lua_register(L,"setAnalogLabel", Lua_SetAnalogChannelLabel);
	lua_register(L,"getAnalogLabel", Lua_GetAnalogChannelLabel);
	
	lua_register(L,"setAnalogSampleRate", Lua_SetAnalogChannelSampleRate);
	lua_register(L,"getAnalogSampleRate", Lua_GetAnalogChannelSampleRate);
	
	lua_register(L,"setAnalogScaling", Lua_SetAnalogChannelScaling);
	lua_register(L,"getAnalogScaling", Lua_GetAnalogChannelScaling);
	
	lua_register(L,"setPWMClockFrequency",Lua_SetPWMClockFrequency);
	lua_register(L,"getPWMClockFrequency",Lua_GetPWMClockFrequency);
	
	lua_register(L,"setPWMLabel", Lua_SetPWMLabel);
	lua_register(L,"getPWMLabel", Lua_GetPWMLabel);
	
	lua_register(L,"setPWMSampleRate", Lua_SetPWMSampleRate);
	lua_register(L,"getPWMSampleRate", Lua_GetPWMSampleRate);
	
	lua_register(L,"setPWMOutputConfig", Lua_SetPWMOutputConfig);
	lua_register(L,"getPWMOutputConfig", Lua_GetPWMOutputConfig);
	
	lua_register(L,"setPWMLoggingConfig", Lua_SetPWMLoggingConfig);
	lua_register(L,"getPWMLoggingConfig", Lua_GetPWMLoggingConfig);
	
	lua_register(L,"setPWMStartupDutyCycle", Lua_SetPWMStartupDutyCycle);
	lua_register(L,"getPWMStartupDutyCycle", Lua_GetPWMStartupDutyCycle);
		
	lua_register(L,"setPWMStartupPeriod", Lua_SetPWMStartupPeriod);
	lua_register(L,"getPWMStartupPeriod", Lua_GetPWMStartupPeriod);
	
	lua_register(L,"setPWMVoltageScaling", Lua_SetPWMVoltageScaling);
	lua_register(L,"getPWMVoltageScaling", Lua_GetPWMVoltageScaling);
	
	lua_register(L,"setGPSInstalled", Lua_SetGPSInstalled);
	lua_register(L,"getGPSInstalled", Lua_GetGPSInstalled);
	
	lua_register(L,"setGPSQualityLabel", Lua_SetGPSQualityLabel);
	lua_register(L,"getGPSQualityLabel", Lua_GetGPSQualityLabel);
	
	lua_register(L,"setGPSSatsLabel", Lua_SetGPSSatsLabel);
	lua_register(L,"getGPSSatsLabel", Lua_GetGPSSatsLabel);
	
	lua_register(L,"setGPSLatitudeLabel", Lua_SetGPSLatitudeLabel);
	lua_register(L,"getGPSLatitudeLabel", Lua_GetGPSLatitudeLabel);
	
	lua_register(L,"setGPSLongitudeLabel", Lua_SetGPSLongitudeLabel);
	lua_register(L,"getGPSLongitudeLabel", Lua_GetGPSLongitudeLabel);
	
	lua_register(L,"setGPSTimeLabel", Lua_SetGPSTimeLabel);
	lua_register(L,"getGPSTimeLabel", Lua_GetGPSTimeLabel);
	
	lua_register(L,"setGPSVelocityLabel", Lua_SetGPSVelocityLabel);
	lua_register(L,"getGPSVelocityLabel", Lua_GetGPSVelocityLabel);
	
	lua_register(L,"setGPSPositionSampleRate", Lua_SetGPSPositionSampleRate);
	lua_register(L,"getGPSPositionSampleRate", Lua_GetGPSPositionSampleRate);
	
	lua_register(L,"setGPSVelocitySampleRate", Lua_SetGPSVelocitySampleRate);
	lua_register(L,"getGPSVelocitySampleRate", Lua_GetGPSVelocitySampleRate);
	
	lua_register(L,"setGPSTimeSampleRate", Lua_SetGPSTimeSampleRate);
	lua_register(L,"getGPSTimeSampleRate", Lua_GetGPSTimeSampleRate);
	
	lua_register(L,"setGPIOLabel", Lua_SetGPIOLabel);
	lua_register(L,"getGPIOLabel", Lua_GetGPIOLabel);
	
	lua_register(L,"setGPIOSampleRate", Lua_SetGPIOSampleRate);
	lua_register(L,"getGPIOSampleRate", Lua_GetGPIOSampleRate);
	
	lua_register(L,"setGPIOConfig", Lua_SetGPIOConfig);
	lua_register(L,"getGPIOConfig", Lua_GetGPIOConfig);
	
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
	
	
}
void setLabelGeneric(char *dest, const char *source){
	strncpy(dest ,source ,DEFAULT_LABEL_LENGTH);
	dest[DEFAULT_LABEL_LENGTH - 1] = 0;
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
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) setLabelGeneric(c->label,lua_tostring(L,2));
	}
	return 0;	
}

int Lua_GetAccelLabel(lua_State *L){
	if (lua_gettop(L) >= 1 ){
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushstring(L,c->label);
			return 1;
		}
	}
	return 0;	
}

int Lua_SetAccelSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2 ){
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->sampleRate = mapSampleRate(lua_tointeger(L,2));
	}	
	return 0;
}

int Lua_GetAccelSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushnumber(L,c->sampleRate);
			return 1;	
		}
	}
	return 0;	
}

int Lua_SetAccelIdleSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->idleSampleRate = mapSampleRate(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetAccelIdleSampleRate(lua_State *L){
	if (lua_gettop(L) >=1 ){
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c){	
			lua_pushnumber(L, c->idleSampleRate);
			return 1;
		}
	}
	return 0;	
}

int Lua_SetAccelConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->config = filterAccelConfig(lua_tointeger(L,2));	
	}
	return 0;
}

int Lua_GetAccelConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushnumber(L, c->config);
			return 1;
		}
	}
	return 0;	
}

int Lua_SetAccelChannel(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->accelChannel = filterAccelChannel(lua_tointeger(L,2));	
	}
	return 0;
}

int Lua_GetAccelChannel(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushnumber(L, c->accelChannel);
			return 1;	
		}
	}
	return 0;
}

int Lua_SetAccelZeroValue(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->zeroValue = filterAccelRawValue(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetAccelZeroValue(lua_State *L){
	if (lua_gettop(L) >=2 ){
		struct AccelConfig *c = getAccelConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->zeroValue);
			return 1;
		}
	}
	return 0;	
}

int Lua_SetTimerLabel(lua_State *L){
	if (lua_gettop(L) >= 3){
		struct TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			setLabelGeneric(c->label,lua_tostring(L,2));
			setLabelGeneric(c->units,lua_tostring(L,3));
		}		
	}
	return 0;
}

int Lua_GetTimerLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushstring(L,c->label);
			lua_pushstring(L,c->units);
			return 2;			
		}
	}
	return 0;	
}


int Lua_SetTimerSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->sampleRate = mapSampleRate(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetTimerSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->sampleRate);
			return 1;
		}
	}
	return 0;
}

int Lua_SetTimerConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->config = filterTimerConfig(lua_tointeger(L,2));
	}	
	return 0;
}

int Lua_GetTimerConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L, c->config);
			return 1;
		}
	}
	return 0;	
}

int Lua_SetTimerPulsePerRevolution(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->pulsePerRevolution = lua_tointeger(L,2);
	}
	return 0;
}

int Lua_GetTimerPulsePerRevolution(lua_State *L){
	if (lua_gettop(L) >=1 ){
		struct TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->pulsePerRevolution);
			return 1;	
		}
	}	
	return 0;
}

int Lua_SetTimerDivider(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->timerDivider = lua_tointeger(L,2);	
	}
	return 0;
}

int Lua_GetTimerDivider(lua_State *L){
	if (lua_gettop(L) >=1 ){
		struct TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->timerDivider);
			return 1;
		}	
	}
	return 0;
}

int Lua_CalculateTimerScaling(lua_State *L){
	if (lua_gettop(L) >=1 ){
		calculateTimerScaling(getWorkingLoggerConfig(),lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetTimerScaling(lua_State *L){
	if (lua_gettop(L) >=1 ){
		struct TimerConfig *c = getTimerConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushinteger(L,c->calculatedScaling);
			return 1;			
		}
	}
	return 0;	
}

int Lua_SetGPIOLabel(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct GPIOConfig *c = getGPIOConfigChannel(lua_tointeger(L,1));
		if (NULL != c) setLabelGeneric(c->label,lua_tostring(L,2));	
	}
	return 0;	
}

int Lua_GetGPIOLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct GPIOConfig *c = getGPIOConfigChannel(lua_tointeger(L,1));
		if (NULL !=c){
			lua_pushstring(L,c->label);
			return 1;	
		}
	}	
	return 0;
}

int Lua_SetGPIOSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct GPIOConfig *c = getGPIOConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->sampleRate = mapSampleRate(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetGPIOSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct GPIOConfig *c = getGPIOConfigChannel(lua_tointeger(L,1));
		if (NULL !=c){
			lua_pushinteger(L,c->sampleRate);
			return 1;	
		}
	}	
	return 0;
}

int Lua_SetGPIOConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct GPIOConfig *c = getGPIOConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->config = filterGPIOConfig(lua_tointeger(L,2));	
	}
	return 0;
}

int Lua_GetGPIOConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct GPIOConfig *c = getGPIOConfigChannel(lua_tointeger(L,1));
		if (NULL !=c){
			lua_pushinteger(L,c->config);
			return 1;	
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
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.qualityLabel,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSQualityLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.qualityLabel);
	return 1;	
}

int Lua_SetGPSSatsLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.satsLabel,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSSatsLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.satsLabel);
	return 1;	
}

int Lua_SetGPSLatitudeLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.latitiudeLabel,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSLatitudeLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.latitiudeLabel);
	return 1;
}

int Lua_SetGPSLongitudeLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.longitudeLabel,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSLongitudeLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.longitudeLabel);
	return 1;	
}

int Lua_SetGPSTimeLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.timeLabel,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSTimeLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.timeLabel);
	return 1;	
}

int Lua_SetGPSVelocityLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		setLabelGeneric(getWorkingLoggerConfig()->GPSConfig.velocityLabel,lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetGPSVelocityLabel(lua_State *L){
	lua_pushstring(L,getWorkingLoggerConfig()->GPSConfig.velocityLabel);
	return 1;	
}

int Lua_SetGPSPositionSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		getWorkingLoggerConfig()->GPSConfig.positionSampleRate = mapSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetGPSPositionSampleRate(lua_State *L){
	lua_pushinteger(L,getWorkingLoggerConfig()->GPSConfig.positionSampleRate);
	return 1;		
}

int Lua_SetGPSVelocitySampleRate(lua_State *L){
	if (lua_gettop(L) >= 1 ){
		getWorkingLoggerConfig()->GPSConfig.velocitySampleRate = mapSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetGPSVelocitySampleRate(lua_State *L){
	lua_pushinteger(L,getWorkingLoggerConfig()->GPSConfig.velocitySampleRate);
	return 1;
}

int Lua_SetGPSTimeSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1 ){
		getWorkingLoggerConfig()->GPSConfig.timeSampleRate = mapSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetGPSTimeSampleRate(lua_State *L){
	lua_pushinteger(L,getWorkingLoggerConfig()->GPSConfig.timeSampleRate);
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
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c) setLabelGeneric(c->label,lua_tostring(L,2));
	}
	return 0;
}

int Lua_GetPWMLabel(lua_State *L){
	if (lua_gettop(L) >=1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushstring(L,c->label);	
		}	
	}	
	return 0;
}

int Lua_SetPWMSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->sampleRate = mapSampleRate(lua_tointeger(L,2));
	}
	return 0;
}

int Lua_GetPWMSampleRate(lua_State *L){
	if (lua_gettop(L) >=1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->sampleRate);	
			return 1;
		}	
	}	
	return 0;
}

int Lua_SetPWMOutputConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->outputConfig = filterPWMOutputConfig(lua_tointeger(L,2));	
	}
	return 0;
}

int Lua_GetPWMOutputConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->outputConfig);
			return 1;	
		}		
	}
	return 0;	
}

int Lua_SetPWMLoggingConfig(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->loggingConfig = filterPWMLoggingConfig(lua_tointeger(L,2));	
	}
	return 0;	
}

int Lua_GetPWMLoggingConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->loggingConfig);
			return 1;	
		}
	}
	return 0;
}

int Lua_SetPWMStartupDutyCycle(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->startupDutyCycle = filterPWMDutyCycle(lua_tointeger(L,2));
		}	
	}
	return 0;	
}

int Lua_GetPWMStartupDutyCycle(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->startupDutyCycle);
			return 1;	
		}		
	}
	return 0;
}

int Lua_SetPWMStartupPeriod(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->startupPeriod = filterPWMPeriod(lua_tointeger(L,2));
		}	
	}
	return 0;	
}

int Lua_GetPWMStartupPeriod(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushinteger(L,c->startupPeriod);
			return 1;	
		}		
	}
	return 0;	
}

int Lua_SetPWMVoltageScaling(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->voltageScaling = lua_tonumber(L,2);
		}	
	}
	return 0;	
}

int Lua_GetPWMVoltageScaling(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			lua_pushnumber(L,c->voltageScaling);
			return 1;			
		}		
	}
	return 0;	
}


int Lua_SetAnalogChannelLabel(lua_State *L){
	if (lua_gettop(L) >= 3){
		struct ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			setLabelGeneric(c->label,lua_tostring(L,2));
			setLabelGeneric(c->units,lua_tostring(L,3));
		}
	}
	return 0;
}

int Lua_GetAnalogChannelLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushstring(L,c->label);
			lua_pushstring(L,c->units);
			return 2;			
		}	
	}	
	return 0;
}

int Lua_SetAnalogChannelSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->sampleRate = mapSampleRate(lua_tointeger(L,2));
		}
	}
	return 0;	
}

int Lua_GetAnalogChannelSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushinteger(L,c->sampleRate);
			return 1;	
		}	
	}	
	return 0;
}

int Lua_SetAnalogChannelScaling(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			c->scaling = lua_tonumber(L,2);
		}
	}
	return 0;	
}

int Lua_GetAnalogChannelScaling(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushnumber(L,c->scaling);			
		}		
	}
	return 0;
}



int Lua_GetAnalog(lua_State *L){
	
	return 0;
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
		struct TimerConfig *c = getTimerConfigChannel(channel);
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
	unsigned int pushbutton = GetGPIOBits() | PIO_PUSHBUTTON_SWITCH;
	lua_pushinteger(L,(pushbutton == 0));
	return 1;	
}

int Lua_GetInput(lua_State *L){
	int result = -1;
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		unsigned int gpioBits = GetGPIOBits();
		switch (channel){
			case 1:
				result = gpioBits | GPIO_1;
				break;
			case 2:
				result = gpioBits | GPIO_2;
				break;
			case 3:
				result = gpioBits | GPIO_3;
		}
	}
	lua_pushinteger(L,(result != 0 ));
	return 1;
}

int Lua_SetOutput(lua_State *L){
	if (lua_gettop(L) >=2){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		unsigned int state = (unsigned int)lua_tointeger(L,2);
		unsigned int gpioBits = 0;
		switch (channel){
			case 1:
				gpioBits = GPIO_1;
				break;
			case 2:
				gpioBits = GPIO_2;
				break;
			case 3:
				gpioBits = GPIO_3;
		}
		if (state){
			SetGPIOBits(gpioBits);
		} else{
			ClearGPIOBits(gpioBits);
		}
	}
	return 0;
}

int Lua_GetGPS(lua_State *L){
	return 0;
}

int Lua_GetAccelerometer(lua_State *L){
	
	return 0;
}

int Lua_GetAccelerometerRaw(lua_State *L){
	int accelValue = -1;
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		if (channel >= ACCELEROMETER_CHANNEL_MIN && channel <= ACCELEROMETER_CHANNEL_MAX){
			accelValue =  getLastAccelRead(channel);
		}
	}
	lua_pushinteger(L,accelValue);
	return 1;
}

int Lua_SetPWMDutyCycle(lua_State *L){
	return 0;
}

int Lua_SetPWMFrequency(lua_State *L){
	return 0;
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

//0 = success; -1 = flash error; -2 = memory error; -3 = incorrect script length; -4 = param error
int Lua_UpdateScriptPage(lua_State *L){
	int result = 0;
	if (lua_gettop(L) < 2){
		result = -4; //param error
	}else{
		unsigned int page = lua_tointeger(L,1);
		const char * encodedScript = lua_tostring(L,2);
		size_t encodedScriptLen = strlen(encodedScript);
		char *decodedScript = base64decode(encodedScript, encodedScriptLen);
		if (NULL == decodedScript){
			result = -2; //memory error	
		} else{
			if (strlen(decodedScript) > MEMORY_PAGE_SIZE){
				result = -3; //script length error
			} else{
				result = flashScriptPage(page, encodedScript);	
			}
			vPortFree(decodedScript);
		}
	}
	lua_pushinteger(L,result);
	return 1;
}

int Lua_GetScriptPage(lua_State *L){
	if (lua_gettop(L) >= 1){
		unsigned int page = lua_tointeger(L,1);
		const char * script = getScript();
		//forward to the requested page
		script += (MEMORY_PAGE_SIZE * page);
		//check for truncated page
		size_t scriptLen = strlen(script);
		if (scriptLen > MEMORY_PAGE_SIZE) scriptLen = MEMORY_PAGE_SIZE;
		char *encoded = base64encode(script,scriptLen);
		if (NULL != encoded){
			lua_pushstring(L,encoded);
			vPortFree(encoded);	
		}
		else{
			//error
			lua_pushinteger(L,-1);	
		}
	}
	else{
		lua_pushinteger(L,-1);	
	}
	return 1;	
}

int Lua_ReloadScript(lua_State *L){
	portENTER_CRITICAL();
	int result = luaL_dostring(L,getScript());
	if (result !=0){
		const char *err = lua_tostring(L,-1);
		char *tmp = pvPortMalloc(strlen(err) + 1);
		strcpy(tmp,err);
		lua_pop(L,1);
		lua_pushstring(L,tmp);
		vPortFree(tmp);
	}
	else{
		lua_pushstring(L,"ok");
	}
	portEXIT_CRITICAL();
	return 1;
}

int Lua_FlashLoggerConfig(lua_State *L){
	int result = flashLoggerConfig();
	lua_pushinteger(L,result);
	return 1;	
}

int Lua_Println(lua_State *L){
	if (lua_gettop(L) >= 1){
		SendString(lua_tostring(L,1));	
		SendCrlf();
	}
	return 0;
}

int Lua_Print(lua_State *L){
	if (lua_gettop(L) >= 1){
		SendString(lua_tostring(L,1));	
	}
	return 0;
}

int Lua_GetStackSize(lua_State *L){
	lua_pushinteger(L,lua_gettop(L));
	return 1;	
}

//-1 = param error; -2 = memory error
int Lua_Base64encode(lua_State *L){
	if (lua_gettop(L) >= 1){
		const char *data = lua_tostring(L,1);
		char *encoded = base64encode(data,strlen(data));
		if (NULL != encoded){
			lua_pushstring(L,encoded);
			vPortFree(encoded);
		} else{
			lua_pushinteger(L,-2);	
		}
	} else{
		lua_pushinteger(L,-1);	
	}
	return 1;	
}

//-1 = param error; -2 = memory error
int Lua_Base64decode(lua_State *L){
	if (lua_gettop(L) >= 1){
		const char *data = lua_tostring(L,1);
		char *decoded = base64decode(data,strlen(data));
		if (NULL != decoded){
			lua_pushstring(L,decoded);
			vPortFree(decoded);
		} else{
			lua_pushinteger(L,-2);	
		}
	} else{
		lua_pushinteger(L,-1);	
	}
	return 1;
}
