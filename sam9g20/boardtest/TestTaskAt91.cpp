#include "TestTaskAt91.h"

#include <fsfw/serviceinterface/ServiceInterface.h>

extern "C" {
#include <sam9g20/utility/print.h>
}

int step = 0;

TestTaskAt91::TestTaskAt91(const char * printName, object_id_t objectId):
        SystemObject(objectId), printName(printName) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
	sif::debug << "TestTask ctor" << std::endl;
#else
#endif
}

TestTaskAt91::~TestTaskAt91() {}

ReturnValue_t TestTaskAt91::performOperation(uint8_t operationCode) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
	sif::debug << this->printName <<": step " << step << std::endl;
#else
#endif
	step++;
	if(step == 1474836487){
		step = 0;
	}
	return HasReturnvaluesIF::RETURN_OK;
}
