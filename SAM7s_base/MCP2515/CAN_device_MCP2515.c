
#include "CAN_device.h"
#include "CAN.h"
#include "board.h"
#include "printk.h"
#include "spi.h"
#include "taskUtil.h"

//MCP2515 Registers
#define MCP2515_REG_RXF0SIDH 	0x00
#define MCP2515_REG_RXF0SIDL 	0x01
#define MCP2515_REG_RXF0EID8 	0x02
#define MCP2515_REG_RXF0EID0 	0x03
#define MCP2515_REG_RXF1SIDH 	0x04
#define MCP2515_REG_RXF1SIDL 	0x05
#define MCP2515_REG_RXF1EID8 	0x06
#define MCP2515_REG_RXF1EID0 	0x07
#define MCP2515_REG_RXF2SIDH 	0x08
#define MCP2515_REG_RXF2SIDL 	0x09
#define MCP2515_REG_RXF2EID8 	0x0A
#define MCP2515_REG_RXF2EID0 	0x0B
#define MCP2515_REG_BFPCTRL 	0x0C
#define MCP2515_REG_TXRTSCTRL 	0x0D
#define MCP2515_REG_CANSTAT 	0x0E
#define MCP2515_REG_CANCTRL 	0x0F
#define MCP2515_REG_RXF3SIDH 	0x10
#define MCP2515_REG_RXF3SIDL 	0x11
#define MCP2515_REG_RXF3EID8 	0x12
#define MCP2515_REG_RXF3EID0 	0x13
#define MCP2515_REG_RXF4SIDH 	0x14
#define MCP2515_REG_RXF4SIDL 	0x15
#define MCP2515_REG_RXF4EID8 	0x16
#define MCP2515_REG_RXF4EID0 	0x17
#define MCP2515_REG_RXF5SIDH 	0x18
#define MCP2515_REG_RXF5SIDL 	0x19
#define MCP2515_REG_RXF5EID8 	0x1A
#define MCP2515_REG_RXF5EID0 	0x1B
#define MCP2515_REG_TEC 		0x1C
#define MCP2515_REG_REC 		0x1D
#define MCP2515_REG_RXM0SIDH 	0x20
#define MCP2515_REG_RXM0SIDL 	0x21
#define MCP2515_REG_RXM0EID8 	0x22
#define MCP2515_REG_RXM0EID0 	0x23
#define MCP2515_REG_RXM1SIDH 	0x24
#define MCP2515_REG_RXM1SIDL 	0x25
#define MCP2515_REG_RXM1EID8 	0x26
#define MCP2515_REG_RXM1EID0 	0x27
#define MCP2515_REG_CNF3 		0x28
#define MCP2515_REG_CNF2 		0x29
#define MCP2515_REG_CNF1 		0x2A
#define MCP2515_REG_CANINTE 	0x2B
#define MCP2515_BIT_MERRE 		7
#define MCP2515_BIT_WAKIE 		6
#define MCP2515_BIT_ERRIE 		5
#define MCP2515_BIT_TX2IE 		4
#define MCP2515_BIT_TX1IE 		3
#define MCP2515_BIT_TX0IE 		2
#define MCP2515_BIT_RX1IE 		1
#define MCP2515_BIT_RX0IE 		0
#define MCP2515_REG_CANINTF 	0x2C
#define MCP2515_BIT_MERRF 		7
#define MCP2515_BIT_WAKIF 		6
#define MCP2515_BIT_ERRIF 		5
#define MCP2515_BIT_TX2IF 		4
#define MCP2515_BIT_TX1IF 		3
#define MCP2515_BIT_TX0IF 		2
#define MCP2515_BIT_RX1IF 		1
#define MCP2515_BIT_RX0IF 		0
#define MCP2515_REG_EFLG 		0x2D
#define MCP2515_REG_TXB0CTRL 	0x30
#define MCP2515_BIT_TXREQ 		3
#define MCP2515_REG_TXB0SIDH 	0x31
#define MCP2515_REG_TXB0SIDL 	0x32
#define MCP2515_BIT_EXIDE 		3
#define MCP2515_REG_TXB0EID8 	0x33
#define MCP2515_REG_TXB0EID0 	0x34
#define MCP2515_REG_TXB0DLC 	0x35
#define MCP2515_BIT_TXRTR 		6
#define MCP2515_REG_TXB0D0 		0x36
#define MCP2515_REG_RXB0CTRL 	0x60
#define MCP2515_BIT_RXM1 		6
#define MCP2515_BIT_RXM0 		5
#define MCP2515_BIT_RXRTR 		3
// Bits 2:0 FILHIT2:0
#define MCP2515_REG_RXB0SIDH 	0x61
#define MCP2515_REG_RXB0SIDL 	0x62
#define MCP2515_REG_RXB0EID8 	0x63
#define MCP2515_REG_RXB0EID0 	0x64
#define MCP2515_REG_RXB0DLC 	0x65
#define MCP2515_REG_RXB0D0 		0x66

