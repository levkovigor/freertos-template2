#include "TestTask.h"
#include "PusTcInjector.h"

#include <objects/systemObjectList.h>
#include <devices/logicalAddresses.h>
#include <tmtc/apid.h>

#include <fsfw_tests/internal/InternalUnitTester.h>
#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <fsfw/globalfunctions/arrayprinter.h>
#include "fsfw/serialize/EndianConverter.h"

#include <array>
#include <cstring>

extern "C" {

}



bool TestTask::oneShotAction = true;
MutexIF* TestTask::testLock = nullptr;
//static constexpr std::array<size_t, 5> TestTask::templateSizes =

TestTask::TestTask(object_id_t objectId_):
	        SystemObject(objectId_), testMode(testModes::A) {
    if(testLock == nullptr) {
        testLock = MutexFactory::instance()->createMutex();
    }
    IPCStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);

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
    // triggerEvent(TEST_EVENT, 0, 0);
    // triggerEvent(TEST_EVENT_HIGH, 5, 4224);
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
        sif::printInfo("%d\n%d\n", testClass.getHeader(), testClass.getTail());
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

/**
ETLIMultiMapWrapper test, can be used later to create unit test. For testing
EtlIMapWrapper, this test can be used as a guideline and with minor modifications
(like only emplacing once for a key) can be used to test EtlIMapWrapper.

#include <etl/map.h>
#include <mission/memory/tmstore/EtlIMultiMapWrapper.h>

void TestTask::performEtlTemplateTest() {
	sif::printInfo("test ETLMultiMapWrapper\n");
	etl::multimap<int, char, 3>* map = new etl::multimap<int, char, 3>();
	EtlIMultiMapWrapper<int, char>* wrapper = new EtlIMultiMapWrapper<int, char>(map);
	wrapper->emplace(1, 'a');
	wrapper->emplace(2, 'b');
	wrapper->emplace(3, 'c');

	if((wrapper->emplace(4, 'a'))==HasReturnvaluesIF::RETURN_FAILED) {
		sif::printInfo("works at point 1\n");
	}

	char returnval = wrapper->get(3).begin->second;
	sif::printInfo("tttt %c\n", returnval);
	if(returnval == 'c') {
		sif::printInfo("works at point 2\n");
	}

	wrapper->erase(3, 'c');

	if(wrapper->get(3).returnValue== HasReturnvaluesIF::RETURN_FAILED) {
		sif::printInfo("works at point 3\n");
	}

	wrapper->clear();

	if(wrapper->get(1).returnValue == HasReturnvaluesIF::RETURN_FAILED) {
		sif::printInfo("works at point 4\n");
	}

	wrapper->emplace(1, 'a');
	wrapper->emplace(1, 'b');

	typename etl::imultimap<int, char>::const_iterator returnIt = wrapper->get(1).begin;
	if(returnIt->second == 'a') {
		sif::printInfo("works at point 5\n");
	}
	returnIt++;
	if(returnIt->second == 'b') {
		sif::printInfo("works at point 6\n");
	}
}
**/
