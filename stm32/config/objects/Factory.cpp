/**
 * \file Factory.cpp
 * \brief Produces system objects.
 *
 * Services like the Event Manager, PUS Services are system objects
 * and need to be initialized in this factory
 * \ingroup init
 */

/* Config + STD Includes */
#include <stm32/config/cdatapool/dataPoolInit.h>
#include <stm32/config/cdatapool/gpsPool.h>
#include <stm32/config/objects/Factory.h>
#include <stm32/config/tmtc/apid.h>
#include <stm32/config/objects/systemObjectList.h>
#include <stm32/config/devices/logicalAddresses.h>
#include <stm32/config/devices/powerSwitcherList.h>
#include <stm32/config/hk/sid.h>
#include <stm32/config/hk/hkInit.h>

/* Flight Software Framework */
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
#include <mission/pus/Service1TelecommandVerification.h>
#include <mission/pus/Service2DeviceAccess.h>
#include <mission/pus/Service5EventReporting.h>
#include <mission/pus/Service6MemoryManagement.h>
#include <mission/pus/Service8FunctionManagement.h>
#include <mission/pus/Service9TimeManagement.h>
#include <mission/pus/Service17Test.h>
#include <mission/pus/Service23FileManagement.h>
#include <mission/pus/CService200ModeCommanding.h>
#include <mission/pus/CService201HealthCommanding.h>
#include <mission/fdir/PCDUFailureIsolation.h>
#include <mission/utility/TimeStamper.h>
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
	new InternalErrorReporter(objects::INTERNAL_ERROR_REPORTER,
			datapool::INTERNAL_ERROR_FULL_MSG_QUEUES,
			datapool::INTERNAL_ERROR_MISSED_LIVE_TM,
			datapool::INTERNAL_ERROR_STORE_FULL);

	/* Pool manager handles storage und mutexes */
	/* Data Stores. Currently reserving 9600 bytes of memory */
	uint16_t numberOfElements[4] = {100, 30, 20, 10};
	uint16_t sizeofElements[4] = {32, 64, 128, 265};
	new PoolManager<4>(objects::IPC_STORE,sizeofElements, numberOfElements);
	new PoolManager<4>(objects::TM_STORE,sizeofElements, numberOfElements);
	new PoolManager<4>(objects::TC_STORE,sizeofElements, numberOfElements);

	new TimeStamper(objects::PUS_TIME);

	/* Distributor Tasks */
	new CCSDSDistributor(apid::SOURCE_OBSW,objects::CCSDS_PACKET_DISTRIBUTOR);
	new PUSDistributor(apid::SOURCE_OBSW,objects::PUS_PACKET_DISTRIBUTOR,
			objects::CCSDS_PACKET_DISTRIBUTOR);

	/* UDP Server */
	new TmTcLwIpUdpBridge(objects::UDP_TMTC_BRIDGE, objects::CCSDS_PACKET_DISTRIBUTOR);
	new EmacPollingTask(objects::EMAC_POLLING_TASK); // used on both boards

	/* TM Destination */
	new TmFunnel(objects::PUS_FUNNEL);

	/* PUS Standalone Services using PusServiceBase */
	new Service1TelecommandVerification(objects::PUS_SERVICE_1);
	//Service3Housekeeping * housekeepingService =
	//		new Service3Housekeeping(objects::PUS_SERVICE_3);
	new Service5EventReporting(objects::PUS_SERVICE_5);
	new Service9TimeManagement(objects::PUS_SERVICE_9);
	new Service17Test(objects::PUS_SERVICE_17);
	new Service23FileManagement(objects::PUS_SERVICE_23);

	/* PUS Gateway Services using CommandingServiceBase */
	new Service2DeviceAccess(objects::PUS_SERVICE_2);
	new Service6MemoryManagement(objects::PUS_SERVICE_6);
	new Service8FunctionManagement(objects::PUS_SERVICE_8);
	new CService200ModeCommanding(objects::PUS_SERVICE_200);
	new CService201HealthCommanding(objects::PUS_SERVICE_201);

	/*
	 * Device Handlers using DeviceHandlerBase.
	 * These includes work without connected hardware via virtualized
	 * devices and interfaces
	 */
	CookieIF * dummyCookie0 = new DummyCookie(addresses::PCDU);
	new PCDUHandler(objects::PCDU_HANDLER,objects::DUMMY_ECHO_COM_IF,
			dummyCookie0);
	CookieIF * dummyCookie1 = new DummyCookie(addresses::DUMMY_ECHO);
	new TestDevice(objects::DUMMY_HANDLER, objects::DUMMY_ECHO_COM_IF,
			dummyCookie1,switches::DUMMY);

	/* Data Structures, pool initialization */
	/* GPS Data Structures */
	struct GpsInit::navDataIdStruct gps0;
	struct GpsInit::navDataIdStruct gps1;
	GpsInit::gpsIdStructInit(&gps0,&gps1);
	//struct hk::hkIdStruct hkIdStruct;
	/* Test Data Structure */
	struct TestInit::TestIdStruct test;
	TestInit::TestStructInit(&test);
	//hk::initHkStruct(&hkIdStruct, gps0, gps1, test);
	//hk::hkInit(housekeepingService, hkIdStruct);

	DummyCookie * dummyCookie2 = new DummyCookie(addresses::DUMMY_GPS0);
#if defined(stm32) || defined(VIRTUAL_BUILD)
	new GPSHandler(objects::GPS0_HANDLER, objects::DUMMY_GPS_COM_IF, dummyCookie2,
			switches::GPS0,gps0);
	DummyCookie * dummyCookie3 = new DummyCookie(addresses::DUMMY_GPS1);
	new GPSHandler(objects::GPS1_HANDLER, objects::DUMMY_GPS_COM_IF, dummyCookie3,
			switches::GPS1,gps1);
	/*
	 * All includes with real devices and connected hardware.
	 * This will be the final flying version
	 */
#elif defined(at91sam9g20)
	new GPSHandler(objects::GPS0_HANDLER,objects::DUMMY_GPS_COM_IF,
			dummyCookie2, switches::GPS0,gps0);
	new GPSHandler(objects::GPS1_HANDLER,objects::DUMMY_GPS_COM_IF,
			dummyCookie2, switches::GPS1,gps1);
#endif

	/* Dummy Communication Interfaces */
	new DummyEchoComIF(objects::DUMMY_ECHO_COM_IF);
	new DummyGPSComIF(objects::DUMMY_GPS_COM_IF);

	/* Test Tasks */
	new TestTask(objects::TEST_TASK);
	/* Board dependant files */

}

void Factory::setStaticFrameworkObjectIds() {
	PusServiceBase::packetSource=objects::PUS_PACKET_DISTRIBUTOR;
	PusServiceBase::packetDestination = objects::PUS_FUNNEL;
	VerificationReporter::messageReceiver = objects::PUS_SERVICE_1;
	DeviceHandlerBase::powerSwitcherId = objects::PCDU_HANDLER;
	DeviceHandlerBase::rawDataReceiverId = objects::PUS_SERVICE_2;
	DeviceHandlerFailureIsolation::powerConfirmationId = objects::PCDU_HANDLER;
	TmPacketStored::timeStamperId = objects::PUS_TIME;
	TmFunnel::downlinkDestination = objects::UDP_TMTC_BRIDGE;
}


