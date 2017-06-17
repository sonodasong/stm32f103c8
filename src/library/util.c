#include "util.h"

void ledInit(void) {
	/* enable gpio c clock*/
	RCC -> APB2ENR |= RCC_APB2ENR_IOPCEN;

	/* led pin, pc13, general purpose output push-pull */
	GPIOC -> CRH &= ~GPIO_CRH_MODE13_0;
	GPIOC -> CRH |= GPIO_CRH_MODE13_1;
	GPIOC -> CRH &= ~GPIO_CRH_CNF13_0;
	GPIOC -> CRH &= ~GPIO_CRH_CNF13_1;
}

void ledOn(void) {
	GPIOC -> BRR = GPIO_BRR_BR13;
}

void ledOff(void) {
	GPIOC -> BSRR = GPIO_BSRR_BS13;
}

void ledToggle(void) {
	if (GPIOC -> IDR & GPIO_IDR_IDR13) {
		GPIOC -> BRR = GPIO_BRR_BR13;
	} else {
		GPIOC -> BSRR = GPIO_BSRR_BS13;
	}
}
