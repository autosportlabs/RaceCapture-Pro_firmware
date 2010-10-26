#include "usb_comm.h"
#include "task.h"
#include "string.h"
#include "USB-CDC.h"
#include "board.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "luaTask.h"
#include "lua.h"
#include "memory.h"
#include "luaScript.h"
#include "base64.h"

#define BUFFER_SIZE MEMORY_PAGE_SIZE * 2

extern unsigned int _CONFIG_HEAP_SIZE;
extern unsigned portCHAR  _heap_address[];


extern unsigned int __heap_start__;
extern unsigned int __heap_end__;

char lineBuffer[BUFFER_SIZE];

char * readLine(){
	int bufIndex = 0;
    char c;
	while(bufIndex < BUFFER_SIZE){
    	vUSBReceiveByte(&c);
    	if (c == '\r') break;
    	SendBytes(&c,1);
		lineBuffer[bufIndex++] = c;
	}
	SendCrlf();
	lineBuffer[bufIndex]='\0';
	return lineBuffer;
}

static void handleLuaCmd(char *line){

	lockLua();
	lua_State *L = getLua();

	int result = luaL_loadstring(L,line);
	if (0 != result){
		SendString("result=\"error:(");
		SendString(lua_tostring(L,-1));
		lua_pop(L,1);
		SendString("\");");
	}else{
		lua_pushvalue(L,-1);
		lua_call(L,0,0);
		lua_pop(L,1);
		SendString("result=\"ok\";");
	}
	lua_gc(L,LUA_GCCOLLECT,0);
	SendString("lua_top=");
	SendInt(lua_gettop(L));
	SendString(";lua_gc_count=");
	SendInt(lua_gc(L,LUA_GCCOUNT,0));
	SendString(";");
	SendCrlf();
	unlockLua();
}

static void handleScriptWrite(char *data){

	char * delim = strchr(data,',');
	int param = 0;

	int page = 0;
	char *scriptPage = NULL;

	int keepParsing = 1;

	while (delim != NULL && keepParsing){
		*delim = '\0';
		switch (param){
			case 0:
				{
					page = modp_atoi(data);
					break;
				}
			case 1:
				{
					scriptPage = data;
					keepParsing = 0;
					break;
				}
			}
			param++;
			data = delim + 1;
			delim = strchr(data,',');
		}
	if (scriptPage != NULL){
		vPortEnterCritical();
		char *decodedScript = base64decode(scriptPage, strlen(scriptPage));
		int result = flashScriptPage(page,decodedScript);
		vPortFree(decodedScript);
	   	vPortExitCritical();
		if (result == 0){
			SendString("ok");
		}
		else{
			SendString("error");
		}
		SendCrlf();
	}
}

void handleScriptRead(char *data){

	int page = modp_atoi(data);

	if (page >=0 && page < SCRIPT_PAGES){

		const char * script = getScript();
		//forward to the requested page
		script += (MEMORY_PAGE_SIZE * page);
		//check for truncated page
		size_t scriptLen = strlen(script);
		if (scriptLen > MEMORY_PAGE_SIZE) scriptLen = MEMORY_PAGE_SIZE;
		char *encoded = base64encode(script,scriptLen);
		if (NULL != encoded){
			SendString(encoded);
			SendCrlf();
			vPortFree(encoded);
		}
	}
}

void onUSBCommTask(void *pvParameters){
	
	while (!vUSBIsInitialized()){
		vTaskDelay(1);
	}
	
    while (1){
    	char * line = readLine();
    	
    	switch (line[0]){

    	case 'e':
    	{
    		handleLuaCmd(line+2);
    		break;
    	}

    	case 'w':
    	{
    		handleScriptWrite(line+2);
    		break;
    	}

    	case 'r':
    	{
    		handleScriptRead(line+2);
    		break;
    	}
    	default:
    	{
    		SendString("unknown command");
    		SendCrlf();
    		break;
    	}
    }
    }
}


void SendInt(int n){
	char buf[12];
	modp_itoa10(n,buf);
	SendString(buf);
}

void SendFloat(float f,int precision){
	char buf[20];
	modp_ftoa(f,buf,precision);
	SendString(buf);	
}

void SendDouble(double f, int precision){
	char buf[30];
	modp_dtoa(f,buf,precision);
	SendString(buf);
}

void SendUint(unsigned int n){
	char buf[20];
	modp_uitoa10(n,buf);
	SendString(buf);	
}

void SendString(const char *s){
	while ( *s ) vUSBSendByte(*s++ );
}

void SendBytes(char *data, unsigned int length){
	while (length > 0){
    	vUSBSendByte(*data);
    	data++;
    	length--;
	}	
}


void SendCrlf(){
	SendString("\r\n");
}

