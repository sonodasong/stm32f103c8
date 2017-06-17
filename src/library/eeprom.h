#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "define.h"

void eepromInit(void);
uint8 eepromRead(uint16 addr);
void eepromWrite(uint16 addr, uint8 data);

#endif
