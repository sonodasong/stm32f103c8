#include "user.h"

static OS_STK stack0[128];
static OS_STK stack1[128];
static OS_STK stack2[128];
static OS_STK stack3[128];

static void setApb2PrescalerDiv2(void);

int main(void) {
	/* when AHB = 72 MHz, SystemInit sets APB1 to 36 MHz */
	/* here manually set APB2 to 36 MHz as well */
	setApb2PrescalerDiv2();
	__disable_irq();
	SysTick_Config(SystemCoreClock / 100);

	OSInit();

	adc1Init();
	eepromInit();
	i2c1Init();
	pwmInit();
	spi1Init();
	usart1PendingInit();

	ledInit();
	mpu6050DMPInit();
	nrf24l01pInit();

	OSTaskCreate(mpu6050Task, (void*)0, &stack0[127], 0);
	OSTaskCreate(nrfRxTask, (void*)0, &stack1[127], 1);
	OSTaskCreate(ledTask, (void*)0, &stack2[127], 2);
	OSTaskCreate(blink, (void*)0, &stack3[127], 3);

	OSStart();

	return 0;
}

static void setApb2PrescalerDiv2(void) {
	/* AHB = 72 MHz */
	RCC -> CFGR &= ~RCC_CFGR_PPRE2;
	RCC -> CFGR |= RCC_CFGR_PPRE2_DIV2;
}
