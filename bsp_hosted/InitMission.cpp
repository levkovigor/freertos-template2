#include <pollingsequence/PollingSequenceFactory.h>
#include <objects/systemObjectList.h>

#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/tasks/PeriodicTaskIF.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/timemanager/Stopwatch.h>

#include <mission/utility/InitMission.h>
#include <ObjectFactory.h>

#include <ostream>

/* Declare global object manager */
ObjectManagerIF* objectManager;

/* Set up output streams */
namespace sif {
ServiceInterfaceStream debug("DEBUG");
ServiceInterfaceStream info("INFO");
ServiceInterfaceStream warning("WARNING");
ServiceInterfaceStream error("ERROR", false, true, true);
}


void initTask();


void initMission() {
	sif::info << "Initiating mission specific code." << std::endl;
	// Allocate object manager here, as global constructors might not be
	// executed, depending on buildchain
	sif::info << "Creating objects" << std::endl;

	objectManager = new ObjectManager(Factory::produce);
	objectManager -> initialize();

	sif::info << "Creating tasks.." << std::endl;
	initTask();
}

void initTask() {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;

    /* Packet Distributor Taks */
    PeriodicTaskIF* PacketDistributorTask = TaskFactory::instance()-> createPeriodicTask(
    		"PACKET_DIST_TASK", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
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

    /* UDP bridge */
    PeriodicTaskIF* UdpBridgeTask = TaskFactory::instance()->createPeriodicTask(
    		"UDP_UNIX_BRIDGE", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.2, nullptr);
    result = UdpBridgeTask->addComponent(objects::UDP_BRIDGE);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("UDP Bridge", objects::UDP_BRIDGE);
    }
    PeriodicTaskIF* UdpPollingTask = TaskFactory::instance()->createPeriodicTask(
            "UDP_POLLING", 80, PeriodicTaskIF::MINIMUM_STACK_SIZE, 2.0, nullptr);
    result = UdpPollingTask->addComponent(objects::UDP_POLLING_TASK);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("UDP Polling", objects::UDP_POLLING_TASK);
    }

    /* PUS Services */
    PeriodicTaskIF* PusService1 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_1", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PusService1->addComponent(objects::PUS_SERVICE_1_VERIFICATION);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 1", objects::PUS_SERVICE_1_VERIFICATION);
    }

    PeriodicTaskIF* PusService2 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_2", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.2, nullptr);
    result = PusService2->addComponent(objects::PUS_SERVICE_2_DEVICE_ACCESS);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 2", objects::PUS_SERVICE_2_DEVICE_ACCESS);
    }

    PeriodicTaskIF* PusService5 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_5", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PusService5->addComponent(objects::PUS_SERVICE_5_EVENT_REPORTING);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 5", objects::PUS_SERVICE_5_EVENT_REPORTING);
    }

    PeriodicTaskIF* PusService8 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_8", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PusService2->addComponent(objects::PUS_SERVICE_8_FUNCTION_MGMT);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 8", objects::PUS_SERVICE_8_FUNCTION_MGMT);
    }

    PeriodicTaskIF* PusService17 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_17", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PusService17->addComponent(objects::PUS_SERVICE_17_TEST);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 17", objects::PUS_SERVICE_17_TEST);
    }

    PeriodicTaskIF* PusService200 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_200", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.4, nullptr);
    result = PusService200->addComponent(objects::PUS_SERVICE_200_MODE_MGMT);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 200", objects::PUS_SERVICE_200_MODE_MGMT);
    }

	/* Test Task */
	PeriodicTaskIF* TestTask = TaskFactory::instance()->createPeriodicTask(
	        "TEST_TASK", 80, PeriodicTaskIF::MINIMUM_STACK_SIZE, 5.0, nullptr);
	result = TestTask->addComponent(objects::TEST_TASK);
	if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("Test Task", objects::TEST_TASK);
	}


	/* Polling Sequence Table Default */
	FixedTimeslotTaskIF * PollingSequenceTableTaskDefault =
	        TaskFactory::instance()-> createFixedTimeslotTask(
			"PST_DEFAULT", 80, PeriodicTaskIF::MINIMUM_STACK_SIZE, 2.0, nullptr);
	result = pst::pollingSequenceInitDefault(PollingSequenceTableTaskDefault);
	if (result != HasReturnvaluesIF::RETURN_OK) {
	    sif::error << "creating PST failed" << std::endl;
	}

	PeriodicTaskIF *attitudeController = TaskFactory::instance()->createPeriodicTask(
	        "ATTITUDE_CTRL", 6, 2048 * 4, 0.4, nullptr);
	result = attitudeController->addComponent(objects::ATTITUDE_CONTROLLER);
	if (result != HasReturnvaluesIF::RETURN_OK) {
	    initmission::printAddObjectError("Attitude Controller", objects::ATTITUDE_CONTROLLER);
	}

	sif::info << "Starting tasks.." << std::endl;
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
	PusService200->startTask();

	attitudeController->startTask();
}
