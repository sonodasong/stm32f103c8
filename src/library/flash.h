#ifndef __FLASH_H__
#define __FLASH_H__

#include "define.h"

#define FLASH_KEY1					0x45670123
#define FLASH_KEY2					0xCDEF89AB

#define FLASH_PROGRAM_ADDR_MASK		0xFFFFFFFE
#define FLASH_ERASE_ADDR_MASK		0xFFFFFC00
#define FLASH_ERASE_VALUE			0xFFFF

#define flashRead(addr)		(*((uint16 *) (addr)))

void flashErase(uint32 addr);
void flashProgram(uint32 addr, uint16 data);

#endif
