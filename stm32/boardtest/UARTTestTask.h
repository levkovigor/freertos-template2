/*
 * UARTTestTask.h
 *
 *  Created on: 13.10.2019
 */

#ifndef TEST_TESTTASK_UARTTESTTASK_H_
#define TEST_TESTTASK_UARTTESTTASK_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

extern "C" {
#include <stm32/bsp/utility/print.h>
}

class UARTTestTask: public SystemObject, public ExecutableObjectIF {
public:

	UARTTestTask(object_id_t objectId);
	virtual ~UARTTestTask();


	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);
private:


};

#endif /* TEST_TESTTASK_UARTTESTTASK_H_ */
