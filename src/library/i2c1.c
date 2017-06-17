#include "i2c1.h"

#define i2c1Start()			I2C1 -> CR1 |= I2C_CR1_START
#define i2c1Ack()			I2C1 -> CR1 |= I2C_CR1_ACK
#define i2c1NAck()			I2C1 -> CR1 &= ~I2C_CR1_ACK
#define i2c1Stop()			I2C1 -> CR1 |= I2C_CR1_STOP
#define i2c1PosSet()		I2C1 -> CR1 |= I2C_CR1_POS
#define i2c1PosClear()		I2C1 -> CR1 &= ~I2C_CR1_POS

static uint8 i2c1Address;
static uint8 i2c1Reg;

static uint8 *i2c1RxPtr;
static uint8 i2c1RxCnt;
static uint8 i2c1RxSize;
static boolean i2c1RxRegPhase;
static boolean i2c1RxDataPhase;

static uint8 *i2c1TxPtr;
static uint8 i2c1TxCnt;
static uint8 i2c1TxSize;

static void (*i2c1HandlerPtr)(void);

static OS_EVENT *i2c1Ready;

static void i2c1StretchScl(boolean stretch);
static void i2c1RegR1Handler(void);
static void i2c1RegR2Handler(void);
static void i2c1RegR2pHandler(void);
static void i2c1RegWHandler(void);

void i2c1Init(void) {
	/* enable gpio b clock */
	RCC -> APB2ENR |= RCC_APB2ENR_IOPBEN;

	/* sda pin, pb7, alternative function output open-drain */
	GPIOB -> CRL &= ~GPIO_CRL_MODE7_0;
	GPIOB -> CRL |= GPIO_CRL_MODE7_1;
	GPIOB -> CRL |= GPIO_CRL_CNF7_0;
	GPIOB -> CRL |= GPIO_CRL_CNF7_1;

	/* scl pin, pb6, setup as no stretch */
	i2c1StretchScl(FALSE);

	/* enable i2c1 clock */
	RCC -> APB1ENR |= RCC_APB1ENR_I2C1EN;

	/* reset i2c1 */
	I2C1 -> CR1 |= I2C_CR1_SWRST;
	I2C1 -> CR1 &= ~I2C_CR1_SWRST;

	/* APB1 = 36 MHz, fm mode */
	I2C1 -> CR2 &= ~I2C_CR2_FREQ;
	I2C1 -> CR2 |= 0x24;
	I2C1 -> CCR |= I2C_CCR_FS;
	I2C1 -> CCR &= ~I2C_CCR_DUTY;
	I2C1 -> CCR &= ~I2C_CCR_CCR;
	I2C1 -> CCR |= 0x1E;
	I2C1 -> TRISE &= ~I2C_TRISE_TRISE;
	I2C1 -> TRISE |= 0x0B;

	/* semaphore */
	i2c1Ready = OSSemCreate(0);

	/* set up interrupt */
	I2C1 -> CR2 |= I2C_CR2_ITERREN;
	I2C1 -> CR2 |= I2C_CR2_ITEVTEN;
	I2C1 -> CR2 &= ~I2C_CR2_ITBUFEN;
	NVIC_ClearPendingIRQ(I2C1_EV_IRQn);
	NVIC_EnableIRQ(I2C1_EV_IRQn);
	NVIC_ClearPendingIRQ(I2C1_ER_IRQn);
	NVIC_EnableIRQ(I2C1_ER_IRQn);

	/* enable i2c1 */
	I2C1 -> CR1 |= I2C_CR1_PE;
}

INT8U i2c1RegR(uint8 address, uint8 reg, uint8 *rx, uint8 rxSize) {
	INT8U err;

	i2c1Address = address;
	i2c1Reg = reg;
	i2c1RxPtr = rx;
	i2c1RxCnt = 0;
	i2c1RxRegPhase = TRUE;
	i2c1RxDataPhase = FALSE;
	if (rxSize == 1) {
		i2c1HandlerPtr = i2c1RegR1Handler;
	} else if (rxSize == 2) {
		i2c1HandlerPtr = i2c1RegR2Handler;
	} else {
		i2c1RxSize = rxSize;
		i2c1HandlerPtr = i2c1RegR2pHandler;
	}
	i2c1Start();
	OSSemPend(i2c1Ready, I2C1_TIME_OUT, &err);
	return err;
}

