#include "flash.h"
#include "eeprom.h"

static uint32 eepromPageBase[EEPROM_PAGE_NUM] = {
	EEPROM_PAGE_0_BASE, EEPROM_PAGE_1_BASE, EEPROM_PAGE_2_BASE, EEPROM_PAGE_3_BASE
};
static uint32 eepromReadAddr = EEPROM_PAGE_0_BASE;
static uint8 eepromNextPageIndex = 1;
static uint32 eepromPageUpdateFlag[(EEPROM_SIZE >> 5) + 1] = {0};

static void eepromPageUpdate(void);

void eepromInit(void) {
	uint8 i;
	uint32 baseAddr;
	uint32 temp;

	for (i = 0; i < EEPROM_PAGE_NUM; i++) {
		baseAddr = eepromPageBase[i];
		temp = baseAddr + (~FLASH_ERASE_ADDR_MASK) - 3;
		if (flashRead(temp) != FLASH_ERASE_VALUE) {
			eepromReadAddr = temp;
			eepromNextPageIndex = (i + 1) % EEPROM_PAGE_NUM;
			temp = eepromPageBase[eepromNextPageIndex];
			if (flashRead(temp) != FLASH_ERASE_VALUE) {
				flashErase(temp);
			}
			break;
		} else if (flashRead(baseAddr) != FLASH_ERASE_VALUE) {
			temp = eepromPageBase[(i + EEPROM_PAGE_NUM - 1) % EEPROM_PAGE_NUM] + (~FLASH_ERASE_ADDR_MASK) - 3;
			if (flashRead(temp) != FLASH_ERASE_VALUE) {
				eepromReadAddr = temp;
				eepromNextPageIndex = i;
				flashErase(baseAddr);
			} else {
				while (flashRead(baseAddr) != FLASH_ERASE_VALUE) {
					baseAddr += 4;
				}
				eepromReadAddr = baseAddr - 4;
				eepromNextPageIndex = (i + 1) % EEPROM_PAGE_NUM;
			}
			break;
		}
	}
}

uint16 eepromRead(uint16 addr) {
	uint32 baseAddr;
	uint32 readAddr;

	baseAddr = eepromReadAddr & FLASH_ERASE_ADDR_MASK;
	for (readAddr = eepromReadAddr; readAddr >= baseAddr; readAddr -= 4) {
		if (flashRead(readAddr) == addr) {
			return flashRead(readAddr + 2);
		}
	}
	return 0;
}

void eepromWrite(uint16 addr, uint16 data) {
	uint32 baseAddr;

	if (eepromRead(addr) == data) {
		return;
	}
	baseAddr = eepromReadAddr & FLASH_ERASE_ADDR_MASK;
	if (((eepromReadAddr + 4) & FLASH_ERASE_ADDR_MASK) > baseAddr) {
		eepromPageUpdate();
		baseAddr = eepromReadAddr & FLASH_ERASE_ADDR_MASK;
	}
	if ((eepromReadAddr != baseAddr) || (flashRead(baseAddr) != FLASH_ERASE_VALUE)) {
		eepromReadAddr += 4;
	}
	baseAddr = eepromReadAddr;
	flashProgram(baseAddr, addr);
	flashProgram(baseAddr + 2, data);
}

static void eepromPageUpdate(void) {
	uint32 baseAddr;
	uint16 addr;
	uint32 temp;

	for (temp = 0; temp <= (EEPROM_SIZE >> 5); temp++) {
		eepromPageUpdateFlag[temp] = 0;
	}
	temp = eepromReadAddr;
	baseAddr = temp & FLASH_ERASE_ADDR_MASK;
	eepromReadAddr = eepromPageBase[eepromNextPageIndex];
	eepromNextPageIndex = (eepromNextPageIndex + 1) % EEPROM_PAGE_NUM;
	while (temp >= baseAddr) {
		addr = flashRead(temp);
		if (addr < EEPROM_SIZE) {
			if ((eepromPageUpdateFlag[addr >> 5] & (0x01 << (addr & 0x001F))) == 0) {
				eepromPageUpdateFlag[addr >> 5] |= 0x01 << (addr & 0x001F);
				flashProgram(eepromReadAddr, addr);
				flashProgram(eepromReadAddr + 2, flashRead(temp + 2));
				eepromReadAddr += 4;
			}
		}
		temp -= 4;
	}
	eepromReadAddr -= 4;
	flashErase(baseAddr);
}
