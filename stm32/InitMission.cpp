/**
 * @file 	init_mission.cpp
 *
 * @brief 	Includes mission and board specific code initialisation,
 *        	starts all services and initializes mission.
 * @date 	10.08.2019
 * @author 	J. Meier, R. Mueller
 * @ingroup init
 */
#include "Factory.h"
#include <stm32/fsfwconfig/cdatapool/dataPoolInit.h>
#include <stm32/fsfwconfig/objects/systemObjectList.h>
#include <stm32/fsfwconfig/pollingsequence/PollingSequenceFactory.h>

extern "C" {
#include <freertos/FreeRTOS.h>
}

#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/timemanager/Clock.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/datapoolglob/GlobalDataPool.h>
#include <fsfw/objectmanager/ObjectManager.h>


/* Initialize Data Pool */
namespace glob {
GlobalDataPool dataPool(datapool::dataPoolInit);
}

namespace sif {
/* Set up output streams */
ServiceInterfaceStream debug("DEBUG", true);
ServiceInterfaceStream info("INFO", true);
ServiceInterfaceStream error("ERROR", true);
ServiceInterfaceStream warning("WARNING", true);
}

/* will be created in main */
ObjectManagerIF* objectManager;

/* Board Tests, not used in mission */
void boardTestTaskInit();

/**
 * @brief   Initializes mission specific implementation of FSFW,
 *          implements and starts all periodic tasks.
 * @details
 * - All Service header files must be included in the Factory.cpp file
 *   in /config/objects/ to be properly working.
 * - Mission specific code is mostly located in the /mission/
 *   and /mission/PUS folder.
 *
 * Tasks are created with the PeriodicTaskIF* or FixedTimeslotTaskIF *,
 * using the respective interface priorities and periodic execution times.
 * The desired functionalities (or classes) are added using
 * the addComponent function. All objects are created in "factory.cpp".

 *
 * Task priorities go from 0 to 10 for now.
 * The maximum value can be adapted in the "freeRTOSConfig.h" file

 *  * Don't forget to start the tasks !
 * \ingroup init
 */
void initMission(void) {
    sif::info << "Initiating mission specific code." << std::endl;

    ReturnValue_t result;
    // Allocate object manager here, as global constructors
    // might not be executed, depending on buildchain
    sif::info << "Creating objects" << std::endl;
    objectManager = new ObjectManager(Factory::produce);
    objectManager -> initialize();

    timeval currentTime;
    sif::info << "Setting time to 0." << std::endl;
    currentTime.tv_sec = 0;
    currentTime.tv_usec = 0;
    sif::info << "Setting initial clock." << std::endl;
    Clock::setClock(&currentTime);

    sif::info << "Creating tasks.." << std::endl;

    /* TMTC Communication Tasks */
    PeriodicTaskIF * TmTcPollingTask = nullptr;
    PeriodicTaskIF* TmTcBridge = nullptr;

    /* EMAC polling task */
    TmTcPollingTask = TaskFactory::instance()->
            createPeriodicTask("EMAC_PollingTask",8,1024,0.1,NULL);
    result = TmTcPollingTask->addComponent(objects::EMAC_POLLING_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component EMAC Task failed" << std::endl;
    }

    /* UDP Task for Ethernet Communication */
    TmTcBridge = TaskFactory::instance()->
            createPeriodicTask("UDP_TMTC_TASK",6, 2048, 0.2, NULL);
    result = TmTcBridge->addComponent(objects::UDP_TMTC_BRIDGE);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component UDP Bridge Client failed" << std::endl;
    }

    /* Packet Distributor Taks */
    PeriodicTaskIF* PacketDistributorTask = TaskFactory::instance()->
            createPeriodicTask("PACKET_DISTRIBUTOR_TASKS",6,2048, 0.4,NULL);

    result = PacketDistributorTask->
            addComponent(objects::CCSDS_PACKET_DISTRIBUTOR);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component CCSDS Packet Distributor failed" << std::endl;
    }
    result = PacketDistributorTask->
            addComponent(objects::PUS_PACKET_DISTRIBUTOR);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Packet Distributor failed" << std::endl;
    }
    result = PacketDistributorTask->addComponent(objects::TM_FUNNEL);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Funnel failed" << std::endl;
    }


    /* Polling Sequence Table Default */
