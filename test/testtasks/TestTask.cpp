#include "TestTask.h"
#include "PusTcInjector.h"

#include <fsfwconfig/devices/logicalAddresses.h>
#include <fsfwconfig/tmtc/apid.h>

#include <fsfw/unittest/internal/InternalUnitTester.h>
#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <fsfw/globalfunctions/arrayprinter.h>

#include <etl/vector.h>
//#include <sam9g20/common/SDCardApi.h>
//#include <sam9g20/memory/SDCardAccess.h>
#include <array>
#include <cstring>

extern "C" {
//#include <privlib/hcc/demo/SDCardTest.h>
#include <memories/sdmmc/MEDSdcard.h>
#include <sam9g20/at91/tinyfatfs/include/tinyfatfs/tff.h>
#include <peripherals/pio/pio.h>
#include <utility/trace.h>
}


Media medias[1];



bool TestTask::oneShotAction = true;
MutexIF* TestTask::testLock = nullptr;
//static constexpr std::array<size_t, 5> TestTask::templateSizes =

TestTask::TestTask(object_id_t objectId_):
	SystemObject(objectId_), testMode(testModes::A) {
	if(testLock == nullptr) {
		testLock = MutexFactory::instance()->createMutex();
	}
	IPCStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);

	//subscribeInTmManager(service, subservice);
}

TestTask::~TestTask() {
}

ReturnValue_t TestTask::performOperation(uint8_t operationCode) {
	ReturnValue_t result = RETURN_OK;
	testLock ->lockMutex(MutexIF::TimeoutType::WAITING, 20);
	if(oneShotAction) {
		// Add code here which should only be run once
		performOneShotAction();
		oneShotAction = false;
	}
	testLock->unlockMutex();

	// Add code here which should only be run once per performOperation
	performPeriodicAction();

	// Add code here which should only be run on alternating cycles.
	if(testMode == testModes::A) {
		performActionA();
		testMode = testModes::B;
	}
	else if(testMode == testModes::B) {
		performActionB();
		testMode = testModes::A;
	}
	return result;
}

