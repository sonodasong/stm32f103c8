#include "usart1.h"

char hexTable[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

static char usart1RxBuf[ex(UART1_RX_Q_SIZE)][ex(UART1_RX_SIZE)];
static uint8 usart1RxCnt;
static uint8 usart1RxQCnt;
static void *usart1RxQPtr[ex(UART1_RX_Q_SIZE)];
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
	/* enable gpio a clock */
	RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;

	/* rx pin, pa10, input floating */
	GPIOA -> CRH &= ~GPIO_CRH_MODE10_0;
	GPIOA -> CRH &= ~GPIO_CRH_MODE10_1;
	GPIOA -> CRH |= GPIO_CRH_CNF10_0;
	GPIOA -> CRH &= ~GPIO_CRH_CNF10_1;

	/* tx pin, pa9, alternative function output push-pull */
	GPIOA -> CRH &= ~GPIO_CRH_MODE9_0;
	GPIOA -> CRH |= GPIO_CRH_MODE9_1;
	GPIOA -> CRH &= ~GPIO_CRH_CNF9_0;
	GPIOA -> CRH |= GPIO_CRH_CNF9_1;

	/* enable usart1 clock */
	RCC -> APB2ENR |= RCC_APB2ENR_USART1EN;

	/* APB2 = 36 MHz, baud rate 115200 */
	USART1 -> BRR = 0x0138;

	/* semaphore and queue */
	usart1RxQ = OSQCreate(usart1RxQPtr, ex(UART1_RX_Q_SIZE));
	usart1TxRdy = OSSemCreate(0);

	/* set up interrupt */
	USART1 -> CR1 |= USART_CR1_RXNEIE;
	NVIC_ClearPendingIRQ(USART1_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);

	/* enable usart1 */
	USART1 -> CR1 |= USART_CR1_RE;
	USART1 -> CR1 |= USART_CR1_TE;
	USART1 -> CR1 |= USART_CR1_UE;
}

INT8U usart1Read(char **str) {
	INT8U err;

	*str = (char *)OSQPend(usart1RxQ, UART1_RX_TIMEOUT, &err);
	return err;
}

INT8U usart1Print(char *str) {
	INT8U err;
	char temp;

	usart1TXEHandlerPtr = usart1PrintHandler;
	usart1TxPtr = str;
	usart1TxCnt = 0;
	temp = str[0];
	if (temp != '\0') {
		USART1 -> DR = temp;
		USART1 -> CR1 |= USART_CR1_TXEIE;
		OSSemPend(usart1TxRdy, UART1_TX_TIMEOUT, &err);
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
		USART1 -> DR = str[0];
		USART1 -> CR1 |= USART_CR1_TXEIE;
		OSSemPend(usart1TxRdy, UART1_TX_TIMEOUT, &err);
	}
	return err;
}

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
	char a = USART1 -> DR;
	char usart1RxChar;

	usart1RxChar = USART1 -> DR;
	if (usart1RxChar == UART1_RX_EOF) {
		usart1RxBuf[usart1RxQCnt][usart1RxCnt] = '\0';
		usart1RxCnt = 0;
		OSIntEnter();
		OSQPost(usart1RxQ, usart1RxBuf[usart1RxQCnt]);
		usart1RxQCnt++;
		usart1RxQCnt &= ex(UART1_RX_Q_SIZE) - 1;
		OSIntExit();
	} else {
		usart1RxBuf[usart1RxQCnt][usart1RxCnt] = usart1RxChar;
		usart1RxCnt++;
		usart1RxCnt &= ex(UART1_RX_SIZE) - 1;
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