uint8 i2c1RegRCount(void) {
	return i2c1RxCnt;
}

INT8U i2c1RegW(uint8 address, uint8 reg, uint8 *tx, uint8 txSize) {
	INT8U err;
	
	i2c1Address = address;
	i2c1Reg = reg;
	i2c1TxPtr = tx;
	i2c1TxCnt = 0;
	i2c1TxSize = txSize;
	i2c1HandlerPtr = i2c1RegWHandler;
	i2c1Start();
	OSSemPend(i2c1Ready, I2C1_TIME_OUT, &err);
	return err;
}

uint8 i2c1RegWCount(void) {
	return i2c1TxCnt;
}

void I2C1_EV_IRQHandler(void) {
	(*i2c1HandlerPtr)();
}

void I2C1_ER_IRQHandler(void) {
	uint16 status;

	status = I2C1 -> SR1;
	if (status & I2C_SR1_AF) {
		I2C1 -> SR1 &= ~I2C_SR1_AF;
		i2c1Stop();
		OSIntEnter();
		OSSemPost(i2c1Ready);
		OSIntExit();
	}
}

static void i2c1StretchScl(boolean stretch) {
	if (stretch) {
		/* scl pin, pb6, general purpose output open-drain */
		GPIOB -> CRL &= ~GPIO_CRL_MODE6_0;
		GPIOB -> CRL |= GPIO_CRL_MODE6_1;
		GPIOB -> CRL |= GPIO_CRL_CNF6_0;
		GPIOB -> CRL &= ~GPIO_CRL_CNF6_1;
		GPIOB -> BRR = GPIO_BRR_BR6;
	} else {
		/* scl pin, pb6, alternative function output open-drain */
		GPIOB -> CRL &= ~GPIO_CRL_MODE6_0;
		GPIOB -> CRL |= GPIO_CRL_MODE6_1;
		GPIOB -> CRL |= GPIO_CRL_CNF6_0;
		GPIOB -> CRL |= GPIO_CRL_CNF6_1;
	}
}

static void i2c1RegR1Handler(void) {
	uint16 status;

	status = I2C1 -> SR1;
	if (status & I2C_SR1_SB) {
		if (i2c1RxRegPhase) {
			I2C1 -> DR = i2c1Address << 1;
		} else {
			i2c1RxDataPhase = TRUE;
			I2C1 -> DR = (i2c1Address << 1) | 0x01;
		}
	} else if (status & I2C_SR1_ADDR) {
		if (i2c1RxRegPhase) {
			status = I2C1 -> SR2;
			I2C1 -> DR = i2c1Reg;
		}
		if (i2c1RxDataPhase) {
			i2c1StretchScl(TRUE);
			status = I2C1 -> SR2;
			i2c1NAck();
			i2c1Stop();
			i2c1StretchScl(FALSE);
		}
	} else if (status & I2C_SR1_BTF) {
		if (i2c1RxRegPhase) {
			i2c1RxRegPhase = FALSE;
			I2C1 -> CR2 |= I2C_CR2_ITBUFEN;
			i2c1Start();
		}
	} else if (status & I2C_SR1_RXNE) {
		I2C1 -> CR2 &= ~I2C_CR2_ITBUFEN;
		i2c1RxPtr[0] = I2C1 -> DR;
		i2c1RxCnt++;
		OSIntEnter();
		OSSemPost(i2c1Ready);
		OSIntExit();
	}
}
/*
#include "usart1_pending.h"
static void i2c1RegR1Handler(void) {
	uint16 status;

	status = I2C1 -> SR1;
	if (status & I2C_SR1_SB) {
		if (i2c1RxRegPhase) {
			I2C1 -> DR = i2c1Address << 1;
		} else {
			i2c1RxDataPhase = TRUE;
			I2C1 -> DR = (i2c1Address << 1) | 0x01;
		}
	} else if (status & I2C_SR1_ADDR) {
		if (i2c1RxRegPhase) {
			status = I2C1 -> SR2;
			I2C1 -> DR = i2c1Reg;
		}
		if (i2c1RxDataPhase) {
			i2c1NAck();
			i2c1StretchScl(TRUE);
			status = I2C1 -> SR2;
			i2c1Stop();
			i2c1StretchScl(FALSE);
		}
	} else if (status & I2C_SR1_BTF) {
		if (i2c1RxRegPhase) {
			i2c1RxRegPhase = FALSE;
			i2c1Start();
		}
		if (i2c1RxDataPhase) {
			i2c1RxPtr[0] = I2C1 -> DR;
			OSIntEnter();
			OSSemPost(i2c1Ready);
			OSIntExit();
		}
	}
}
*/
static void i2c1RegR2Handler(void) {
	uint16 status;

	status = I2C1 -> SR1;
	if (status & I2C_SR1_SB) {
		if (i2c1RxRegPhase) {
			I2C1 -> DR = i2c1Address << 1;
		} else {
			i2c1RxDataPhase = TRUE;
			I2C1 -> DR = (i2c1Address << 1) | 0x01;
		}
	} else if (status & I2C_SR1_ADDR) {
		if (i2c1RxRegPhase) {
			status = I2C1 -> SR2;
			I2C1 -> DR = i2c1Reg;
		}
		if (i2c1RxDataPhase) {
			i2c1PosSet();
			i2c1Ack();
			i2c1StretchScl(TRUE);
			status = I2C1 -> SR2;
			i2c1NAck();
			i2c1StretchScl(FALSE);
		}
	} else if (status & I2C_SR1_BTF) {
		if (i2c1RxRegPhase) {
			i2c1RxRegPhase = FALSE;
			i2c1Start();
		}
		if (i2c1RxDataPhase) {
			i2c1StretchScl(TRUE);
			i2c1Stop();
			i2c1PosClear();
			i2c1RxPtr[0] = I2C1 -> DR;
			i2c1RxCnt++;
			i2c1StretchScl(FALSE);
			i2c1RxPtr[1] = I2C1 -> DR;
			i2c1RxCnt++;
			OSIntEnter();
			OSSemPost(i2c1Ready);
			OSIntExit();
		}
	}
}

