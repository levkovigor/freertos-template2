#include <fsfw/storagemanager/PoolManager.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/timemanager/Stopwatch.h>

#include <sam9g20/boardtest/AtmelTestTask.h>
#include <sam9g20/comIF/GpioDeviceComIF.h>
#include <sam9g20/memory/SDCardAccess.h>
#include <sam9g20/memory/SDCardHandler.h>

extern "C" {
#if defined(AT91SAM9G20_EK)
#include <led_ek.h>
#endif

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sam9g20/utility/portwrapper.h>
#include <at91/utility/hamming.h>
#include <hcc/demo/demo_sd.h>
#include <sam9g20/memory/SDCardApi.h>

#ifdef ISIS_OBC_G20
#include <hal/Storage/NORflash.h>
#include <sam9g20/common/FRAMApi.h>
#include <hal/supervisor.h>
#endif

}

#include <cstring>


AtmelTestTask::AtmelTestTask(object_id_t object_id): TestTask(object_id) {
    countdown.setTimeout(3000);
}

AtmelTestTask::~AtmelTestTask() {}

ReturnValue_t AtmelTestTask::performPeriodicAction() {
	//performDataSetTesting(testMode);
	// This leads to a crash!
	//performExceptionTest();
#ifdef ISIS_OBC_G20
#endif

	//sif::info << "Hello, I am alive!" << std::endl;
	return TestTask::performPeriodicAction();
}


ReturnValue_t AtmelTestTask::performOneShotAction() {
    //Stopwatch stopwatch;
    //performSDCardDemo();
    //printFilesTest();

#ifdef ISIS_OBC_G20
	performIOBCTest();
#endif
	//performHammingTest();
    return TestTask::performOneShotAction();
}

ReturnValue_t AtmelTestTask::performActionA() {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t AtmelTestTask::performActionB() {
    return HasReturnvaluesIF::RETURN_OK;
}

void AtmelTestTask::performExceptionTest() {
    AtmelTestTask* exceptTest = nullptr;
    exceptTest->getObjectId();
}


//void AtmelTestTask::performNewPoolManagerAccessTests() {
//	uint16_t numberOfElements[1] = {1};
//	uint16_t sizeofElements[1] = {10};
//	PoolManager<1> testPool = PoolManager<1>(0, sizeofElements, numberOfElements);
//	std::array<uint8_t, 20> testDataArray;
//	std::array<uint8_t, 20> receptionArray;
//	store_address_t testStoreId;
//	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
//
//	for(size_t i = 0; i < testDataArray.size(); i++) {
//		testDataArray[i] = i;
//	}
//	size_t size = 10;
//
//	result = testPool.addData(&testStoreId, testDataArray.data(), size);
//	if(result != RETURN_OK) {
//		sif::error << "Error when adding data" << std::endl;
//	}
//
//	auto accessorPair = testPool.modifyData(testStoreId);
//	if(accessorPair.first != RETURN_OK) {
//		sif::error << "Error when modifying data" << std::endl;
//	}
//	accessorPair.second.print();
//	accessorPair.second.getDataCopy(receptionArray.data(), receptionArray.size());
//	accessorPair.second.release();
//
//	auto constAccess = testPool.getData(testStoreId);
//	if(constAccess.first != RETURN_OK) {
//		sif::error << "Error when modifying data" << std::endl;
//	}
//	accessorPair.second.print();
//}

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
    //performNorFlashTest(false);
    //performSupervisorTest();
    performFRAMTest();

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

void AtmelTestTask::performNorFlashTest(bool displayDebugOutput) {
    int result = NORflash_start();

    if(result != 0) {
        if(displayDebugOutput) {
            sif::info << "AtmelTestTask::performNorFlashTest: Starting failed"
                    << std::endl;
        }

        return;
    }
    Stopwatch stopwatch;

    // Sectors needs to be eraed, otherwise write operations will fail!
    result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA22_ADDRESS);

    // erasing takes a long time. writing of large sectors possibly too..
    // so we can only write one large cycle per performOperation
    // (and multiple smaller ones, but that might not be relevant if
    // the bootload is never overwritten..)
    stopwatch.stop(true);

    stopwatch.start();

    if(displayDebugOutput) {
        sif::info << "AtmelTestTask::performNorFlashTest: Erase result: "
                << result << std::endl;
    }

    uint8_t test[4] = {1, 2, 3, 5};
    uint8_t reception[4] = { 0, 0, 0, 0 };
    int counter = 0;
    result = 1;
    while(result != 0 and counter < 3) {
        result = NORFLASH_WriteData(&NORFlash, NORFLASH_SA22_ADDRESS, test, 4);
        counter ++;
    }

    result = NORFLASH_ReadData(&NORFlash, NORFLASH_SA22_ADDRESS, reception, 4);
    if(displayDebugOutput) {
        sif::info << "AtmelTestTask::performNorFlashTest: Read result: "
                << (int) result << std::endl;
        sif::info << "Read API: " << (int) reception[0] <<
                (int) reception[1]  << (int) reception[2] << (int) reception[3] <<
                std::endl;
    }


    std::memcpy(reception, reinterpret_cast<const void*>(
            NOR_FLASH_BASE_ADDRESS + NORFLASH_SA22_ADDRESS), 4);

    if(displayDebugOutput) {
        sif::info << "memcpy: " << (int) reception[0] <<
                (int) reception[1] << (int) reception[2] << (int) reception[3] <<
                std::endl;
    }


    result = NORFLASH_WriteData(&NORFlash, NORFLASH_SA22_ADDRESS + 256,
            test, 4);
    if(result != 0) {
        if(displayDebugOutput)  {
            sif::info << "AtmelTestTask::performNorFlashTest: Second write "
                    "failed" << std::endl;
        }

        return;
    }

    if(displayDebugOutput)  {
        sif::info << "AtmelTestTask::performNorFlashTest: Second write operation "
                "successfull" << std::endl;
    }


    std::memcpy(reception, reinterpret_cast<const void*>(
            NOR_FLASH_BASE_ADDRESS + NORFLASH_SA22_ADDRESS + 256), 4);

    if(displayDebugOutput)  {
        sif::info << "memcpy second: " << (int) reception[0] <<
                (int) reception[1] << (int) reception[2] << (int) reception[3] <<
                std::endl;
    }

    // we can't write to uneraed sectors.
    result = NORFLASH_WriteData(&NORFlash, NORFLASH_SA0_ADDRESS, test, 4);
    if(result != 0) {
        if(displayDebugOutput) {
            sif::info << "AtmelTestTask::performNorFlashTest: Third write failed "
                    "(expected, sector not erased!)" <<  std::endl;
        }

    }
}


