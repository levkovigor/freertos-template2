/*
 * UART2TestTask.h
 *
 *  Created on: 20.10.2019
 *      Author: jakob
 */

#ifndef _UART2TESTTASK_H_
#define _UART2TESTTASK_H_

#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/objectmanager/SystemObjectIF.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <fsfwconfig/objects/systemObjectList.h>

extern "C" {
#include "UART.h"
}

#include <array>
#include <vector>

class UART2TestTask: public SystemObject,
		public ExecutableObjectIF,
		public HasReturnvaluesIF
{
public:
	UART2TestTask(const char * printName, object_id_t objectId);
	virtual ~UART2TestTask();

	void setRxTimeout(uint32_t timeoutMs);

	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);
	const char * printName;
private:
	static const uint32_t RX_TIMEOUT_MS = 200;
	static const uint32_t MAX_REPLY_LEN = 128;

	void performReadSendTest();
	void performSendTest();

	void performNonBlockingOperation();
	void performNonBlockingWriteRead();
	void checkComStatus();
	void checkWriteStatus();
	void checkReadStatus();

	static void UartCallback(SystemContext context,  xSemaphoreHandle semaphore);

	unsigned int readSize1 = 4;
	unsigned char readData1[16] = { 0 }, writeData1[16] = { 0 };

	enum UartMode: uint8_t {
		SEND_TEST,
		READ_SEND_TEST,
		NON_BLOCKING
	};

	enum UartState: uint8_t {
		WRITE,
		WAIT_REPLY
	};

	UartMode uartMode;
	UartState uartState;
	UARTbus uartBus2 = bus2_uart;
	UARTconfig configBus2;
	std::array<uint8_t, 3> writeData { 1, 2, 3 };
	std::vector<uint8_t> readData { 0 };
	uint16_t replySize = 0;
	UARTtransferStatus currentWriteStatus = UARTtransferStatus::done_uart;
	UARTtransferStatus currentReadStatus = UARTtransferStatus::done_uart;
	BinarySemaphore writeSemaphore;
	BinarySemaphore readSemaphore;
	int retValInt = 0;

};

#endif /* _UART2TESTTASK_H_ */
