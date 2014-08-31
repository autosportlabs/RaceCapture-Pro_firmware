#ifndef LUALOGGERBINDING_H_
#define LUALOGGERBINDING_H_

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


void registerLuaLoggerBindings(lua_State *L);

int Lua_GetBackgroundStreaming(lua_State *L);
int Lua_SetBackgroundStreaming(lua_State *L);

int Lua_GetAnalog(lua_State *L);

int Lua_GetRPM(lua_State *L);
int Lua_GetPeriodMs(lua_State *L);
int Lua_GetFrequency(lua_State *L);
int Lua_GetTimerRaw(lua_State *L);
int Lua_ResetTimerCount(lua_State *L);
int Lua_GetTimerCount(lua_State *L);

int Lua_GetButton(lua_State *L);
int Lua_GetGPIO(lua_State *L);
int Lua_SetGPIO(lua_State *L);

int Lua_ReadSerialLine(lua_State *L);
int Lua_WriteSerial(lua_State *L);

int Lua_GetGPSPosition(lua_State *L);
int Lua_GetGPSSpeed(lua_State *L);
int Lua_GetGPSQuality(lua_State *L);
int Lua_GetGPSTime(lua_State *L);
int Lua_GetLapTime(lua_State *L);
int Lua_GetGPSDistance(lua_State *L);
int Lua_GetLapCount(lua_State *L);
int Lua_GetGPSSecondsSinceMidnight(lua_State *L);
int Lua_GetGPSAtStartFinish(lua_State *L);
int Lua_GetTimeDiff(lua_State *L);
int Lua_GetTimeSince(lua_State *L);

int Lua_ReadImu(lua_State *L);
int Lua_ReadImuRaw(lua_State *L);

int Lua_SetPWMDutyCycle(lua_State *L);
int Lua_SetPWMPeriod(lua_State *L);
int Lua_SetPWMClockFrequency(lua_State *L);
int Lua_GetPWMClockFrequency(lua_State *L);

int Lua_SetAnalogOut(lua_State *L);

int Lua_StartLogging(lua_State *L);
int Lua_StopLogging(lua_State *L);
int Lua_IsLogging(lua_State *L);

int Lua_SetLED(lua_State *L);

int Lua_InitCAN(lua_State *L);
int Lua_SendCANMessage(lua_State *L);
int Lua_ReceiveCANMessage(lua_State *L);
int Lua_SetCANFilter(lua_State *L);
int Lua_ReadOBD2(lua_State *L);


//logger config data
int Lua_CalibrateImuZero(lua_State *L);

int Lua_GetAtStartFinish(lua_State *L);

int Lua_FlashLoggerConfig(lua_State *L);

int Lua_AddVirtualChannel(lua_State *L);
int Lua_SetVirtualChannelValue(lua_State *L);


#endif /*LUALOGGERBINDING_H_*/
