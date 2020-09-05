#include <logicalAddresses.h>
#include <fsfw/storagemanager/PoolManager.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <sam9g20/boardtest/AtmelTestTask.h>
#include <sam9g20/comIF/GpioDeviceComIF.h>

extern "C" {
#if defined(at91sam9g20_ek)
#include <led_ek.h>
#endif

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sam9g20/utility/portwrapper.h>

#include <hcc/demo/demo_sd.h>
#include <hal/Storage/NORflash.h>
#include <privlib/hal/include/hal/supervisor.h>
}


AtmelTestTask::AtmelTestTask(object_id_t object_id,
        TestInit::TestIdStruct id_struct): TestTask(object_id),
        testDataSet(id_struct) {
}

AtmelTestTask::~AtmelTestTask() {}

ReturnValue_t AtmelTestTask::performPeriodicAction() {
	performDataSetTesting(testMode);
	//performRunTimeStatsTesting();
	// This leads to a crash!
	//performExceptionTest();
	//sif::info << "Hello, I am alive!" << std::endl;
	return TestTask::performPeriodicAction();
}


ReturnValue_t AtmelTestTask::performOneShotAction() {
    //Stopwatch stopwatch;
    //performSDCardDemo();
#ifdef ISIS_OBC_G20
	//performIOBCTest();
#endif
    return TestTask::performOneShotAction();
}

