#ifndef LUALOGGERBINDING_H_
#define LUALOGGERBINDING_H_

#include "lua.h"


void RegisterLuaRaceCaptureFunctions(lua_State *L);

int Lua_GetAnalog(lua_State *L);
int Lua_GetAnalogRaw(lua_State *L);

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

int Lua_WriteScriptPage(lua_State *L);

int Lua_Print(lua_State *L);
int Lua_Println(lua_State *L);

#endif /*LUALOGGERBINDING_H_*/
