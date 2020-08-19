#include "TestTaskAt91.h"

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

extern "C" {
#include <sam9g20/utility/print.h>
}

int step = 0;

TestTaskAt91::TestTaskAt91(const char * printName, object_id_t objectId):
        SystemObject(objectId), printName(printName) {
	sif::debug << "TestTask ctor" << std::endl;
}

TestTaskAt91::~TestTaskAt91() {}

ReturnValue_t TestTaskAt91::performOperation(uint8_t operationCode) {
	sif::debug << this->printName <<": step " << step << std::endl;
	step++;
	if(step == 21474836487){
		step = 0;
	}
	return HasReturnvaluesIF::RETURN_OK;
}
