#include "usart1.h"

static char usart1RxBuf[ex(USART1_RX_Q_SIZE)][ex(USART1_RX_SIZE)];
static uint8 usart1RxCnt;
static uint8 usart1RxQCnt;
static void *usart1RxQPtr[ex(USART1_RX_Q_SIZE)];
static OS_EVENT *usart1RxQ;

static char *usart1TxPtr;
static uint8 usart1TxLen;
static uint8 usart1TxCnt;
static void (*usart1TXEHandlerPtr)(void);
static OS_EVENT *usart1TxRdy;

static void usart1RXNEHandler(void);
static void usart1PrintHandler(void);
static void usart1PrintLenHandler(void);

void usart1Init(void) {
	/* enable clock */
	RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC -> APB2ENR |= RCC_APB2ENR_USART1EN;

	/* rx pin, pa10, input floating */
	GPIOA -> CRH &= ~GPIO_CRH_MODE10_0;
	GPIOA -> CRH &= ~GPIO_CRH_MODE10_1;
	GPIOA -> CRH |= GPIO_CRH_CNF10_0;
	GPIOA -> CRH &= ~GPIO_CRH_CNF10_1;

	/* tx pin, pa9, alternative function push-pull */
	GPIOA -> CRH &= ~GPIO_CRH_MODE9_0;
	GPIOA -> CRH |= GPIO_CRH_MODE9_1;
	GPIOA -> CRH &= ~GPIO_CRH_CNF9_0;
	GPIOA -> CRH |= GPIO_CRH_CNF9_1;

	/* baud rate 115200 */
	USART1 -> BRR = 0x0271;

	USART1 -> CR1 |= USART_CR1_RXNEIE;
	USART1 -> CR1 |= USART_CR1_RE;
	USART1 -> CR1 |= USART_CR1_TE;
	USART1 -> CR1 |= USART_CR1_UE;

	NVIC_ClearPendingIRQ(USART1_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);

	usart1RxQ = OSQCreate(usart1RxQPtr, ex(USART1_RX_Q_SIZE));
	usart1TxRdy = OSSemCreate(0);
}

INT8U usart1Read(char **str) {
	INT8U err;

	*str = (char *)OSQPend(usart1RxQ, USART1_RX_TIMEOUT, &err);
	return err;
}

INT8U usart1Print(char *str) {
	INT8U err;
	char temp;

	usart1TXEHandlerPtr = usart1PrintHandler;
	usart1TxPtr = str;
	usart1TxCnt = 0;
	temp = *str;
	if (temp != '\0') {
		USART1 -> DR = temp;
		USART1 -> CR1 |= USART_CR1_TXEIE;
		OSSemPend(usart1TxRdy, USART1_TX_TIMEOUT, &err);
	}
	return err;
}

INT8U usart1PrintLen(char *str, uint8 len) {
	INT8U err;

	usart1TXEHandlerPtr = usart1PrintLenHandler;
	usart1TxPtr = str;
	usart1TxLen = len;
	usart1TxCnt = 0;
	if (len > 0) {
		USART1 -> DR = *str;
		USART1 -> CR1 |= USART_CR1_TXEIE;
		OSSemPend(usart1TxRdy, USART1_TX_TIMEOUT, &err);
	}
	return err;
}

#if USART1_ENABLE_PRINTF

#include <stdio.h>
#include <stdarg.h>

static char usart1TxBuf[256];

void usart1Printf(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vsprintf(usart1TxBuf, fmt, ap);
	va_end(ap);
	usart1Print(usart1TxBuf);
}

#endif

void USART1_IRQHandler(void) {
	uint32 status;

	status = USART1 -> SR;
	if (status & USART_SR_RXNE) {
		usart1RXNEHandler();
	} else if (status & USART_SR_TXE) {
		usart1TXEHandlerPtr();
	}
}

static void usart1RXNEHandler(void) {
	char usart1RxChar;

	usart1RxChar = USART1 -> DR;
	if (usart1RxChar == USART1_RX_EOF) {
		usart1RxBuf[usart1RxQCnt][usart1RxCnt] = '\0';
		usart1RxCnt = 0;
		OSIntEnter();
		OSQPost(usart1RxQ, usart1RxBuf[usart1RxQCnt]);
		usart1RxQCnt++;
		usart1RxQCnt &= ex(USART1_RX_Q_SIZE) - 1;
		OSIntExit();
	} else {
		usart1RxBuf[usart1RxQCnt][usart1RxCnt] = usart1RxChar;
		usart1RxCnt++;
		usart1RxCnt &= ex(USART1_RX_SIZE) - 1;
	}
}

static void usart1PrintHandler(void) {
	usart1TxCnt++;
	if (usart1TxPtr[usart1TxCnt] == '\0') {
		USART1 -> CR1 &= ~USART_CR1_TXEIE;
		OSIntEnter();
		OSSemPost(usart1TxRdy);
		OSIntExit();
	} else {
		USART1 -> DR = usart1TxPtr[usart1TxCnt];
	}
}

static void usart1PrintLenHandler(void) {
	usart1TxCnt++;
	if (usart1TxCnt == usart1TxLen) {
		USART1 -> CR1 &= ~USART_CR1_TXEIE;
		OSIntEnter();
		OSSemPost(usart1TxRdy);
		OSIntExit();
	} else {
		USART1 -> DR = usart1TxPtr[usart1TxCnt];
	}
}
