#ifndef IGNITION_CONFIG_H_
#define IGNITION_CONFIG_H_

#include "ignition_config_defaults.h"

#define RPM_BIN_COUNT 			12
#define LOAD_BIN_COUNT 			12
#define USER_DEFINED_OUTPUTS 	7
#define IGNITION_CONFIGS		2
#define MIN_RPM					0
#define MAX_RPM					15000
#define MIN_SCALED_RPM			0
#define MAX_SCALED_RPM			150
#define CRANKING_RPM			500
#define MAX_CYLINDERS			8
#define MAX_COIL_DRIVERS		4

enum OUTPUT_TYPE{
	OUTPUT_TYPE_RPM = 0,
	OUTPUT_TYPE_MAP
};

enum OUTPUT_MODE{
	MODE_NORMAL = 0,
	MODE_INVERTED
}; 

enum OUTPUT_STATE{
	OUTPUT_INACTIVE = 0,
	OUTPUT_ACTIVE
};
enum LOAD_TYPE{
	LOAD_TYPE_MAP = 0,
	LOAD_TYPE_TPS
};

struct ignition_map{
	int rpm_bins[RPM_BIN_COUNT];
	int load_bins[LOAD_BIN_COUNT];
	int ignition_advance[LOAD_BIN_COUNT][RPM_BIN_COUNT]; 	
};

struct output_config{
	enum OUTPUT_TYPE 		output_type;
	enum OUTPUT_MODE 		output_mode;
	int 					trigger_threshold;
};

struct ignition_config{
	struct ignition_map 		map;
	struct output_config 		user_defined_outputs[USER_DEFINED_OUTPUTS];
};

struct load_calibration{
	int 					calibration[1024];
	enum LOAD_TYPE			load_axis_type;
	
};

struct cylinder_config{
		unsigned int tdcFireDegrees;
		unsigned int coilDriver;
};


struct engine_config{
	unsigned int 				cylinderCount;
	struct cylinder_config		cylinderConfigs[MAX_CYLINDERS];
	int							missingToothBTDCDegrees;
	int							cranking_degrees;							
};




#endif /*IGNITION_CONFIG_H_*/
