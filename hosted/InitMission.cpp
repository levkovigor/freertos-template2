#include <PollingSequenceFactory.h>
#include <dataPoolInit.h>
#include <Factory.h>
#include <systemObjectList.h>

#include <unittest/internal/InternalUnitTester.h>
#include <unittest/internal/IntTestMutex.h>
#include <fsfw/datapoolglob/GlobalDataPool.h>
#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/tasks/PeriodicTaskIF.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

#include <ostream>

/* Declare global object manager */
ObjectManagerIF* objectManager;

/* Initialize Data Pool */
namespace glob {
GlobalDataPool dataPool(datapool::dataPoolInit);
}


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
#if defined(hosted)
	testmutex::testMutex();
	sif::info << "Test" << std::endl;
	return;
#endif
	// Allocate object manager here, as global constructors might not be
	// executed, depending on buildchain
	sif::info << "Creating objects" << std::endl;
	objectManager = new ObjectManager(Factory::produce);
	objectManager -> initialize();

	initTask();

}

void initTask() {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;

    /* Packet Distributor Taks */
    PeriodicTaskIF* PacketDistributorTask =
    		TaskFactory::instance()-> createPeriodicTask(
    		"PACKET_DIST_TASK", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE,
			0.4, nullptr);
    result = PacketDistributorTask->
            addComponent(objects::CCSDS_PACKET_DISTRIBUTOR);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component CCSDS Packet Distributor failed"
        		<< std::endl;
    }
    result = PacketDistributorTask->
            addComponent(objects::PUS_PACKET_DISTRIBUTOR);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Packet Distributor failed"
        		<< std::endl;
    }
    result = PacketDistributorTask->addComponent(objects::PUS_FUNNEL);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Funnel failed" << std::endl;
    }

#ifdef linux
    /* Unix UDP bridge */
    PeriodicTaskIF* UdpBridgeTask = TaskFactory::instance()->createPeriodicTask(
    		"UDP_UNIX_BRIDGE", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE,
			0.2, nullptr);
    result = UdpBridgeTask->addComponent(objects::UNIX_UDP_BRIDGE);
    if(result != HasReturnvaluesIF::RETURN_OK) {
    	sif::error << "Add component UDP Unix Bridge failed" << std::endl;
    }

    PeriodicTaskIF* UdpPollingTask = TaskFactory::instance()->
    		createPeriodicTask("UDP_POLLING", 80,
    		PeriodicTaskIF::MINIMUM_STACK_SIZE, 0.01, nullptr);
    result = UdpPollingTask->addComponent(objects::UNIX_UDP_POLLING_TASK);
    if(result != HasReturnvaluesIF::RETURN_OK) {
    	sif::error << "Add component UDP Unix Bridge failed" << std::endl;
    }
#endif

    /* PUS Services */
    PeriodicTaskIF* PusService1 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_1", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE,
			0.4, nullptr);
    result = PusService1->addComponent(objects::PUS_SERVICE_1);
    if(result != HasReturnvaluesIF::RETURN_OK) {
    	sif::error << "Add component Verification Reporter failed" << std::endl;
    }

    PeriodicTaskIF* PusService2 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_2", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE,
			0.2, nullptr);
    result = PusService2->addComponent(objects::PUS_SERVICE_2);
    if(result != HasReturnvaluesIF::RETURN_OK) {
    	sif::error << "Add component Device Access failed" << std::endl;
    }

    PeriodicTaskIF* PusService5 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_5", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE,
			0.4, nullptr);
    result = PusService5->addComponent(objects::PUS_SERVICE_5);
    if(result != HasReturnvaluesIF::RETURN_OK) {
    	sif::error << "Add component Event Service failed" << std::endl;
    }

    PeriodicTaskIF* PusService8 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_8", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE,
			0.4, nullptr);
    result = PusService2->addComponent(objects::PUS_SERVICE_8);
    if(result != HasReturnvaluesIF::RETURN_OK) {
    	sif::error << "Add component Function MGMT failed" << std::endl;
    }

    PeriodicTaskIF* PusService17 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_17", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE,
			0.4, nullptr);
    result = PusService17->addComponent(objects::PUS_SERVICE_17);
    if(result != HasReturnvaluesIF::RETURN_OK) {
    	sif::error << "Add component Test Service failed" << std::endl;
    }

    PeriodicTaskIF* PusService200 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_SRV_200", 50, PeriodicTaskIF::MINIMUM_STACK_SIZE,
			0.4, nullptr);
    result = PusService200->addComponent(objects::PUS_SERVICE_200);
    if(result != HasReturnvaluesIF::RETURN_OK) {
    	sif::error << "Add component Mode MGMT failed" << std::endl;
    }


	/* Test Task */
	PeriodicTaskIF* TestTask = TaskFactory::instance()->
			createPeriodicTask("TEST_TASK", 80,
			PeriodicTaskIF::MINIMUM_STACK_SIZE, 5.0, nullptr);
	result = TestTask->addComponent(objects::TEST_TASK);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "Add component Test Task failed" << std::endl;
	}


	/* Polling Sequence Table Default */
	FixedTimeslotTaskIF * PollingSequenceTableTaskDefault =
			TaskFactory::instance()-> createFixedTimeslotTask(
			"PST_DEFAULT", 80, PeriodicTaskIF::MINIMUM_STACK_SIZE, 2.0,
			nullptr);
	result = pst::pollingSequenceInitDefault(PollingSequenceTableTaskDefault);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "creating PST failed" << std::endl;
	}

	TestTask->startTask();
	PacketDistributorTask->startTask();
	PollingSequenceTableTaskDefault->startTask();
#ifdef linux
	UdpBridgeTask->startTask();
	UdpPollingTask->startTask();
#endif

	PusService1->startTask();
	PusService2->startTask();
	PusService5->startTask();
	PusService8->startTask();
	PusService17->startTask();
	PusService200->startTask();
}
