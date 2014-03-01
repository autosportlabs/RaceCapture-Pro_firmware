#include "OBD2_task.h"
#include "taskUtil.h"
#include "loggerConfig.h"
#include "stddef.h"
#include "CAN.h"
#include "OBD2.h"
#include "printk.h"

#define OBD2_PID_TIMEOUT_MS 200

void OBD2Task(void *pvParameters){
	LoggerConfig *config = getWorkingLoggerConfig();
	OBD2Config *oc = &config->OBD2Config;
	while(1){
		for (size_t i = 0; i < oc->enabledPids; i++){
			PidConfig *pidCfg = &oc->pids[i];
			int value;
			int pid = pidCfg->pid;
			if (! OBD2_request_PID(pid, &value, OBD2_PID_TIMEOUT_MS)){
				pr_warning("read OBD2 PID fail: ");
				pr_warning_int(pid);
				pr_warning("\r\n");
			}
			OBD2_set_current_PID_value(i, value);
			delayTicks(oc->obd2SampleRate);
		}
	}
}
