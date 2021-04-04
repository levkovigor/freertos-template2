#include <fsfw/storagemanager/PoolManager.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/globalfunctions/arrayprinter.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/timemanager/Stopwatch.h>

#include <bsp_sam9g20/boardtest/AtmelTestTask.h>
#include <bsp_sam9g20/comIF/GpioDeviceComIF.h>
#include <bsp_sam9g20/common/SRAMApi.h>
#include <bsp_sam9g20/memory/SDCardAccess.h>
#include <bsp_sam9g20/memory/SDCardHandler.h>

extern "C" {
#if defined(AT91SAM9G20_EK)
#include <led_ek.h>
#endif

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <portwrapper.h>
#include <at91/utility/hamming.h>
#include <hcc/demo/demo_sd.h>
#include <bsp_sam9g20/common/SDCardApi.h>

#ifdef ISIS_OBC_G20
#include <hal/Storage/NORflash.h>
#include <hal/Storage/FRAM.h>
#include <bsp_sam9g20/common/fram/FRAMApi.h>
#include <hal/supervisor.h>
#endif

//#include <memories/sdmmc/MEDSdcard.h>
//#include <bsp_sam9g20/at91/tinyfatfs/include/tinyfatfs/tff.h>
//#include <peripherals/pio/pio.h>
//#include <utility/trace.h>

}

#include <cstring>

//Media medias[1];

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
    //moveFileTest();
    //int32_t test = get_sram0_status_field();
    //TRACE_INFO("SRAM status field: %d\n\r", (int) test);
//    uint8_t read_buf[128];
//    int retval = FRAM_read(read_buf, FRAM_END_ADDR - 64, 64);
//    if(retval != 0) {
//        sif::printWarning("FRAM read failed with %d\n", retval);
//    }
//    arrayprinter::print(read_buf, 64);

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
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "AtmelTestTask: SD Card demo success" << std::endl;
#else
#endif
    }
}

#ifdef ISIS_OBC_G20

void AtmelTestTask::performIOBCTest() {
    //performNorFlashTest(false);
    //performSupervisorTest();
    //performFRAMTest();

}

void AtmelTestTask::performSupervisorTest() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Testing supervisor." << std::endl;
#else
    sif::printInfo("Testing supervisor.\n");
#endif

    Supervisor_start(NULL, 0);

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Supervisor started successfull!" << std::endl;
#else
    sif::printInfo("Supervisor started successfull!\n");
#endif

    uint8_t supervisorIndex = -1;
    supervisor_housekeeping_t houseKeeping;
    int result = Supervisor_getHousekeeping(&houseKeeping, supervisorIndex);

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Supervisor Housekeeping Call Result: " << result << std::endl;
#else
    sif::printInfo("Supervisor Housekeeping Call Result: %d", result);
#endif

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

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Supervisor test successfull!" << std::endl;
#else
    sif::printInfo("Supervisor test successfull!\n");
#endif

}

