#include "bsp_hosted/ObjectFactory.h"
#include "boardtest/TestTaskHost.h"
#include "fsfwconfig/objects/systemObjectList.h"
#include "fsfwconfig/tmtc/apid.h"
#include "fsfwconfig/tmtc/pusIds.h"

#include <test/testinterfaces/TestEchoComIF.h>
#include <test/testinterfaces/DummyCookie.h>
#include <test/testdevices/TestDeviceHandler.h>

#include <fsfw/events/EventManager.h>
#include <fsfw/health/HealthTable.h>
#include <fsfw/internalError/InternalErrorReporter.h>
#include <fsfw/objectmanager/frameworkObjects.h>
#include <fsfw/timemanager/TimeStamper.h>

#include <fsfw/storagemanager/PoolManager.h>
#include <fsfw/tcdistribution/CCSDSDistributor.h>
#include <fsfw/tcdistribution/PUSDistributor.h>
#include <fsfw/tmtcservices/PusServiceBase.h>
#include <fsfw/pus/Service1TelecommandVerification.h>
#include <fsfw/pus/Service2DeviceAccess.h>
#include <fsfw/pus/Service5EventReporting.h>
#include <fsfw/pus/Service8FunctionManagement.h>
#include <fsfw/pus/CService200ModeCommanding.h>
#include <fsfw/pus/Service3Housekeeping.h>

/* UDP server includes */
#include <fsfw/osal/common/UdpTcPollingTask.h>
#include <fsfw/osal/common/UdpTmTcBridge.h>
#include <fsfw/pus/Service20ParameterManagement.h>
#include <fsfw/tmtcpacket/pus/TmPacketBase.h>

/* Mission includes*/
#include <mission/pus/Service17CustomTest.h>
#include <mission/utility/TmFunnel.h>

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
    {
        // TODO: make this configurable via OBSWConfig.h
        LocalPool::LocalPoolConfig poolConfig = {
                {250, 32}, {120, 64}, {100, 128}, {50, 256},
                {25, 512}, {10, 1024}, {10, 2048}
        };
        PoolManager* ipcStore = new PoolManager(objects::IPC_STORE, poolConfig);
        size_t additionalSize = 0;
        size_t storeSize = ipcStore->getTotalSize(&additionalSize);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Allocating " << storeSize + additionalSize << " bytes for the IPC Store.." <<
                std::endl;
#else
        sif::printInfo("Allocating %u bytes for the IPC store..\n", storeSize + additionalSize);
#endif
    }

    {
        LocalPool::LocalPoolConfig poolConfig = {
                {500, 32}, {250, 64}, {120, 128}, {60, 256},
                {30, 512}, {15, 1024}, {10, 2048}
        };
        PoolManager* tmStore = new PoolManager(objects::TM_STORE, poolConfig);
        size_t additionalSize = 0;
        size_t storeSize = tmStore->getTotalSize(&additionalSize);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Allocating " << storeSize + additionalSize << " bytes for the TM Store.." <<
                std::endl;
#else
        sif::printInfo("Allocating %u bytes for the TM store..\n", storeSize + additionalSize);
#endif
    }

    {
        LocalPool::LocalPoolConfig poolConfig = {
                {500, 32}, {250, 64}, {120, 128},
                {60, 256}, {30, 512}, {15, 1024}, {10, 2048}
        };
        PoolManager* tcStore =new PoolManager(objects::TC_STORE, poolConfig);
        size_t additionalSize = 0;
        size_t storeSize = tcStore->getTotalSize(&additionalSize);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Allocating " << storeSize + additionalSize << " bytes for the TC Store.." <<
                std::endl;
#else
        sif::printInfo("Allocating %u bytes for the TC store..\n", storeSize + additionalSize);
#endif
    }

    new TimeStamper(objects::PUS_TIME);

    /* Distributor Tasks */
    new CCSDSDistributor(apid::SOURCE_OBSW, objects::CCSDS_PACKET_DISTRIBUTOR);
    new PUSDistributor(apid::SOURCE_OBSW, objects::PUS_PACKET_DISTRIBUTOR,
            objects::CCSDS_PACKET_DISTRIBUTOR);

    /* TM Destination */
    new TmFunnel(objects::TM_FUNNEL);

    new UdpTmTcBridge(objects::UDP_BRIDGE,
            objects::CCSDS_PACKET_DISTRIBUTOR, objects::TM_STORE,
            objects::TC_STORE);
    new UdpTcPollingTask(objects::UDP_POLLING_TASK,
            objects::UDP_BRIDGE);

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
    new Service20ParameterManagement(objects::PUS_SERVICE_20_PARAMETERS, apid::SOURCE_OBSW,
            pus::PUS_SERVICE_20);


    /* Test Tasks */
    CookieIF* dummyCookie = new DummyCookie(0, 128);
    new TestEchoComIF(objects::DUMMY_INTERFACE);
    new TestDevice(objects::DUMMY_HANDLER_0, objects::DUMMY_INTERFACE, dummyCookie);
    new TestTaskHost(objects::TEST_TASK, false);
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

    TmPacketBase::timeStamperId = objects::PUS_TIME;
    TmFunnel::downlinkDestination = objects::UDP_BRIDGE;
}


