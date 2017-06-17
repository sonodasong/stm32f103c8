#include "spi1.h"
#include "nrf24l01p_port.h"

static void (*nrf24l01pHandlerPtr)(void);

uint8 nrf24l01pCmdW(uint8 cmd) {
	spi1RW(&cmd, &cmd, 1);
	return cmd;
}

uint8 nrf24l01pCmdByteR(uint8 cmd) {
	uint8 temp;

	spi1RegR(cmd, &temp, 1);
	return temp;
}

void nrf24l01pCmdByteW(uint8 cmd, uint8 value) {
	spi1RegW(cmd, &value, 1);
}

void nrf24l01pCmdBufR(uint8 cmd, uint8 *buf, uint8 size) {
	spi1RegR(cmd, buf, size);
}

void nrf24l01pCmdBufW(uint8 cmd, uint8 *buf, uint8 size) {
	spi1RegW(cmd, buf, size);
}

uint8 nrf24l01pRegByteR(uint8 reg) {
	uint8 temp;

	spi1RegR(reg, &temp, 1);
	return temp;
}

void nrf24l01pRegByteW(uint8 reg, uint8 value) {
	spi1RegW(0x20 | reg, &value, 1);
}

void nrf24l01pRegBufR(uint8 reg, uint8 *buf, uint8 size) {
	spi1RegR(reg, buf, size);
}

void nrf24l01pRegBufW(uint8 reg, uint8 *buf, uint8 size) {
	spi1RegW(0x20 | reg, buf, size);
}

void nrf24l01pIRQInit(void (*handlerPtr)(void)) {
	nrf24l01pHandlerPtr = handlerPtr;

	/* enable gpio a clock */
	RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;

	/* nrf irq pin, pa1, input pull-up */
	GPIOA -> CRL &= ~GPIO_CRL_MODE1_0;
	GPIOA -> CRL &= ~GPIO_CRL_MODE1_1;
	GPIOA -> CRL &= ~GPIO_CRL_CNF1_0;
	GPIOA -> CRL |= GPIO_CRL_CNF1_1;
	GPIOA -> BSRR = GPIO_BSRR_BS1;

	/* enable afio clock */
	RCC -> APB2ENR |= RCC_APB2ENR_AFIOEN;

	/* disable interrupt */
	nrf24l01pIRQDisable();

	/* gpio a for exti1 */
	AFIO -> EXTICR[0] &= ~AFIO_EXTICR1_EXTI1;
	AFIO -> EXTICR[0] |= AFIO_EXTICR1_EXTI1_PA;

	/* set up interrupt as falling trigger */
	EXTI -> RTSR &= ~EXTI_RTSR_TR1;
	EXTI -> FTSR |= EXTI_FTSR_TR1;

	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI1_IRQn);
}

void nrf24l01pCEInit(void) {
	/* enable gpio a clock */
	RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;

	/* ce pin, pa8, general purpose output push-pull */
	GPIOA -> CRH &= ~GPIO_CRH_MODE8_0;
	GPIOA -> CRH |= GPIO_CRH_MODE8_1;
	GPIOA -> CRH &= ~GPIO_CRH_CNF8_0;
	GPIOA -> CRH &= ~GPIO_CRH_CNF8_1;
}

void EXTI1_IRQHandler(void) {
	OSIntEnter();
	nrf24l01pIRQClear();
	nrf24l01pHandlerPtr();
	OSIntExit();
}
