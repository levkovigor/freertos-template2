/**
 * Additional layer for hardware initialization.
 */

#ifndef STM32_BSP_HARDWARE_INIT_H_
#define STM32_BSP_HARDWARE_INIT_H_

#include <stdbool.h>

#include "../stm32/Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal.h"
#include "../stm32/Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_gpio.h"
#include "../stm32/Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_uart.h"
#include "../stm32/Inc/stm32h7xx_nucleo_144.h"
#include "utility/print.h"
/**
 * Declaration of global hardware handlers here.
 * Definition inside respective .cpp or .c files.
 */
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef uart6;
extern UART_HandleTypeDef uart2;
extern GPIO_InitTypeDef gpio_uart_init_struct;

extern bool debugAvailable;

//extern GPIO_InitTypeDef gpio_uart_init_struct2;
void MX_USART3_UART_Init(uint32_t baudRate);
void MX_USART2_UART_Init(void);
void MX_USART6_UART_Init(void);

#endif /* STM32_BSP_HARDWARE_INIT_H_ */
