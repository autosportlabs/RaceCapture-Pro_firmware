/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LUALOGGERBINDING_H_
#define LUALOGGERBINDING_H_

#include "cpp_guard.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

CPP_GUARD_BEGIN

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

int Lua_InitSerial(lua_State *L);
int Lua_ReadSerialChar(lua_State *L);
int Lua_ReadSerialLine(lua_State *L);
int Lua_WriteSerialChar(lua_State *L);
int Lua_WriteSerialLine(lua_State *L);

int Lua_GetGPSSatellites(lua_State *L);
int Lua_GetGPSPosition(lua_State *L);
int Lua_GetGPSSpeed(lua_State *L);
int Lua_GetGPSQuality(lua_State *L);
int Lua_GetLapTime(lua_State *L);
int Lua_GetGPSDistance(lua_State *L);
int Lua_GetLapCount(lua_State *L);
int Lua_GetGpsSecondsSinceFirstFix(lua_State *L);
int Lua_GetGPSAtStartFinish(lua_State *L);
int Lua_GetTickCount(lua_State *L);
int Lua_GetTicksPerSecond(lua_State *L);

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

CPP_GUARD_END

#endif /*LUALOGGERBINDING_H_*/
