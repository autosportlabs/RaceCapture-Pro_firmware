#ifndef LUATASK_H_
#define LUATASK_H_
#include <stddef.h>

void lockLua(void);
void unlockLua(void);

void startLuaTask(int priority);
void luaTask(void *params);

void *myAlloc (void *ud, void *ptr, size_t osize,size_t nsize);

void * getLua(void);

unsigned int getLastPointer();

void setAllocDebug(int enableDebug);
int getAllocDebug();

int getShouldReloadScript(void);
void setShouldReloadScript(int reload);

void set_ontick_freq(size_t freq);
size_t get_ontick_freq();

#endif /*LUATASK_H_*/
