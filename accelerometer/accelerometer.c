#include "accelerometer.h"
#include "loggerConfig.h"
#include <string.h>

#define SPI_CSR_NUM      2

//1G point for Kionix KXR94-2353
#define ACCEL_COUNTS_PER_G 				819
#define YAW_COUNTS_PER_DEGREE_PER_SEC	6.826666

#define ACCEL_MAX_RANGE 				ACCEL_COUNTS_PER_G * 4

/* PCS_0 for NPCS0, PCS_1 for NPCS1 ... */
#define PCS_0 ((0<<0)|(1<<1)|(1<<2)|(1<<3))
#define PCS_1 ((1<<0)|(0<<1)|(1<<2)|(1<<3))
#define PCS_2 ((1<<0)|(1<<1)|(0<<2)|(1<<3))
#define PCS_3 ((1<<0)|(1<<1)|(1<<2)|(0<<3))
/* TODO: ## */
#if (SPI_CSR_NUM == 0)
#define SPI_MR_PCS       PCS_0
#elif (SPI_CSR_NUM == 1)
#define SPI_MR_PCS       PCS_1
#elif (SPI_CSR_NUM == 2)
#define SPI_MR_PCS       PCS_2
#elif (SPI_CSR_NUM == 3)
#define SPI_MR_PCS       PCS_3
#else
#error "SPI_CSR_NUM invalid"
// not realy - when using an external address decoder...
// but this code takes over the complete SPI-interace anyway
#endif

/* in variable periph. select PSDEC=1 is used
   so the already defined values for SPC_MR_PCS can be
   reused */
#define SPI_TDR_PCS      SPI_MR_PCS


#define CS0_SPI_SPEED 1000000  /* 1MHz clock*/
#define SPI_DLYBCT 1
#define SPI_DLYBS 20
#define MCK 48054840

#define SPI_TRANSFER (AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK)

unsigned int g_averagedAccelValues[CONFIG_ACCEL_CHANNELS];

static unsigned int g_accelBuffer[CONFIG_ACCEL_CHANNELS][ACCELEROMETER_BUFFER_SIZE];
static int g_accelBufferPointers[CONFIG_ACCEL_CHANNELS];
static unsigned int g_accelBufferPointer[CONFIG_ACCEL_CHANNELS];

#define NCPS_PDR_BIT     AT91C_PA10_NPCS2
#define NCPS_ASR_BIT     0
#define NPCS_BSR_BIT     AT91C_PA10_NPCS2

void accel_initSPI(){

	AT91PS_SPI pSPI      = AT91C_BASE_SPI;
	AT91PS_PIO pPIOA     = AT91C_BASE_PIOA;
	AT91PS_PMC pPMC      = AT91C_BASE_PMC;
	// not used: AT91PS_PDC pPDC_SPI  = AT91C_BASE_PDC_SPI;

	// disable PIO from controlling MOSI, MISO, SCK (=hand over to SPI)
	// keep CS untouched - used as GPIO pin during init
	pPIOA->PIO_PPUDR = AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK;

	pPIOA->PIO_PDR = AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK; //  | NCPS_PDR_BIT;
	// set pin-functions in PIO Controller
	pPIOA->PIO_ASR = AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK; /// not here: | NCPS_ASR_BIT;
	/// not here: pPIOA->PIO_BSR = NPCS_BSR_BIT;


	// enable peripheral clock for SPI ( PID Bit 5 )
	pPMC->PMC_PCER = ( 1 << AT91C_ID_SPI ); // n.b. IDs are just bit-numbers

	// SPI mode: master, variable periph. sel., FDIV=0, fault detection disabled
	// Chip-Select-Decoder Mode (write state of CS-Lines in TDR)
	//pSPI->SPI_MR = AT91C_SPI_MSTR | AT91C_SPI_MODFDIS | AT91C_SPI_PCSDEC ;
	pSPI->SPI_MR = AT91C_SPI_PS_VARIABLE | AT91C_SPI_MSTR | AT91C_SPI_MODFDIS;

	// set chip-select-register
	// 8 bits per transfer, CPOL=1, ClockPhase=0, DLYBCT = 0
	// TODO: Why has CPOL to be active here and non-active on LPC2000?
	//       Take closer look on timing diagrams in datasheets.
	// not working pSPI->SPI_CSR[SPI_CSR_NUM] = AT91C_SPI_CPOL | AT91C_SPI_BITS_8 | AT91C_SPI_NCPHA;
	// not working pSPI->SPI_CSR[SPI_CSR_NUM] = AT91C_SPI_BITS_8 | AT91C_SPI_NCPHA;
	pSPI->SPI_CSR[SPI_CSR_NUM] = AT91C_SPI_CPOL | AT91C_SPI_BITS_8 | AT91C_SPI_CSAAT;
	// not working pSPI->SPI_CSR[SPI_CSR_NUM] = AT91C_SPI_BITS_8;

	// slow during init
	accel_spiSetSpeed(0xFE);

	// enable
	pSPI->SPI_CR = AT91C_SPI_SPIEN;

	/* enable automatic chip-select */
	// reset PIO-registers of CS-pin to default
	pPIOA->PIO_ODR  = NCPS_PDR_BIT; // input
	pPIOA->PIO_CODR = NCPS_PDR_BIT; // clear
	// disable PIO from controlling the CS pin (=hand over to SPI)
	pPIOA->PIO_PDR = NCPS_PDR_BIT;
	// set pin-functions in PIO Controller (function NCPS for CS-pin)
	pPIOA->PIO_ASR = NCPS_ASR_BIT;
	pPIOA->PIO_BSR = NPCS_BSR_BIT;

	//spi speed is 48054840 / value in MHz
	//normal speed
	//accel_spiSetSpeed(10);
	accel_spiSetSpeed(48);

}

