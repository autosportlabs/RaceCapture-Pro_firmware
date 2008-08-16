#include "runtime.h"
#include "led_debug.h"
#include "USB-CDC.h"
#include "Board.h"


#include <string.h>

char debugMsg[100];

xSemaphoreHandle 			xOnRevolutionHandle;

//****************************************************
//Runtime data
volatile unsigned int		g_wheelSyncAttempts;
volatile unsigned int 		g_toothCountAtLastSyncAttempt;
volatile unsigned int		g_currentCrankRevolutionPeriodRaw;
volatile unsigned int 		g_lastCrankRevolutionPeriodRaw;
volatile unsigned int		g_lastCrankRevolutionPeriodUSec;
volatile unsigned int 		g_lastInterToothPeriodRaw;
volatile unsigned int 		g_currentToothPeriodOverflowCount;
volatile unsigned int 		g_wheelSynchronized;
volatile unsigned int 		g_currentRPM;
volatile unsigned int		g_currentDwellUSec;
volatile unsigned int 		g_currentLoad;
volatile unsigned int 		g_currentLoadBin;
volatile unsigned int 		g_currentRPMBin;
volatile int 				g_currentAdvance;
volatile unsigned int 		g_currentDwellDegrees;
volatile unsigned int 		g_engineIsRunning;
struct logical_coil_driver *g_active_logical_coil_drivers;
struct logical_coil_driver 	g_logical_coil_drivers[2][MAX_COIL_DRIVERS];

unsigned int				g_logicalCoilDriverCount;
unsigned int				g_currentTooth;
unsigned int				g_coilDriversToFire;
unsigned int				g_coilDriversToCharge;
//****************************************************

int							g_selectedIgnitionConfigIndex;
enum OUTPUT_STATE			g_output_state[USER_DEFINED_OUTPUTS];

/* Engine configuration */
struct engine_config 		g_active_engine_config;
struct ignition_config		g_active_ignition_config[IGNITION_CONFIGS];
struct ignition_config 		*g_selected_ignition_config;

//Flash storage for configuration data

//Ignition configuration
const struct ignition_config g_saved_ignition_config[IGNITION_CONFIGS] __attribute__((section(".text\n\t#"))) = {DEFAULT_IGNITION_CONFIG,DEFAULT_IGNITION_CONFIG};
//Engine configuration	
const struct engine_config g_saved_engine_config __attribute__((section(".text\n\t#"))) = DEFAULT_ENGINE_CONFIG;


#define TIMER0_INTERRUPT_LEVEL		5
#define TIMER1_INTERRUPT_LEVEL		7

/*-----------------*/
/* Clock Selection */
/*-----------------*/
#define TC_CLKS                  0x7
#define TC_CLKS_MCK2             0x0
#define TC_CLKS_MCK8             0x1
#define TC_CLKS_MCK32            0x2
#define TC_CLKS_MCK128           0x3
#define TC_CLKS_MCK1024          0x4

	extern void ( coilPack_irq_handler )( void );

//*----------------------------------------------------------------------------
//* Function Name       : timer_init
//* Object              : Init timer counter
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------
void triggerWheelTimerInit( void )
{
	extern void ( triggerWheel_irq_handler )( void );
		

  /* Set PIO pins for Timer Counter 0 */
   AT91F_TC0_CfgPIO();
   /* Enable TC0's clock in the PMC controller */
   AT91F_TC0_CfgPMC();

	AT91F_TC_Open ( 
	AT91C_BASE_TC0,
	
	TC_CLKS_MCK2 | 
	AT91C_TC_ETRGEDG_FALLING |	
	AT91C_TC_ABETRG |
	AT91C_TC_LDRA_RISING|
	AT91C_TC_LDRB_FALLING
	
	,AT91C_ID_TC0
	);

	AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC0, TIMER0_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, triggerWheel_irq_handler);

	AT91C_BASE_TC0->TC_IER = AT91C_TC_COVFS | AT91C_TC_LDRBS;  //  IRQ enable RB loading
	
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC0);
}


void coilPackTimerInit( void ){

	AT91PS_TCB pTCB = AT91C_BASE_TCB;		// create a pointer to TC Global Register structure
	pTCB->TCB_BCR = 0;						// SYNC trigger not used
	

	AT91F_TC_Open ( 
	AT91C_BASE_TC2,
	
	TC_CLKS_MCK8    | 
	AT91C_TC_WAVE   |
	AT91C_TC_CPCSTOP
	
	,AT91C_ID_TC2
	);

	AT91F_AIC_ConfigureIt (
			AT91C_BASE_AIC, 
			AT91C_ID_TC2, 
			TIMER1_INTERRUPT_LEVEL,
			AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE, 
			coilPack_irq_handler
			);
	
	AT91C_BASE_TC2->TC_IER = AT91C_TC_CPCS;  	// IRQ enable RC compare
	
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC2);
}