ReturnValue_t AtmelTestTask::performActionA() {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t AtmelTestTask::performActionB() {
    return HasReturnvaluesIF::RETURN_OK;
}


void AtmelTestTask::performRunTimeStatsTesting() {
    counter ++;
    // This will move to the CoreController. CoreController will
    // generate information and keep a 64-bit idle counter by tracking
    // how often the 32-bit timer counter (running with 10kHz) has overflown.

    // The task run time counters should take a very long time to overflow.
    if(counter == 40) {
        // now measure cpu usage.
        uint16_t numberOfTasks = uxTaskGetNumberOfTasks();
        char runTimeStats[numberOfTasks * 80];
        vTaskGetRunTimeStats(runTimeStats);
        //      sif::debug << "FreeRTOS: " << numberOfTasks << " tasks running."
        //              << std::endl;
        //      sif::debug << "Current timer ticks: " << vGetCurrentTimerCounterValue()
        //              << std::endl;
        sif::info << "FreeRTOS Run Time Stats: " << std::endl;
        sif::info << "Absolute time in 10kHz ticks" << std::endl;
        printf("Task Name\tAbsolute Time\tRelative Time\r\n");
        printf("%s", runTimeStats);

        counter = 0;
    }
}

TestDataSet::TestDataSet(TestInit::TestIdStruct testStruct) :
     testBool(testStruct.p.testBoolId, this, PoolVariableIF::VAR_READ_WRITE),
     testUint8(testStruct.p.testUint8Id, this, PoolVariableIF::VAR_READ_WRITE),
     testUint16(testStruct.p.testUint16Id, this, PoolVariableIF::VAR_READ_WRITE),
     testUint32(testStruct.p.testUint32Id, this, PoolVariableIF::VAR_READ_WRITE),
     testFloatVector(testStruct.p.testFloatVectorId, this,
				PoolVariableIF::VAR_READ_WRITE) {
}

void AtmelTestTask::performExceptionTest() {
    TestDataSet* exceptTest = nullptr;
    exceptTest->read(20);
}


ReturnValue_t AtmelTestTask::performDataSetTesting(uint8_t testMode) {
    if(testMode == testModes::A) {
        ReturnValue_t result = testDataSet.read();
        if(result != RETURN_OK) {
            sif::debug << "Test Task: Operartion A, reading test data set failed "
                    "with code " << std::hex << result << std::dec << std::endl;
            return result;
        }
        testDataSet.testBool = true;
        testDataSet.testUint8 = 1;
        testDataSet.testUint16 = 9001;
        testDataSet.testUint32 = 999999;
        testDataSet.testFloatVector.value[0] = 1.294;
        testDataSet.testFloatVector.value[1] = -5.25;
        testDataSet.testBool.setValid(PoolVariableIF::VALID);
        testDataSet.testUint32.setValid(PoolVariableIF::VALID);
        result = testDataSet.commit();
        if(result != RETURN_OK) {
            sif::debug << "Test Task: Operartion A, comitting data set failed "
                    "with code " << std::hex << result << std::dec << std::endl;
            return result;
        }
    }
    else {
        ReturnValue_t result = testDataSet.read();
        if(result != RETURN_OK) {
            sif::debug << "Test Task: Operartion B, reading test data set failed "
                    "with code " << std::hex << result << std::dec << std::endl;
            return result;
        }
        testDataSet.testBool = false;
        testDataSet.testUint8 = 0;
        testDataSet.testUint16 = 0;
        testDataSet.testUint32 = 0;
        testDataSet.testFloatVector.value[0] = 0;
        testDataSet.testFloatVector.value[1] = 0;
        testDataSet.testBool.setValid(PoolVariableIF::INVALID);
        testDataSet.testUint32.setValid(PoolVariableIF::INVALID);
        result = testDataSet.commit();
        if(result != RETURN_OK) {
            sif::debug << "Test Task: Operartion B, comitting data set failed with "
                    "code " << std::hex << result << std::dec << std::endl;
            return result;
        }
    }
    return RETURN_OK;
}

void AtmelTestTask::performNewPoolManagerAccessTests() {
	uint16_t numberOfElements[1] = {1};
	uint16_t sizeofElements[1] = {10};
	PoolManager<1> testPool = PoolManager<1>(0, sizeofElements, numberOfElements);
	std::array<uint8_t, 20> testDataArray;
	std::array<uint8_t, 20> receptionArray;
	store_address_t testStoreId;
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;

	for(size_t i = 0; i < testDataArray.size(); i++) {
		testDataArray[i] = i;
	}
	size_t size = 10;

	result = testPool.addData(&testStoreId, testDataArray.data(), size);
	if(result != RETURN_OK) {
		sif::error << "Error when adding data" << std::endl;
	}

	auto accessorPair = testPool.modifyData(testStoreId);
	if(accessorPair.first != RETURN_OK) {
		sif::error << "Error when modifying data" << std::endl;
	}
	accessorPair.second.print();
	accessorPair.second.getDataCopy(receptionArray.data(), receptionArray.size());
	accessorPair.second.release();

	auto constAccess = testPool.getData(testStoreId);
	if(constAccess.first != RETURN_OK) {
		sif::error << "Error when modifying data" << std::endl;
	}
	accessorPair.second.print();
}

void AtmelTestTask::performSDCardDemo() {
    /* Demo for SD Card 0, SD card slot J35 at at91sam9g20-ek */
    int result = DEMO_SD_Basic(0);
    if(result != 0) {
        result = DEMO_SD_Basic(1);
    }
    if(result == 0) {
        sif::info << "AtmelTestTask: SD Card demo success" << std::endl;
    }
}

#ifdef ISIS_OBC_G20

void AtmelTestTask::performIOBCTest() {
    performSupervisorTest();
}

void AtmelTestTask::performNorflashTest() {
    int result  = NORflash_start();
    if(result == 0) {
        sif::info << "AtmelTestTask: Nor flash init success" << std::endl;
    }
    else {
        sif::info << "AtmelTestTask: Nor flash init failure" << std::endl;
    }

    result  = NORflash_test(824);
    if(result == 0) {
        sif::info << "AtmelTestTask: Nor flash test success" << std::endl;
    }
    else {
        sif::info << "AtmelTestTask: Nor flash test failure" << std::endl;
    }
}


void AtmelTestTask::performSupervisorTest() {

    sif::info << "Testing supervisor." << std::endl;
    Supervisor_start(NULL, 0);
    sif::info << "Supervisor started successfull!" << std::endl;
    uint8_t supervisorIndex = -1;
    supervisor_housekeeping_t houseKeeping;
    int result = Supervisor_getHousekeeping(&houseKeeping, supervisorIndex);
    sif::info << "Supervisor Housekeeping Call Result: " << result << std::endl;
    printf("Supervisor Uptime       : %03d:%02d:%02d \n\r",
            (int)(houseKeeping.fields.supervisorUptime / 3600),
            (int)(houseKeeping.fields.supervisorUptime % 3600) / 60,
            (int)(houseKeeping.fields.supervisorUptime % 3600) % 60);
    printf("IOBC Uptime             : %03d:%02d:%02d \n\r",
            (int)(houseKeeping.fields.iobcUptime / 3600),
            (int)(houseKeeping.fields.iobcUptime % 3600) / 60,
            (int)(houseKeeping.fields.iobcUptime % 3600) % 60);
    printf("IOBC Power Cycle Count  : %d \n\r",
            (int)(houseKeeping.fields.iobcResetCount));

    supervisor_enable_status_t* temporaryEnable = &(houseKeeping.fields.enableStatus);
    printf("\n\r Supervisor Enable Status \n\r");
    printf("Power OBC               : %d \n\r", temporaryEnable->fields.powerObc);
    printf("Power RTC               : %d \n\r", temporaryEnable->fields.powerRtc);
    printf("Is in Supervisor Mode   : %d \n\r", temporaryEnable->fields.isInSupervisorMode);
    printf("Busy RTC                : %d \n\r", temporaryEnable->fields.busyRtc);
    printf("Power off RTC           : %d \n\r", temporaryEnable->fields.poweroffRtc);

    int16_t adcValue[SUPERVISOR_NUMBER_OF_ADC_CHANNELS] = {0};
    Supervisor_calculateAdcValues(&houseKeeping, adcValue);
    printf("\n\r Analog to Digital Channels [Update Flag: 0x%02X] \n\r", houseKeeping.fields.adcUpdateFlag);
    printf("_temperature_measurement: %04d | %d C \n\r\n\r", houseKeeping.fields.adcData[_temperature_measurement], adcValue[_temperature_measurement]);

    printf("_voltage_measurement_3v3in: %04d | %d mV \n\r", houseKeeping.fields.adcData[_voltage_measurement_3v3in], adcValue[_voltage_measurement_3v3in]);
    printf("_voltage_reference_2v5    : %04d | %d mV \n\r", houseKeeping.fields.adcData[_voltage_reference_2v5], adcValue[_voltage_reference_2v5]);
    printf("_voltage_measurement_rtc  : %04d | %d mV \n\r", houseKeeping.fields.adcData[_voltage_measurement_rtc], adcValue[_voltage_measurement_rtc]);
    printf("_voltage_measurement_3v3  : %04d | %d mV \n\r", houseKeeping.fields.adcData[_voltage_measurement_3v3], adcValue[_voltage_measurement_3v3]);
    printf("_voltage_measurement_1v8  : %04d | %d mV \n\r", houseKeeping.fields.adcData[_voltage_measurement_1v8], adcValue[_voltage_measurement_1v8]);
    printf("_voltage_measurement_1v0  : %04d | %d mV \n\r\n\r", houseKeeping.fields.adcData[_voltage_measurement_1v0], adcValue[_voltage_measurement_1v0]);

    printf("_current_measurement_3v3  : %04d | %d mA \n\r", houseKeeping.fields.adcData[_current_measurement_3v3], adcValue[_current_measurement_3v3]);
    printf("_current_measurement_1v8  : %04d | %d mA \n\r", houseKeeping.fields.adcData[_current_measurement_1v8], adcValue[_current_measurement_1v8]);
    printf("_current_measurement_1v0  : %04d | %d mA \n\r\n\r", houseKeeping.fields.adcData[_current_measurement_1v0], adcValue[_current_measurement_1v0]);
    sif::info << "Supervisor test successfull!" << std::endl;
}



#endif
