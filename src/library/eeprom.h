#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "define.h"

#define EEPROM_SIZE				64

#define EEPROM_PAGE_NUM			4
#define EEPROM_PAGE_0_BASE		0x0801F000
#define EEPROM_PAGE_1_BASE		0x0801F400
#define EEPROM_PAGE_2_BASE		0x0801F800
#define EEPROM_PAGE_3_BASE		0x0801FC00

void eepromInit(void);
uint16 eepromRead(uint16 addr);
void eepromWrite(uint16 addr, uint16 data);

#endif
