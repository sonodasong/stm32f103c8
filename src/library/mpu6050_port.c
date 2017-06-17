#include "i2c1.h"
#include "mpu6050_port.h"

static void (*mpu6050HandlerPtr)(void);

static uint16 mpu6050ReverseWord(uint16 data);

int8 readBit(uint8 devAddr, uint8 regAddr, uint8 bitNum, uint8 *data) {
	uint8 b;
	int8 count;

	count = readByte(devAddr, regAddr, &b);
	*data = b & (1 << bitNum);
	return count;
}

int8 readBitW(uint8 devAddr, uint8 regAddr, uint8 bitNum, uint16 *data) {
	uint16 b;
	int8 count;

	count = readWord(devAddr, regAddr, &b);
	*data = b & (1 << bitNum);
	return count;
}

int8 readBits(uint8 devAddr, uint8 regAddr, uint8 bitStart, uint8 length, uint8 *data) {
	uint8 b;
	int8 count;
	uint8 mask;

	if ((count = readByte(devAddr, regAddr, &b)) != 0) {
		mask = ((1 << length) - 1) << (bitStart - length + 1);
		b &= mask;
		b >>= (bitStart - length + 1);
		*data = b;
	}
	return count;
}

int8 readBitsW(uint8 devAddr, uint8 regAddr, uint8 bitStart, uint8 length, uint16 *data) {
	uint16 w;
	int8 count;
	uint16 mask;

	if ((count = readWord(devAddr, regAddr, &w)) != 0) {
		mask = ((1 << length) - 1) << (bitStart - length + 1);
		w &= mask;
		w >>= (bitStart - length + 1);
		*data = w;
	}
	return count;
}

int8 readByte(uint8 devAddr, uint8 regAddr, uint8 *data) {
	return readBytes(devAddr, regAddr, 1, data);
}

int8 readWord(uint8 devAddr, uint8 regAddr, uint16 *data) {
	return readWords(devAddr, regAddr, 1, data);
}

int8 readBytes(uint8 devAddr, uint8 regAddr, uint8 length, uint8 *data) {
	int8 count;

	i2c1RegR(devAddr, regAddr, data, length);
	if (i2c1RegRCount() == length) {
		count = length & 0x7F;
	} else {
		count = -1;
	}
	return count;
}

int8 readWords(uint8 devAddr, uint8 regAddr, uint8 length, uint16 *data) {
	int8 count;
	uint8 i;

	i2c1RegR(devAddr, regAddr, (uint8 *)data, length * 2);
	if (i2c1RegRCount() == length * 2) {
		count = length & 0x7F;
		for (i = 0; i < length; i++) {
			data[i] = mpu6050ReverseWord(data[i]);
		}
	} else {
		count = -1;
	}
	return count;
}

boolean writeBit(uint8 devAddr, uint8 regAddr, uint8 bitNum, uint8 data) {
	uint8 b;

	readByte(devAddr, regAddr, &b);
	b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
	return writeByte(devAddr, regAddr, b);
}

boolean writeBitW(uint8 devAddr, uint8 regAddr, uint8 bitNum, uint16 data) {
	uint16 w;

	readWord(devAddr, regAddr, &w);
	w = (data != 0) ? (w | (1 << bitNum)) : (w & ~(1 << bitNum));
	return writeWord(devAddr, regAddr, w);
}

boolean writeBits(uint8 devAddr, uint8 regAddr, uint8 bitStart, uint8 length, uint8 data) {
	uint8 b;

	if (readByte(devAddr, regAddr, &b) != 0) {
		uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
		data <<= (bitStart - length + 1);
		data &= mask;
		b &= ~(mask);
		b |= data;
		return writeByte(devAddr, regAddr, b);
	} else {
		return FALSE;
	}
}

boolean writeBitsW(uint8 devAddr, uint8 regAddr, uint8 bitStart, uint8 length, uint16 data) {
	uint16 w;

	if (readWord(devAddr, regAddr, &w) != 0) {
		uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
		data <<= (bitStart - length + 1);
		data &= mask;
		w &= ~(mask);
		w |= data;
		return writeWord(devAddr, regAddr, w);
	} else {
		return FALSE;
	}
}

boolean writeByte(uint8 devAddr, uint8 regAddr, uint8 data) {
	return writeBytes(devAddr, regAddr, 1, &data);
}

boolean writeWord(uint8 devAddr, uint8 regAddr, uint16 data) {
	return writeWords(devAddr, regAddr, 1, &data);
}

boolean writeBytes(uint8 devAddr, uint8 regAddr, uint8 length, uint8 *data) {
	boolean b;

	i2c1RegW(devAddr, regAddr, data, length);
	if (i2c1RegWCount() == length) {
		b = TRUE;
	} else {
		b = FALSE;
	}
	return b;
}

boolean writeWords(uint8 devAddr, uint8 regAddr, uint8 length, uint16 *data) {
	boolean b;
	uint8 i;

	for (i = 0; i < length; i++) {
		data[i] = mpu6050ReverseWord(data[i]);
	}
	i2c1RegW(devAddr, regAddr, (uint8 *)data, length * 2);
	if (i2c1RegWCount() == length * 2) {
		b = TRUE;
	} else {
		b = FALSE;
	}
	return b;
}

void mpu6050IRQInit(void (*handlerPtr)(void)) {
	mpu6050HandlerPtr = handlerPtr;

	/* enable gpio a clock */
	RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;

	/* mpu irq pin, pa0, input pull-down */
	GPIOA -> CRL &= ~GPIO_CRL_MODE0_0;
	GPIOA -> CRL &= ~GPIO_CRL_MODE0_1;
	GPIOA -> CRL &= ~GPIO_CRL_CNF0_0;
	GPIOA -> CRL |= GPIO_CRL_CNF0_1;
	GPIOA -> BRR = GPIO_BRR_BR0;

	/* enable afio clock */
	RCC -> APB2ENR |= RCC_APB2ENR_AFIOEN;

	/* disable interrupt */
	mpu6050IRQDisable();

	/* gpio a for exti0 */
	AFIO -> EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;
	AFIO -> EXTICR[0] |= AFIO_EXTICR1_EXTI0_PA;

	/* set up interrupt as rising trigger */
	EXTI -> RTSR |= EXTI_RTSR_TR0;
	EXTI -> FTSR &= ~EXTI_FTSR_TR0;

	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI0_IRQn);
}

void EXTI0_IRQHandler(void) {
	OSIntEnter();
	mpu6050IRQClear();
	mpu6050HandlerPtr();
	OSIntExit();
}

static uint16 mpu6050ReverseWord(uint16 data) {
	uint8 temp;

	temp = (uint8)data;
	data = data >> 8;
	data |= temp << 8;
	return data;
}
