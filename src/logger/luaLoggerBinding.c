#include "luaLoggerBinding.h"

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
#include "usart.h"
#include "printk.h"
#include "modp_numtoa.h"
#include "loggerTaskEx.h"
#include "loggerSampleData.h"
#include "virtual_channel.h"

extern xSemaphoreHandle g_xLoggerStart;
extern int g_loggingShouldRun;

#define TEMP_BUFFER_LEN 200
#define DEFAULT_CAN_TIMEOUT 	100


char g_tempBuffer[TEMP_BUFFER_LEN];

void registerLuaLoggerBindings(lua_State *L){

	//Read / control inputs and outputs
	lua_registerlight(L,"getGpio",Lua_GetGPIO);
	lua_registerlight(L,"setGpio",Lua_SetGPIO);
	lua_registerlight(L,"getButton",Lua_GetButton);

	lua_registerlight(L,"setPWMDutyCycle",Lua_SetPWMDutyCycle);
	lua_registerlight(L,"setPwmPeriod",Lua_SetPWMPeriod);
	lua_registerlight(L,"setAnalogOut",Lua_SetAnalogOut);

	lua_registerlight(L,"getTimerRpm",Lua_GetRPM);
	lua_registerlight(L,"getTimerPeriodMs",Lua_GetPeriodMs);
	lua_registerlight(L,"getTimerFrequency",Lua_GetFrequency);
	lua_registerlight(L,"getTimerRaw",Lua_GetTimerRaw);
	lua_registerlight(L,"resetTimerCount",Lua_ResetTimerCount);
	lua_registerlight(L,"getTimerCount",Lua_GetTimerCount);
	
	lua_registerlight(L,"getAnalog",Lua_GetAnalog);
	lua_registerlight(L,"getAnalogRaw",Lua_GetAnalogRaw);

	lua_registerlight(L,"getImu",Lua_ReadImu);
	lua_registerlight(L,"getImuRaw",Lua_ReadImuRaw);

	lua_registerlight(L,"getGpsPos", Lua_GetGPSPosition);
	lua_registerlight(L,"getGpsSpeed",Lua_GetGPSSpeed);
	lua_registerlight(L,"getGpsQuality", Lua_GetGPSQuality);
	lua_registerlight(L,"getGpsTime", Lua_GetGPSTime);
	lua_registerlight(L,"getGpsDist", Lua_GetGPSDistance);
	lua_registerlight(L,"getLapCount", Lua_GetLapCount);
	lua_registerlight(L,"getLapTime", Lua_GetLapTime);
	lua_registerlight(L,"getGpsSec", Lua_GetGPSSecondsSinceMidnight);
	lua_registerlight(L,"getAtStartFinish",Lua_GetGPSAtStartFinish);

	lua_registerlight(L,"getTimeDiff", Lua_GetTimeDiff);
	lua_registerlight(L,"getTimeSince", Lua_GetTimeSince);

	lua_registerlight(L, "initCAN", Lua_InitCAN);
	lua_registerlight(L, "txCAN", Lua_SendCANMessage);
	lua_registerlight(L, "rxCAN", Lua_ReceiveCANMessage);
	lua_registerlight(L, "setCANfilter", Lua_SetCANFilter);
	lua_registerlight(L, "setCANmask", Lua_SetCANMask);
	lua_registerlight(L, "readOBD2", Lua_ReadOBD2);

	lua_registerlight(L,"startLogging",Lua_StartLogging);
	lua_registerlight(L,"stopLogging",Lua_StopLogging);

	lua_registerlight(L,"setLed",Lua_SetLED);

	lua_registerlight(L,"readSerial", Lua_ReadSerialLine);
	lua_registerlight(L,"writeSerial", Lua_WriteSerial);

	//Logger configuration editing
	lua_registerlight(L,"flashLoggerConfig", Lua_FlashLoggerConfig);

	lua_registerlight(L,"setAnalogSampleRate", Lua_SetAnalogChannelSampleRate);
	lua_registerlight(L,"getAnalogSampleRate", Lua_GetAnalogChannelSampleRate);

	lua_registerlight(L,"setPwmClockFrequency",Lua_SetPWMClockFrequency);
	lua_registerlight(L,"getPwmClockFrequency",Lua_GetPWMClockFrequency);

	lua_registerlight(L,"setPwmSampleRate", Lua_SetPWMSampleRate);
	lua_registerlight(L,"getPwmSampleRate", Lua_GetPWMSampleRate);

	lua_registerlight(L,"setGpsSampleRate", Lua_SetGPSSampleRate);
	lua_registerlight(L,"getGpsSampleRate", Lua_GetGPSSampleRate);
	
	lua_registerlight(L,"setLapCountSampleRate", Lua_SetLapCountSampleRate);
	lua_registerlight(L,"getLapCountSampleRate", Lua_GetLapCountSampleRate);

	lua_registerlight(L,"setLapTimeSampleRate", Lua_SetLapTimeSampleRate);
	lua_registerlight(L,"getLapTimeSampleRate", Lua_GetLapTimeSampleRate);
	
	lua_registerlight(L,"setGpioSampleRate", Lua_SetGPIOSampleRate);
	lua_registerlight(L,"getGpioSampleRate", Lua_GetGPIOSampleRate);

	lua_registerlight(L,"setTimerSampleRate", Lua_SetTimerSampleRate);
	lua_registerlight(L,"getTimerSampleRate", Lua_GetTimerSampleRate);

	lua_registerlight(L,"setImuSampleRate",Lua_SetImuSampleRate);
	lua_registerlight(L,"getImuSampleRate",Lua_GetImuSampleRate);
	
	lua_registerlight(L,"calibrateImuZero",Lua_CalibrateImuZero);

	lua_registerlight(L,"setGpsStartFinish",Lua_SetGPSStartFinish);
	lua_registerlight(L,"getGpsStartFinish",Lua_GetGPSStartFinish);
	
	lua_registerlight(L,"setBgStream", Lua_SetBackgroundStreaming);
	lua_registerlight(L,"getBgStream", Lua_GetBackgroundStreaming);

	lua_registerlight(L, "addChannel", Lua_AddVirtualChannel);
	lua_registerlight(L, "setChannel", Lua_SetVirtualChannelValue);
}