ReturnValue_t TestTask::performOneShotAction() {
	// Everything here will only be performed once.
    //SDCardTest();
//    int res = 0;
    //sdTest();
//    SDCardAccess accessToken;
    //int res = open_filesystem(VolumeId::SD_CARD_0);

    //res = select_sd_card(VolumeId::SD_CARD_0);

//    const char* const testString = "abc";
//    F_FILE* file = f_open("test.bin", "w");
//    if(file == nullptr) {
//        return HasReturnvaluesIF::RETURN_FAILED;
//    }
//    res = f_write(testString, 3, 1, file);
//
//    f_close(file);

    //performEtlTemplateTest();
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TestTask::performPeriodicAction() {
	ReturnValue_t result = RETURN_OK;
	return result;
}

ReturnValue_t TestTask::performActionA() {
	ReturnValue_t result = RETURN_OK;
	// Add periodically executed code here
	return result;
}

ReturnValue_t TestTask::performActionB() {
	ReturnValue_t result = RETURN_OK;
	// Add periodically executed code here
	return result;
}


void TestTask::performPusInjectorTest() {
	PusTcInjector tcInjector(objects::TC_INJECTOR,
	        objects::CCSDS_PACKET_DISTRIBUTOR, objects::TC_STORE,
	        apid::SOURCE_OBSW);
	tcInjector.initialize();
#if FSFW_CPP_OSTREAM_ENABLED == 1
	sif::info << "TestTask: injecting pus telecommand" << std::endl;
#else
	sif::printInfo("TestTask: injecting pus telecommand\n");
#endif
	tcInjector.injectPusTelecommand(17,1);
}

void TestTask::examplePacketTest() {
	object_id_t header = 42;
	//uint8_t testArray1[3] = {1,2,3};
	//std::vector<uint8_t> testArray3 = {1,2,3};
	std::array<uint8_t, 3> testArray {1,2,3};
	ParameterId_t tail = 96;
	size_t packetMaxSize = 256;
	uint8_t packet [packetMaxSize];
	size_t packetLen = 0;

	// make big-endian packet (like packets from ground) and deserialize it.
	{
		header = EndianConverter::convertBigEndian(header);
		std::memcpy(packet, &header, sizeof(header));
		packetLen += sizeof(header);

		std::copy(testArray.data(), testArray.data() + testArray.size(),
				packet + sizeof(header));
		packetLen += testArray.size();

		tail = EndianConverter::convertBigEndian(tail);
		std::memcpy(packet + packetLen, &tail, sizeof(tail));
		packetLen += sizeof(tail);

		// The deserialized buffer will be stored in this adaptee
		std::array<uint8_t, 3> bufferAdaptee = {};
		arrayprinter::print(packet, packetLen, OutputType::DEC);
		TestExamplePacket testClass(packet, packetLen, bufferAdaptee.data(),
				bufferAdaptee.size());
		const uint8_t * pointer = packet;
		size_t size = packetLen;
		ReturnValue_t result = testClass.deSerialize(&pointer, &size,
		        SerializeIF::Endianness::BIG);
		if(result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
			sif::error << "Deserialization did not work" << std::endl;
#else
			sif::printError("Deserialization did not work\n");
#endif
			return;
		}
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::info << "Printing deserialized packet members: " << std::endl;
		sif::info << testClass.getHeader() << std::endl;
		sif::info << testClass.getTail() << std::endl;
#else
		sif::printInfo("Printing deserialized packet members: \n");
		sif::printInfo("%s\n%s\n", testClass.getHeader(), testClass.getTail());
#endif
		arrayprinter::print(testClass.getBuffer(), testClass.getBufferLength());
	}

	// Serialization (e.g. for ground packet)
	{
		TestExamplePacket testClass2(header, tail, testArray.data(), testArray.size());
		size_t serializedSize = 0;
		uint8_t* packetPointer = packet;
		// serialize for ground: bigEndian = true.
		ReturnValue_t result = testClass2.serialize(&packetPointer,
				&serializedSize, packetMaxSize, SerializeIF::Endianness::BIG);
		if(result == RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
			sif::info << "Priting serialized packet:" << std::endl;
#else
			sif::printInfo("Priting serialized packet: \n");
#endif
			arrayprinter::print(packet, packetLen, OutputType::DEC);
		}
	}
}


void TestTask::performEtlTemplateTest() {
    const uint32_t poolId = 0;
    insertNewTmManagerStruct<templateSizes[poolId]>(poolId);
    // now we should be able to access it like this
    auto iter = testMap.find(poolId);
    if(iter == testMap.end()) {
        return;
    }
    struct TmManagerStruct<templateSizes[poolId]>* test = dynamic_cast<
            struct TmManagerStruct<templateSizes[poolId]>*>(iter->second);
    if(test) {}
}


void TestTask::sdTest(void) {
    FATFS fs;
    FIL fileObject;

#ifdef ISIS_OBC_G20
    uint8_t sdCard = 0;
    Pin sdSelectPin[1] = {PIN_SDSEL};
    PIO_Configure(sdSelectPin, PIO_LISTSIZE(sdSelectPin));
//    bool high = PIO_Get(sdSelectPin);
//    if(high) {
//        PIO_Clear(sdSelectPin);
//    }
//    else {
//        PIO_Set(sdSelectPin);
//    }
    if(sdCard == 0) {
        PIO_Set(sdSelectPin);
    }
    else {
        PIO_Clear(sdSelectPin);
    }


    Pin npWrPins[2] = {PIN_NPWR_SD0, PIN_NPWR_SD1};
    PIO_Configure(npWrPins, PIO_LISTSIZE(npWrPins));
    if(sdCard == 0) {
        PIO_Clear(npWrPins);
    }
    if(sdCard == 1) {
        PIO_Clear(npWrPins + 1);
    }

//    Pin pinsMci1Off[2] = {PINS_MCI1_OFF};
//    PIO_Configure(pinsMci1Off, PIO_LISTSIZE(pinsMci1Off));
//    PIO_Set(pinsMci1Off);
//    PIO_Set(pinsMci1Off +  1);

#endif

    if(sdCard == 0) {
        PIO_Set(npWrPins);
        for(int idx = 0; idx<10000; idx++) {}
        PIO_Clear(npWrPins);
    }
    if(sdCard == 1) {
        PIO_Set(npWrPins + 1);
        for(int idx = 0; idx<10000; idx++) {}
        PIO_Clear(npWrPins);
    }

    const int ID_DRV = 0;
    MEDSdcard_Initialize(&medias[ID_DRV], 0);

    memset(&fs, 0, sizeof(FATFS));  // Clear file system object
    int res = f_mount(0, &fs);
    if( res != FR_OK ) {
        printf("f_mount pb: 0x%X\n\r", res);
    }

//    char file_name [strlen(config::SW_REPOSITORY) + strlen(config::SW_UPDATE_SLOT_NAME) + 2];
//    snprintf(file_name, sizeof (file_name) + 1, "/%s/%s", config::SW_REPOSITORY,
//            config::SW_UPDATE_SLOT_NAME);

//#ifdef ISIS_OBC_G20
//    PIO_Set(npWrPins);
//    for(int idx = 0; idx < 100000; idx++) {};
//    PIO_Clear(npWrPins);
//#endif

    res = f_open(&fileObject, "test.bin", FA_OPEN_EXISTING|FA_READ);
    if( res != FR_OK ) {
        TRACE_ERROR("f_open read pb: 0x%X\n\r", res);
    }

//    res = f_open(&fileObject, file_name, FA_OPEN_EXISTING|FA_READ);
//    if( res != FR_OK ) {
//        TRACE_ERROR("f_open read pb: 0x%X\n\r", res);
//    }

    size_t bytes_read = 0;
    uint8_t* alotofMemory= new uint8_t[200000];
    res = f_read(&fileObject, (void*) alotofMemory, 3, &bytes_read);
    if(res != FR_OK) {
        TRACE_ERROR("f_read pb: 0x%X\n\r", res);
    }

    delete(alotofMemory);
}


