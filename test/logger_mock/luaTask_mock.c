#include "luaTask.h"

void lockLua(void)
{

}

void unlockLua(void)
{

}

void startLuaTask(int priority)
{

}

void luaTask(void *params)
{

}

void *myAlloc (void *ud, void *ptr, size_t osize,size_t nsize)
{
    return NULL;
}

void * getLua(void)
{
    return NULL;
}

unsigned int getLastPointer()
{
    return 0;
}

void setAllocDebug(int enableDebug)
{

}

int getAllocDebug()
{
    return 0;
}

int getShouldReloadScript(void)
{
    return 0;
}

void setShouldReloadScript(int reload)
{

}

void set_ontick_freq(size_t freq)
{

}
size_t get_ontick_freq()
{
    return 1;
}

void terminate_lua() {}
void initialize_lua() {}
void run_lua_interactive_cmd(Serial *serial, const char* cmd) {}
