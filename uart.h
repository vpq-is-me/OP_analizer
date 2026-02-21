#ifndef UART_H
#define UART_H


#include "main.h"

#define BAUDRATE 115200
#define UART2_CORE_CLK_FREQ APB1_CLOCK_FREQ

uint8_t USART2IsTxBusy(void);
uint8_t USART2IsTxCompleted(void);
void USART2init(void);
int USART2TransmitBuf(char* buf,uint32_t length);
#endif