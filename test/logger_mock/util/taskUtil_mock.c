#include "taskUtil.h"

size_t getCurrentTicks(){
	return 0;  //TODO return current system ticks, whatever that means
}

int isTimeoutMs(unsigned int startTicks, unsigned int timeoutMs){
	return 1; //TODO make this work correctly on the test platform when we start needing it
}

void delayMs(unsigned int delay){

}

void delayTicks(size_t ticks){

}

size_t msToTicks(size_t ms){
	return ms;  //TODO make this work correctly on the test platform when we start needing it
}

size_t ticksToMs(size_t ticks){
	return ticks;  //TODO make this work correctly on the test platform when we start needing it
}
