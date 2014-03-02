#include "OBD2_task.h"
#include "taskUtil.h"
#include "loggerConfig.h"
#include "stddef.h"
#include "CAN.h"
#include "OBD2.h"
#include "printk.h"

#define OBD2_PID_TIMEOUT_MS 100

void OBD2Task(void *pvParameters){
	pr_info("Start OBD2 task\r\n");
	LoggerConfig *config = getWorkingLoggerConfig();
	OBD2Config *oc = &config->OBD2Configs;
	while(1){
		for (size_t i = 0; i < oc->enabledPids; i++){
			PidConfig *pidCfg = &oc->pids[i];
			int value;
			unsigned char pid = pidCfg->pid;
			if (OBD2_request_PID(pid, &value, OBD2_PID_TIMEOUT_MS)){
				OBD2_set_current_PID_value(i, value);
				if (DEBUG_LEVEL){
					pr_debug("read OBD2 PID ");
					pr_debug_int(pid);
					pr_debug("=")
					pr_debug_int(value);
					pr_debug("\r\n");
				}
			}
			else{
				pr_warning("read OBD2 PID fail: ");
				pr_warning_int(pid);
				pr_warning("\r\n");
			}
			delayTicks(oc->obd2SampleRate);
		}
		delayTicks(oc->obd2SampleRate);
	}
}
