#ifndef __NRF24L01P_H__
#define __NRF24L01P_H__

#include "define.h"

#define ADDRESS_WIDTH		3

void nrf24l01pInit(void);
void nrf24l01pInitRxFromTask(void);
uint8 nrf24l01pRxAckPay(uint8 *buf, uint8 *size, uint32 timeout);
void nrf24l01pTxAckPay(uint8 *buf, uint8 size);
uint8 nrf24l01pObserveTx(void);

#endif