/*
		if (theData == 'm'){
			
			SendString("heap start: ");
			SendUint((unsigned int)(&__heap_start__));
			SendString("; heap end: ");
			SendUint((unsigned int)(&__heap_end__));
			SendCrlf();
			
			SendString("heap size: ");
			SendUint((unsigned int)(&_CONFIG_HEAP_SIZE));
			SendString("; heap address: ");
			SendUint((unsigned int) _heap_address);
			SendCrlf();

		}
		if (theData == 'l'){
			SendString("lua status: ");
			testLua();
		}
		
		if (theData == 'w'){
			SendString("flashing...");
			int result = flashLoggerConfig();
			SendString("done: ");
			SendInt(result);
			SendCrlf();
		}
		if (theData == 'p'){
			SendString("address: ");
			SendUint((unsigned int)&g_savedLoggerConfig);
			SendString("sizeof config: ");
			SendUint((unsigned int)sizeof(struct LoggerConfig));
			SendCrlf();
		}
		if (theData == 'z'){
			duty--;
			SendString("Duty: ");
			SendInt(duty);
			SendCrlf();
			PWM_SetDutyCycle(0,duty);
			PWM_SetDutyCycle(1,duty);
			PWM_SetDutyCycle(2,duty);
			PWM_SetDutyCycle(3,duty);	
		}
		if (theData == 'X'){
			SendString("Reset Duty");
			SendCrlf();
			duty=MIN_DUTY_CYCLE;
			PWM_SetDutyCycle(0,duty);
			PWM_SetDutyCycle(1,duty);
			PWM_SetDutyCycle(2,duty);
			PWM_SetDutyCycle(3,duty);	
		}
		if (theData == 'x'){
			duty++;
			SendString("Duty: ");
			SendInt(duty);
			SendCrlf();
			PWM_SetDutyCycle(0,duty);
			PWM_SetDutyCycle(1,duty);
			PWM_SetDutyCycle(2,duty);
			PWM_SetDutyCycle(3,duty);
		}
		if (theData == '1'){
			period--;
			SendString("Period: ");
			SendInt(period);
			SendCrlf();
			PWM_SetPeriod(0,period);
			PWM_SetPeriod(1,period);
			PWM_SetPeriod(2,period);
			PWM_SetPeriod(3,period);
		}
		if (theData == '2'){
			period++;
			SendString("Period: ");
			SendInt(period);
			SendCrlf();
			PWM_SetPeriod(0,period);
			PWM_SetPeriod(1,period);
			PWM_SetPeriod(2,period);
			PWM_SetPeriod(3,period);
		}
		if (theData == 'c'){
			for (int i=1; i < 1000; i++){
				vTaskDelay(4 / portTICK_RATE_MS);
				PWM_SetDutyCycle(0,i);
				PWM_SetDutyCycle(1,i);
				PWM_SetDutyCycle(2,i);
				PWM_SetDutyCycle(3,i);	
			}	
			for (int i=999; i >=1; i--){
				vTaskDelay(4 / portTICK_RATE_MS);
				PWM_SetDutyCycle(0,i);
				PWM_SetDutyCycle(1,i);
				PWM_SetDutyCycle(2,i);
				PWM_SetDutyCycle(3,i);	
			}
		}
		if (theData == 'y'){
			accel_init();
			accel_setup();
		}
		if (theData == 'k'){
		 	int x = accel_readAxis(0);
		 	int y = accel_readAxis(1);
		 	int z = accel_readAxis(2);
		 	SendString("RawG X,Y,Z: ");
		 	SendInt(x);
		 	SendString(",");
		 	SendInt(y);
		 	SendString(",");
		 	SendInt(z);
		 	SendCrlf();
		 	struct LoggerConfig *loggerConfig;
		 	loggerConfig = getWorkingLoggerConfig();
		 	SendString("G X,Y,Z: ");
		 	SendFloat(accel_rawToG(x,loggerConfig->AccelConfig[0].zeroValue),2);
		 	SendString(",");
		 	SendFloat(accel_rawToG(y,loggerConfig->AccelConfig[1].zeroValue),2);
		 	SendString(",");
		 	SendFloat(accel_rawToG(z,loggerConfig->AccelConfig[2].zeroValue),2);
		 	SendCrlf();
		}
		if (theData == 'b'){
		 	int x = accel_readControlRegister();
		 	SendString("accel control: ");
		 	SendInt(x);
		 	SendCrlf();			
		}
		if (theData == 'd'){
			unsigned int size = getRx1QueueWaiting();
			SendString("usart rx1 queue waiting ");
			SendInt(size);
			SendCrlf();
		}
		if (theData == '?'){
			SendString("sizeof float: ");
			SendInt(sizeof(float));
			SendCrlf();
			SendString("sizeof double: ");
			SendInt(sizeof(double));
			SendCrlf();	
			{
				char *test = "-123.456";
				float testf = modp_atof(test);
				SendFloat(testf,3);
				SendCrlf();
			}
			{
				char *test = "12345";
				float testf = modp_atof(test);
				SendFloat(testf,3);
				SendCrlf();
			}
			{
				char *test = "12345.0";
				float testf = modp_atof(test);
				SendFloat(testf,3);
				SendCrlf();
			}
			{
				char *test = "1111.2222";
				float testf = modp_atof(test);
				SendFloat(testf,3);
				SendCrlf();
			}
			{
				char *test = "-12346.789123";
				double testd = modp_atod(test);
				SendDouble(testd,6);
				SendCrlf();
			}
			SendString("sizeof loggerConfig; ");
			SendInt(sizeof(g_savedLoggerConfig));
			SendCrlf();
		}
		if (theData == 'g'){
			SendDouble(getLongitude(),6);
			SendString(",");
			SendDouble(getLatitude(),6);
			SendString(" Vel: ");
			SendFloat(getGPSVelocity(),2);
			SendString(" Qual: ");
			SendInt(getGPSQuality());
			SendString(" Sats: ");
			SendInt(getSatellitesUsedForPosition());
			SendString(" Time: ");
			SendFloat(getUTCTime(),5);
			SendCrlf();
			//SendString(getGPSDataLine());
			//SendCrlf();	
		}
#if ( configUSE_TRACE_FACILITY == 1 )		
		if (theData == 't'){
			SendString("status\tpri\tstack\tnum\tname\r\n");
			char text[300]; //fixme
			vTaskList(text);
			SendString(text);
			SendCrlf();
		}
#endif
		if (theData == 'q'){
			SendString("timer: ");
			SendInt(getTimer0Period());
			SendString(",");
			SendInt(getTimer1Period());
			SendString(",");
			SendInt(getTimer2Period());
			SendString("  ");
			SendInt(AT91C_BASE_TC0->TC_CV);
			SendString(",");
			SendInt(AT91C_BASE_TC1->TC_CV);
			SendString(",");
			SendInt(AT91C_BASE_TC2->TC_CV);
			SendString("  ");
			SendInt(AT91C_BASE_TC0->TC_RB);
			SendString(",");
			SendInt(AT91C_BASE_TC1->TC_RB);
			SendString(",");
			SendInt(AT91C_BASE_TC2->TC_RB);
			SendCrlf();
			struct LoggerConfig *c = getWorkingLoggerConfig();
			calculateTimerScaling(c,0);
			calculateTimerScaling(c,1);
			calculateTimerScaling(c,2);
			SendString("Scaling: ");
			SendInt(c->TimerConfigs[0].calculatedScaling);
			SendString(",");
			SendInt(c->TimerConfigs[1].calculatedScaling);
			SendString(",");
			SendInt(c->TimerConfigs[2].calculatedScaling);
			SendCrlf();
			SendString("Hz: ");
			SendInt(calculateFrequencyHz(getTimer0Period(),c->TimerConfigs[0].calculatedScaling));
			SendString(",");
			SendInt(calculateFrequencyHz(getTimer1Period(),c->TimerConfigs[1].calculatedScaling));
			SendString(",");
			SendInt(calculateFrequencyHz(getTimer2Period(),c->TimerConfigs[2].calculatedScaling));
			SendCrlf();
			SendString("Period Ms: ");
			SendInt(calculatePeriodMs(getTimer0Period(),c->TimerConfigs[0].calculatedScaling));
			SendString(",");
			SendInt(calculatePeriodMs(getTimer1Period(),c->TimerConfigs[1].calculatedScaling));
			SendString(",");
			SendInt(calculatePeriodMs(getTimer2Period(),c->TimerConfigs[2].calculatedScaling));
			SendCrlf();
			SendString("Period Usec: ");
			SendInt(calculatePeriodUsec(getTimer0Period(),c->TimerConfigs[0].calculatedScaling));
			SendString(",");
			SendInt(calculatePeriodUsec(getTimer1Period(),c->TimerConfigs[1].calculatedScaling));
			SendString(",");
			SendInt(calculatePeriodUsec(getTimer2Period(),c->TimerConfigs[2].calculatedScaling));
			SendCrlf();
			SendString("RPM: ");
			SendInt(calculateRPM(getTimer0Period(),c->TimerConfigs[0].calculatedScaling));
			SendString(",");
			SendInt(calculateRPM(getTimer1Period(),c->TimerConfigs[1].calculatedScaling));
			SendString(",");
			SendInt(calculateRPM(getTimer2Period(),c->TimerConfigs[2].calculatedScaling));
			SendCrlf();
		}
		if (theData == 'a'){
			unsigned int a0,a1,a2,a3,a4,a5,a6,a7;
			ReadAllADC(&a0,&a1,&a2,&a3,&a4,&a5,&a6,&a7);
			SendString("ADC: ");
			SendInt(a0);
			SendString(",");
			SendInt(a1);
			SendString(",");
			SendInt(a2);
			SendString(",");
			SendInt(a3);
			SendString(",");
			SendInt(a4);
			SendString(",");
			SendInt(a5);
			SendString(",");
			SendInt(a6);
			SendString(",");
			SendInt(a7);
			SendCrlf();
		}
    }

*/
