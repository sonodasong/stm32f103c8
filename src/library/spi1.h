#ifndef __SPI1_H__
#define __SPI1_H__

#include "define.h"

#define SPI1_TIMEOUT		0xFFFFFFFF

void spi1Init(void);
INT8U spi1R(uint8 *buf, uint8 size);
INT8U spi1W(uint8 *buf, uint8 size);
INT8U spi1RW(uint8 *rxBuf, uint8 *txBuf, uint8 size);
INT8U spi1RegR(uint8 reg, uint8 *buf, uint8 size);
INT8U spi1RegW(uint8 reg, uint8 *buf, uint8 size);
INT8U spi1RegRW(uint8 reg, uint8 *rxBuf, uint8 *txBuf, uint8 size);

#endif
