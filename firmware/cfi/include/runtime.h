#ifndef RUNTIME_H_
#define RUNTIME_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "ignition_config.h"


#define USER_OUTPUT_RPM_HYSTERESIS  		100
#define USER_OUTPUT_LOAD_HYSTERESIS 		2
#define DEFAULT_DWELL_USEC		 			6000
#define CRANK_TEETH							36
#define MAX_DEGREES							360
#define DEGREES_PER_TOOTH					(MAX_DEGREES / CRANK_TEETH)
#define CRANKING_REVOLUTION_PERIOD_RAW		3003427 //480 RPM
#define MAX_DWELL_DEGREES					170  
//8 possible coil drivers from PAO12 to PAO19
#define COIL_DRIVER_0_PORT					0x1000
#define COIL_DRIVER_ALL_PORTS				0xF000

struct logical_coil_driver{
	int cylinderTDC;
	int coilOnDegrees;
	int coilOnTooth;
	int coilOnInterToothDegrees;
	int coilFireDegrees;
	int coilFireTooth;
	int coilFireInterToothDegrees;
	unsigned int physicalCoilDriverPorts;
};

void coilPackTimerInit ( void );

void triggerWheelTimerInit ( void );

//On revolution
void onRevolutionTask(void *);

//maps the cylinder configuration to the logical coil driver structure
//to make runtime processing of the coil drivers more efficient.
void initLogicalCoilDrivers(struct logical_coil_driver *);


//returns an existing logical coil driver for the specified cylinderTDC
struct logical_coil_driver * findMatchingLogicalCoilDriver(int , struct logical_coil_driver *);


//calculate the current crank period microseconds based on the raw crank period
unsigned int calculateCrankPeriodUSec();

//calculate the current RPM based on the last 
//crank revolution period
void calculateRPM();

//calculate the ignition advance
void calculateAdvance();

//calculate the dwell period in crankshaft degrees
//based on current RPM and desired dwell duration
void calculateDwellDegrees();

//Updates the user outputs
void updateUserOutputs();

//perform a standard linear interpolation
int linearInterpolate(int x, int x0, int y0, int x1, int y1);

//Initialize the runtime data
void initRuntimeData();

//Update the cylinder runtime data based on the current ignition advance
void updateLogicalCoilDriverRuntimes(struct logical_coil_driver *);



#endif /*RUNTIME_H_*/
