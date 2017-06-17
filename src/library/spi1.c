#include "spi1.h"

#define spi1NSSHigh()		GPIOA -> BSRR = GPIO_BSRR_BS4
#define spi1NSSLow()		GPIOA -> BRR = GPIO_BRR_BR4

static uint8 *spi1TxBuf;
static uint8 *spi1RxBuf;
static uint8 spi1Size;
static uint8 spi1Cnt;

static OS_EVENT *spi1Ready;

static void (*spi1HandlerPtr)(void);
static void spi1RHandler(void);
static void spi1WHandler(void);
static void spi1RWHandler(void);
static void spi1RegRHandler(void);
static void spi1RegWHandler(void);
static void spi1RegRWHandler(void);

void spi1Init(void) {
	/* enable gpio a clock */
	RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;

	/* mosi pin, pa7, alternative function output push-pull */
	GPIOA -> CRL &= ~GPIO_CRL_MODE7_0;
	GPIOA -> CRL |= GPIO_CRL_MODE7_1;
	GPIOA -> CRL &= ~GPIO_CRL_CNF7_0;
	GPIOA -> CRL |= GPIO_CRL_CNF7_1;

	/* miso pin, pa6, input floating */
	GPIOA -> CRL &= ~GPIO_CRL_MODE6_0;
	GPIOA -> CRL &= ~GPIO_CRL_MODE6_1;
	GPIOA -> CRL |= GPIO_CRL_CNF6_0;
	GPIOA -> CRL &= ~GPIO_CRL_CNF6_1;

	/* sck pin, pa5, alternative function output push-pull */
	GPIOA -> CRL &= ~GPIO_CRL_MODE5_0;
	GPIOA -> CRL |= GPIO_CRL_MODE5_1;
	GPIOA -> CRL &= ~GPIO_CRL_CNF5_0;
	GPIOA -> CRL |= GPIO_CRL_CNF5_1;

	/* nss pin, pa4, general purpose output push-pull */
	GPIOA -> CRL &= ~GPIO_CRL_MODE4_0;
	GPIOA -> CRL |= GPIO_CRL_MODE4_1;
	GPIOA -> CRL &= ~GPIO_CRL_CNF4_0;
	GPIOA -> CRL &= ~GPIO_CRL_CNF4_1;
	spi1NSSHigh();

	/* enable spi1 clock */
	RCC -> APB2ENR |= RCC_APB2ENR_SPI1EN;

	/* APB2 = 36 MHz, baud rate 1.125 MHz */
	SPI1 -> CR1 &= ~SPI_CR1_BR_0;
	SPI1 -> CR1 &= ~SPI_CR1_BR_1;
	SPI1 -> CR1 |= SPI_CR1_BR_2;

	/* CPOL = 0, CPHA = 0 */
	SPI1 -> CR1 &= ~SPI_CR1_CPHA;
	SPI1 -> CR1 &= ~SPI_CR1_CPOL;

	/* configure spi1 */
	SPI1 -> CR1 &= ~SPI_CR1_DFF;
	SPI1 -> CR1 &= ~SPI_CR1_LSBFIRST;
	SPI1 -> CR2 |= SPI_CR2_SSOE;
	SPI1 -> CR1 |= SPI_CR1_MSTR;

	/* semaphore */
	spi1Ready = OSSemCreate(0);

	/* set up interrupt */
	SPI1 -> CR2 |= SPI_CR2_RXNEIE;
	NVIC_ClearPendingIRQ(SPI1_IRQn);
	NVIC_EnableIRQ(SPI1_IRQn);

	/* enable i2c1 */
	SPI1 -> CR1 |= SPI_CR1_SPE;
}

INT8U spi1R(uint8 *buf, uint8 size) {
	INT8U err;

	spi1RxBuf = buf;
	spi1Size = size;
	spi1Cnt = 0;
	spi1HandlerPtr = spi1RHandler;
	spi1NSSLow();
	SPI1 -> DR = 0x00;
	OSSemPend(spi1Ready, SPI1_TIMEOUT, &err);
	return err;
}

INT8U spi1W(uint8 *buf, uint8 size) {
	INT8U err;

	spi1TxBuf = buf;
	spi1Size = size;
	spi1Cnt = 1;
	spi1HandlerPtr = spi1WHandler;
	spi1NSSLow();
	SPI1 -> DR = buf[0];
	OSSemPend(spi1Ready, SPI1_TIMEOUT, &err);
	return err;
}

INT8U spi1RW(uint8 *rxBuf, uint8 *txBuf, uint8 size) {
	INT8U err;

	spi1RxBuf = rxBuf;
	spi1TxBuf = txBuf;
	spi1Size = size;
	spi1Cnt = 0;
	spi1HandlerPtr = spi1RWHandler;
	spi1NSSLow();
	SPI1 -> DR = txBuf[0];
	OSSemPend(spi1Ready, SPI1_TIMEOUT, &err);
	return err;
}