void initRuntimeData( void ){

	memset(g_output_state,0,sizeof(g_output_state));
	memset(g_logical_coil_drivers, 0, sizeof(g_logical_coil_drivers));
	
	vSemaphoreCreateBinary( xOnRevolutionHandle );
	
	//default to the first ignition configuration
	//TODO: read the input state and actually select the right one
	g_selectedIgnitionConfigIndex = 0;
	
	//copy from flash to RAM
	memcpy(g_active_ignition_config, g_saved_ignition_config,sizeof(g_active_ignition_config));
	memcpy(&g_active_engine_config, &g_saved_engine_config, sizeof(g_active_engine_config));
	
	//set the pointer to the active config
	g_selected_ignition_config = &(g_active_ignition_config[g_selectedIgnitionConfigIndex]);
	
	g_toothCountAtLastSyncAttempt = 0;
	g_wheelSyncAttempts = 0;	
	g_currentTooth  = 0;
	g_wheelSynchronized = 0;
	g_currentCrankRevolutionPeriodRaw = 0;
	g_lastCrankRevolutionPeriodUSec = 0;	
	g_lastCrankRevolutionPeriodRaw = 0;
	g_lastInterToothPeriodRaw = 0;
	g_currentToothPeriodOverflowCount = 0;
	g_currentRPM = 0;
	g_currentLoad = 0;
	g_currentLoadBin = 0;
	g_currentRPMBin = 0;
	g_engineIsRunning = 0;
	g_currentDwellUSec = DEFAULT_DWELL_USEC;
	g_currentAdvance = g_active_engine_config.cranking_degrees;
	g_lastCrankRevolutionPeriodRaw = CRANKING_REVOLUTION_PERIOD_RAW;
	g_coilDriversToCharge = 0;
	g_coilDriversToFire = 0;
	initLogicalCoilDrivers(g_logical_coil_drivers[0]);
	initLogicalCoilDrivers(g_logical_coil_drivers[1]);
	g_active_logical_coil_drivers = g_logical_coil_drivers[0];
	calculateDwellDegrees();
	updateLogicalCoilDriverRuntimes(g_active_logical_coil_drivers);
}


void onRevolutionTask(void *pvParameters){
	
	initRuntimeData();

	portENTER_CRITICAL();
	triggerWheelTimerInit();
	coilPackTimerInit();
	enableIRQ();
	portEXIT_CRITICAL();
	
	while(1){
		if ( xSemaphoreTake(xOnRevolutionHandle, 1) == pdTRUE){
				unsigned int crankPeriodUSec = calculateCrankPeriodUSec();
				calculateRPM(crankPeriodUSec);
				calculateDwellDegrees(crankPeriodUSec);
				g_lastCrankRevolutionPeriodUSec = crankPeriodUSec;
				g_currentLoad = 20;
				calculateAdvance();
				if (g_active_logical_coil_drivers == g_logical_coil_drivers[0]){
					updateLogicalCoilDriverRuntimes(g_logical_coil_drivers[1]);				
				}
				else{
					updateLogicalCoilDriverRuntimes(g_logical_coil_drivers[0]);
				}
//				updateUserOutputs();
		}
	}
}

//map the cylinder configuration to the logical coil driver structure
//to make runtime processing of the coil drivers more efficient.
void initLogicalCoilDrivers(struct logical_coil_driver *logical_coil_drivers){
	
	g_logicalCoilDriverCount = 0;
	for (int i = 0; i < g_active_engine_config.cylinderCount;i++){
		struct cylinder_config *cylinderCfg = g_active_engine_config.cylinderConfigs + i;
		int coilDriver = cylinderCfg->coilDriver;
		
		//account for missing tooth offset
		int cylinderTDC = cylinderCfg->tdcFireDegrees + g_active_engine_config.missingToothBTDCDegrees;
		if (cylinderTDC >= MAX_DEGREES) cylinderTDC = cylinderTDC - MAX_DEGREES;
		
		struct logical_coil_driver *logicalCoilDriver = findMatchingLogicalCoilDriver(cylinderTDC,logical_coil_drivers);
		if (NULL == logicalCoilDriver){
			logicalCoilDriver = logical_coil_drivers + g_logicalCoilDriverCount;
			logicalCoilDriver->cylinderTDC = cylinderTDC;
			//set the physical port
			logicalCoilDriver->physicalCoilDriverPorts = ( COIL_DRIVER_0_PORT << coilDriver );
			g_logicalCoilDriverCount++;
		}
		else{
			//Add the pysical port to the existing one
			logicalCoilDriver->physicalCoilDriverPorts = logicalCoilDriver->physicalCoilDriverPorts |  ( COIL_DRIVER_0_PORT << coilDriver);
		}
	}
}

