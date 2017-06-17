#include "nrf24l01p_port.h"
#include "nrf24l01p.h"

#define R_RX_PAYLOAD			0x61
#define W_TX_PAYLOAD			0xA0
#define FLUSH_TX				0xE1
#define FLUSH_RX				0xE2
#define REUSE_TX_PL				0xE3
#define R_RX_RL_WID				0x60
#define W_ACK_PAYLOAD			0xA8
#define W_TX_PAYLOAD_NO_ACK		0xB0
#define NOP						0xFF

#define CONFIG					0x00
#define EN_AA					0x01
#define EN_RXADDR				0x02
#define SETUP_AW				0x03
#define SETUP_RETR				0x04
#define RF_CH					0x05
#define RF_SETUP				0x06
#define STATUS					0x07
#define OBSERVE_TX				0x08
#define RPD						0x09
#define RX_ADDR_P0				0x0A
#define RX_ADDR_P1				0x0B
#define RX_ADDR_P2				0x0C
#define RX_ADDR_P3				0x0D
#define RX_ADDR_P4				0x0E
#define RX_ADDR_P5				0x0F
#define TX_ADDR					0x10
#define RX_PW_P0				0x11
#define RX_PW_P1				0x12
#define RX_PW_P2				0x13
#define RX_PW_P3				0x14
#define RX_PW_P4				0x15
#define RX_PW_P5				0x16
#define FIFO_STATUS				0x17
#define DYNPD					0x1C
#define FEATURE					0x1D

#define ERR_TIMEOUT				0x80
#define RX_DR					0x40
#define TX_DS					0x20
#define MAX_RT					0x10

static OS_EVENT *nrf24l01pRdy;

static void nrf24l01pHandler(void);

void nrf24l01pInit(void)
{
	nrf24l01pCEInit();
	nrf24l01pCELow();
	nrf24l01pIRQInit(nrf24l01pHandler);
	nrf24l01pIRQClear();
	nrf24l01pIRQEnable();
	nrf24l01pRdy = OSSemCreate(0);
}

void nrf24l01pInitRxFromTask(void)
{
	uint8 nrf24l01pAddress[ADDRESS_WIDTH];

	nrf24l01pAddress[0] = 0x34;
	nrf24l01pAddress[1] = 0x43;
	nrf24l01pAddress[2] = 0x01;

	OSTimeDly(OS_TICKS_PER_SEC / 10 + 1);
	nrf24l01pRegByteW(CONFIG, 0x0F);
	nrf24l01pRegByteW(EN_AA, 0x01);
	nrf24l01pRegByteW(EN_RXADDR, 0x01);
	/* more than one option */
	nrf24l01pRegByteW(SETUP_AW, 0x01);
	/* more than one option */
	nrf24l01pRegByteW(SETUP_RETR, 0x07);
	nrf24l01pRegByteW(RF_CH, 60);
	/* 0x0E: 2Mbps; 0x06: 1Mbps; 0x26: 250kbps */
	nrf24l01pRegByteW(RF_SETUP, 0x0E);
	nrf24l01pRegBufW(RX_ADDR_P0, nrf24l01pAddress, ADDRESS_WIDTH);
	nrf24l01pRegByteW(DYNPD, 0x01);
	nrf24l01pRegByteW(FEATURE, 0x06);
	nrf24l01pCmdW(FLUSH_TX);
	nrf24l01pCmdW(FLUSH_RX);
	nrf24l01pRegByteW(STATUS, 0x70);
	nrf24l01pCEHigh();
	OSTimeDly(2);
}

uint8 nrf24l01pRxAckPay(uint8 *buf, uint8 *size, uint32 timeout)
{
	INT8U err;
	uint8 length;
	uint8 status;

	OSSemPend(nrf24l01pRdy, timeout, &err);
	if (err == OS_ERR_NONE) {
		status = nrf24l01pCmdW(NOP);
		nrf24l01pRegByteW(STATUS, status & 0x70);
		if (status & RX_DR) {
			length = nrf24l01pCmdByteR(R_RX_RL_WID);
			nrf24l01pCmdBufR(R_RX_PAYLOAD, buf, length);
			*size = length;
		}
	} else {
		//nrf24l01pIRQDisable();
		nrf24l01pRegByteW(CONFIG, 0x0C);
		nrf24l01pRegByteW(CONFIG, 0x0E);
		status = ERR_TIMEOUT;
	}
	return status;
}

void nrf24l01pTxAckPay(uint8 *buf, uint8 size)
{
	nrf24l01pCmdW(FLUSH_TX);
	nrf24l01pCmdBufW(W_ACK_PAYLOAD | 0x00, buf, size);
}

uint8 nrf24l01pObserveTx(void)
{
	return nrf24l01pRegByteR(OBSERVE_TX);
}

static void nrf24l01pHandler(void)
{
	//nrf24l01pIRQDisable();
	OSSemPost(nrf24l01pRdy);
}
