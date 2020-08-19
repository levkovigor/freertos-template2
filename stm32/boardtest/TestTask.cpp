/*
 * TestTask.cpp
 *
 *  Created on: 16.07.2018
 *      Author: mohr
 */

#include <stm32/boardtest/TestTask.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

extern "C" {
#include <stm32/bsp/utility/print.h>
}

TestTaskStm32::TestTaskStm32(const char * printName, object_id_t objectId): SystemObject(objectId), printName(printName) {
	print_uart3("Task ctor\n");

}

TestTaskStm32::~TestTaskStm32() {
}

ReturnValue_t TestTaskStm32::performOperation(uint8_t operationCode) {
	ServiceInterfaceStream blaStream("BLA");

	std::ostream bla(blaStream.rdbuf());

	bla << this->printName <<": step " << (int) operationCode << std::endl;

	return HasReturnvaluesIF::RETURN_OK;
}
