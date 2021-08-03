#include "CatchFactory.h"
#include "objects/systemObjectList.h"

#include <fsfw/datapoollocal/LocalDataPoolManager.h>
#include <fsfw/devicehandlers/DeviceHandlerBase.h>

#include <fsfw/events/EventManager.h>
#include <fsfw/health/HealthTable.h>
#include <fsfw/internalerror/InternalErrorReporter.h>
#include <fsfw/objectmanager/frameworkObjects.h>
#include <fsfw/storagemanager/PoolManager.h>
#include <fsfw/tmtcpacket/pus/tm.h>
#include <fsfw/tmtcservices/CommandingServiceBase.h>
#include <fsfw/tmtcservices/PusServiceBase.h>
#include <fsfw_tests/unit/mocks/HkReceiverMock.h>

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
void Factory::produce(void* args) {
	setStaticFrameworkObjectIds();
	new EventManager(objects::EVENT_MANAGER);
	new HealthTable(objects::HEALTH_TABLE);
	new InternalErrorReporter(objects::INTERNAL_ERROR_REPORTER);

	//new LocalPoolOwnerBase (objects::TEST_LOCAL_POOL_OWNER_BASE);
	new HkReceiverMock(objects::HK_RECEIVER_MOCK);

	{
		PoolManager::LocalPoolConfig poolCfg = {
		        {100, 16}, {50, 32}, {25, 64} , {15, 128}, {5, 1024}
		};
		new PoolManager(objects::TC_STORE, poolCfg);
	}

	{
        PoolManager::LocalPoolConfig poolCfg = {
                {100, 16}, {50, 32}, {25, 64} , {15, 128}, {5, 1024}
        };
		new PoolManager(objects::TM_STORE, poolCfg);
	}

	{
        PoolManager::LocalPoolConfig poolCfg = {
                {100, 16}, {50, 32}, {25, 64} , {15, 128}, {5, 1024}
        };
		new PoolManager(objects::IPC_STORE, poolCfg);
	}

}

void Factory::setStaticFrameworkObjectIds() {
	PusServiceBase::packetSource = objects::NO_OBJECT;
	PusServiceBase::packetDestination = objects::NO_OBJECT;

	CommandingServiceBase::defaultPacketSource = objects::NO_OBJECT;
	CommandingServiceBase::defaultPacketDestination = objects::NO_OBJECT;

	VerificationReporter::messageReceiver = objects::PUS_SERVICE_1_VERIFICATION;

	DeviceHandlerBase::powerSwitcherId = objects::NO_OBJECT;
	DeviceHandlerBase::rawDataReceiverId = objects::PUS_SERVICE_2_DEVICE_ACCESS;

	LocalDataPoolManager::defaultHkDestination = objects::HK_RECEIVER_MOCK;
	DeviceHandlerFailureIsolation::powerConfirmationId = objects::NO_OBJECT;

	TmPacketBase::timeStamperId = objects::NO_OBJECT;
}



