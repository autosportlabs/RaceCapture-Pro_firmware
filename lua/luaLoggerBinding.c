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

extern xSemaphoreHandle g_xLoggerStart;
extern int g_loggingShouldRun;


void RegisterLuaRaceCaptureFunctions(lua_State *L){

	lua_register(L,"getInput",Lua_GetInput);
	lua_register(L,"getButton",Lua_GetButton);
	lua_register(L,"setOutput",Lua_SetOutput);

	lua_register(L,"setPWMDutyCycle",Lua_SetPWMDutyCycle);
	lua_register(L,"setPWMDutyCycleRaw",Lua_SetPWMDutyCycleRaw);

	lua_register(L,"setPWMFrequency",Lua_SetPWMFrequency);
	lua_register(L,"setPWMPeriodRaw",Lua_SetPWMPeriodRaw);

	lua_register(L,"getFrequency",Lua_GetFrequency);
	lua_register(L,"getTimerRaw",Lua_GetTimerRaw);
	
	lua_register(L,"getAnalog",Lua_GetAnalog);
	lua_register(L,"getAnalogRaw",Lua_GetAnalogRaw);
	lua_register(L,"setAnalogOut",Lua_SetAnalogOut);

	lua_register(L,"getGPS",Lua_GetGPS);
					
	lua_register(L,"getAccelerometer",Lua_GetAccelerometer);
	lua_register(L,"getAccelerometerRaw",Lua_GetAccelerometerRaw);
	
	lua_register(L,"startLogging",Lua_StartLogging);
	lua_register(L,"stopLogging",Lua_StopLogging);	

	lua_register(L,"setLED",Lua_SetLED);
	
	lua_register(L,"writeScriptPage",Lua_WriteScriptPage);
	
	lua_register(L,"print",Lua_Print);
	lua_register(L,"println", Lua_Println);
}

int Lua_GetAnalog(lua_State *L){
	
	return 0;
}

int Lua_GetAnalogRaw(lua_State *L){
	int result = -1;
	if (lua_gettop(L) >= 1){
		//make 1-basd
		int channel = (unsigned int)lua_tonumber(L,1) - 1;
		if (channel >= ADC_CHANNEL_MIN && channel <= ADC_CHANNEL_MAX){
			result = (int)ReadADC(channel);
		}
	}
	lua_pushnumber(L,result);
	return 1;
}


int Lua_GetFrequency(lua_State *L){

	return 0;
}

int Lua_GetTimerRaw(lua_State *L){
	int result = -1;
	if (lua_gettop(L) >= 1){
		//make 1-based
		unsigned int channel = (unsigned int)lua_tonumber(L,1) - 1;
		if (channel >= TIMER_CHANNEL_MIN && channel <= TIMER_CHANNEL_MAX){
			result = (int)getTimerPeriod(channel);
		}
	}
	lua_pushnumber(L,result);
	return 1;
}

int Lua_GetButton(lua_State *L){
	unsigned int pushbutton = GetGPIOBits() | PIO_PUSHBUTTON_SWITCH;
	lua_pushnumber(L,(pushbutton != 0));
	return 1;	
}

int Lua_GetInput(lua_State *L){
	int result = -1;
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tonumber(L,1);
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
	lua_pushnumber(L,(result !=0 ));
	return 1;
}

int Lua_SetOutput(lua_State *L){
	if (lua_gettop(L) >=2){
		unsigned int channel = (unsigned int)lua_tonumber(L,1);
		unsigned int state = (unsigned int)lua_tonumber(L,2);
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
		//make 1-based
		unsigned int channel = (unsigned int)lua_tonumber(L,1) - 1;
		if (channel >= ACCELEROMETER_CHANNEL_MIN && channel <= ACCELEROMETER_CHANNEL_MAX){
			accelValue = accel_readAxis(channel);
		}
	}
	lua_pushnumber(L,accelValue);
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
		//make 1-based
		unsigned int channel = (unsigned int)lua_tonumber(L,1) - 1;
		unsigned int dutyCycleRaw = (unsigned int)lua_tonumber(L,2);
		if (channel >= PWM_CHANNEL_MIN && channel <= PWM_CHANNEL_MAX){
			PWM_SetDutyCycle( channel, (unsigned short)dutyCycleRaw);
		}
	}
	return 0;
}

int Lua_SetPWMPeriodRaw(lua_State *L){
	if (lua_gettop(L) >= 2){
		//make 1-based	
		unsigned int channel = (unsigned int)lua_tonumber(L,1) - 1;
		unsigned int periodRaw = (unsigned int)lua_tonumber(L,2);
		if (channel >= PWM_CHANNEL_MIN && channel <= PWM_CHANNEL_MAX){
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
		unsigned int LED = lua_tonumber(L,1);
		unsigned int state = lua_tonumber(L,2);
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

int Lua_WriteScriptPage(lua_State *L){
	if (lua_gettop(L) >= 2){
		unsigned int page = lua_tonumber(L,1);
		const char * data = lua_tostring(L,2);	
		flashScriptPage(page,data);
	}	
	return 0;
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