//MCP2515 Command Bytes
#define MCP2515_CMD_RESET 		0xC0
#define MCP2515_CMD_READ 		0x03
#define MCP2515_CMD_READ_RX_BUF	0x90
#define MCP2515_CMD_WRITE 		0x02
#define MCP2515_CMD_LOAD_TX_BUF	0x40
#define MCP2515_CMD_RTS 		0x80
#define MCP2515_CMD_READ_STATUS	0xA0
#define MCP2515_CMD_RX_STATUS 	0xB0
#define MCP2515_CMD_BIT_MODIFY 	0x05


#define SPI_CSR_NUM      1

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
#define SPI_TDR_PCS     SPI_MR_PCS
#define SPI_TRANSFER	(AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK)
#define NCPS_PDR_BIT    AT91C_PA9_NPCS1
#define NCPS_ASR_BIT    0
#define NPCS_BSR_BIT	AT91C_PA9_NPCS1

static void AT91_CAN_SPI_set_speed(unsigned char speed)
{
	unsigned int reg;
	AT91PS_SPI pSPI      = AT91C_BASE_SPI;

	reg = pSPI->SPI_CSR[SPI_CSR_NUM];
	reg = ( reg & ~(AT91C_SPI_SCBR) ) | ( (unsigned int)speed << 8 );
	pSPI->SPI_CSR[SPI_CSR_NUM] = reg;
}

static void AT91_CAN_SPI_init(){

	lock_spi();
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
	pSPI->SPI_MR = AT91C_SPI_PS_VARIABLE | AT91C_SPI_MSTR | AT91C_SPI_MODFDIS;

	// set chip-select-register
	// 8 bits per transfer, CPOL=1, ClockPhase=0, DLYBCT = 0
	pSPI->SPI_CSR[SPI_CSR_NUM] = AT91C_SPI_CPOL | AT91C_SPI_BITS_8 | AT91C_SPI_CSAAT;// | (0xFF << 16); //DLYBS

	// slow during init
	AT91_CAN_SPI_set_speed(0xFE);

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
	//normal speed is ~1 MHz. Reduce this if accelerometer readings are unstable
	//TODO slow for testing, speed up later
	AT91_CAN_SPI_set_speed(48);
	unlock_spi();
}

static unsigned char CAN_SPI_send(unsigned char outgoing, int last)
{
	unsigned char incoming;

	while(!(*AT91C_SPI_SR & AT91C_SPI_TDRE));
	*AT91C_SPI_TDR = ( (unsigned short)(outgoing) | ((unsigned int)(SPI_TDR_PCS)<<16) | (last ? AT91C_SPI_LASTXFER : 0) );

	while(!(*AT91C_SPI_SR & AT91C_SPI_RDRF));
	incoming = (unsigned char)*AT91C_SPI_RDR;
	return incoming;
}

static unsigned char MCP2515_read_reg(unsigned char regNum){
	CAN_SPI_send(MCP2515_CMD_READ, 0);
	CAN_SPI_send(regNum, 0);
	unsigned char regValue = CAN_SPI_send(0, 1);
	return regValue;
}

static void MCP2515_write_reg(unsigned char regNum, unsigned char value){
	CAN_SPI_send(MCP2515_CMD_WRITE, 0);
	CAN_SPI_send(regNum, 0);
	CAN_SPI_send(value, 1);
}

static void MCP2515_write_reg_bit(unsigned char regNum, unsigned char bitNum, unsigned char value){
	CAN_SPI_send(MCP2515_CMD_BIT_MODIFY, 0);
	CAN_SPI_send(regNum, 0);
	CAN_SPI_send(1 << bitNum, 0);
	if(value != 0){
		CAN_SPI_send(0xff, 1);
	}
	else{
		CAN_SPI_send(0x00, 1);
	}
}

static int MCP2515_setup(){
	int rc = 0;
	lock_spi();
	CAN_SPI_send(MCP2515_CMD_RESET, 1);
	for (int i=0; i< 1000000; i++){}
	int mode = MCP2515_read_reg(MCP2515_REG_CANSTAT) >> 5;
	if (mode == 0x04){ //0b100
		pr_info("MCP2515 CAN controller reset\r\n");
		rc = 1;
	}
	else{
		pr_info_int(mode);
		pr_error("=mode; Failed to SW reset MCP2515 CAN controller\r\n");
	}
	unlock_spi();
	return rc;
}

