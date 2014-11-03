#include "luaLoggerBinding.h"
#include "dateTime.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerConfig.h"
#include "loggerData.h"
#include "gps.h"
#include "imu.h"
#include "ADC.h"
#include "timer.h"
#include "CAN.h"
#include "OBD2.h"
#include "PWM.h"
#include "LED.h"
#include "GPIO.h"
#include "luaScript.h"
#include "luaTask.h"
#include "mod_string.h"
#include "serial.h"
#include "printk.h"
#include "modp_numtoa.h"
#include "loggerTaskEx.h"
#include "loggerSampleData.h"
#include "virtual_channel.h"

#define TEMP_BUFFER_LEN 200
#define DEFAULT_CAN_TIMEOUT 	100


char g_tempBuffer[TEMP_BUFFER_LEN];

void registerLuaLoggerBindings(lua_State *L){

	//Read / control inputs and outputs
	lua_registerlight(L,"getGpio",Lua_GetGPIO);
	lua_registerlight(L,"setGpio",Lua_SetGPIO);
	lua_registerlight(L,"getButton",Lua_GetButton);

	lua_registerlight(L,"setPwmDutyCycle",Lua_SetPWMDutyCycle);
	lua_registerlight(L,"setPwmPeriod",Lua_SetPWMPeriod);
	lua_registerlight(L,"setAnalogOut",Lua_SetAnalogOut);
	lua_registerlight(L,"setPwmClockFreq",Lua_SetPWMClockFrequency);
	lua_registerlight(L,"getPwmClockFreq",Lua_GetPWMClockFrequency);

	lua_registerlight(L,"getTimerRpm",Lua_GetRPM);
	lua_registerlight(L,"getTimerPeriodMs",Lua_GetPeriodMs);
	lua_registerlight(L,"getTimerFreq",Lua_GetFrequency);
	lua_registerlight(L,"getTimerRaw",Lua_GetTimerRaw);
	lua_registerlight(L,"resetTimerCount",Lua_ResetTimerCount);
	lua_registerlight(L,"getTimerCount",Lua_GetTimerCount);

	lua_registerlight(L,"getAnalog",Lua_GetAnalog);

	lua_registerlight(L,"getImu",Lua_ReadImu);
	lua_registerlight(L,"getImuRaw",Lua_ReadImuRaw);

	lua_registerlight(L,"getGpsPos", Lua_GetGPSPosition);
	lua_registerlight(L,"getGpsSpeed",Lua_GetGPSSpeed);
	lua_registerlight(L,"getGpsQuality", Lua_GetGPSQuality);
	lua_registerlight(L,"getGpsDist", Lua_GetGPSDistance);

	lua_registerlight(L, "getLapCount", Lua_GetLapCount);
	lua_registerlight(L, "getLapTime", Lua_GetLapTime);
	lua_registerlight(L, "getGpsSec", Lua_GetGpsSecondsSinceFirstFix);
	lua_registerlight(L, "getAtStartFinish",Lua_GetGPSAtStartFinish);

   lua_registerlight(L, "getTickCount", Lua_GetTickCount);
   lua_registerlight(L, "getTicksPerSecond", Lua_GetTicksPerSecond);

	lua_registerlight(L, "initCAN", Lua_InitCAN);
	lua_registerlight(L, "txCAN", Lua_SendCANMessage);
	lua_registerlight(L, "rxCAN", Lua_ReceiveCANMessage);
	lua_registerlight(L, "setCANfilter", Lua_SetCANFilter);
	lua_registerlight(L, "readOBD2", Lua_ReadOBD2);

	lua_registerlight(L,"startLogging",Lua_StartLogging);
	lua_registerlight(L,"stopLogging",Lua_StopLogging);
	lua_registerlight(L,"isLogging" , Lua_IsLogging);

	lua_registerlight(L,"setLed",Lua_SetLED);

	lua_registerlight(L,"readSerial", Lua_ReadSerialLine);
	lua_registerlight(L,"writeSerial", Lua_WriteSerial);

	//Logger configuration editing
	lua_registerlight(L,"flashLoggerCfg", Lua_FlashLoggerConfig);

	lua_registerlight(L,"calibrateImuZero",Lua_CalibrateImuZero);

	lua_registerlight(L,"setBgStream", Lua_SetBackgroundStreaming);
	lua_registerlight(L,"getBgStream", Lua_GetBackgroundStreaming);

	lua_registerlight(L, "addChannel", Lua_AddVirtualChannel);
	lua_registerlight(L, "setChannel", Lua_SetVirtualChannelValue);
}

