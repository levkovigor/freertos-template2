#include "ObjectFactory.h"
#include <pollingsequence/PollingSequenceFactory.h>
#include <objects/systemObjectList.h>

#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/tasks/PeriodicTaskIF.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/osal/windows/winTaskHelpers.h>
#include <fsfw/timemanager/Stopwatch.h>

#include <mission/utility/InitMission.h>


/* Declare global object manager */
ObjectManagerIF* objectManager;

#if FSFW_CPP_OSTREAM_ENABLED == 1

#include <ostream>

/* Set up output streams */
namespace sif {
ServiceInterfaceStream debug("DEBUG");
ServiceInterfaceStream info("INFO");
ServiceInterfaceStream warning("WARNING");
ServiceInterfaceStream error("ERROR", false, true, true);
}
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */


void initTask();


void initMission() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Initiating mission specific code." << std::endl;
    // Allocate object manager here, as global constructors might not be
    // executed, depending on buildchain
    sif::info << "Creating objects" << std::endl;
#else
    sif::printInfo("Initiating mission specific code\n");
    sif::printInfo("Creating objects\n");
#endif

    objectManager = new ObjectManager(Factory::produce);
    objectManager -> initialize();

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Creating tasks.." << std::endl;
#else
    sif::printInfo("Creating tasks..\n");
#endif
    initTask();
}

void initTask() {
    TaskFactory* taskFactory = TaskFactory::instance();
    if(taskFactory == nullptr) {
        return;
    }

    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    TaskPriority taskPrio = 0;
#ifdef _WIN32
    taskPrio = tasks::makeWinPriority();
#endif

    TaskDeadlineMissedFunction deadlineMissedFunc = nullptr;

#if OBSW_PRINT_MISSED_DEADLINES  == 1
    deadlineMissedFunc = TaskFactory::printMissedDeadline;
#endif

#ifdef __unix__
    taskPrio = 50;
#endif
    /* Packet Distributor Taks */
    PeriodicTaskIF* packetDistributorTask = TaskFactory::instance()-> createPeriodicTask(
            "PACKET_DIST_TASK", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4,
            deadlineMissedFunc);
    result = packetDistributorTask->
            addComponent(objects::CCSDS_PACKET_DISTRIBUTOR);
    if(result != HasReturnvaluesIF::RETURN_OK){
        initmission::printAddObjectError("CCSDS distributor", objects::CCSDS_PACKET_DISTRIBUTOR);
    }
    result = packetDistributorTask->
            addComponent(objects::PUS_PACKET_DISTRIBUTOR);
    if(result != HasReturnvaluesIF::RETURN_OK){
        initmission::printAddObjectError("PUS packet distributor", objects::PUS_PACKET_DISTRIBUTOR);

    }
    result = packetDistributorTask->
            addComponent(objects::CFDP_PACKET_DISTRIBUTOR);
    if(result != HasReturnvaluesIF::RETURN_OK){
        initmission::printAddObjectError("CFDP packet distributor", objects::CFDP_PACKET_DISTRIBUTOR);

    }
    result = packetDistributorTask->addComponent(objects::TM_FUNNEL);
    if(result != HasReturnvaluesIF::RETURN_OK){
        initmission::printAddObjectError("TM funnel", objects::TM_FUNNEL);
    }

#ifdef __unix__
    taskPrio = 50;
#endif
    PeriodicTaskIF* eventTask = taskFactory->createPeriodicTask(
            "EVENT", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.2, deadlineMissedFunc
    );
    result = eventTask->addComponent(objects::EVENT_MANAGER);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("Event Manager", objects::EVENT_MANAGER);
    }

#ifdef __unix__
    taskPrio = 50;
#endif
    // TCPIP bridge
    PeriodicTaskIF* tcpipBridgeTask = TaskFactory::instance()->createPeriodicTask(
            "TCPIP_BRIDGE", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.2,
            deadlineMissedFunc);
    result = tcpipBridgeTask->addComponent(objects::TCPIP_BRIDGE);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("TCPIP Bridge", objects::TCPIP_BRIDGE);
    }
#ifdef __unix__
    taskPrio = 80;
#endif
    // Can be TCP server or UDP polling task, depending on configuration
    PeriodicTaskIF* tcpipPollingTask = TaskFactory::instance()->createPeriodicTask(
            "TCPIP_HELPER", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 2.0,
            deadlineMissedFunc);
    result = tcpipPollingTask->addComponent(objects::TCPIP_HELPER);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("TCPIP Helper", objects::TCPIP_HELPER);
    }

    /* PUS Services */
