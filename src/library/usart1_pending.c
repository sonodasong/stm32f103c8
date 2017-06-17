#include "usart1_pending.h"

extern char hexTable[16];

void usart1PendingInit(void) {
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

	/* enable usart1 */
	USART1 -> CR1 |= USART_CR1_RE;
	USART1 -> CR1 |= USART_CR1_TE;
	USART1 -> CR1 |= USART_CR1_UE;
}

void usart1PendingPrint(char *str) {
	char temp;
	uint8 cnt;

	cnt = 0;
	temp = str[0];
	while (temp != '\0') {
		USART1 -> DR = temp;
		cnt++;
		temp = str[cnt];
		while(!(USART1 -> SR & USART_SR_TXE));
	}
}

void usart1PendingPrintLen(char *str, uint8 len) {
	uint8 i;
	
	for (i = 0; i < len; i++) {
		USART1 -> DR = str[i];
		while(!(USART1 -> SR & USART_SR_TXE));
	}
}

void usart1PendingHex8(uint8 hex8) {
	char buffer[5];

	buffer[0] = '0';
	buffer[1] = 'x';
	buffer[2] = hexTable[(hex8>>4)&0x0F];
	buffer[3] = hexTable[hex8&0x0F];
	buffer[4] = '\0';
	usart1PendingPrint(buffer);
}

void usart1PendingHex16(uint16 hex16) {
	char buffer[7];

	buffer[0] = '0';
	buffer[1] = 'x';
	buffer[2] = hexTable[(hex16>>12)&0x0F];
	buffer[3] = hexTable[(hex16>>8)&0x0F];
	buffer[4] = hexTable[(hex16>>4)&0x0F];
	buffer[5] = hexTable[hex16&0x0F];
	buffer[6] = '\0';
	usart1PendingPrint(buffer);
}

void usart1PendingHex32(uint32 hex32) {
	char buffer[11];

	buffer[0] = '0';
	buffer[1] = 'x';
	buffer[2] = hexTable[(hex32>>28)&0x0F];
	buffer[3] = hexTable[(hex32>>24)&0x0F];
	buffer[4] = hexTable[(hex32>>20)&0x0F];
	buffer[5] = hexTable[(hex32>>16)&0x0F];
	buffer[6] = hexTable[(hex32>>12)&0x0F];
	buffer[7] = hexTable[(hex32>>8)&0x0F];
	buffer[8] = hexTable[(hex32>>4)&0x0F];
	buffer[9] = hexTable[hex32&0x0F];
	buffer[10] = '\0';
	usart1PendingPrint(buffer);
}

void usart1PendingLongInt(uint32 x) {
	char digit[11];
	uint8 i;

	digit[10] = '\0';
	i = 9;
	do {
		digit[i] = '0' + x%10;
		x /= 10;
		i--;
	} while (x != 0);
	i++;
	usart1PendingPrint(&digit[i]);
}