void AtmelTestTask::performFRAMTest() {
    uint8_t swVersion = 0;
    uint8_t swSubversion = 0;
    int result = read_software_version(&swVersion, &swSubversion);
    if(result == 0) {
        sif::info << "AtmelTestTask::performFRAMTest: Software version " <<
                (int) swVersion << std::endl;
        sif::info << "AtmelTestTask::performFRAMTest: Software subversion " <<
                (int) swSubversion << std::endl;
    }
}

#endif

void AtmelTestTask::performHammingTest() {
    uint8_t test[256];
    for(auto idx = 0; idx < 128; idx++) {
        test[idx] = idx;
    }
    for(auto idx = 0; idx < 128; idx++) {
        test[idx + 128] = 128 - idx ;
    }
    uint8_t hamming[3];
    Hamming_Compute256x(test, 256, hamming);
    sif::info << "Hamming code: " << (int) hamming[0] << (int) hamming[1]
             << (int) hamming[2] << std::endl;
    int result = Hamming_Verify256x(test, 256, hamming);
    if(result != 0) {
        sif::error << "Hamming Verification failed with code "
                << result << "!" << std::endl;
    }
    else if(result == 0) {
        sif::info << "Hamming code verification success!" << std::endl;
    }

    // introduce bit error
    test[0] = test[0] ^ 1;
    result = Hamming_Verify256x(test, 256, hamming);
    if(result == Hamming_ERROR_SINGLEBIT) {
        sif::info << "Hamming code one bit error corrected!" << std::endl;

    }
    else {
        sif::error << "Hamming Verification failed with code "
                << result << "!" << std::endl;
        return;
    }

    // introduce bit error else where
    test[156] = test[156] ^ (1 << 5);
    result = Hamming_Verify256x(test, 256, hamming);
    if(result == Hamming_ERROR_SINGLEBIT) {
        sif::info << "Hamming code one bit error corrected!" << std::endl;

    }
    else {
        sif::error << "Hamming Verification failed with code "
                << result << "!" << std::endl;
        return;
    }

    // introduce two bit errors
    test[24] = test[24] ^ 1;
    test[0] = test[0] ^ 1;
    result = Hamming_Verify256x(test, 256, hamming);
    if(result == Hamming_ERROR_MULTIPLEBITS) {
        sif::info << "Hamming code two bit error detected!" << std::endl;

    }
    else {
        sif::error << "Hamming Verification failed with code "
                << result << "!" << std::endl;
        return;
    }
}

void AtmelTestTask::printFilesTest() {
    Stopwatch stopwatch;
    SDCardAccess access;
    f_chdir("/");
    // create 2 files
    create_file(NULL, "F1", NULL, 0);
    create_file(NULL, "F2", NULL, 0);
    // create 3 folders
    create_directory(NULL, "D1");
    create_directory(NULL, "D2");
    create_directory(NULL, "D3");
    // create a file inside folder D2
    create_file("D2/", "D2F1", NULL, 0);
    create_file("D2/", "D2F2", NULL, 0);

    // create a folder inside folder D3
    create_directory("D3/", "D3D1");
    create_file("D3/D3D1/", "D3D1F1", NULL, 0);
    stopwatch.stop(true);
    stopwatch.start();

    SDCardHandler::printSdCard();
    stopwatch.stop(true);
    stopwatch.start();
    int result = clear_sd_card();
    if(result == F_NO_ERROR) {
        sif::info << "SD card cleared without errors" << std::endl;
    }
    else {
        sif::info << "Errors clearing SD card" << std::endl;
    }
    stopwatch.stop(true);
    stopwatch.start();
    SDCardHandler::printSdCard();

}
