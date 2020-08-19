#include <Factory.h>
#include <systemObjectList.h>
#include <dataPoolInit.h>
#include <apid.h>
#include <pusIds.h>

#include <test/testinterfaces/DummyEchoComIF.h>

#include <mission/pus/Service17Test.h>
#include <mission/utility/TimeStamper.h>
#include <mission/utility/TmFunnel.h>

#include <fsfw/events/EventManager.h>
#include <fsfw/health/HealthTable.h>
#include <fsfw/internalError/InternalErrorReporter.h>
#include <fsfw/objectmanager/frameworkObjects.h>
#include <fsfw/osal/linux/TmTcUnixUdpBridge.h>
#include <fsfw/storagemanager/PoolManager.h>
#include <fsfw/tcdistribution/CCSDSDistributor.h>
#include <fsfw/tcdistribution/PUSDistributor.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>
#include <fsfw/tmtcservices/PusServiceBase.h>
#include <fsfw/osal/linux/TcUnixUdpPollingTask.h>
#include <fsfw/pus/Service1TelecommandVerification.h>
#include <fsfw/pus/Service2DeviceAccess.h>
#include <fsfw/pus/Service5EventReporting.h>
#include <fsfw/pus/Service8FunctionManagement.h>
#include <hosted/boardtest/TestTaskHost.h>
#include <fsfw/pus/CService200ModeCommanding.h>
#include <test/testdevices/TestDeviceHandler.h>
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
	new InternalErrorReporter(objects::INTERNAL_ERROR_REPORTER,
				datapool::INTERNAL_ERROR_FULL_MSG_QUEUES,
				datapool::INTERNAL_ERROR_MISSED_LIVE_TM,
				datapool::INTERNAL_ERROR_STORE_FULL);

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
	new TmFunnel(objects::PUS_FUNNEL);
	new TmTcUnixUdpBridge(objects::UNIX_UDP_BRIDGE,
			objects::CCSDS_PACKET_DISTRIBUTOR, objects::TM_STORE,
			objects::TC_STORE);
	new TcUnixUdpPollingTask(objects::UNIX_UDP_POLLING_TASK,
			objects::UNIX_UDP_BRIDGE);

	/* PUS Service Base Services */
	new Service1TelecommandVerification(objects::PUS_SERVICE_1,
			apid::SOURCE_OBSW, pus::PUS_SERVICE_1, objects::PUS_FUNNEL);

	new Service5EventReporting(objects::PUS_SERVICE_5, apid::SOURCE_OBSW,
			pus::PUS_SERVICE_5);
	new Service17Test(objects::PUS_SERVICE_17);

	/* Commanding Service Base Services */
	new Service2DeviceAccess(objects::PUS_SERVICE_2, apid::SOURCE_OBSW,
				pus::PUS_SERVICE_2);
	new Service8FunctionManagement(objects::PUS_SERVICE_8, apid::SOURCE_OBSW,
			pus::PUS_SERVICE_8);
	new CService200ModeCommanding(objects::PUS_SERVICE_200, apid::SOURCE_OBSW,
				pus::PUS_SERVICE_200);


	/* Test Tasks */
	CookieIF* dummyCookie = new DummyCookie(0);
	new DummyEchoComIF(objects::DUMMY_INTERFACE);
	new TestDevice(objects::DUMMY_HANDLER, objects::DUMMY_INTERFACE,
	        dummyCookie, true);
	new TestTaskHost(objects::TEST_TASK);
}

void Factory::setStaticFrameworkObjectIds() {
	PusServiceBase::packetSource = objects::PUS_PACKET_DISTRIBUTOR;
	PusServiceBase::packetDestination = objects::PUS_FUNNEL;

	CommandingServiceBase::defaultPacketSource = objects::PUS_PACKET_DISTRIBUTOR;
	CommandingServiceBase::defaultPacketDestination = objects::PUS_FUNNEL;

	VerificationReporter::messageReceiver = objects::PUS_SERVICE_1;
	DeviceHandlerBase::rawDataReceiverId = objects::PUS_SERVICE_2;
	DeviceHandlerBase::powerSwitcherId = objects::NO_OBJECT;

	TmPacketStored::timeStamperId = objects::PUS_TIME;
	TmFunnel::downlinkDestination = objects::UNIX_UDP_BRIDGE;
}