//    FixedTimeslotTaskIF * PollingSequenceTableTaskDefault =
//    TaskFactory::instance()->
//    createFixedTimeslotTask("POLLING_SEQUENCE_TABLE_DEFAULT",4,1024,0.4,
//            nullptr);
//    result = pst::pollingSequenceInitDefault(PollingSequenceTableTaskDefault);
//    if (result != HasReturnvaluesIF::RETURN_OK) {
//        sif::error << "creating PST failed" << std::endl;
//    }


    /* Event Manager */
    PeriodicTaskIF* EventManager = TaskFactory::instance()->
            createPeriodicTask("EVENT_MANAGER",8,1024,0.2,NULL);
    result = EventManager->addComponent(objects::EVENT_MANAGER);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component Event Manager failed" << std::endl;
    }

    /* PUS Services */
    PeriodicTaskIF* PusService01 = TaskFactory::instance()->
            createPeriodicTask("PUS_VERFICIATION_SERVICE_1",5,2048,0.4,NULL);
    result = PusService01->addComponent(objects::PUS_SERVICE_1_VERIFICATION);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Verification Service 1 failed" << std::endl;
    }

    PeriodicTaskIF* PusService02 = TaskFactory::instance()->
            createPeriodicTask("PUS_DEVICE_ACCESS_SERVICE_2",5,2048,0.2,NULL);
    result = PusService02->addComponent(objects::PUS_SERVICE_2_DEVICE_ACCESS);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Device Access Service 2 failed" << std::endl;
    }

    PeriodicTaskIF* PusService05 = TaskFactory::instance()->
            createPeriodicTask("PUS_EVENT_REPORTER_SERVICE_5",4,3072,0.2,NULL);
    result = PusService05->addComponent(objects::PUS_SERVICE_5_EVENT_REPORTING);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Event Reporting "
                 "Service 5 failed" << std::endl;
    }

    PeriodicTaskIF* PusService06 = TaskFactory::instance()->
            createPeriodicTask("PUS_MEMORY_MGMT_SERVICE_6",4,2048,0.2,NULL);
    result = PusService06->addComponent(objects::PUS_SERVICE_6);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Memory Management "
                 "Service 6 failed" << std::endl;
    }

    PeriodicTaskIF* PusService08 = TaskFactory::instance()->
            createPeriodicTask("PUS_FUNCTION_MGMT_SERVICE_8",3,2048,0.8,NULL);
    result = PusService08 -> addComponent(objects::PUS_SERVICE_8_FUNCTION_MGMT);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Function Management "
                 "Service 8 failed" << std::endl;
    }

    PeriodicTaskIF* PusService09 = TaskFactory::instance()->
            createPeriodicTask("PUS_TIME_SERVICE_9", 5, 2048, 0.4, nullptr);
    result = PusService09->addComponent(objects::PUS_SERVICE_9_TIME_MGMT);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Time "
                 "Management Service 9 failed" << std::endl;
    }

    PeriodicTaskIF* PusService17 = TaskFactory::instance()->
            createPeriodicTask("PUS_TEST_SERVICE_17",2,1024,2, nullptr);
    result = PusService17->addComponent(objects::PUS_SERVICE_17_TEST);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component PUS Test Service 17 failed" << std::endl;
    }

//    PeriodicTaskIF* PusService23 = TaskFactory::instance()->
//            createPeriodicTask("PUS_FILEMANAGEMENT_SERVICE_23",3,2048,2,nullptr);
//    result = PusService23->addComponent(objects::PUS_SERVICE_23_FILE_MGMT);
//    if (result != HasReturnvaluesIF::RETURN_OK) {
//        sif::error << "Add component PUS Test Service 23 failed" << std::endl;
//    }

    PeriodicTaskIF* PusService200 =
            TaskFactory::instance()->createPeriodicTask(
            "PUS_MODE_COMMANDING_SERVICE_200",2,2048,1.2,NULL);
    result = PusService200->addComponent(objects::PUS_SERVICE_200_MODE_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component PUS Mode "
                 "Commanding Service 200 failed" << std::endl;
    }

    PeriodicTaskIF* PusService201 =
            TaskFactory::instance()->createPeriodicTask(
            "PUS_HEALTH_COMMANDING_SERVICE_201",3,2048,1.2,NULL);
    result = PusService201->addComponent(objects::PUS_SERVICE_201);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component PUS Health "
                 "Commanding Service 201 failed" << std::endl;
    }

    /* Test Task */
    PeriodicTaskIF* TestTask = TaskFactory::instance()->
            createPeriodicTask("TEST_TASK",2,1048,1,NULL);
    result = TestTask->addComponent(objects::TEST_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component Test Task failed" << std::endl;
    }

    /* Comment out for mission build */
    boardTestTaskInit();

    sif::info << "Free heap size: " << std::dec <<
            xPortGetFreeHeapSize() << "\n\r" << std::flush;
    sif::info << "Starting mission tasks..\n\r" << std::flush;

    TmTcPollingTask -> startTask();
    TmTcBridge -> startTask();
    PacketDistributorTask -> startTask();
    //PollingSequenceTableTaskDefault -> startTask();
    EventManager -> startTask();

    PusService01 -> startTask();
    PusService02 -> startTask();
    //PusService03 -> startTask();
    PusService05 -> startTask();
    PusService06 -> startTask();
    PusService08 -> startTask();
    PusService09 -> startTask();
    PusService17 -> startTask();
    //PusService23 -> startTask();
    PusService200 -> startTask();
    PusService201 -> startTask();

    TestTask -> startTask();

    sif::info << "Tasks started\n\r" << std::flush;

    TaskFactory::instance()->deleteTask();
    for (;;) {
    }
}

void boardTestTaskInit() {
}
