#include "task.h"

void blink(void *pdata) {
	(void)pdata;
	while (1) {
		ledToggle();
		OSTimeDly(25);
	}
}

void serial(void *pdata) {
	char *str;
	uint16 i;

	(void)pdata;
	while (1) {
		usart1Read(&str);
		usart1Print(str);
		usart1Print("\r\n");
		if (str[0] == 't') {
			for (i = 0; i < 2000; i++) {
				eepromWrite(i % 64, i + 1);
				if (eepromRead(i % 64) != i + 1) {
					usart1Print("eeprom fail\r\n");
					break;
				}
			}
			if (i == 2000) {
				usart1Print("eeprom succeed\r\n");
			}
		} else if (str[0] == 'r') {
			usart1Printf("%04x\r\n", eepromRead(str[1] - '0'));
		} else if (str[0] == 'w') {
			eepromWrite(str[1] - '0', str[2] - '0');
		}
	}
}