void AtmelTestTask::performNorFlashTest(bool displayDebugOutput) {
    int result = NORflash_start();

    if(result != 0) {
        if(displayDebugOutput) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "AtmelTestTask::performNorFlashTest: Starting failed"
                    << std::endl;
#else
            sif::printInfo("AtmelTestTask::performNorFlashTest: Starting failed\n");
#endif
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "AtmelTestTask::performNorFlashTest: Erase result: "
                << result << std::endl;
#else
        sif::printInfo("AtmelTestTask::performNorFlashTest: Erase result: %d", result);
#endif
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "AtmelTestTask::performNorFlashTest: Read result: " << (int) result
                << std::endl;
        sif::info << "Read API: " << (int) reception[0] <<
                (int) reception[1]  << (int) reception[2] << (int) reception[3] <<
                std::endl;
#else
        sif::printInfo("AtmelTestTask::performNorFlashTest: Read result: %d\n", result);
        sif::printInfo("Read API: %d%d%d%d\n", reception[0], reception[1],
                reception[2], reception[3]);
#endif
    }


    std::memcpy(reception, reinterpret_cast<const void*>(
            NOR_FLASH_BASE_ADDRESS + NORFLASH_SA22_ADDRESS), 4);

    if(displayDebugOutput) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "memcpy: " << (int) reception[0] << (int) reception[1] << (int) reception[2]  <<
                (int) reception[3] << std::endl;
#else
        sif::printInfo("memcpy: %d%d%d%d\n", reception);
#endif
    }


    result = NORFLASH_WriteData(&NORFlash, NORFLASH_SA22_ADDRESS + 256,
            test, 4);
    if(result != 0) {
        if(displayDebugOutput)  {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "AtmelTestTask::performNorFlashTest: Second write "
                    "failed" << std::endl;
#else
            sif::printInfo("AtmelTestTask::performNorFlashTest: Second write "
                    "failed\n");
#endif
        }

        return;
    }

    if(displayDebugOutput)  {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "AtmelTestTask::performNorFlashTest: Second write operation "
                "successfull" << std::endl;
#else
        sif::printInfo("AtmelTestTask::performNorFlashTest: Second write operation "
                "successfull\n");
#endif
    }


    std::memcpy(reception, reinterpret_cast<const void*>(
            NOR_FLASH_BASE_ADDRESS + NORFLASH_SA22_ADDRESS + 256), 4);

    if(displayDebugOutput)  {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "memcpy second: " << (int) reception[0] << (int) reception[1] <<
                (int) reception[2] << (int) reception[3] << std::endl;
#else
        sif::printInfo("memcpy second: %d%d%d%d", reception[0], reception[1],
                reception[2], reception[3]);
#endif
    }

    // we can't write to uneraed sectors.
    result = NORFLASH_WriteData(&NORFlash, NORFLASH_SA0_ADDRESS, test, 4);
    if(result != 0) {
        if(displayDebugOutput) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "AtmelTestTask::performNorFlashTest: Third write failed "
                    "(expected, sector not erased!)" <<  std::endl;
#else
            sif::printInfo("AtmelTestTask::performNorFlashTest: Third write failed "
                    "(expected, sector not erased!)\n");
#endif
        }

    }
}


void AtmelTestTask::performFRAMTest() {
    uint8_t swVersion = 0;
    uint8_t swSubversion = 0;
    uint8_t swSubsubversion = 0;
    int result = fram_read_software_version(&swVersion, &swSubversion,
            &swSubsubversion);
    if(result == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "AtmelTestTask::performFRAMTest: Software version "
                << (int) swVersion << std::endl;
        sif::info << "AtmelTestTask::performFRAMTest: Software subversion "
                << (int) swSubversion << std::endl;
        sif::info << "AtmelTestTask::performFRAMTest: Software subsubversion "
                << (int) swSubsubversion << std::endl;
#else
        sif::printInfo("AtmelTestTask::performFRAMTest: Software version %d",
                static_cast<int>(swVersion));
        sif::printInfo("AtmelTestTask::performFRAMTest: Software subversion %d",
                static_cast<int>(swSubversion));
        sif::printInfo("AtmelTestTask::performFRAMTest: Software subsubversion %d",
                static_cast<int>(swSubsubversion));
#endif
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

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Hamming code: " << std::hex << "0x" << (int) hamming[0] << ", 0x" <<
            (int) hamming[1] << ", 0x" << (int) hamming[2] << std::endl;
#else
#endif
    int result = Hamming_Verify256x(test, 256, hamming);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Hamming Verification failed with code "
                << result << "!" << std::endl;
#else
#endif
    }
    else if(result == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Hamming code verification success!" << std::endl;
#else
#endif
    }

    // introduce bit error
    test[0] = test[0] ^ 1;
    result = Hamming_Verify256x(test, 256, hamming);
    if(result == Hamming_ERROR_SINGLEBIT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Hamming code one bit error corrected!" << std::endl;
#else
#endif

    }
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Hamming Verification failed with code "
                << result << "!" << std::endl;
#else
#endif
        return;
    }

    // introduce bit error else where
    test[156] = test[156] ^ (1 << 5);
    result = Hamming_Verify256x(test, 256, hamming);
    if(result == Hamming_ERROR_SINGLEBIT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Hamming code one bit error corrected!" << std::endl;
#else
#endif

    }
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Hamming Verification failed with code "
                << result << "!" << std::endl;
