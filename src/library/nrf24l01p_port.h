#ifndef __NRF24L01P_PORT_H__
#define __NRF24L01P_PORT_H__

#include "define.h"

uint8 nrf24l01pCmdW(uint8 cmd);
uint8 nrf24l01pCmdByteR(uint8 cmd);
void nrf24l01pCmdByteW(uint8 cmd, uint8 value);
void nrf24l01pCmdBufR(uint8 cmd, uint8 *buf, uint8 size);
void nrf24l01pCmdBufW(uint8 cmd, uint8 *buf, uint8 size);
uint8 nrf24l01pRegByteR(uint8 reg);
void nrf24l01pRegByteW(uint8 reg, uint8 value);
void nrf24l01pRegBufR(uint8 reg, uint8 *buf, uint8 size);
void nrf24l01pRegBufW(uint8 reg, uint8 *buf, uint8 size);

void nrf24l01pIRQInit(void (*handlerPtr)(void));
#define nrf24l01pIRQEnable()		EXTI -> IMR |= EXTI_IMR_MR1
#define nrf24l01pIRQDisable()		EXTI -> IMR &= ~EXTI_IMR_MR1
#define nrf24l01pIRQClear()			EXTI -> PR = EXTI_PR_PR1

void nrf24l01pCEInit(void);
#define nrf24l01pCEHigh()			GPIOA -> BSRR = GPIO_BSRR_BS8
#define nrf24l01pCELow()			GPIOA -> BRR = GPIO_BRR_BR8

#endif