INT8U spi1RegR(uint8 reg, uint8 *buf, uint8 size) {
	INT8U err;

	spi1RxBuf = buf;
	spi1Size = size;
	spi1Cnt = 0xFF;
	spi1HandlerPtr = spi1RegRHandler;
	spi1NSSLow();
	SPI1 -> DR = reg;
	OSSemPend(spi1Ready, SPI1_TIMEOUT, &err);
	return err;
}

INT8U spi1RegW(uint8 reg, uint8 *buf, uint8 size) {
	INT8U err;

	spi1TxBuf = buf;
	spi1Size = size;
	spi1Cnt = 0xFF;
	spi1HandlerPtr = spi1RegWHandler;
	spi1NSSLow();
	SPI1 -> DR = reg;
	OSSemPend(spi1Ready, SPI1_TIMEOUT, &err);
	return err;
}

INT8U spi1RegRW(uint8 reg, uint8 *rxBuf, uint8 *txBuf, uint8 size) {
	INT8U err;

	spi1RxBuf = rxBuf;
	spi1TxBuf = txBuf;
	spi1Size = size;
	spi1Cnt = 0xFF;
	spi1HandlerPtr = spi1RegRWHandler;
	spi1NSSLow();
	SPI1 -> DR = reg;
	OSSemPend(spi1Ready, SPI1_TIMEOUT, &err);
	return err;
}

void SPI1_IRQHandler(void) {
	(*spi1HandlerPtr)();
}

static void spi1RHandler(void) {
	spi1RxBuf[spi1Cnt] = SPI1 -> DR;
	spi1Cnt++;
	if (spi1Cnt < spi1Size) {
		SPI1 -> DR = 0x00;
	} else {
		spi1NSSHigh();
		OSIntEnter();
		OSSemPost(spi1Ready);
		OSIntExit();
	}
}

static void spi1WHandler(void) {
	uint8 dummy;

	dummy = SPI1 -> DR;
	(void)dummy;
	if (spi1Cnt < spi1Size) {
		SPI1 -> DR = spi1TxBuf[spi1Cnt];
		spi1Cnt++;
	} else {
		spi1NSSHigh();
		OSIntEnter();
		OSSemPost(spi1Ready);
		OSIntExit();
	}
}

static void spi1RWHandler(void) {
	spi1RxBuf[spi1Cnt] = SPI1 -> DR;
	spi1Cnt++;
	if (spi1Cnt < spi1Size) {
		SPI1 -> DR = spi1TxBuf[spi1Cnt];
	} else {
		spi1NSSHigh();
		OSIntEnter();
		OSSemPost(spi1Ready);
		OSIntExit();
	}
}

static void spi1RegRHandler(void) {
	if (spi1Cnt == 0xFF) {
		spi1RxBuf[spi1Cnt] = SPI1 -> DR;
		SPI1 -> DR = 0x00;
		spi1Cnt = 0;
	} else {
		spi1RxBuf[spi1Cnt] = SPI1 -> DR;
		spi1Cnt++;
		if (spi1Cnt < spi1Size) {
			SPI1 -> DR = 0x00;
		} else {
			spi1NSSHigh();
			OSIntEnter();
			OSSemPost(spi1Ready);
			OSIntExit();
		}
	}
}

static void spi1RegWHandler(void) {
	uint8 dummy;

	dummy = SPI1 -> DR;
	(void)dummy;
	if (spi1Cnt == 0xFF) {
		SPI1 -> DR = spi1TxBuf[0];
		spi1Cnt = 1;
	} else {
		if (spi1Cnt < spi1Size) {
			SPI1 -> DR = spi1TxBuf[spi1Cnt];
			spi1Cnt++;
		} else {
			spi1NSSHigh();
			OSIntEnter();
			OSSemPost(spi1Ready);
			OSIntExit();
		}
	}
}

static void spi1RegRWHandler(void) {
	if (spi1Cnt == 0xFF) {
		spi1RxBuf[spi1Cnt] = SPI1 -> DR;
		SPI1 -> DR = spi1TxBuf[0];
		spi1Cnt = 0;
	} else {
		spi1RxBuf[spi1Cnt] = SPI1 -> DR;
		spi1Cnt++;
		if (spi1Cnt < spi1Size) {
			SPI1 -> DR = spi1TxBuf[spi1Cnt];
		} else {
			spi1NSSHigh();
			OSIntEnter();
			OSSemPost(spi1Ready);
			OSIntExit();
		}
	}
}