void accel_spiSetSpeed(unsigned char speed)
{
	unsigned int reg;
	AT91PS_SPI pSPI      = AT91C_BASE_SPI;

	reg = pSPI->SPI_CSR[SPI_CSR_NUM];
	reg = ( reg & ~(AT91C_SPI_SCBR) ) | ( (unsigned int)speed << 8 );
	pSPI->SPI_CSR[SPI_CSR_NUM] = reg;
}

unsigned char accel_spiSend(unsigned char outgoing, int last)
{
	unsigned char incoming;
	
	while(!(*AT91C_SPI_SR & AT91C_SPI_TDRE));
	//*AT91C_SPI_TDR = outgoing;
	*AT91C_SPI_TDR = ( (unsigned short)(outgoing) | ((unsigned int)(SPI_TDR_PCS)<<16) | (last ? AT91C_SPI_LASTXFER : 0) );
	
	while(!(*AT91C_SPI_SR & AT91C_SPI_RDRF));
	incoming = (unsigned char)*AT91C_SPI_RDR;
	return incoming;
}

void accel_init(){
	accel_initSPI();
	accel_setup();
	initAccelBuffer();
}

void initAccelBuffer(){
	for (int channel = 0; channel < CONFIG_ACCEL_CHANNELS; channel++){
		for (int i = 0; i < ACCELEROMETER_BUFFER_SIZE; i++){
			g_accelBuffer[channel][i] = readAccelerometerDevice(channel);
		}
		g_accelBufferPointers[channel] = 0;
	}
}

void accel_setup(){
	memset(g_averagedAccelValues,0,sizeof(g_averagedAccelValues));
	accel_spiSend(0x04, 0);
	accel_spiSend(0x04, 1);
	for (unsigned int d = 0; d < 1000000;d++){} //200 ns???? recalcualate this...
}

unsigned char accel_readControlRegister(){
	accel_spiSend(0x03, 0);
	unsigned char ctrl = accel_spiSend(0xff, 1);
	return ctrl;	
}

float convertYawRawToDegreesPerSec(int yawRaw, unsigned int zeroValue){
	yawRaw = yawRaw - zeroValue;
	float dps = (float)yawRaw / YAW_COUNTS_PER_DEGREE_PER_SEC;
	return dps;
}

float convertAccelRawToG(int accelRaw, unsigned int zeroValue){
	
	accelRaw = accelRaw - zeroValue;
	float gforce = (float)accelRaw / ACCEL_COUNTS_PER_G;
	return gforce;	
}

unsigned int calculateAccelAverage(unsigned char channel){
	unsigned int total = 0;
	for (int i = 0; i < ACCELEROMETER_BUFFER_SIZE;i++){
		total+=g_accelBuffer[channel][i];
	}
	return total / ACCELEROMETER_BUFFER_SIZE;
}


unsigned int readAccelChannel(unsigned char channel){
	//read the accel channel, add to buffer,
	//move pointer and calculate average
	unsigned int value = readAccelerometerDevice(channel);
	int currentIndex = g_accelBufferPointer[channel];
	g_accelBuffer[channel][currentIndex] = value;
	currentIndex++;
	if (currentIndex >= ACCELEROMETER_BUFFER_SIZE) currentIndex = 0;
	g_accelBufferPointer[channel]=currentIndex;
	unsigned int averageValue = calculateAccelAverage(channel);
	g_averagedAccelValues[channel] = averageValue;
	return averageValue;
}



unsigned int readAccelerometerDevice(unsigned char channel){
	
	//aux input (i.e. Yaw input) is mapped to 0x07 on the
	//kionix KXR94
	unsigned char readChannel = channel;
	if (readChannel == 3) readChannel = 7;
	accel_spiSend(readChannel, 0);
	for (unsigned int d = 0; d < 200;d++){} //200 ns???? recalcualate this...
	unsigned char dataMSB = accel_spiSend(0x00, 0);
	unsigned char dataLSB = accel_spiSend(0x00, 1);
	for (unsigned int d = 0; d < 1000;d++){} //40 us???? recalcualate this...
	unsigned int value = (dataMSB << 4) + ((dataLSB >> 4) & 0x0f);
	return value;
}

unsigned int getLastAccelRead(unsigned char channel){
	return g_averagedAccelValues[channel];
}
