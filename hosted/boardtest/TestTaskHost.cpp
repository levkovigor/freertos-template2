#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <hosted/boardtest/TestTaskHost.h>

TestTaskHost::TestTaskHost(object_id_t object_id):
	TestTask(object_id) {
}

TestTaskHost::~TestTaskHost() {
}

ReturnValue_t TestTaskHost::performOperation(uint8_t operationCode) {
    ReturnValue_t result = TestTask::performOperation(operationCode);
//    sif::info << "TestTaskLinux::performOperation: Hello, I am alive."
//    		<< std::endl;
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