static void i2c1RegR2pHandler(void) {
	uint16 status;

	status = I2C1 -> SR1;
	if (status & I2C_SR1_SB) {
		if (i2c1RxRegPhase) {
			I2C1 -> DR = i2c1Address << 1;
		} else {
			i2c1RxDataPhase = TRUE;
			I2C1 -> DR = (i2c1Address << 1) | 0x01;
		}
	} else if (status & I2C_SR1_ADDR) {
		if (i2c1RxRegPhase) {
			status = I2C1 -> SR2;
			I2C1 -> DR = i2c1Reg;
		}
		if (i2c1RxDataPhase) {
			i2c1Ack();
			status = I2C1 -> SR2;
		}
	} else if (status & I2C_SR1_BTF) {
		if (i2c1RxRegPhase) {
			i2c1RxRegPhase = FALSE;
			i2c1Start();
		}
		if (i2c1RxDataPhase) {
			if (i2c1RxCnt < i2c1RxSize - 3) {
				i2c1RxPtr[i2c1RxCnt] = I2C1 -> DR;
				i2c1RxCnt++;
			} else if (i2c1RxCnt < i2c1RxSize - 2) {
				i2c1NAck();
				i2c1RxPtr[i2c1RxCnt] = I2C1 -> DR;
				i2c1RxCnt++;
			} else if (i2c1RxCnt < i2c1RxSize - 1) {
				i2c1StretchScl(TRUE);
				i2c1Stop();
				i2c1RxPtr[i2c1RxCnt] = I2C1 -> DR;
				i2c1RxCnt++;
				i2c1StretchScl(FALSE);
				i2c1RxPtr[i2c1RxCnt] = I2C1 -> DR;
				i2c1RxCnt++;
				OSIntEnter();
				OSSemPost(i2c1Ready);
				OSIntExit();
			}
		}
	}
}

static void i2c1RegWHandler(void) {
	uint16 status;

	status = I2C1 -> SR1;
	if (status & I2C_SR1_SB) {
		I2C1 -> DR = i2c1Address << 1;
	} else if (status & I2C_SR1_ADDR) {
		status = I2C1 -> SR2;
		I2C1 -> DR = i2c1Reg;
	} else if (status & I2C_SR1_BTF) {
		if (i2c1TxCnt < i2c1TxSize) {
			I2C1 -> DR = i2c1TxPtr[i2c1TxCnt];
			i2c1TxCnt++;
		} else {
			i2c1Stop();
			OSIntEnter();
			OSSemPost(i2c1Ready);
			OSIntExit();
		}
	}
}
