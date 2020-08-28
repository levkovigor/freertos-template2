#include <stm32/boardtest/UARTTestTask.h>

extern "C" {
#include "hardware_init.h"
#include "stm32h7xx_hal_uart.h"
}


static const uint16_t testBufferSize = 8;
static uint8_t pBuffer[testBufferSize+1];

UARTTestTask::UARTTestTask(object_id_t objectId): SystemObject(objectId) {
	sif::info << "UARTTestTask object created!" << std::endl;
	/// Size of the receive buffer used

	/// Receive buffer.

}

ReturnValue_t UARTTestTask::performOperation(uint8_t operationCode){
	char data[40] = "Hello, this is a STM32 UART Test!\0";
	uint8_t result = 0;
	sif::info << data << std::endl;

	//print_uart3("Testing UART 3 (ST-LINK) Transmit\r\n");
	//print_uart2("Testing UART 2 Transmit\r\n");
	if(HAL_UART_IsDataAvailable(&uart2) == true) {
		sif::info << "UART 2 has something !" << std::endl;
		result = HAL_UART_Receive(&uart2,pBuffer, testBufferSize, 5);
		sif::debug << "UART 2 receive returned: " << (int)result << std::endl;
		if(result == HAL_TIMEOUT) {
			HAL_UART_SetRxReady(&uart2);
			HAL_UART_UnlockRx(&uart2);
		}
	}
	if(result == HAL_OK) {
		sif::info << "UART 2 received: "<< pBuffer << std::endl;
	}
	//print_uart6((const char *)pBuffer);
	//info << "UART 2 received: "<< pBuffer << std::endl; //does not really work properly
	return HasReturnvaluesIF::RETURN_OK;
}

UARTTestTask::~UARTTestTask() {
}
