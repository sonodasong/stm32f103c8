#include "adc1.h"

void adc1Init(void) {
	/* enable gpio a clock */
	RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;

	/* adc pin, adc1 ch1, pa2, input analog */
	GPIOA -> CRL &= ~GPIO_CRL_MODE2_0;
	GPIOA -> CRL &= ~GPIO_CRL_MODE2_1;
	GPIOA -> CRL &= ~GPIO_CRL_CNF2_0;
	GPIOA -> CRL &= ~GPIO_CRL_CNF2_1;

	/* enable adc1 clock */
	RCC -> APB2ENR |= RCC_APB2ENR_ADC1EN;

	/* APB2 = 36 MHz, adc input clock */
	RCC -> CFGR |= RCC_CFGR_ADCPRE_0;
	RCC -> CFGR &= ~RCC_CFGR_ADCPRE_1;

	/* adc1 configuration */
	ADC1 -> CR2 &= ~ADC_CR2_CONT;
	ADC1 -> CR2 &= ~ADC_CR2_ALIGN;
	ADC1 -> CR2 |= ADC_CR2_EXTTRIG;

	ADC1 -> CR2 |= ADC_CR2_EXTSEL_0;
	ADC1 -> CR2 |= ADC_CR2_EXTSEL_1;
	ADC1 -> CR2 |= ADC_CR2_EXTSEL_2;

	ADC1 -> SMPR2 |= ADC_SMPR2_SMP2_0;
	ADC1 -> SMPR2 &= ~ADC_SMPR2_SMP2_1;
	ADC1 -> SMPR2 &= ~ADC_SMPR2_SMP2_2;

	ADC1 -> SQR1 &= ~ADC_SQR1_L_0;
	ADC1 -> SQR1 &= ~ADC_SQR1_L_1;
	ADC1 -> SQR1 &= ~ADC_SQR1_L_2;
	ADC1 -> SQR1 &= ~ADC_SQR1_L_3;
	ADC1 -> SQR1 &= ~ADC_SQR1_L_0;

	ADC1 -> SQR3 &= ~ADC_SQR3_SQ1_0;
	ADC1 -> SQR3 |= ADC_SQR3_SQ1_1;
	ADC1 -> SQR3 &= ~ADC_SQR3_SQ1_2;
	ADC1 -> SQR3 &= ~ADC_SQR3_SQ1_3;
	ADC1 -> SQR3 &= ~ADC_SQR3_SQ1_4;

	/* enable adc1 */
	ADC1 -> CR2 |= ADC_CR2_ADON;
}

uint16 adc1Read(void) {
	ADC1 -> CR2 |= ADC_CR2_SWSTART;
	while (!(ADC1 -> SR & ADC_SR_EOC));
	return ADC1 -> DR;
}
