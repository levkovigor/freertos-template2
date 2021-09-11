#include "PVCHTestTask.h"

#include "devices/logicalAddresses.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/globalfunctions/arrayprinter.h"

PVCHTestTask::PVCHTestTask(object_id_t objectId): SystemObject(objectId), gpioExpander(i2cMux) {
}

PVCHTestTask::~PVCHTestTask() {

}

ReturnValue_t PVCHTestTask::initialize() {
    ReturnValue_t result = i2cMux.intialize();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("PCA9554 I2C Demultiplexer Init failed\n");
    }

    switch(testMode) {
    case(TestModes::MAX_7301): {
        result = gpioExpander.initialize();
        uint8_t readByte = 0;
        result = gpioExpander.read(Max7301::CmdAddr::P_CONF_7_TO_4, readByte);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            sif::printWarning("Max7301: Reading port config register failed\n");
        }
        sif::printInfo("Max7301: Read byte from registers 0x%02x: 0x%02x\n",
                Max7301::CmdAddr::P_CONF_7_TO_4, readByte);
    }
    }
    return result;
}



ReturnValue_t PVCHTestTask::performOperation(uint8_t opCode) {
    return HasReturnvaluesIF::RETURN_OK;
}

