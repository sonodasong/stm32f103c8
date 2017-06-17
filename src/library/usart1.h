#ifndef __USART1_H__
#define __USART1_H__

#include "define.h"

/* queue size is 2 ^ UART1_RX_Q_SIZE */
#define UART1_RX_Q_SIZE			2
/* buffer size is 2 ^ UART1_RX_SIZE */
#define UART1_RX_SIZE			4

#define UART1_RX_TIMEOUT		0xFFFFFFFF
#define UART1_RX_EOF			'\r'

#define UART1_TX_TIMEOUT		0xFFFFFFFF

void usart1Init(void);
INT8U usart1Read(char **str);
INT8U usart1Print(char *str);
INT8U usart1PrintLen(char *str, uint8 len);

#endif