////////////////////////////////////////////////////
// common functions
////////////////////////////////////////////////////

static LapConfig * getLapConfig(){
	return &(getWorkingLoggerConfig()->LapConfigs);
}

static int setLuaSampleRate(lua_State *L, unsigned short *sampleRate){
	if (lua_gettop(L) >= 1 ){
		*sampleRate = encodeSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

static int getLuaSampleRate(lua_State *L, unsigned short sampleRate){
	lua_pushinteger(L,sampleRate);
	return 1;
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

int Lua_SetImuSampleRate(lua_State *L){
	if (lua_gettop(L) >= 2 ){
		ImuConfig *c = getImuConfigChannel(lua_tointeger(L,1));
		if (NULL != c) c->cfg.sampleRate = encodeSampleRate(lua_tointeger(L,2));
	}	
	return 0;
}

int Lua_GetImuSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		ImuConfig *c = getImuConfigChannel(lua_tointeger(L,1));
		if (NULL !=c ){
			lua_pushnumber(L,c->cfg.sampleRate);
			return 1;	
		}
	}
	return 0;	
}

int Lua_CalibrateImuZero(lua_State *L){
	imu_calibrate_zero();
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

int Lua_SetGPSStartFinish(lua_State *L){
	if (lua_gettop(L) >= 2){
		TrackConfig *c = &(getWorkingLoggerConfig()->TrackConfigs);
		c->track.startFinish.latitude = lua_tonumber(L,1);
		c->track.startFinish.longitude = lua_tonumber(L,2);
		if (lua_gettop(L) >=3) c->radius = lua_tonumber(L,3);
	}
	return 0;
}

int Lua_GetGPSStartFinish(lua_State *L){
	TrackConfig *c = &(getWorkingLoggerConfig()->TrackConfigs);
	lua_pushnumber(L,c->track.startFinish.latitude);
	lua_pushnumber(L,c->track.startFinish.longitude);
	lua_pushnumber(L,c->radius);
	return 3;
}

int Lua_GetGPSAtStartFinish(lua_State *L){
	lua_pushinteger(L,getAtStartFinish());
	return 1;
}

int Lua_GetAtSplit(lua_State *L){
	lua_pushinteger(L,getAtSector());
	return 1;
}

int Lua_SetGPSSampleRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		LoggerConfig *cfg = getWorkingLoggerConfig();
		cfg->GPSConfigs.sampleRate = encodeSampleRate(lua_tointeger(L,1));
	}
	return 0;
}

int Lua_GetGPSSampleRate(lua_State *L){
	lua_pushinteger(L, decodeSampleRate(getWorkingLoggerConfig()->GPSConfigs.sampleRate));
	return 1;		
}

int Lua_SetLapCountSampleRate(lua_State *L){
	return setLuaSampleRate(L, &getLapConfig()->lapCountCfg.sampleRate);
}

int Lua_GetLapCountSampleRate(lua_State *L){
	return getLuaSampleRate(L, getLapConfig()->lapCountCfg.sampleRate);
}

int Lua_SetLapTimeSampleRate(lua_State *L){
	return setLuaSampleRate(L, &getLapConfig()->lapTimeCfg.sampleRate);
}

int Lua_GetLapTimeSampleRate(lua_State *L){
	return getLuaSampleRate(L, getLapConfig()->lapTimeCfg.sampleRate);
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
				analogValue = get_mapped_value((float)adcRaw,&(ac->scalingMap));
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

int Lua_GetGPSTime(lua_State *L){
	lua_pushnumber(L,getUTCTime());
	return 1;
}

int Lua_GetGPSDistance(lua_State *L){
	lua_pushnumber(L, getGpsDistance());
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

int Lua_SetCANFilter(lua_State *L){
	if (lua_gettop(L) >= 3){
		uint8_t id = lua_tointeger(L, 1);
		uint8_t extended = lua_tointeger(L, 2);
		uint32_t filter = lua_tointeger(L, 3);
		int rc = CAN_set_filter(id, extended, filter);
		lua_pushinteger(L, rc);
		return 1;
	}
	return 0;
}

int Lua_SetCANMask(lua_State *L){
	if (lua_gettop(L) >= 3){
		uint8_t id = lua_tointeger(L, 1);
		uint8_t extended = lua_tointeger(L, 2);
		uint32_t mask = lua_tointeger(L, 3);
		int rc = CAN_set_mask(id, extended, mask);
		lua_pushinteger(L, rc);
		return 1;
	}
	return 0;
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
	size_t timeout = DEFAULT_CAN_TIMEOUT;
	if (lua_gettop(L) >= 1) timeout = lua_tointeger(L, 1);

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

int Lua_AddVirtualChannel(lua_State *L){
	if (lua_gettop(L) >= 2){
		const char *name = lua_tostring(L, 1);
		unsigned short sampleRate = encodeSampleRate(lua_tonumber(L, 2));
		int result = create_virtual_channel(name, sampleRate);
		lua_pushinteger(L, result);
		return 1;
	}
	return 0;

}

int Lua_SetVirtualChannelValue(lua_State *L){
	if (lua_gettop(L) >= 2){
		int id = lua_tointeger(L, 1);
		float value = lua_tonumber(L, 2);
		set_virtual_channel_value(id,value);
	}
	return 0;
}

