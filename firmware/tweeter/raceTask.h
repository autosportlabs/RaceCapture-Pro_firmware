#ifndef RACETASK_H_
#define RACETASK_H_

#include "lua.h"

void registerRaceTaskLuaBindings();
int Lua_PokeSMS(lua_State *L);
int Lua_InitCellModem(lua_State *L);
int Lua_SendText(lua_State *L);
int Lua_ReceiveText(lua_State *L);
int Lua_DeleteAllTexts(lua_State *L);
int Lua_SetTweetNumber(lua_State *L);
int Lua_SetStartFinishPoint(lua_State *L);
int Lua_GetStartFinishPoint(lua_State *L);
int Lua_SayMessage(lua_State *L);

void startRaceTask(void);
void raceTask(void *params);
void textSenderTask(void *params);
void setStartFinishPoint(float latitude, float longitude, float radius);
float getStartFinishLatitude(void);
float getStartFinishLongitude(void);
float getStartFinishRadius(void);
void setTweetNumber(const char *number);

#endif /*LUATASK_H_*/
