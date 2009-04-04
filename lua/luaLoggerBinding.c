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
	lua_register(L,"printScriptPage",Lua_PrintScriptPage);
	lua_register(L,"reloadScript",Lua_ReloadScript);
	
	//Utility
	lua_register(L,"print",Lua_Print);
	lua_register(L,"println", Lua_Println);
	
	//Logger configuration editing
	lua_register(L,"flashLoggerConfig", Lua_FlashLoggerConfig);

	lua_register(L,"setAnalogLabel", Lua_SetAnalogChannelLabel);
	lua_register(L,"setAnalogSampleRate", Lua_SetAnalogChannelSampleRate);
	lua_register(L,"setAnalogScaling", Lua_SetAnalogChannelScaling);
	
	lua_register(L,"setPWMLabel", Lua_SetPWMLabel);
	lua_register(L,"setPWMSampleRate", Lua_SetPWMSampleRate);
	lua_register(L,"setPWMOutputConfig", Lua_SetPWMOutputConfig);
	lua_register(L,"setPWMLoggingConfig", Lua_SetPWMLoggingConfig);
	lua_register(L,"setPWMStartupDutyCycle", Lua_SetPWMStartupDutyCycle);
	lua_register(L,"setPWMStartupPeriod", Lua_SetPWMStartupPeriod);
	lua_register(L,"setPWMVoltageScaling", Lua_SetPWMVoltageScaling);

}

int Lua_SetPWMLabel(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			strncpy(c->label, lua_tostring(L,2),DEFAULT_LABEL_LENGTH);
			c->label[DEFAULT_LABEL_LENGTH - 1] = 0;
		}
	}
	return 0;
}

int Lua_SetPWMSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->sampleRate = mapSampleRate(lua_tointeger(L,2));
		}	
	}
	return 0;
}

int Lua_SetPWMOutputConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->outputConfig = filterPWMOutputConfig(lua_tointeger(L,2));	
		}	
	}
	return 0;
}

int Lua_SetPWMLoggingConfig(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->loggingConfig = filterPWMLoggingConfig(lua_tointeger(L,2));	
		}	
	}
	return 0;	
}

int Lua_SetPWMStartupDutyCycle(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->startupDutyCycle = filterPWMDutyCycle(lua_tointeger(L,2));
		}	
	}
	return 0;	
}

int Lua_SetPWMStartupPeriod(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->startupPeriod = filterPWMPeriod(lua_tointeger(L,2));
		}	
	}
	return 0;	
}

int Lua_SetPWMVoltageScaling(lua_State *L){
	if (lua_gettop(L) >= 1){
		struct PWMConfig *c = getPWMConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->voltageScaling = lua_tonumber(L,2);
		}	
	}
	return 0;	
}


int Lua_SetAnalogChannelLabel(lua_State *L){
	if (lua_gettop(L) >= 3){
		struct ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			strncpy(c->label, lua_tostring(L,2),DEFAULT_LABEL_LENGTH);
			c->label[DEFAULT_LABEL_LENGTH - 1] = 0;
			
			strncpy(c->units, lua_tostring(L,3),DEFAULT_UNITS_LENGTH);
			c->label[DEFAULT_UNITS_LENGTH - 1] = 0;
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

int Lua_SetAnalogChannelScaling(lua_State *L){
	if (lua_gettop(L) >= 2){
		struct ADCConfig *c = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL != c){
			c->scaling = lua_tonumber(L,2);
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

int Lua_GetButton(lua_State *L){
	unsigned int pushbutton = GetGPIOBits() | PIO_PUSHBUTTON_SWITCH;
	lua_pushinteger(L,(pushbutton != 0));
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
	lua_pushinteger(L,(result !=0 ));
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
			accelValue = getLastAccelRead(channel);
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

int Lua_UpdateScriptPage(lua_State *L){
	int result = -1;
	if (lua_gettop(L) >= 2){
		unsigned int page = lua_tointeger(L,1);
		const char * data = lua_tostring(L,2);	
		result = flashScriptPage(page,data);
	}	
	lua_pushinteger(L,result);
	return 1;
}

int Lua_PrintScriptPage(lua_State *L){
	if (lua_gettop(L) >= 1){
		unsigned int page = lua_tointeger(L,1);
		char *tmp = pvPortMalloc(MEMORY_PAGE_SIZE + 1);
		if (tmp){
			const char * script = getScript();
			script += (MEMORY_PAGE_SIZE * page);
			memset(tmp,0,MEMORY_PAGE_SIZE + 1);
			strncpy(tmp, script, MEMORY_PAGE_SIZE);
			SendString(tmp);
			vPortFree(tmp);	
		}
		else{
			SendInt(-1);
		}
		SendCrlf();
	}
	return 0;	
}


int Lua_GetScriptPage(lua_State *L){
	if (lua_gettop(L) >= 1){
		unsigned int page = lua_tointeger(L,1);
		char *tmp = pvPortMalloc(MEMORY_PAGE_SIZE + 1);
		if (tmp){
			memset(tmp,0,MEMORY_PAGE_SIZE + 1);
			const char * script = getScript();
			script += (MEMORY_PAGE_SIZE * page);
			strncpy(tmp, script, MEMORY_PAGE_SIZE);
			lua_pushstring(L,tmp);
			vPortFree(tmp);
		}
		else{
			//error
			lua_pushinteger(L,-1);	
		}
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

int Lua_FlashLoggerConfig(lua_State *L){
	int result = flashLoggerConfig();
	lua_pushinteger(L,result);
	return 1;	
}
