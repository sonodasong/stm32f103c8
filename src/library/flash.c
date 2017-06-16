#include "flash.h"

void flashErase(uint32 addr) {
	while (FLASH -> SR & FLASH_SR_BSY);
	if (FLASH -> CR & FLASH_CR_LOCK) {
		FLASH -> KEYR = FLASH_KEY1;
		FLASH -> KEYR = FLASH_KEY2;
	}
	FLASH -> CR |= FLASH_CR_PER;
	FLASH -> AR = addr & FLASH_ERASE_ADDR_MASK;
	FLASH -> CR |= FLASH_CR_STRT;
	while (FLASH -> SR & FLASH_SR_BSY);
	FLASH -> CR &= ~FLASH_CR_PER;
}

void flashProgram(uint32 addr, uint16 data) {
	while (FLASH -> SR & FLASH_SR_BSY);
	if (FLASH -> CR & FLASH_CR_LOCK) {
		FLASH -> KEYR = FLASH_KEY1;
		FLASH -> KEYR = FLASH_KEY2;
	}
	addr &= FLASH_PROGRAM_ADDR_MASK;
	if (flashRead(addr) == FLASH_ERASE_VALUE) {
		FLASH -> CR |= FLASH_CR_PG;
		*((uint16 *) addr) = data;
		while (FLASH -> SR & FLASH_SR_BSY);
		FLASH -> CR &= ~FLASH_CR_PG;
	}
}
