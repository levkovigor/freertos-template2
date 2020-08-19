#ifndef PRINT_H_
#define PRINT_H_
#include <stdint.h>
#include <string.h>
#include "hardware_init.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"

/**
 *
 * @param s
 * @param uart_number Default uart_number is 3 to print to ST-Link connector, but other uarts can also be specified
 */
uint8_t print_uart(const char * s, uint8_t uart_number);
uint8_t print_uart3(const char *s);
uint8_t print_uart2(const char *s);
uint8_t print_uart6(const char *s);
void processString(const char * inputString,char * outputString, uint16_t * outputSize);
/*void printPtr(void * pointer);*/

void printChar(const char* character);

#endif /* PRINT_H_ */