//returns an existing logical coil driver for the specified cylinderTDC
struct logical_coil_driver * findMatchingLogicalCoilDriver(int cylinderTDC,struct logical_coil_driver *logical_coil_drivers){	
	for (int i = 0; i < g_logicalCoilDriverCount; i++){
		struct logical_coil_driver * logicalCoilDriver = logical_coil_drivers + i;
		if ( logicalCoilDriver->cylinderTDC == cylinderTDC ) return logicalCoilDriver;
	}
	return NULL;
}

void updateLogicalCoilDriverRuntimes(struct logical_coil_driver *logical_coil_drivers){

	int count = g_logicalCoilDriverCount;
	
	struct logical_coil_driver *logicalCoilPack = logical_coil_drivers;
	while (count > 0){
		
		int coilFireDegrees = logicalCoilPack->cylinderTDC - g_currentAdvance;
		if ( coilFireDegrees < 0 ) coilFireDegrees = coilFireDegrees + MAX_DEGREES;
		int coilFireTooth = coilFireDegrees / DEGREES_PER_TOOTH;
		int coilFireInterToothDegrees = coilFireDegrees % DEGREES_PER_TOOTH;		

		int coilOnDegrees = coilFireDegrees - g_currentDwellDegrees;
		if ( coilOnDegrees < 0 ) coilOnDegrees = coilOnDegrees + MAX_DEGREES;
		int coilOnTooth = coilOnDegrees / DEGREES_PER_TOOTH;
		int coilOnInterToothDegrees = coilOnDegrees % DEGREES_PER_TOOTH;

		logicalCoilPack->coilFireDegrees = coilFireDegrees;
		logicalCoilPack->coilFireTooth = coilFireTooth;
		logicalCoilPack->coilFireInterToothDegrees = coilFireInterToothDegrees;

		logicalCoilPack->coilOnDegrees = coilOnDegrees;
		logicalCoilPack->coilOnTooth = coilOnTooth;
		logicalCoilPack->coilOnInterToothDegrees = coilOnInterToothDegrees;
		
		logicalCoilPack++;
		count--;	
	}
}

unsigned int calculateCrankPeriodUSec(){
	//if master clock is 48054840 Hz and MCK/2
	//USec = (timer value * 100000) / 2402742		
	return (g_lastCrankRevolutionPeriodRaw * 100) / 2403;
}

void calculateRPM(unsigned int crankRevolutionPeriodUSec){
	if (crankRevolutionPeriodUSec > 0){
		g_currentRPM = ((1000000000 / crankRevolutionPeriodUSec) * 6) / 100;
	}
}

void calculateDwellDegrees(unsigned int crankRevolutionPeriodUSec){
	
	unsigned int dwellDegrees = (MAX_DEGREES * g_currentDwellUSec ) / crankRevolutionPeriodUSec; 
	if (dwellDegrees > MAX_DWELL_DEGREES) dwellDegrees = MAX_DWELL_DEGREES;
	g_currentDwellDegrees = dwellDegrees;  
}

void updateUserOutputs(){

	struct output_config *output;
	
	int runtimeValue;
	int hysteresis;
	enum OUTPUT_STATE *state;
	
	state = g_output_state;
	
	for (int i = 0; i < USER_DEFINED_OUTPUTS; i++){ 	
		output = &(g_selected_ignition_config->user_defined_outputs[i]);

		switch (output->output_type){			
			case OUTPUT_TYPE_RPM:
				runtimeValue = g_currentRPM;
				hysteresis =  USER_OUTPUT_RPM_HYSTERESIS;
				break;
			case OUTPUT_TYPE_MAP:
				runtimeValue = g_currentLoad;
				hysteresis = USER_OUTPUT_LOAD_HYSTERESIS;
				break;
			default:
				runtimeValue = 0;
				hysteresis = 0;
				//TODO: signal error condition here
				break;
		}
		
		if (OUTPUT_ACTIVE == *state){
			if (runtimeValue < output->trigger_threshold - hysteresis) *state = OUTPUT_INACTIVE;	
		}
		else{
			if (runtimeValue > output->trigger_threshold) *state = OUTPUT_ACTIVE;	
		}
		
		int outputPortHigh = (*state == OUTPUT_ACTIVE);
		if (output->output_mode == MODE_INVERTED) outputPortHigh = !outputPortHigh;
		//TODO: enable output port here
		
		//move to the next saved state
		state++;
	}
}