#else
#endif
        return;
    }

    // introduce two bit errors
    test[24] = test[24] ^ 1;
    test[0] = test[0] ^ 1;
    result = Hamming_Verify256x(test, 256, hamming);
    if(result == Hamming_ERROR_MULTIPLEBITS) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Hamming code two bit error detected!" << std::endl;
#else
#endif

    }
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Hamming Verification failed with code "
                << result << "!" << std::endl;
#else
#endif
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "SD card cleared without errors" << std::endl;
#else
#endif
    }
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Errors clearing SD card" << std::endl;
#else
#endif
    }
    stopwatch.stop(true);
    stopwatch.start();
    SDCardHandler::printSdCard();

}

void AtmelTestTask::moveFileTest() {
    /* Used to verify that f_move can't be used to overwrite existing files. */
    Stopwatch stopwatch;
    SDCardAccess access;
    f_chdir("/");
    // create 2 files
    create_file(NULL, "F1", NULL, 0);
    create_file(NULL, "F2", NULL, 0);

    int result = f_move("F1", "F2");
    /* Was 6 (F_ERR_DUPLICATED) */
    if(result) {}
    SDCardHandler::printSdCard();
}

// name-clash with HCC lib.
//void AtmelTestTask::tinyfatFsTest(void) {
//    FATFS fs;
//    FIL fileObject;
//
//#ifdef ISIS_OBC_G20
//    uint8_t sdCard = 0;
//    Pin sdSelectPin[1] = {PIN_SDSEL};
//    PIO_Configure(sdSelectPin, PIO_LISTSIZE(sdSelectPin));
//    bool high = PIO_Get(sdSelectPin);
//    if(high) {
//        PIO_Clear(sdSelectPin);
//    }
//    else {
//        PIO_Set(sdSelectPin);
//    }
//    if(sdCard == 0) {
//        PIO_Set(sdSelectPin);
//    }
//    else {
//        PIO_Clear(sdSelectPin);
//    }
//
//
//    Pin npWrPins[2] = {PIN_NPWR_SD0, PIN_NPWR_SD1};
//    PIO_Configure(npWrPins, PIO_LISTSIZE(npWrPins));
//    if(sdCard == 0) {
//        PIO_Clear(npWrPins);
//    }
//    if(sdCard == 1) {
//        PIO_Clear(npWrPins + 1);
//    }

//    Pin pinsMci1Off[2] = {PINS_MCI1_OFF};
//    PIO_Configure(pinsMci1Off, PIO_LISTSIZE(pinsMci1Off));
//    PIO_Set(pinsMci1Off);
//    PIO_Set(pinsMci1Off +  1);

//#endif
//
//    const int ID_DRV = 0;
//    MEDSdcard_Initialize(&medias[ID_DRV], 0);
//
//    memset(&fs, 0, sizeof(FATFS));  // Clear file system object
//    int res = f_mount(0, &fs);
//    if( res != FR_OK ) {
//        printf("f_mount pb: 0x%X\n\r", res);
//    }

//    char file_name [strlen(config::SW_REPOSITORY) + strlen(config::SW_UPDATE_SLOT_NAME) + 2];
//    snprintf(file_name, sizeof (file_name) + 1, "/%s/%s", config::SW_REPOSITORY,
//            config::SW_UPDATE_SLOT_NAME);

//#ifdef ISIS_OBC_G20
//    PIO_Set(npWrPins);
//    for(int idx = 0; idx < 100000; idx++) {};
//    PIO_Clear(npWrPins);
//#endif

//    res = f_open(&fileObject, "test.bin", FA_OPEN_EXISTING|FA_READ);
//    if( res != FR_OK ) {
//        TRACE_ERROR("f_open read pb: 0x%X\n\r", res);
//    }

//    res = f_open(&fileObject, file_name, FA_OPEN_EXISTING|FA_READ);
//    if( res != FR_OK ) {
//        TRACE_ERROR("f_open read pb: 0x%X\n\r", res);
//    }

//    size_t bytes_read = 0;
//    uint8_t* alotofMemory= new uint8_t[200000];
//    res = f_read(&fileObject, (void*) alotofMemory, 3, &bytes_read);
//    if(res != FR_OK) {
//        TRACE_ERROR("f_read pb: 0x%X\n\r", res);
//    }

//    delete(alotofMemory);
//}