static int MCP2515_set_baud(int baud){
	lock_spi();
	unsigned char brp;

	//BRP<5:0> = 00h, so divisor (0+1)*2 for 125ns per quantum at 16MHz for 500K
	//SJW<1:0> = 00h, Sync jump width = 1
	switch(baud)
	{
	case CAN_BAUD_100K: brp = 4; break;
	case CAN_BAUD_125K: brp = 3; break;
	case CAN_BAUD_250K: brp = 1; break;
	case CAN_BAUD_500K: brp = 0; break;
	default:
		pr_warning_int(baud);
		pr_warning(" = unknown CAN baud rate. defaulting to 500K\r\n");
		brp = 0;
		break;
	}
	CAN_SPI_send(MCP2515_CMD_WRITE, 0);
	CAN_SPI_send(MCP2515_REG_CNF1, 0);
	CAN_SPI_send(brp & 0x3f, 1); //0b00111111

	//PRSEG<2:0> = 0x01, 2 time quantum for prop
	//PHSEG<2:0> = 0x06, 7 time constants to PS1 sample
	//SAM = 0, just 1 sampling
	//BTLMODE = 1, PS2 determined by CNF3
	CAN_SPI_send(MCP2515_CMD_WRITE, 0);
	CAN_SPI_send(MCP2515_REG_CNF2, 0);
	CAN_SPI_send(0xB0, 1); //0b10110001

	//PHSEG2<2:0> = 5 for 6 time constants after sample
	CAN_SPI_send(MCP2515_CMD_WRITE, 0);
	CAN_SPI_send(MCP2515_REG_CNF3, 0);
	CAN_SPI_send(0x06, 1);

	//SyncSeg + PropSeg + PS1 + PS2 = 1 + 2 + 7 + 6 = 16

	unlock_spi();
	return 1;
}

static int MCP2515_set_normal_mode(int oneShotMode){

	//REQOP2<2:0> = 000 for normal mode
	//ABAT = 0, do not abort pending transmission
	//OSM = 0, not one shot
	//CLKEN = 1, disable output clock
	//CLKPRE = 0b11, clk/8

//	unsigned char settings = (0x02 << 5) | 0x07 | (oneShotMode << 3);
	unsigned char settings = 0x07 | (oneShotMode << 3);
	pr_info_int(settings);
	pr_info("setting mode\r\n");

	MCP2515_write_reg(MCP2515_REG_CANCTRL, settings);
	//Read mode and make sure it is normal
	unsigned char mode = MCP2515_read_reg(MCP2515_REG_CANSTAT) >> 5;
	pr_info_int(mode);
	pr_info(" MCP2515 mode confirm\r\n");
	if(mode != 0) return 0; else return 1;
}
static void MCP2515_read_reg_values(unsigned char reg, unsigned char * values, unsigned int length){
	  CAN_SPI_send(MCP2515_CMD_READ, 0);
	  CAN_SPI_send(MCP2515_REG_RXB0D0, 0);
	  for(int i = 0; i < length; i++){
		  values[i] = CAN_SPI_send(0, (i == length - 1));
		  pr_info_int(values[i]);
		  pr_info(" ");
	  }
	  pr_info("= can msg\r\n");
}

int CAN_device_init(){
	AT91_CAN_SPI_init();
	return 	MCP2515_setup() &&
			MCP2515_set_baud(CAN_BAUD_500K) &&
			MCP2515_set_normal_mode(0);
}

int CAN_device_set_baud(int baud){
	return MCP2515_set_baud(baud);
}

