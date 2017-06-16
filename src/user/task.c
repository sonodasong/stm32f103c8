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

	(void)pdata;
	while (1) {
		usart1Read(&str);
		usart1Print(str);
	}
}
