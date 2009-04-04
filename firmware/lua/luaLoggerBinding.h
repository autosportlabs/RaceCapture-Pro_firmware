#ifndef LUALOGGERBINDING_H_
#define LUALOGGERBINDING_H_

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int Lua_Print(lua_State *L);
int Lua_Println(lua_State *L);


void RegisterLuaRaceCaptureFunctions(lua_State *L);

int Lua_GetAnalog(lua_State *L);
int Lua_GetAnalogRaw(lua_State *L);

int calculateXTimerValue(lua_State *L, unsigned int (*scaler)(unsigned int,unsigned int));
int Lua_GetRPM(lua_State *L);
int Lua_GetPeriodMs(lua_State *L);
int Lua_GetPeriodUsec(lua_State *L);
int Lua_GetFrequency(lua_State *L);
int Lua_GetTimerRaw(lua_State *L);

int Lua_GetButton(lua_State *L);
int Lua_GetInput(lua_State *L);
int Lua_SetOutput(lua_State *L);

int Lua_GetGPS(lua_State *L);
int Lua_GetAccelerometer(lua_State *L);
int Lua_GetAccelerometerRaw(lua_State *L);

int Lua_SetPWMDutyCycle(lua_State *L);
int Lua_SetPWMDutyCycleRaw(lua_State *L);

int Lua_SetPWMFrequency(lua_State *L);
int Lua_SetPWMPeriodRaw(lua_State *L);

int Lua_SetAnalogOut(lua_State *L);

int Lua_StartLogging(lua_State *L);
int Lua_StopLogging(lua_State *L);

int Lua_SetLED(lua_State *L);

int Lua_UpdateScriptPage(lua_State *L);
int Lua_GetScriptPage(lua_State *L);
int Lua_PrintScriptPage(lua_State *L);
int Lua_ReloadScript(lua_State *L);

//logger config data
int Lua_SetPWMLabel(lua_State *L);
int Lua_SetPWMSampleRate(lua_State *L);
int Lua_SetPWMOutputConfig(lua_State *L);
int Lua_SetPWMLoggingConfig(lua_State *L);
int Lua_SetPWMStartupDutyCycle(lua_State *L);
int Lua_SetPWMStartupPeriod(lua_State *L);
int Lua_SetPWMVoltageScaling(lua_State *L);

int Lua_SetAnalogChannelLabel(lua_State *L);
int Lua_SetAnalogChannelSampleRate(lua_State *L);
int Lua_SetAnalogChannelScaling(lua_State *L);

int Lua_FlashLoggerConfig(lua_State *L);

#endif /*LUALOGGERBINDING_H_*/