int CAN_device_tx_msg(CAN_msg *msg, unsigned int timeoutMs){
	unsigned int startTime = getCurrentTicks();
	lock_spi();

	if(!msg->isExtendedAdrs)
	{
		unsigned short standardID = (short)msg->adrsValue;
		//Write standard ID registers
		unsigned short val = standardID >> 3;
		MCP2515_write_reg(MCP2515_REG_TXB0SIDH, val);
		val = standardID << 5;
		MCP2515_write_reg(MCP2515_REG_TXB0SIDL, val);
	}
	else
	{
		//Write extended ID registers, which use the standard ID registers
		unsigned short val = msg->adrsValue >> 21;
		MCP2515_write_reg(MCP2515_REG_TXB0SIDH, val);
		val = msg->adrsValue >> 16;
		val = val & 0x3; // 0b00000011
		val = val | (msg->adrsValue >> 13 & 0xE0); //0b11100000
		val |= 1 << MCP2515_BIT_EXIDE;
		MCP2515_write_reg(MCP2515_REG_TXB0SIDL, val);
		val = msg->adrsValue >> 8;
		MCP2515_write_reg(MCP2515_REG_TXB0EID8, val);
		val = msg->adrsValue;
		MCP2515_write_reg(MCP2515_REG_TXB0EID0, val);
	}

	unsigned short msgLen = msg->dataLength & 0x0f;
	if(msg->remoteTxRequest) msgLen = msgLen |  (1 << MCP2515_BIT_TXRTR);
	MCP2515_write_reg(MCP2515_REG_TXB0DLC, msgLen);

	//Message bytes
	CAN_SPI_send(MCP2515_CMD_WRITE, 0);
	CAN_SPI_send(MCP2515_REG_TXB0D0, 0);
	int dataLen = msg->dataLength;
	for(int i = 0; i < dataLen; i++){
	  CAN_SPI_send(msg->data[i], (i == dataLen - 1));
	}

	//Transmit the message
	MCP2515_write_reg_bit(MCP2515_REG_TXB0CTRL, MCP2515_BIT_TXREQ, 1);

	int sentMessage = 0;
	int x = 0;
	while (x++ < 20)
	//while(!isTimeoutMs(startTime, timeoutMs))
	{
		unsigned char regVal = MCP2515_read_reg(MCP2515_REG_CANINTF);
		pr_info_int(regVal);
		pr_info("=reg \r\n");
		if(regVal & (1 << MCP2515_BIT_TX0IF))
		{
			pr_info("CAN sent\r\n");
			sentMessage = 1;
			break;
		}
	}

	unsigned char errFlags = MCP2515_read_reg(MCP2515_REG_EFLG);
	pr_info_int(errFlags);
	pr_info("=error flags\r\n");

	unsigned char regFlags = MCP2515_read_reg(MCP2515_REG_TXB0CTRL);
	pr_info_int(regFlags);
	pr_info("=reg flags\r\n");

	if(!sentMessage){
		pr_warning("CAN msg TX timeout\r\n");
	}

	//Abort the send if failed
	MCP2515_write_reg_bit(MCP2515_REG_TXB0CTRL, MCP2515_BIT_TXREQ, 0);

	//And clear write interrupt
	MCP2515_write_reg_bit(MCP2515_REG_CANINTF, MCP2515_BIT_TX0IF, 0);

	unlock_spi();
	return sentMessage;
}

int CAN_device_rx_msg(CAN_msg *msg, unsigned int timeoutMs){
	lock_spi();
	unsigned int startTicks = getCurrentTicks();
	unsigned short standardID = 0;
	int gotMessage = 0;
	int x = 0;
		while (x++ < 200)
	//while(! isTimeoutMs(startTicks, timeoutMs))
	{
	  unsigned char val = MCP2515_read_reg(MCP2515_REG_CANINTF);
	  //If we have a message available, read it
	  if(val & (1 << MCP2515_BIT_RX0IF))
	  {
		pr_info("rx message\r\n");
		gotMessage = 1;
		break;
	  }
	}

	if(gotMessage)
	{
	  unsigned char val = MCP2515_read_reg(MCP2515_REG_RXB0CTRL);
	  msg->remoteTxRequest = (val & (1 << MCP2515_BIT_RXRTR)) ? 1 : 0;

	  //Address received from
	  val = MCP2515_read_reg(MCP2515_REG_RXB0SIDH);
	  standardID |= (val << 3);
	  val = MCP2515_read_reg(MCP2515_REG_RXB0SIDL);
	  standardID |= (val >> 5);

	  msg->adrsValue = (long)standardID;
	  msg->isExtendedAdrs = ((val & (1 << MCP2515_BIT_EXIDE)) ? 1 : 0);
	  if(msg->isExtendedAdrs)
	  {
		pr_info("msg is extended\r\n");
		msg->adrsValue = ((msg->adrsValue << 2) | (val & 0x03));
		val = MCP2515_read_reg(MCP2515_REG_RXB0EID8);
		msg->adrsValue = (msg->adrsValue << 8) | val;
		val = MCP2515_read_reg(MCP2515_REG_RXB0EID0);
		msg->adrsValue = (msg->adrsValue << 8) | val;
	  }
	  msg->adrsValue = 0x1FFFFFFF & msg->adrsValue; // mask out extra bits
	  //Read data bytes
	  pr_info_int(msg->adrsValue);
	  pr_info("=CAN address\r\n");
	  val = MCP2515_read_reg(MCP2515_REG_RXB0DLC);
	  msg->dataLength = (val & 0xf);
	  MCP2515_read_reg_values(MCP2515_REG_RXB0D0, msg->data, msg->dataLength);

	  //And clear read interrupt
	  MCP2515_write_reg_bit(MCP2515_REG_CANINTF, MCP2515_BIT_RX0IF, 0);
	}
	unlock_spi();
	return gotMessage;
}
