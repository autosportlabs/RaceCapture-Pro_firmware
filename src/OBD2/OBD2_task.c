#include "OBD2_task.h"
#include "taskUtil.h"
#include "loggerConfig.h"
#include "stddef.h"
#include "CAN.h"
#include "OBD2.h"
#include "printk.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "taskUtil.h"
#include "capabilities.h"

#define OBD2_TASK_STACK 	100

#define OBD2_FEATURE_DISABLED_DELAY_MS 2000
void startOBD2Task(int priority){
	xTaskCreate( OBD2Task, ( signed portCHAR * )"OBD2Task", OBD2_TASK_STACK, NULL, 	priority, NULL );
}

void OBD2Task(void *pvParameters){
	pr_info("Start OBD2 task\r\n");
	size_t max_obd2_samplerate = msToTicks((TICK_RATE_HZ / MAX_OBD2_SAMPLE_RATE));
	LoggerConfig *config = getWorkingLoggerConfig();
	OBD2Config *oc = &config->OBD2Configs;
	while(1){
		while(oc->enabled && oc->enabledPids > 0){
			for (size_t i = 0; i < oc->enabledPids; i++){
				PidConfig *pidCfg = &oc->pids[i];
				int value;
				unsigned char pid = pidCfg->pid;
				if (OBD2_request_PID(pid, &value, OBD2_PID_DEFAULT_TIMEOUT_MS)){
					OBD2_set_current_PID_value(i, value);
					if (TRACE_LEVEL){
						pr_trace("read OBD2 PID ");
						pr_trace_int(pid);
						pr_trace("=")
						pr_trace_int(value);
						pr_trace("\r\n");
					}
				}
				else{
					pr_warning("read OBD2 PID fail: ");
					pr_warning_int(pid);
					pr_warning("\r\n");
				}
				delayTicks(max_obd2_samplerate);
			}
		}
		delayMs(OBD2_FEATURE_DISABLED_DELAY_MS);
	}
}
