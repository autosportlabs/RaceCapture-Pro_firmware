#include "accelerometer.h"
#include "usb_comm.h"


#define SPI_CSR_NUM      1          

#define ACCEL_COUNTS_PER_G 				1024
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
 
void accel_initSPI(){
	
	// enable peripheral clock for SPI ( PID Bit 5 )
	AT91C_BASE_PMC->PMC_PCER = ( 1 << AT91C_ID_SPI );
	
	//disable reset
	//*AT91C_SPI_CR = AT91C_SPI_SPIDIS | AT91C_SPI_SWRST;
	
	*AT91C_PMC_PCER = (1 << AT91C_ID_SPI);
	
	*AT91C_PIOA_ASR = SPI_TRANSFER;
	*AT91C_PIOA_BSR = AT91C_PA9_NPCS1;
	
	*AT91C_PIOA_ODR = AT91C_PA9_NPCS1;
	*AT91C_PIOA_CODR = AT91C_PA9_NPCS1;
	
	*AT91C_PIOA_PDR = SPI_TRANSFER | AT91C_PA9_NPCS1;
	//*AT91C_PIOA_PPUER = AT91C_PA11_NPCS0 | AT91C_PA9_NPCS1;
	
	*AT91C_SPI_MR = AT91C_SPI_PS_VARIABLE | AT91C_SPI_MSTR | AT91C_SPI_MODFDIS;
//	*AT91C_SPI_MR = (AT91C_SPI_MSTR | AT91C_SPI_PS_FIXED
//                   | AT91C_SPI_MODFDIS | AT91C_SPI_PCS);
 
     /* It seems necessary to set the clock speed for chip select 0
        even if it's not used. */
	     AT91C_SPI_CSR[0] = (MCK/CS0_SPI_SPEED)<<8;

	AT91C_SPI_CSR[SPI_CSR_NUM] = AT91C_SPI_CPOL | AT91C_SPI_BITS_8 | AT91C_SPI_CSAAT;

	accel_spiSetSpeed(48);
	//accel_spiSetSpeed(0xFE);

	*AT91C_SPI_CR = AT91C_SPI_SPIEN;
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
}

void accel_setup(){
	//selectChip(SPI_CSR_NUM);
	accel_spiSend(0x04, 0);
	accel_spiSend(0x04, 1);
	for (unsigned int d = 0; d < 1000000;d++){} //200 ns???? recalcualate this...
	
}

unsigned char accel_readControlRegister(){
	accel_spiSend(0x03, 0);
	unsigned char ctrl = accel_spiSend(0xff, 1);
	return ctrl;	
}

float accel_rawToG(unsigned int accelRaw, unsigned int zeroValue){
	
	accelRaw = accelRaw - zeroValue;
	
	float gforce = (float)accelRaw / ACCEL_COUNTS_PER_G;
	return gforce;		
}

float accel_readAxisG(unsigned char axis, unsigned int zeroValue){

	unsigned int accelRaw = accel_readAxis(axis);
	return accel_rawToG(accelRaw,zeroValue);
}

unsigned int accel_readAxis(unsigned char axis){
	accel_spiSend(axis, 0);
	for (unsigned int d = 0; d < 200;d++){} //200 ns???? recalcualate this...
	unsigned char dataMSB = accel_spiSend(0x00, 0);
	unsigned char dataLSB = accel_spiSend(0x00, 1);
	for (unsigned int d = 0; d < 1000;d++){} //40 us???? recalcualate this...
	
	return (dataMSB << 4) + ((dataLSB >> 4) & 0x0f);
}
