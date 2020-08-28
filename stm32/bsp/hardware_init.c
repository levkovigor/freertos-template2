#include <stm32/bsp/hardware_init.h>

/**
 * ST-LINK UART3
 * CN5 pins on board
 */
UART_HandleTypeDef huart3;
UART_HandleTypeDef uart2;
UART_HandleTypeDef uart6;
GPIO_InitTypeDef gpio_uart_init_struct;


/**
 * UART 6
 * CN10 pins on board
 */

bool debugAvailable;
void MX_USART3_UART_Init(uint32_t baudRate)
{

	__HAL_RCC_USART3_CONFIG(RCC_USART3CLKSOURCE_HSI);
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_USART3_CLK_ENABLE();
	/*Configure GPIO pins : PD8 PD9 */
	gpio_uart_init_struct.Pin = GPIO_PIN_8|GPIO_PIN_9;
	gpio_uart_init_struct.Mode = GPIO_MODE_AF_PP;
	gpio_uart_init_struct.Pull = GPIO_NOPULL;
	gpio_uart_init_struct.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_uart_init_struct.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOD, &gpio_uart_init_struct);

	int result;
	huart3.Instance = USART3;
	huart3.Init.BaudRate = baudRate;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart3.Init.Prescaler = UART_PRESCALER_DIV1;
	huart3.Init.FIFOMode = UART_FIFOMODE_DISABLE;
	huart3.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
	huart3.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
	huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	// we can't do error handling (simple print out first) here because UART3 is our print interface
	result = HAL_UART_Init(&huart3);
	if(result==HAL_OK) {
		print_uart3("\rUART3 configured successfully !\r\n\0");
		debugAvailable = true;
	}
}

/**
 * UART 6 used for testing of devices like GPS
 */
void MX_USART6_UART_Init(void) {
	__HAL_RCC_USART6_CONFIG(RCC_USART6CLKSOURCE_HSI);
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_USART6_CLK_ENABLE();
	/*Configure GPIO pins : PC6 PC7 */
	gpio_uart_init_struct.Pin = GPIO_PIN_6|GPIO_PIN_7;
	gpio_uart_init_struct.Mode = GPIO_MODE_AF_PP;
	gpio_uart_init_struct.Pull = GPIO_NOPULL;
	gpio_uart_init_struct.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_uart_init_struct.Alternate = GPIO_AF7_USART6;
	HAL_GPIO_Init(GPIOC, &gpio_uart_init_struct);
	/* PC5 */
	gpio_uart_init_struct.Pin = GPIO_PIN_5;
	gpio_uart_init_struct.Mode = GPIO_MODE_AF_PP;
	gpio_uart_init_struct.Pull = GPIO_NOPULL;
	gpio_uart_init_struct.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_uart_init_struct.Alternate = GPIO_AF7_USART6;
	HAL_GPIO_Init(GPIOC, &gpio_uart_init_struct);
	/* PG9 */
	gpio_uart_init_struct.Pin = GPIO_PIN_9;
	gpio_uart_init_struct.Mode = GPIO_MODE_AF_PP;
	gpio_uart_init_struct.Pull = GPIO_NOPULL;
	gpio_uart_init_struct.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_uart_init_struct.Alternate = GPIO_AF7_USART6;
	HAL_GPIO_Init(GPIOG, &gpio_uart_init_struct);

	int result;
	uart6.Instance = USART6;
	uart6.Init.BaudRate = 9600;
	uart6.Init.WordLength = UART_WORDLENGTH_8B;
	uart6.Init.StopBits = UART_STOPBITS_1;
	uart6.Init.Parity = UART_PARITY_NONE;
	uart6.Init.Mode = UART_MODE_TX_RX;
	uart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart6.Init.OverSampling = UART_OVERSAMPLING_16;
	uart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	uart6.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
	uart6.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
	uart6.Init.Prescaler = UART_PRESCALER_DIV1;
	uart6.Init.FIFOMode = UART_FIFOMODE_DISABLE;
	uart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	result = HAL_UART_Init(&uart6);
	if(result == HAL_OK && debugAvailable == true){
		print_uart3("UART6 configured successfully !\r\n\0");
	}
	else if(debugAvailable == true){
		print_uart3("UART6 configuration failed !\r\n\0");
	}
}

/**
 * UART 2
 */
void MX_USART2_UART_Init(void) {
	__HAL_RCC_USART2_CONFIG(RCC_USART2CLKSOURCE_HSI);
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_USART2_CLK_ENABLE();
	/*Configure GPIO pins : PA3  */
	gpio_uart_init_struct.Pin = GPIO_PIN_3;
	gpio_uart_init_struct.Mode = GPIO_MODE_AF_PP;
	gpio_uart_init_struct.Pull = GPIO_NOPULL;
	gpio_uart_init_struct.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_uart_init_struct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &gpio_uart_init_struct);
	/*Configure GPIO pins : PD5 PD6  */
	gpio_uart_init_struct.Pin = GPIO_PIN_5|GPIO_PIN_6;
	gpio_uart_init_struct.Mode = GPIO_MODE_AF_PP;
	gpio_uart_init_struct.Pull = GPIO_NOPULL;
	gpio_uart_init_struct.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_uart_init_struct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOD, &gpio_uart_init_struct);
	int result;
	uart2.Instance = USART2;
	uart2.Init.BaudRate = 9600;
	uart2.Init.WordLength = UART_WORDLENGTH_8B;
	uart2.Init.StopBits = UART_STOPBITS_1;
	uart2.Init.Parity = UART_PARITY_NONE;
	uart2.Init.Mode = UART_MODE_TX_RX;
	uart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart2.Init.OverSampling = UART_OVERSAMPLING_16;
	uart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	uart2.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
	uart2.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
	uart2.Init.Prescaler = UART_PRESCALER_DIV1;
	uart2.Init.FIFOMode = UART_FIFOMODE_DISABLE;
	uart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	result = HAL_UART_Init(&uart2);
	if(result == HAL_OK && debugAvailable == true){
		print_uart2("UART2 configured successfully !\r\n\0");
	}
	else if(debugAvailable == true){
		print_uart2("UART2 configuration failed !\r\n\0");
	}
}