////////////////////////////////////////////////////

int Lua_SetBackgroundStreaming(lua_State *L){
	if (lua_gettop(L) >= 1){
		getWorkingLoggerConfig()->ConnectivityConfigs.telemetryConfig.backgroundStreaming = (lua_tointeger(L,1) == 1);
	}
	return 0;
}

int Lua_GetBackgroundStreaming(lua_State *L){
	lua_pushinteger(L, getWorkingLoggerConfig()->ConnectivityConfigs.telemetryConfig.backgroundStreaming == 1);
	return 1;
}

int Lua_CalibrateImuZero(lua_State *L){
	imu_calibrate_zero();
	return 0;
}

int Lua_GetGPSAtStartFinish(lua_State *L){
	lua_pushinteger(L,getAtStartFinish());
	return 1;
}

int Lua_GetAtSplit(lua_State *L){
	lua_pushinteger(L,getAtSector());
	return 1;
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

int Lua_GetAnalog(lua_State *L){
	float analogValue = -1;
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		ADCConfig *ac = getADCConfigChannel(lua_tointeger(L,1));
		if (NULL != ac){
			float adcRaw = ADC_read(channel);
			switch(ac->scalingMode){
			case SCALING_MODE_RAW:
				analogValue = adcRaw;
				break;
			case SCALING_MODE_LINEAR:
				analogValue = (ac->linearScaling * (float)adcRaw);
				break;
			case SCALING_MODE_MAP:
				analogValue = get_mapped_value((float)adcRaw,&(ac->scalingMap));
				break;
			}
		}
	}
	lua_pushnumber(L, analogValue);
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
		int serialPort = lua_tointeger(L,1);
		Serial *serial = get_serial(serialPort);
		if (serial){
			const char * data = lua_tostring(L, 2);
			serial->put_s(data);
		}
	}
	return 0;
}

int Lua_ReadSerialLine(lua_State *L){
	if (lua_gettop(L) >= 1){
		int serialPort = lua_tointeger(L,1);
		Serial *serial = get_serial(serialPort);
		if (serial){
			serial->get_line(g_tempBuffer, TEMP_BUFFER_LEN);
			lua_pushstring(L,g_tempBuffer);
			return 1;
		}
	}
	return 0; //missing or bad parameter
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

int Lua_GetGPSPosition(lua_State *L){
	lua_pushnumber(L, getLatitude());
	lua_pushnumber(L, getLongitude());
	return 2;
}

int Lua_GetGPSSpeed(lua_State *L){
	lua_pushnumber(L,getGPSSpeed());
	return 1;
}

int Lua_GetGPSQuality(lua_State *L){
	lua_pushnumber(L,getGPSQuality());
	return 1;
}

int Lua_GetGPSDistance(lua_State *L){
	lua_pushnumber(L, getGpsDistance());
	return 1;
}

int Lua_GetLapTime(lua_State *L){
   // XXX: TIME_HACK.  Convert to millis.
	lua_pushnumber(L, tinyMillisToMinutes(getLastLapTime()));
	return 1;
}

int Lua_GetLapCount(lua_State *L){
	lua_pushinteger(L,getLapCount());
	return 1;
}

int Lua_GetGpsSecondsSinceFirstFix(lua_State *L) {
   const tiny_millis_t millis = getMillisSinceFirstFix();
   // XXX: TIME_HACK.  Convert to millis.
   lua_pushnumber(L, tinyMillisToSeconds(millis));
   return 1;
}

int Lua_GetTickCount(lua_State *L) {
   const float f = (float) xTaskGetTickCount();
   lua_pushnumber(L, f);
   return 1;
}

int Lua_GetTicksPerSecond(lua_State *L) {
   const float f = (float) configTICK_RATE_HZ;
   lua_pushnumber(L, f);
   return 1;
}

int Lua_ReadImu(lua_State *L){
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		if (channel >= 0 && channel < CONFIG_IMU_CHANNELS){
			ImuConfig *ac = &getWorkingLoggerConfig()->ImuConfigs[channel];
			float value = imu_read_value(channel,ac);
			lua_pushnumber(L, value);
			return 1;
		}
	}
	return 0;
}

int Lua_ReadImuRaw(lua_State *L){
	if (lua_gettop(L) >= 1){
		unsigned int channel = (unsigned int)lua_tointeger(L,1);
		if (channel >= 0 && channel <= CONFIG_IMU_CHANNELS){
			int value =  imu_read(channel);
			lua_pushinteger(L,value);
			return 1;
		}
	}
	return 0;
}


