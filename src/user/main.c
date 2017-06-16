#include "user.h"

static OS_STK stack0[128];
static OS_STK stack1[128];

int main(void) {
	__disable_irq();
	SystemInit();
	SysTick_Config(SystemCoreClock / 100);

	OSInit();

	usart1Init();

	ledInit();

	OSTaskCreate(serial, (void*)0, &stack0[127], 0);
	OSTaskCreate(blink, (void*)0, &stack1[127], 1);

	OSStart();

	return 0;
}
