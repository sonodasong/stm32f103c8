#ifndef __USART1_PENDING_H__
#define __USART1_PENDING_H__

#include "define.h"

void usart1PendingInit(void);
void usart1PendingPrint(char *str);
void usart1PendingPrintLen(char *str, uint8 len);
void usart1PendingHex8(uint8 hex8);
void usart1PendingHex16(uint16 hex16);
void usart1PendingHex32(uint32 hex32);
void usart1PendingLongInt(uint32 x);

#endif
