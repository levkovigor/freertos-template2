/*
 * TestTask.h
 *
 *  Created on: 16.07.2018
 *      Author: mohr
 */

#ifndef TESTTASKSTM32_H_
#define TESTTASKSTM32_H_


#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>

class TestTaskStm32 : public SystemObject, public ExecutableObjectIF {
public:
	TestTaskStm32(const char * printName, object_id_t objectId);
	virtual ~TestTaskStm32();

	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);

	const char * printName;
};

#endif /* TESTTASK_H_ */
