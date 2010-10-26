#ifndef LUALOGGERBINDING_H_
#define LUALOGGERBINDING_H_

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

// utility functions
int Lua_Print(lua_State *L);
int Lua_Println(lua_State *L);
int Lua_GetStackSize(lua_State *L);
int Lua_Base64encode(lua_State *L);
int Lua_Base64decode(lua_State *L);



void RegisterLuaRaceCaptureFunctions(lua_State *L);

int Lua_GetAnalog(lua_State *L);
int Lua_GetAnalogRaw(lua_State *L);

int calculateXTimerValue(lua_State *L, unsigned int (*scaler)(unsigned int,unsigned int));
int Lua_GetRPM(lua_State *L);
int Lua_GetPeriodMs(lua_State *L);
int Lua_GetPeriodUsec(lua_State *L);
int Lua_GetFrequency(lua_State *L);
int Lua_GetTimerRaw(lua_State *L);
int Lua_GetTimerCount(lua_State *L);

int Lua_GetButton(lua_State *L);
int Lua_GetInput(lua_State *L);
int Lua_SetOutput(lua_State *L);

int Lua_ReadSerialLine(lua_State *L);
int Lua_WriteSerial(lua_State *L);

int Lua_GetGPSLongitude(lua_State *L);
int Lua_GetGPSLatitude(lua_State *L);
int Lua_GetGPSVelocity(lua_State *L);
int Lua_GetGPSQuality(lua_State *L);
int Lua_GetGPSTime(lua_State *L);
int Lua_GetGPSSecondsSinceMidnight(lua_State *L);
int Lua_GetTimeDiff(lua_State *L);
int Lua_GetTimeSince(lua_State *L);



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
int Lua_ReloadScript(lua_State *L);

//CellModem Bindings
int Lua_InitCellModem(lua_State *L);
int Lua_SendText(lua_State *L);
int Lua_ReceiveText(lua_State *L);
int Lua_DeleteAllTexts(lua_State *L);

//logger config data


void setLabelGeneric(char *dest, const char *source);

int Lua_SetAccelInstalled(lua_State *L);
int Lua_GetAccelInstalled(lua_State *L);

int Lua_SetAccelLabel(lua_State *L);
int Lua_GetAccelLabel(lua_State *L);

int Lua_SetAccelSampleRate(lua_State *L);
int Lua_GetAccelSampleRate(lua_State *L);

int Lua_SetAccelIdleSampleRate(lua_State *L);
int Lua_GetAccelIdleSampleRate(lua_State *L);

int Lua_SetAccelConfig(lua_State *L);
int Lua_GetAccelConfig(lua_State *L);

int Lua_SetAccelChannel(lua_State *L);
int Lua_GetAccelChannel(lua_State *L);

int Lua_SetAccelZeroValue(lua_State *L);
int Lua_GetAccelZeroValue(lua_State *L);

int Lua_SetTimerLabel(lua_State *L);
int Lua_GetTimerLabel(lua_State *L);

int Lua_SetTimerSampleRate(lua_State *L);
int Lua_GetTimerSampleRate(lua_State *L);

int Lua_SetTimerConfig(lua_State *L);
int Lua_GetTimerConfig(lua_State *L);

int Lua_SetTimerPulsePerRevolution(lua_State *L);
int Lua_GetTimerPulsePerRevolution(lua_State *L);

int Lua_SetTimerDivider(lua_State *L);
int Lua_GetTimerDivider(lua_State *L);

int Lua_CalculateTimerScaling(lua_State *L);
int Lua_GetTimerScaling(lua_State *L);


int Lua_SetGPIOLabel(lua_State *L);
int Lua_GetGPIOLabel(lua_State *L);

int Lua_SetGPIOSampleRate(lua_State *L);
int Lua_GetGPIOSampleRate(lua_State *L);

int Lua_SetGPIOConfig(lua_State *L);
int Lua_GetGPIOConfig(lua_State *L);

int Lua_SetGPSInstalled(lua_State *L);
int Lua_GetGPSInstalled(lua_State *L);

int Lua_SetGPSQualityLabel(lua_State *L);
int Lua_GetGPSQualityLabel(lua_State *L);

int Lua_SetGPSSatsLabel(lua_State *L);
int Lua_GetGPSSatsLabel(lua_State *L);

int Lua_SetGPSLatitudeLabel(lua_State *L);
int Lua_GetGPSLatitudeLabel(lua_State *L);

int Lua_SetGPSLongitudeLabel(lua_State *L);
int Lua_GetGPSLongitudeLabel(lua_State *L);

int Lua_SetGPSTimeLabel(lua_State *L);
int Lua_GetGPSTimeLabel(lua_State *L);

int Lua_SetGPSVelocityLabel(lua_State *L);
int Lua_GetGPSVelocityLabel(lua_State *L);

int Lua_SetGPSPositionSampleRate(lua_State *L);
int Lua_GetGPSPositionSampleRate(lua_State *L);

int Lua_SetGPSVelocitySampleRate(lua_State *L);
int Lua_GetGPSVelocitySampleRate(lua_State *L);

int Lua_SetGPSTimeSampleRate(lua_State *L);
int Lua_GetGPSTimeSampleRate(lua_State *L);

int Lua_SetPWMClockFrequency(lua_State *L);
int Lua_GetPWMClockFrequency(lua_State *L);

int Lua_SetPWMLabel(lua_State *L);
int Lua_GetPWMLabel(lua_State *L);

int Lua_SetPWMSampleRate(lua_State *L);
int Lua_GetPWMSampleRate(lua_State *L);

int Lua_SetPWMOutputConfig(lua_State *L);
int Lua_GetPWMOutputConfig(lua_State *L);

int Lua_SetPWMLoggingConfig(lua_State *L);
int Lua_GetPWMLoggingConfig(lua_State *L);

int Lua_SetPWMStartupDutyCycle(lua_State *L);
int Lua_GetPWMStartupDutyCycle(lua_State *L);

int Lua_SetPWMStartupPeriod(lua_State *L);
int Lua_GetPWMStartupPeriod(lua_State *L);

int Lua_SetPWMVoltageScaling(lua_State *L);
int Lua_GetPWMVoltageScaling(lua_State *L);

int Lua_SetAnalogChannelLabel(lua_State *L);
int Lua_GetAnalogChannelLabel(lua_State *L);

int Lua_SetAnalogChannelSampleRate(lua_State *L);
int Lua_GetAnalogChannelSampleRate(lua_State *L);

int Lua_SetAnalogChannelScaling(lua_State *L);
int Lua_GetAnalogChannelScaling(lua_State *L);

int Lua_FlashLoggerConfig(lua_State *L);

#endif /*LUALOGGERBINDING_H_*/
