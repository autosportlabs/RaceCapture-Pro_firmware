#ifndef LUALOGGERBINDING_H_
#define LUALOGGERBINDING_H_

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


void registerLuaLoggerBindings();

int Lua_GetBackgroundStreaming(lua_State *L);
int Lua_SetBackgroundStreaming(lua_State *L);

int Lua_GetAnalog(lua_State *L);
int Lua_GetAnalogRaw(lua_State *L);

int Lua_GetRPM(lua_State *L);
int Lua_GetPeriodMs(lua_State *L);
int Lua_GetPeriodUsec(lua_State *L);
int Lua_GetFrequency(lua_State *L);
int Lua_GetTimerRaw(lua_State *L);
int Lua_ResetTimerCount(lua_State *L);
int Lua_GetTimerCount(lua_State *L);

int Lua_GetButton(lua_State *L);
int Lua_GetGPIO(lua_State *L);
int Lua_SetGPIO(lua_State *L);

int Lua_ReadSerialLine(lua_State *L);
int Lua_WriteSerial(lua_State *L);

int Lua_GetGPSLongitude(lua_State *L);
int Lua_GetGPSLatitude(lua_State *L);
int Lua_GetGPSSpeed(lua_State *L);
int Lua_GetGPSQuality(lua_State *L);
int Lua_GetGPSTime(lua_State *L);
int Lua_GetLapTime(lua_State *L);
int Lua_GetLapCount(lua_State *L);
int Lua_GetGPSSecondsSinceMidnight(lua_State *L);
int Lua_GetGPSAtStartFinish(lua_State *L);
int Lua_GetTimeDiff(lua_State *L);
int Lua_GetTimeSince(lua_State *L);

int Lua_ReadAccelerometer(lua_State *L);
int Lua_ReadAccelerometerRaw(lua_State *L);

int Lua_SetPWMDutyCycle(lua_State *L);
int Lua_SetPWMDutyCycleRaw(lua_State *L);

int Lua_SetPWMPeriod(lua_State *L);
int Lua_SetPWMPeriodRaw(lua_State *L);

int Lua_SetAnalogOut(lua_State *L);

int Lua_StartLogging(lua_State *L);
int Lua_StopLogging(lua_State *L);

int Lua_SetLED(lua_State *L);

int Lua_InitCAN(lua_State *L);
int Lua_SendCANMessage(lua_State *L);
int Lua_ReceiveCANMessage(lua_State *L);
int Lua_QueryOBD(lua_State *L);


//logger config data

int Lua_SetAccelSampleRate(lua_State *L);
int Lua_GetAccelSampleRate(lua_State *L);

int Lua_SetAccelZeroValue(lua_State *L);
int Lua_GetAccelZeroValue(lua_State *L);

int Lua_CalibrateAccelZero(lua_State *L);

int Lua_SetTimerSampleRate(lua_State *L);
int Lua_GetTimerSampleRate(lua_State *L);

int Lua_SetGPIOSampleRate(lua_State *L);
int Lua_GetGPIOSampleRate(lua_State *L);

int Lua_SetGPSStartFinish(lua_State *L);
int Lua_GetGPSStartFinish(lua_State *L);
int Lua_GetAtStartFinish(lua_State *L);

int Lua_GetSplit(lua_State *L);
int Lua_SetSplit(lua_State *L);
int Lua_GetAtSplit(lua_State *L);

int Lua_SetGPSPositionSampleRate(lua_State *L);
int Lua_GetGPSPositionSampleRate(lua_State *L);

int Lua_SetGPSSpeedSampleRate(lua_State *L);
int Lua_GetGPSSpeedSampleRate(lua_State *L);

int Lua_SetGPSTimeSampleRate(lua_State *L);
int Lua_GetGPSTimeSampleRate(lua_State *L);

int Lua_SetLapCountSampleRate(lua_State *L);
int Lua_GetLapCountSampleRate(lua_State *L);

int Lua_SetLapTimeSampleRate(lua_State *L);
int Lua_GetLapTimeSampleRate(lua_State *L);

int Lua_SetPWMClockFrequency(lua_State *L);
int Lua_GetPWMClockFrequency(lua_State *L);

int Lua_SetPWMSampleRate(lua_State *L);
int Lua_GetPWMSampleRate(lua_State *L);

int Lua_SetAnalogChannelSampleRate(lua_State *L);
int Lua_GetAnalogChannelSampleRate(lua_State *L);

int Lua_FlashLoggerConfig(lua_State *L);


#endif /*LUALOGGERBINDING_H_*/