int Lua_SetPWMDutyCycle(lua_State *L){
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
				float dutyCycle = (float)lua_tonumber(L,2) / PWM_VOLTAGE_SCALING;
				PWM_set_duty_cycle(channel,(unsigned short)dutyCycle);
			}
		}
	}
	return 0;
}

int Lua_InitCAN(lua_State *L){
	if (lua_gettop(L) >= 1){
		size_t port = 0;
		if (lua_gettop(L) >= 2) port = (size_t)lua_tointeger(L, 2);

		uint32_t baud = lua_tointeger(L, 1);
		int rc = CAN_init_port(port, baud);
		lua_pushinteger(L, rc);
		return 1;
	}
	else{
		return 0;
	}
}

int Lua_SetCANFilter(lua_State *L){
	if (lua_gettop(L) >= 5){
		uint8_t channel = lua_tointeger(L, 1);
		uint8_t id = lua_tointeger(L, 2);
		uint8_t extended = lua_tointeger(L, 3);
		uint32_t filter = lua_tointeger(L, 4);
		uint32_t mask = lua_tointeger(L, 5);
		int rc = CAN_set_filter(channel, id, extended, filter, mask);
		lua_pushinteger(L, rc);
		return 1;
	}
	return 0;
}

int Lua_SendCANMessage(lua_State *L){
	size_t timeout = 1000;
	int rc = -1;
	if (lua_gettop(L) >= 4){
		CAN_msg msg;
		uint8_t channel = (uint8_t)lua_tointeger(L, 1);
		msg.addressValue = (unsigned int)lua_tointeger(L, 2);
		msg.isExtendedAddress = lua_tointeger(L, 3);

		int size = luaL_getn(L, 4);
		if (size <= CAN_MSG_SIZE){
			for (int i = 1; i <= size; i++){
				lua_pushnumber(L,i);
				lua_gettable(L, 4);
				int val = lua_tonumber(L, -1);
				msg.data[i - 1] = val;
				lua_pop(L, 1);
			}
		}
		msg.dataLength = size;
		rc = CAN_tx_msg(channel, &msg, timeout);
	}
	lua_pushinteger(L, rc);
	return rc;
}

int Lua_ReceiveCANMessage(lua_State *L){
	size_t timeout = DEFAULT_CAN_TIMEOUT;
	if (lua_gettop(L) >= 1){
		uint8_t channel = (uint8_t)lua_tointeger(L, 1);
		if (lua_gettop(L) >= 2) timeout = lua_tointeger(L, 2);

		CAN_msg msg;
		int rc = CAN_rx_msg(channel, &msg, timeout);
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
		return 0;
	}
	lua_pushnumber(L, -1);
	return 1;
}

int Lua_ReadOBD2(lua_State *L){
	if (lua_gettop(L) >= 1){
		unsigned char pid = (unsigned char)lua_tointeger(L, 1);
		int value;
		if (OBD2_request_PID(pid, &value, OBD2_PID_DEFAULT_TIMEOUT_MS)){
			lua_pushnumber(L, value);
			return 1;
		}
	}
	return 0;
}

int Lua_StartLogging(lua_State *L){
	startLogging();
	return 0;
}

int Lua_StopLogging(lua_State *L){
	stopLogging();
	return 0;
}

int Lua_IsLogging(lua_State *L){
	lua_pushinteger(L, isLogging());
	return 1;
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

int Lua_AddVirtualChannel(lua_State *L){
	if (lua_gettop(L) != 6)
      return 0;

   ChannelConfig chCfg;

   strncpy(chCfg.label, lua_tostring(L, 1), DEFAULT_LABEL_LENGTH);
   strncpy(chCfg.units, lua_tostring(L, 2), DEFAULT_UNITS_LENGTH);
   chCfg.min = lua_tonumber(L, 3);
   chCfg.max = lua_tonumber(L, 4);
   chCfg.precision = (unsigned char) lua_tointeger(L, 5);
   chCfg.sampleRate = (unsigned short) lua_tointeger(L, 6);

   lua_pushinteger(L, create_virtual_channel(chCfg));

   return 1;
}

int Lua_SetVirtualChannelValue(lua_State *L){
	if (lua_gettop(L) >= 2){
		int id = lua_tointeger(L, 1);
		float value = lua_tonumber(L, 2);
		set_virtual_channel_value(id,value);
	}
	return 0;
}
