#include "Factory.h"
#include <stm32/fsfwconfig/cdatapool/dataPoolInit.h>
#include <stm32/fsfwconfig/devices/logicalAddresses.h>
#include <stm32/fsfwconfig/devices/powerSwitcherList.h>
#include <stm32/fsfwconfig/objects/systemObjectList.h>
#include <stm32/fsfwconfig/tmtc/apid.h>
#include <stm32/fsfwconfig/tmtc/pusIds.h>

/* Config + STD Includes */
#include <fsfw/internalError/InternalErrorReporter.h>
#include <fsfw/storagemanager/PoolManager.h>
#include <fsfw/tcdistribution/CCSDSDistributor.h>
#include <fsfw/tcdistribution/PUSDistributor.h>
#include <fsfw/events/EventManager.h>
#include <fsfw/fdir/FailureIsolationBase.h>
#include <fsfw/health/HealthTable.h>
#include <fsfw/pus/Service3Housekeeping.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>

/* PUS Includes */
#include <fsfw/tmtcservices/PusServiceBase.h>
#include <fsfw/pus/Service1TelecommandVerification.h>
#include <fsfw/pus/Service2DeviceAccess.h>
#include <fsfw/pus/Service5EventReporting.h>
#include <mission/pus/Service6MemoryManagement.h>
#include <fsfw/pus/Service8FunctionManagement.h>
#include <fsfw/pus/Service9TimeManagement.h>
#include <fsfw/pus/Service17Test.h>
#include <mission/pus/Service23FileManagement.h>
#include <fsfw/pus/CService200ModeCommanding.h>
#include <fsfw/pus/CService201HealthCommanding.h>
#include <mission/fdir/PCDUFailureIsolation.h>
#include <fsfw/timemanager/TimeStamper.h>
#include <mission/utility/TmFunnel.h>


/* Devices */
#include <mission/devices/PCDUHandler.h>
#include <mission/devices/GPSHandler.h>


/* Test files */
#include <test/testinterfaces/DummyEchoComIF.h>
#include <test/testinterfaces/DummyGPSComIF.h>
#include <test/testinterfaces/DummyCookie.h>
#include <test/testtasks/TestTask.h>
#include <stm32/tmtcbridge/EMACPollingTask.h>
#include <stm32/tmtcbridge/TmTcLwIpUdpBridge.h>
#include <test/testdevices/TestDeviceHandler.h>
#include <cstdint>
/**
 * Build tasks by using SystemObject Interface (Interface).
 * Header files of all tasks must be included
 * Please note that an object has to implement the system object interface
 * if the nterface validity is checked or retrieved later by using the
 * get<TargetInterface>(object_id) function from the ObjectManagerIF.
 *
 * Framework objects are created first.
 *
 * \ingroup init
 */
