#include <Factory.h>
#include <systemObjectList.h>
#include <dataPoolInit.h>
#include <apid.h>
#include <pusIds.h>

#include <test/testinterfaces/DummyEchoComIF.h>

#include <mission/pus/Service17CustomTest.h>
#include <mission/utility/TimeStamper.h>
#include <mission/utility/TmFunnel.h>

#include <fsfw/events/EventManager.h>
#include <fsfw/health/HealthTable.h>
#include <fsfw/internalError/InternalErrorReporter.h>
#include <fsfw/objectmanager/frameworkObjects.h>

#include <fsfw/storagemanager/PoolManager.h>
#include <fsfw/tcdistribution/CCSDSDistributor.h>
#include <fsfw/tcdistribution/PUSDistributor.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>
#include <fsfw/tmtcservices/PusServiceBase.h>
#include <fsfw/pus/Service1TelecommandVerification.h>
#include <fsfw/pus/Service2DeviceAccess.h>
#include <fsfw/pus/Service5EventReporting.h>
#include <fsfw/pus/Service8FunctionManagement.h>
#include <hosted/boardtest/TestTaskHost.h>
#include <fsfw/pus/CService200ModeCommanding.h>
#include <fsfw/pus/Service3Housekeeping.h>
#include <test/testdevices/TestDeviceHandler.h>

#ifdef LINUX
#include <fsfw/osal/linux/TmTcUnixUdpBridge.h>
#include <fsfw/osal/linux/TcUnixUdpPollingTask.h>
#elif WIN32
#include <fsfw/osal/windows/TcWinUdpPollingTask.h>
#include <fsfw/osal/windows/TmTcWinUdpBridge.h>
#endif

#include <cstdint>

/**
 * @brief Produces system objects.
 * @details
 * Build tasks by using SystemObject Interface (Interface).
 * Header files of all tasks must be included
 * Please note that an object has to implement the system object interface
 * if the interface validity is checked or retrieved later by using the
 * get<TargetInterface>(object_id) function from the ObjectManagerIF.
 *
 * Framework objects are created first.
 *
 * @ingroup init
 */
void Factory::produce(void) {
	setStaticFrameworkObjectIds();
	new EventManager(objects::EVENT_MANAGER);
	new HealthTable(objects::HEALTH_TABLE);
	new InternalErrorReporter(objects::INTERNAL_ERROR_REPORTER);

	/* Pool manager handles storage und mutexes */
	/* Data Stores. Currently reserving 9600 bytes of memory */
	uint16_t numberOfElements[4] = {100, 30, 20, 10};
	uint16_t sizeOfElements[4] = {32, 64, 128, 256};
	new PoolManager<4>(objects::IPC_STORE, sizeOfElements, numberOfElements);
	new PoolManager<4>(objects::TM_STORE, sizeOfElements, numberOfElements);
	new PoolManager<4>(objects::TC_STORE, sizeOfElements, numberOfElements);

	new TimeStamper(objects::PUS_TIME);

	/* Distributor Tasks */
	new CCSDSDistributor(apid::SOURCE_OBSW, objects::CCSDS_PACKET_DISTRIBUTOR);
	new PUSDistributor(apid::SOURCE_OBSW, objects::PUS_PACKET_DISTRIBUTOR,
			objects::CCSDS_PACKET_DISTRIBUTOR);

	/* TM Destination */
	new TmFunnel(objects::TM_FUNNEL);

#ifdef LINUX
	new TmTcUnixUdpBridge(objects::UDP_BRIDGE,
	        objects::CCSDS_PACKET_DISTRIBUTOR, objects::TM_STORE,
	        objects::TC_STORE);
	new TcUnixUdpPollingTask(objects::UDP_POLLING_TASK,
	        objects::UDP_BRIDGE);
#elif WIN32
	new TmTcWinUdpBridge(objects::UDP_BRIDGE,
	        objects::CCSDS_PACKET_DISTRIBUTOR, objects::TM_STORE,
	        objects::TC_STORE);
	new TcWinUdpPollingTask(objects::UDP_POLLING_TASK,
	        objects::UDP_BRIDGE);
#endif

	/* PUS Service Base Services */
	new Service1TelecommandVerification(objects::PUS_SERVICE_1_VERIFICATION,
			apid::SOURCE_OBSW, pus::PUS_SERVICE_1, objects::TM_FUNNEL, 30);
	new Service3Housekeeping(objects::PUS_SERVICE_3_HOUSEKEEPING, apid::SOURCE_OBSW,
			pus::PUS_SERVICE_3);
	new Service5EventReporting(objects::PUS_SERVICE_5_EVENT_REPORTING, apid::SOURCE_OBSW,
			pus::PUS_SERVICE_5);
	new Service17CustomTest(objects::PUS_SERVICE_17_TEST, apid::SOURCE_OBSW,
			pus::PUS_SERVICE_17);

	/* Commanding Service Base Services */
	new Service2DeviceAccess(objects::PUS_SERVICE_2_DEVICE_ACCESS,
			apid::SOURCE_OBSW, pus::PUS_SERVICE_2);
	new Service8FunctionManagement(objects::PUS_SERVICE_8_FUNCTION_MGMT,
			apid::SOURCE_OBSW, pus::PUS_SERVICE_8);
	new CService200ModeCommanding(objects::PUS_SERVICE_200_MODE_MGMT,
			apid::SOURCE_OBSW,pus::PUS_SERVICE_200);


	/* Test Tasks */
	CookieIF* dummyCookie = new TestCookie(0);
	new TestEchoComIF(objects::DUMMY_INTERFACE);
	new TestDevice(objects::DUMMY_HANDLER, objects::DUMMY_INTERFACE,
	        dummyCookie, true);
	new TestTaskHost(objects::TEST_TASK);
}

void Factory::setStaticFrameworkObjectIds() {
	PusServiceBase::packetSource = objects::PUS_PACKET_DISTRIBUTOR;
	PusServiceBase::packetDestination = objects::TM_FUNNEL;

	CommandingServiceBase::defaultPacketSource = objects::PUS_PACKET_DISTRIBUTOR;
	CommandingServiceBase::defaultPacketDestination = objects::TM_FUNNEL;

	LocalDataPoolManager::defaultHkDestination = objects::PUS_SERVICE_3_HOUSEKEEPING;

	VerificationReporter::messageReceiver = objects::PUS_SERVICE_1_VERIFICATION;
	DeviceHandlerBase::rawDataReceiverId = objects::PUS_SERVICE_2_DEVICE_ACCESS;
	DeviceHandlerBase::powerSwitcherId = objects::NO_OBJECT;

	TmPacketStored::timeStamperId = objects::PUS_TIME;
	TmFunnel::downlinkDestination = objects::UDP_BRIDGE;
}


