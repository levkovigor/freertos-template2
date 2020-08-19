#ifndef TESTTASKAT91_H_
#define TESTTASKAT91_H_


#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>

class TestTaskAt91 : public SystemObject, public ExecutableObjectIF {
public:
	TestTaskAt91(const char * printName, object_id_t objectId);
	virtual ~TestTaskAt91();

	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);

	const char * printName;
};

#endif /* TESTTASK_H_ */
