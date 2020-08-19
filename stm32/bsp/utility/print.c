#include "utility/print.h"

uint8_t print_uart(const char * s, uint8_t uart_number) {
	uint8_t result;
	switch(uart_number) {
	case 2: result = print_uart2(s); break;
	case 3: result = print_uart3(s); break;
	case 6: result = print_uart6(s); break;
	default: return HAL_ERROR;
	}
	return result;
}

uint8_t print_uart2(const char *s) {
	char * data = 0;
	uint16_t dataOutputSize;
	processString(s,data,&dataOutputSize);
	char fixedData[dataOutputSize];
	strcpy(fixedData,data);
	uint8_t result = HAL_BUSY;
	while(result != HAL_OK) {
		result = HAL_UART_Transmit(&uart2, (uint8_t*)fixedData, dataOutputSize, 10000);
		switch(result) {
		case HAL_TIMEOUT:
		case HAL_ERROR:
		case HAL_OK:
			return result;
			break;
		case HAL_BUSY:
			break;
		default:
			return result;
			break;
		}
	}
	return result;
}


uint8_t print_uart3(const char *s) {
	uint16_t stringSize = strlen(s);
	char fixedSizeString[stringSize];
	strcpy(fixedSizeString,s);
	uint8_t result = HAL_BUSY;
	while(result != HAL_OK) {
		result = HAL_UART_Transmit(&huart3, (uint8_t*)fixedSizeString, stringSize, 10000);
		switch(result) {
		case HAL_TIMEOUT:
		case HAL_ERROR:
		case HAL_OK:
			return result;
			break;
		case HAL_BUSY:
			break;
		default:
			return result;
			break;
		}
	}
	return result;
}


uint8_t print_uart6(const char *s) {
	uint16_t stringSize = strlen(s);
	char fixedSizeString[stringSize];
	strcpy(fixedSizeString,s);
	uint8_t result = HAL_BUSY;
	while(result != HAL_OK) {
		result = HAL_UART_Transmit(&uart6, (uint8_t*)fixedSizeString, stringSize, 10000);
		switch(result) {
		case HAL_TIMEOUT:
		case HAL_ERROR:
		case HAL_OK:
			return result;
			break;
		case HAL_BUSY:
			break;
		default:
			return result;
			break;
		}
	}
	return result;
}


void processString(const char * inputString,char * outputString, uint16_t * outputSize) {
	uint16_t i = 0;

	while (*inputString != '\0') { /* Loop until end of string */
		inputString++;
		i++;
	}
	*outputSize = i;
	inputString = inputString - i;
	i = 0;

	while (*inputString != '\0') { /* Loop until end of string */
		*(outputString+i) = *inputString;
		inputString++;
		i++;
	}
	if(outputString) {} // remove compiler warning
}


void printChar(const char* character) {
	HAL_UART_Transmit(&huart3, (uint8_t *)character, 1 , 10000);
}