void Factory::produce(void) {
	setStaticFrameworkObjectIds();
	new EventManager(objects::EVENT_MANAGER);
	new HealthTable(objects::HEALTH_TABLE);
	new InternalErrorReporter(objects::INTERNAL_ERROR_REPORTER);

    /* Pool manager handles storage und mutexes */
    {
        // TODO: make this configurable via OBSWConfig.h
        LocalPool::LocalPoolConfig poolConfig = {
                {100, 32}, {30, 64}, {20, 128}, {10, 256}
        };
        PoolManager* ipcStore = new PoolManager(objects::IPC_STORE, poolConfig);
        size_t additionalSize = 0;
        size_t storeSize = ipcStore->getTotalSize(&additionalSize);
        sif::info << "Allocating " << storeSize + additionalSize << " bytes "
                <<"for the IPC Store.." << std::endl;
    }

    {
        LocalPool::LocalPoolConfig poolConfig = {
                {100, 32}, {30, 64}, {20, 128}, {10, 256}
        };
        PoolManager* tmStore = new PoolManager(objects::TM_STORE, poolConfig);
        size_t additionalSize = 0;
        size_t storeSize = tmStore->getTotalSize(&additionalSize);
        sif::info << "Allocating " << storeSize + additionalSize << " bytes "
                <<"for the TM Store.." << std::endl;
    }

    {
        LocalPool::LocalPoolConfig poolConfig = {
                {100, 32}, {30, 64}, {20, 128}, {10, 256}
        };
        PoolManager* tcStore =new PoolManager(objects::TC_STORE, poolConfig);
        size_t additionalSize = 0;
        size_t storeSize = tcStore->getTotalSize(&additionalSize);
        sif::info << "Allocating " << storeSize + additionalSize << " bytes "
                <<"for the TC Store.." << std::endl;
    }

	new TimeStamper(objects::PUS_TIME);

	/* Distributor Tasks */
	new CCSDSDistributor(apid::SOURCE_OBSW,objects::CCSDS_PACKET_DISTRIBUTOR);
	new PUSDistributor(apid::SOURCE_OBSW,objects::PUS_PACKET_DISTRIBUTOR,
			objects::CCSDS_PACKET_DISTRIBUTOR);

	/* UDP Server */
	//new TmTcLwIpUdpBridge(objects::UDP_TMTC_BRIDGE, objects::CCSDS_PACKET_DISTRIBUTOR);
	new EmacPollingTask(objects::EMAC_POLLING_TASK); // used on both boards

	/* TM Destination */
	new TmFunnel(objects::TM_FUNNEL);

    /* PUS Standalone Services using PusServiceBase */
    new Service1TelecommandVerification(objects::PUS_SERVICE_1_VERIFICATION,
            apid::SOURCE_OBSW, pus::PUS_SERVICE_1, objects::TM_FUNNEL,
            20);
    // TODO: move 20 to OBSWConfig
    new Service5EventReporting(objects::PUS_SERVICE_5_EVENT_REPORTING,
            apid::SOURCE_OBSW, pus::PUS_SERVICE_5, 20);
    new Service9TimeManagement(objects::PUS_SERVICE_9_TIME_MGMT,
            apid::SOURCE_OBSW, pus::PUS_SERVICE_9);
    new Service17Test(objects::PUS_SERVICE_17_TEST, apid::SOURCE_OBSW,
            pus::PUS_SERVICE_17);

    /* PUS Gateway Services using CommandingServiceBase */
    new Service2DeviceAccess(objects::PUS_SERVICE_2_DEVICE_ACCESS,
            apid::SOURCE_OBSW, pus::PUS_SERVICE_2);
    new Service6MemoryManagement(objects::PUS_SERVICE_6,
            apid::SOURCE_OBSW, pus::PUS_SERVICE_6);
    new Service8FunctionManagement(objects::PUS_SERVICE_8_FUNCTION_MGMT,
            apid::SOURCE_OBSW, pus::PUS_SERVICE_8);
    //new Service20ParameterManagement(objects::PUS_SERVICE_20);
//    new Service23FileManagement(objects::PUS_SERVICE_23_FILE_MGMT, apid::SOURCE_OBSW,
//            pus::PUS_SERVICE_23);
    new CService200ModeCommanding(objects::PUS_SERVICE_200_MODE_MGMT,
            apid::SOURCE_OBSW, pus::PUS_SERVICE_200);
    new CService201HealthCommanding(objects::PUS_SERVICE_201, apid::SOURCE_OBSW,
            pus::PUS_SERVICE_201);

	/*
	 * Device Handlers using DeviceHandlerBase.
	 * These includes work without connected hardware via virtualized
	 * devices and interfaces
	 */
//	CookieIF * dummyCookie0 = new DummyCookie(addresses::PCDU);
//	new PCDUHandler(objects::PCDU_HANDLER,objects::DUMMY_ECHO_COM_IF,
//			dummyCookie0);
//	CookieIF * dummyCookie1 = new DummyCookie(addresses::DUMMY_ECHO);
//	new TestDevice(objects::DUMMY_HANDLER, objects::DUMMY_ECHO_COM_IF,
//			dummyCookie1,switches::DUMMY);

	/* Data Structures, pool initialization */
	/* GPS Data Structures */
//	struct GpsInit::navDataIdStruct gps0;
//	struct GpsInit::navDataIdStruct gps1;
//	GpsInit::gpsIdStructInit(&gps0,&gps1);
	//struct hk::hkIdStruct hkIdStruct;
	/* Test Data Structure */
//	struct TestInit::TestIdStruct test;
//	TestInit::TestStructInit(&test);
	//hk::initHkStruct(&hkIdStruct, gps0, gps1, test);
	//hk::hkInit(housekeepingService, hkIdStruct);

//	DummyCookie * dummyCookie2 = new DummyCookie(addresses::DUMMY_GPS0);
//#if defined(stm32) || defined(VIRTUAL_BUILD)
//	new GPSHandler(objects::GPS0_HANDLER, objects::DUMMY_GPS_COM_IF, dummyCookie2,
//			switches::GPS0,gps0);
//	DummyCookie * dummyCookie3 = new DummyCookie(addresses::DUMMY_GPS1);
//	new GPSHandler(objects::GPS1_HANDLER, objects::DUMMY_GPS_COM_IF, dummyCookie3,
//			switches::GPS1,gps1);
//	/*
//	 * All includes with real devices and connected hardware.
//	 * This will be the final flying version
//	 */
//#elif defined(at91sam9g20)
//	new GPSHandler(objects::GPS0_HANDLER,objects::DUMMY_GPS_COM_IF,
//			dummyCookie2, switches::GPS0,gps0);
//	new GPSHandler(objects::GPS1_HANDLER,objects::DUMMY_GPS_COM_IF,
//			dummyCookie2, switches::GPS1,gps1);
//#endif

	/* Dummy Communication Interfaces */
	new TestEchoComIF(objects::DUMMY_ECHO_COM_IF);
	new DummyGPSComIF(objects::DUMMY_GPS_COM_IF);

	/* Test Tasks */
	new TestTask(objects::TEST_TASK);
	/* Board dependant files */

}

void Factory::setStaticFrameworkObjectIds() {
	PusServiceBase::packetSource=objects::PUS_PACKET_DISTRIBUTOR;
	PusServiceBase::packetDestination = objects::TM_FUNNEL;
	VerificationReporter::messageReceiver = objects::PUS_SERVICE_1_VERIFICATION;
	DeviceHandlerBase::powerSwitcherId = objects::PCDU_HANDLER;
	DeviceHandlerBase::rawDataReceiverId = objects::PUS_SERVICE_2_DEVICE_ACCESS;
	DeviceHandlerFailureIsolation::powerConfirmationId = objects::PCDU_HANDLER;
	TmPacketStored::timeStamperId = objects::PUS_TIME;
	TmFunnel::downlinkDestination = objects::UDP_TMTC_BRIDGE;
}