#ifdef __unix__
    taskPrio = 45;
#endif
    PeriodicTaskIF* pusVerification = taskFactory->createPeriodicTask(
            "PUS_VERIF_1", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.200,
            deadlineMissedFunc);
    result = pusVerification->addComponent(objects::PUS_SERVICE_1_VERIFICATION);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 1", objects::PUS_SERVICE_1_VERIFICATION);
    }

#ifdef __unix__
    taskPrio = 50;
#endif
    PeriodicTaskIF* pusHighPrio = taskFactory->createPeriodicTask(
            "PUS_HIGH_PRIO", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.2, deadlineMissedFunc);
    result = pusHighPrio->addComponent(objects::PUS_SERVICE_2_DEVICE_ACCESS);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 2", objects::PUS_SERVICE_2_DEVICE_ACCESS);
    }
    result = pusHighPrio->addComponent(objects::PUS_SERVICE_5_EVENT_REPORTING);
    if(result != HasReturnvaluesIF::RETURN_OK){
        initmission::printAddObjectError("PUS 5",objects::PUS_SERVICE_5_EVENT_REPORTING);
    }
    result = pusHighPrio->addComponent(objects::PUS_SERVICE_9_TIME_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 9", objects::PUS_SERVICE_9_TIME_MGMT);
    }

#ifdef __unix__
    taskPrio = 40;
#endif
    PeriodicTaskIF* pusMedPrio = taskFactory->createPeriodicTask(
            "PUS_MED_PRIO", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.6, deadlineMissedFunc);
    result = pusMedPrio->addComponent(objects::PUS_SERVICE_3_HOUSEKEEPING);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 3", objects::PUS_SERVICE_3_HOUSEKEEPING);
    }
    result = pusMedPrio->addComponent(objects::PUS_SERVICE_8_FUNCTION_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 8", objects::PUS_SERVICE_8_FUNCTION_MGMT);
    }
    result = pusMedPrio->addComponent(objects::PUS_SERVICE_20_PARAMETERS);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 20", objects::PUS_SERVICE_20_PARAMETERS);
    }
    result = pusMedPrio->addComponent(objects::PUS_SERVICE_200_MODE_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 200", objects::PUS_SERVICE_200_MODE_MGMT);
    }

#ifdef __unix__
    taskPrio = 30;
#endif
    PeriodicTaskIF* pusLowPrio = taskFactory->createPeriodicTask(
            "PUS_LOW_PRIO", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 1.2, deadlineMissedFunc);
    result = pusLowPrio->addComponent(objects::PUS_SERVICE_17_TEST);
    if(result!=HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 17", objects::PUS_SERVICE_17_TEST);
    }

#ifdef __unix__
    taskPrio = 80;
#endif
    /* Test Task */
    PeriodicTaskIF* testTask = TaskFactory::instance()->createPeriodicTask(
            "TEST_TASK", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 5.0, nullptr);
    result = testTask->addComponent(objects::TEST_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("Test Task", objects::TEST_TASK);
    }

#ifdef __unix__
    taskPrio = 80;
#endif
    /* Polling Sequence Table Default */
    FixedTimeslotTaskIF * pollingSequenceTableTaskDefault =
            TaskFactory::instance()-> createFixedTimeslotTask(
                    "PST_DEFAULT", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 2.0, nullptr);
    result = pst::pollingSequenceInitDefault(pollingSequenceTableTaskDefault);
    if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "creating PST failed" << std::endl;
#endif
    }

    /* Core Controller task */
#ifdef __unix__
    taskPrio = 60;
#endif
    PeriodicTaskIF *attitudeController = TaskFactory::instance()->createPeriodicTask(
            "ATTITUDE_CTRL", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE,  0.4, nullptr);
    result = attitudeController->addComponent(objects::ATTITUDE_CONTROLLER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("Attitude Controller", objects::ATTITUDE_CONTROLLER);
    }

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Starting tasks.." << std::endl;
#else
    sif::printInfo("Starting tasks..\n");
#endif

    eventTask-> startTask();
    packetDistributorTask->startTask();
    tcpipBridgeTask->startTask();
    tcpipPollingTask->startTask();

    pusVerification->startTask();
    pusHighPrio->startTask();
    pusMedPrio->startTask();
    pusLowPrio->startTask();

    attitudeController->startTask();
    testTask->startTask();
    pollingSequenceTableTaskDefault->startTask();
}