void calculateAdvance(){

	//Local variable declarations

	//local copies of current RPM and current Load values	
	unsigned int currentRPM, currentLoad;
		
	//all of the different advance values
	//selected for linear interpolation purposes
	int advance,           advance_next_rpm;
	int advance_next_load, advance_next_rpm_load;
	
	//the interpolated advance values across the two axis
	int advanceRpmInterpolated, advanceRpmInterpolatedNextLoad;

	//the different bins used for interpolation purposes	
	unsigned int rpmBin, nextRpmBin;
	unsigned int loadBin, nextLoadBin;

	//the RPM bin values, extracted and saved
	unsigned int rpmBinValue, nextRpmBinValue;
	
	//a pointer to the ignition map
	struct ignition_map *map;
	
	//pointers to rpm / load bins
	int *rpmBins;
	int *loadBins;
	int (*advanceTable)[RPM_BIN_COUNT];

	//////////////////////////////
	//end of variable declarations
	//////////////////////////////
	
	//get local copies of the global variables
	currentRPM = g_currentRPM;
	currentLoad = g_currentLoad;
	
	//get our currently selected ignition map
	map = &(g_selected_ignition_config->map);


	///////////////////
	//Find the RPM Bin
	///////////////////
	//start at the end of the rpm bins
	rpmBins = (map->rpm_bins) + RPM_BIN_COUNT - 1;
	rpmBin = RPM_BIN_COUNT - 1;

	//determine our base RPM bin	
	while (currentRPM < *rpmBins && rpmBin > 0){
		rpmBins--;
		rpmBin--;	
	}
	//determine the next RPM bin
	//check edge conditions
	//RPM below the lowest load bin?
	if (rpmBin == 0 && currentRPM < *rpmBins){
		nextRpmBin = 0;	
	}
	else{ 
		nextRpmBin = rpmBin;
		//RPM above the highest RPM Bin?
		if (rpmBin < RPM_BIN_COUNT - 1) nextRpmBin++;
	}

	////////////////////////
	//Find the Load Bin
	////////////////////////
	//start at the end of the load bins	
	loadBins = (map->load_bins) + LOAD_BIN_COUNT - 1;
	loadBin = LOAD_BIN_COUNT - 1;

	//determine our base Load bin
	while (currentLoad < *loadBins && loadBin > 0){
		loadBins--;
		loadBin--;	
	}
	//determine the next Load bin
	//check edge conditions
	//load below the lowest load bin?
	if (loadBin == 0 && currentLoad < *loadBins){
		nextLoadBin = 0;	
	}
	else{
		nextLoadBin = loadBin;
		//load above the highest load bin?
		if (loadBin < LOAD_BIN_COUNT - 1) nextLoadBin++;			
	}

	//get our advance values to feed into the linear
	//interpolation formulas
	advanceTable = map->ignition_advance;
	
	advance 				= advanceTable[loadBin][rpmBin];
	advance_next_rpm 		= advanceTable[loadBin][nextRpmBin];
	advance_next_load 		= advanceTable[nextLoadBin][rpmBin];
	advance_next_rpm_load 	= advanceTable[nextLoadBin][nextRpmBin];


	
	rpmBins = map->rpm_bins;
	loadBins = map->load_bins;

	//pre-fetch these because they are used twice		
	rpmBinValue = rpmBins[rpmBin];
	nextRpmBinValue = rpmBins[nextRpmBin];

	//perform linear interpolation
	//between rpmBin and nextRpmBin
	//for base Load bin
	//y = advance dimension; x = rpm dimension
	
	
	advanceRpmInterpolated = 
		linearInterpolate(currentRPM, 
							rpmBinValue,
							advance, 
							nextRpmBinValue,
							advance_next_rpm);
							
							
	//linear interpolate between rpmBin and nextRpmBin
	//for next Load bin
	//y = advance dimension; x = rpm dimension		
	advanceRpmInterpolatedNextLoad = 
		linearInterpolate(currentRPM,
							rpmBinValue,
							advance_next_load,
							nextRpmBinValue,
							advance_next_rpm_load);
							
	
	//final linear interpolation step
	//y = advance dimension; x = load dimension
	g_currentAdvance = 
		linearInterpolate(currentLoad,
							loadBins[loadBin],
							advanceRpmInterpolated,
							loadBins[nextLoadBin],
							advanceRpmInterpolatedNextLoad);

	//save the global values
	g_currentLoadBin = loadBin;
	g_currentRPMBin = rpmBin;
}


//linear interpolation routine
//            (y2 - y1)
//  y = y1 +  --------- * (x - x1)
//            (x2 - x1)
int linearInterpolate(int x, int x1, int y1, int x2, int y2){
	return y1 + (((((y2 - y1) << 16)  / (x2 - x1)) * (x - x1)) >> 16);
}



