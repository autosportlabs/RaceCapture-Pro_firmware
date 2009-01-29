#include "loggerConfig.h"
#include "string.h"
#include "memory.h"


//Ignition configuration
struct __attribute__((section(".text\n\t#"))) LoggerConfig g_savedLoggerConfig __attribute__ ((packed, aligned (128))) = DEFAULT_LOGGER_CONFIG;
struct LoggerConfig g_workingLoggerConfig;



void updateActiveLoggerConfig(){
	memcpy(&g_workingLoggerConfig,&g_savedLoggerConfig,sizeof(struct LoggerConfig));
}

int flashLoggerConfig(){
	return flash_write((void *)&g_savedLoggerConfig,(void *)&g_workingLoggerConfig);	
}


