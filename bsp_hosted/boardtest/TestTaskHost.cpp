#include "TestTaskHost.h"
#include <fsfw/serviceinterface/ServiceInterface.h>


TestTaskHost::TestTaskHost(object_id_t object_id, bool periodicPrint):
	TestTask(object_id), periodicPrint(periodicPrint) {
}

TestTaskHost::~TestTaskHost() {
}

ReturnValue_t TestTaskHost::performOperation(uint8_t operationCode) {
    ReturnValue_t result = TestTask::performOperation(operationCode);
    if(periodicPrint) {
        sif::info << "TestTaskHost::performOperation: Hello, I am alive."
        		<< std::endl;
    }

	return result;
}

ReturnValue_t TestTaskHost::performOneShotAction() {
	//testBaseAsFriend();
	return TestTask::performOneShotAction();
}

ReturnValue_t TestTaskHost::performPeriodicAction() {
    return TestTask::performPeriodicAction();
}

void TestTaskHost::testBaseAsFriend() {
	Child A;
	A.childPublicInterface();
	A.hackToAccessBasePrivate();
}

void TestTaskHost::sharedPtrPoolTest() {

}
