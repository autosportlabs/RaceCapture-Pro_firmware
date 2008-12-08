#include "accelerometer.h"
#include "usb_comm.h"




#define SPI_SPEED 1000000  /* 1MHz clock*/
#define SPI_DLYBCT 1
#define SPI_DLYBS 20
#define MCK 48054840

#define SPI_TRANSFER (AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK)
 
void accel_initSPI(){
	
	// enable peripheral clock for SPI ( PID Bit 5 )
	AT91C_BASE_PMC->PMC_PCER = ( 1 << AT91C_ID_SPI );
	
	*AT91C_SPI_CR = AT91C_SPI_SPIDIS | AT91C_SPI_SWRST;
	*AT91C_PMC_PCER = (1 << AT91C_ID_SPI);
	*AT91C_PIOA_ASR = SPI_TRANSFER | AT91C_PA11_NPCS0;
	*AT91C_PIOA_BSR = AT91C_PA9_NPCS1;
	
	*AT91C_PIOA_PDR = SPI_TRANSFER | AT91C_PA11_NPCS0 | AT91C_PA9_NPCS1;
	*AT91C_PIOA_PPUER = AT91C_PA11_NPCS0 | AT91C_PA9_NPCS1;
	
	*AT91C_SPI_MR = (AT91C_SPI_MSTR | AT91C_SPI_PS_FIXED
                   | AT91C_SPI_MODFDIS | AT91C_SPI_PCS);
 
     /* It seems necessary to set the clock speed for chip select 0
        even if it's not used. */
	     AT91C_SPI_CSR[0] = (MCK/SPI_SPEED)<<8;
 
     *AT91C_SPI_CR = AT91C_SPI_SPIEN;
}

void spi_init_chip_select(unsigned int chip, unsigned int speed,
                      unsigned int dlybct,
                      unsigned int dlybs, unsigned int phase,
                      unsigned int polarity)
{
   AT91C_SPI_CSR[chip] =
     ((dlybct<<24) | (dlybs<<16) | (((MCK+speed/2)/speed)<<8)
      | (phase?AT91C_SPI_NCPHA:0) | (polarity?AT91C_SPI_CPOL:0)
      | AT91C_SPI_BITS_8 | AT91C_SPI_CSAAT);
}


void accel_spiSetSpeed(unsigned char speed)
{
	unsigned int reg;
	AT91PS_SPI pSPI      = AT91C_BASE_SPI;

	reg = pSPI->SPI_CSR[1];
	reg = ( reg & ~(AT91C_SPI_SCBR) ) | ( (unsigned int)speed << 8 );
	pSPI->SPI_CSR[1] = reg;
}

unsigned char accel_spiSend(unsigned char outgoing)
{
	unsigned char incoming;
	
	
	while(!(*AT91C_SPI_SR & AT91C_SPI_TXEMPTY)); /* wait unti previous transfer is done */
   
	/* Clear any data left in the receiver */
	(void)*AT91C_SPI_RDR;
	(void)*AT91C_SPI_RDR;

 
	while(!(*AT91C_SPI_SR & AT91C_SPI_TDRE));
	*AT91C_SPI_TDR = outgoing;
	
	while(!(*AT91C_SPI_SR & AT91C_SPI_RDRF));
	incoming = *AT91C_SPI_RDR;


	return incoming;
}


void selectChip(unsigned char chip){
	/* Select chip */	*AT91C_SPI_MR = ((*AT91C_SPI_MR & ~AT91C_SPI_PCS)
                    | ((~(1 << chip ) & 0x0f) << 16));	
}

void accel_init(){

	SendString("init\r\n");
	accel_initSPI();
	spi_init_chip_select(1,SPI_SPEED,0, 0,0,1);
	//accel_spiSetSpeed(0xfe);

	SendString("after init spi\r\n");

}

void accel_setup(){
	selectChip(1);
	accel_spiSend(0x04);
	accel_spiSend(0x04);
	*AT91C_SPI_CR = AT91C_SPI_LASTXFER;

	for (unsigned int d = 0; d < 1000000;d++){} //200 ns???? recalcualate this...
	
}

unsigned char accel_readControlRegister(){
	selectChip(1);
	accel_spiSend(0x03);
	unsigned char ctrl = accel_spiSend(0xff);
	*AT91C_SPI_CR = AT91C_SPI_LASTXFER;
	return ctrl;	
}

unsigned int accel_readAxis(unsigned char axis){
	
	selectChip(1);
	accel_spiSend(axis);
	for (unsigned int d = 0; d < 200;d++){} //200 ns???? recalcualate this...
	unsigned char dataMSB = accel_spiSend(0x00);
	unsigned char dataLSB = accel_spiSend(0x00);
	*AT91C_SPI_CR = AT91C_SPI_LASTXFER;
	for (unsigned int d = 0; d < 1000;d++){} //40 us???? recalcualate this...
	
	return (dataMSB << 4) + ((dataLSB >> 4) & 0x0f);
}
