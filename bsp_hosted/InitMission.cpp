#include <pollingsequence/PollingSequenceFactory.h>
#include <objects/systemObjectList.h>

#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/tasks/PeriodicTaskIF.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/osal/windows/winTaskHelpers.h>
#include <fsfw/timemanager/Stopwatch.h>

#include <mission/utility/InitMission.h>
#include <ObjectFactory.h>

#include <ostream>

/* Declare global object manager */
ObjectManagerIF* objectManager;

#if FSFW_CPP_OSTREAM_ENABLED == 1
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
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    TaskPriority taskPrio = 0;
#ifdef _WIN32
    taskPrio = tasks::makeWinPriority();
#endif


#ifdef __unix__
    taskPrio = 50;
#endif
    /* Packet Distributor Taks */
    PeriodicTaskIF* PacketDistributorTask = TaskFactory::instance()-> createPeriodicTask(
            "PACKET_DIST_TASK", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PacketDistributorTask->
            addComponent(objects::CCSDS_PACKET_DISTRIBUTOR);
    if(result != HasReturnvaluesIF::RETURN_OK){
        initmission::printAddObjectError("CCSDS distributor", objects::CCSDS_PACKET_DISTRIBUTOR);
    }
    result = PacketDistributorTask->
            addComponent(objects::PUS_PACKET_DISTRIBUTOR);
    if(result != HasReturnvaluesIF::RETURN_OK){
        initmission::printAddObjectError("PUS packet distributor", objects::PUS_PACKET_DISTRIBUTOR);

    }
    result = PacketDistributorTask->addComponent(objects::TM_FUNNEL);
    if(result != HasReturnvaluesIF::RETURN_OK){
        initmission::printAddObjectError("TM funnel", objects::TM_FUNNEL);
    }

#ifdef __unix__
    taskPrio = 50;
#endif
    /* UDP bridge */
    PeriodicTaskIF* UdpBridgeTask = TaskFactory::instance()->createPeriodicTask(
            "UDP_UNIX_BRIDGE", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.2, nullptr);
    result = UdpBridgeTask->addComponent(objects::UDP_BRIDGE);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("UDP Bridge", objects::UDP_BRIDGE);
    }
#ifdef __unix__
    taskPrio = 80;
#endif
    PeriodicTaskIF* UdpPollingTask = TaskFactory::instance()->createPeriodicTask(
            "UDP_POLLING", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 2.0, nullptr);
    result = UdpPollingTask->addComponent(objects::UDP_POLLING_TASK);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("UDP Polling", objects::UDP_POLLING_TASK);
    }

#ifdef __unix__
    taskPrio = 50;
#endif
    /* PUS Services */
    PeriodicTaskIF* PusService1 = TaskFactory::instance()->createPeriodicTask(
            "PUS_SRV_1", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PusService1->addComponent(objects::PUS_SERVICE_1_VERIFICATION);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 1", objects::PUS_SERVICE_1_VERIFICATION);
    }

#ifdef __unix__
    taskPrio = 50;
#endif
    PeriodicTaskIF* PusService2 = TaskFactory::instance()->createPeriodicTask(
            "PUS_SRV_2", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.2, nullptr);
    result = PusService2->addComponent(objects::PUS_SERVICE_2_DEVICE_ACCESS);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 2", objects::PUS_SERVICE_2_DEVICE_ACCESS);
    }

#ifdef __unix__
    taskPrio = 50;
#endif
    PeriodicTaskIF* PusService5 = TaskFactory::instance()->createPeriodicTask(
            "PUS_SRV_5", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PusService5->addComponent(objects::PUS_SERVICE_5_EVENT_REPORTING);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 5", objects::PUS_SERVICE_5_EVENT_REPORTING);
    }

#ifdef __unix__
    taskPrio = 50;
#endif
    PeriodicTaskIF* PusService8 = TaskFactory::instance()->createPeriodicTask(
            "PUS_SRV_8", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PusService2->addComponent(objects::PUS_SERVICE_8_FUNCTION_MGMT);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 8", objects::PUS_SERVICE_8_FUNCTION_MGMT);
    }

#ifdef __unix__
    taskPrio = 50;
#endif
    PeriodicTaskIF* PusService17 = TaskFactory::instance()->createPeriodicTask(
            "PUS_SRV_17", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PusService17->addComponent(objects::PUS_SERVICE_17_TEST);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 17", objects::PUS_SERVICE_17_TEST);
    }

#ifdef __unix__
    taskPrio = 50;
#endif
    PeriodicTaskIF* PusService200 = TaskFactory::instance()->createPeriodicTask(
            "PUS_SRV_200", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PusService200->addComponent(objects::PUS_SERVICE_200_MODE_MGMT);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 200", objects::PUS_SERVICE_200_MODE_MGMT);
    }

#ifdef __unix__
    taskPrio = 50;
#endif
    PeriodicTaskIF* PusService20 = TaskFactory::instance()->createPeriodicTask(
            "PUS_SRV_20", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PusService20->addComponent(objects::PUS_SERVICE_20_PARAMETERS);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 20", objects::PUS_SERVICE_20_PARAMETERS);
    }

#ifdef __unix__
    taskPrio = 80;
#endif
    /* Test Task */
    PeriodicTaskIF* TestTask = TaskFactory::instance()->createPeriodicTask(
            "TEST_TASK", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 5.0, nullptr);
    result = TestTask->addComponent(objects::TEST_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("Test Task", objects::TEST_TASK);
    }

#ifdef __unix__
    taskPrio = 80;
#endif
    /* Polling Sequence Table Default */
    FixedTimeslotTaskIF * PollingSequenceTableTaskDefault =
            TaskFactory::instance()-> createFixedTimeslotTask(
                    "PST_DEFAULT", taskPrio, PeriodicTaskIF::MINIMUM_STACK_SIZE, 2.0, nullptr);
    result = pst::pollingSequenceInitDefault(PollingSequenceTableTaskDefault);
    if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "creating PST failed" << std::endl;
#endif
    }

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Starting tasks.." << std::endl;
#else
    sif::printInfo("Starting tasks..\n");
#endif

    TestTask->startTask();
    PacketDistributorTask->startTask();
    PollingSequenceTableTaskDefault->startTask();
#ifdef LINUX
    UdpBridgeTask->startTask();
    UdpPollingTask->startTask();
#elif WIN32
    UdpBridgeTask->startTask();
    UdpPollingTask->startTask();
#endif

    PusService1->startTask();
    PusService2->startTask();
    PusService5->startTask();
    PusService8->startTask();
    PusService17->startTask();
    PusService20->startTask();
    PusService200->startTask();
}
