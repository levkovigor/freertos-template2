                                                                                    /**
 * @file 	InitMission.cpp
 * @brief 	Includes mission and board specific code initialisation,
 *        	starts all services and initializes mission.
 * @date 	10.08.2019
 * @author  J. Meier, R. Mueller
 * @ingroup init
 */
#include <dataPoolInit.h>
#include <systemObjectList.h>
#include <Factory.h>
#include <PollingSequenceFactory.h>

#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/timemanager/Clock.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/datapoolglob/GlobalDataPool.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>

#include <sam9g20/utility/FreeRTOSStackMonitor.h>
#include <freertos/FreeRTOS.h>


/* Initialize Data Pool */
namespace glob {
GlobalDataPool dataPool(datapool::dataPoolInit);
}

namespace sif {
/* Set up output streams
 * Don't call these streams in HAL callback functions ! */
#ifdef ISIS_OBC_G20
#ifdef ADD_CR
ServiceInterfaceStream debug("DEBUG", true);
ServiceInterfaceStream info("INFO", true);
ServiceInterfaceStream warning("WARNING", true);
ServiceInterfaceStream error("ERROR", true);
#else
ServiceInterfaceStream debug("DEBUG");
ServiceInterfaceStream info("INFO");
ServiceInterfaceStream warning("WARNING");
ServiceInterfaceStream error("ERROR");
#endif
#else
ServiceInterfaceStream debug("DEBUG", true);
ServiceInterfaceStream info("INFO", true);
ServiceInterfaceStream warning("WARNING", true);
ServiceInterfaceStream error("ERROR", true);
#endif
}

/* will be created in main */
ObjectManagerIF* objectManager;

/* Board Tests, not used in mission */
void boardTestTaskInit();

/**
 * @brief   Initializes mission specific implementation of FSFW,
 *          implements and starts all periodic tasks.
 * @details
 *  - All Service header files must be included in the Factory.cpp file
 *    in /config/objects/ to be properly working.
 *  - Mission specific code is mostly located in the /mission/
 *    and /mission/PUS folder.
 *
 * The periodic tasks are created with the PeriodicTaskIF* or
 * FixedTimeslotTaskIF *, using the respective interface priorities and
 * periodic execution times.
 * The desired functionalities (or classes) are added using
 * the addComponent function. All objects are created in "factory.cpp".
 *
 * Task priorities go from 0 to 10 for now.
 *
 * The maximum value can be adapted in the "freeRTOSConfig.h" file.
 * Stack size is specified in words, so the byte size is the number
 * specified times four for an uint32_t stack depth.
 * For stress tests of the software, call getTaskStackHighWatermark()
 * for TaskManagement to track the used stack.
 *
 *  Don't forget to start the tasks !
 * @ingroup init
 */
void initMission(void) {
    sif::info << "Initiating mission specific code." << std::endl;
    ReturnValue_t result;
    // Allocate object manager here, as global constructors
    // might not be executed, depending on buildchain
    sif::info << "Creating objects." << std::endl;
    objectManager = new ObjectManager(Factory::produce);

    objectManager -> initialize();

    // TODO(Robin): We should use GPS time here or assign variable
    // stored in FRAM...
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
#ifdef ETHERNET
    /* EMAC polling task */
    TmTcPollingTask = TaskFactory::instance()->
            createPeriodicTask("EMAC_PollingTask", 8,1024 * 4 ,0.1, nullptr);
    result = TmTcPollingTask->addComponent(objects::EMAC_POLLING_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component EMAC Task failed" << std::endl;
    }

    /* UDP Task for Ethernet Communication */
    TmTcBridge = TaskFactory::instance()->
            createPeriodicTask("UDP_TMTC_TASK",6, 2048 * 4, 0.2, nullptr);
    result = TmTcBridge->addComponent(objects::UDP_TMTC_BRIDGE);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component UDP Bridge Client failed" << std::endl;
    }
#else
    /* Serial Polling Task */
    TmTcPollingTask = TaskFactory::instance()->
            createPeriodicTask("SERIAL_TC_POLLING", 8, 3096 * 4, 0.1, nullptr);
    result = TmTcPollingTask ->addComponent(objects::SERIAL_POLLING_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component Serial Polling Task failed" << std::endl;
    }

    /* Serial Bridge Task for UART Communication */
    TmTcBridge = TaskFactory::instance()->
            createPeriodicTask("SERIAL_TMTC_TASK", 7, 2048 * 4, 0.1, nullptr);
    result = TmTcBridge->addComponent(objects::SERIAL_TMTC_BRIDGE);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component Serial Bridge failed" << std::endl;
    }
#endif
    /* Utility Task */
    PeriodicTaskIF* TaskMonitorTask = TaskFactory::instance()->
            createPeriodicTask("TASK_MONITOR", 2, 2048 * 4, 2.0, nullptr);
    result = TaskMonitorTask->addComponent(objects::FREERTOS_TASK_MONITOR);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component Task Monitor failed" << std::endl;
    }


    /* Packet Distributor Taks */
    PeriodicTaskIF* PacketDistributorTask =
    		TaskFactory::instance()-> createPeriodicTask(
    		"TMTC_DISTRIBUTOR",6, 2048 * 4, 0.4,nullptr);
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


    /* Polling Sequence Table Default */
    FixedTimeslotTaskIF * PollingSequenceTableTaskDefault =
    TaskFactory::instance()-> createFixedTimeslotTask(
    		"PST_TASK_DEFAULT", 4, 2048 * 4, 0.4, nullptr);
    result = pst::pollingSequenceInitDefault(PollingSequenceTableTaskDefault);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "creating PST failed" << std::endl;
    }


    /* Event Manager */
    PeriodicTaskIF* EventManager = TaskFactory::instance()->createPeriodicTask(
    		"EVENT_MANAGER", 8, 2048 * 4, 0.2, NULL);
    result = EventManager->addComponent(objects::EVENT_MANAGER);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component Event Manager failed" << std::endl;
    }

    /* PUS Services */
    PeriodicTaskIF* PusService01 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_VERFICIATION_1", 5, 2048 * 4, 0.4, nullptr);
    result = PusService01->addComponent(objects::PUS_SERVICE_1);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Verification Service 1 failed" << std::endl;
    }

    PeriodicTaskIF* PusService02 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_DEVICE_ACCESS_2", 5, 2048 * 4, 0.2, nullptr);
    result = PusService02->addComponent(objects::PUS_SERVICE_2);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Device Access Service 2 failed" << std::endl;
    }

    PeriodicTaskIF* PusService03 = TaskFactory::instance()-> createPeriodicTask(
    		"PUS_HOUSEKEEPING_3", 4, 2048 * 4, 0.2, nullptr);
    result = PusService03->addComponent(objects::PUS_SERVICE_3/*_PSB*/);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Housekeeping Service 3 failed" << std::endl;
    }

    PeriodicTaskIF* PusService05 = TaskFactory::instance()-> createPeriodicTask(
    		"PUS_EVENT_REPORTER_5", 4, 2048 * 4, 0.2, nullptr);
    result = PusService05->addComponent(objects::PUS_SERVICE_5);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Event Reporting "
                 "Service 5 failed" << std::endl;
    }

    PeriodicTaskIF* PusService06 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_MEMORY_MGMT_6", 4, 2048 * 4, 0.2, nullptr);
    result = PusService06->addComponent(objects::PUS_SERVICE_6);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Memory Management "
                 "Service 6 failed" << std::endl;
    }

    PeriodicTaskIF* PusService08 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_FUNCTION_MGMT_8", 3, 2048 * 4, 0.8, nullptr);
    result = PusService08 -> addComponent(objects::PUS_SERVICE_8);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Function Management "
                 "Service 8 failed" << std::endl;
    }

    PeriodicTaskIF* PusService09 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_TIME_9", 5, 1024 * 4, 0.4, nullptr);
    result = PusService09->addComponent(objects::PUS_SERVICE_9);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Add component PUS Time "
                 "Management Service 9 failed" << std::endl;
    }

    PeriodicTaskIF* PusService17 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_TEST_17", 2, 1024 * 4, 2, nullptr);
    result = PusService17->addComponent(objects::PUS_SERVICE_17);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component PUS Test Service 17 failed" << std::endl;
    }

    PeriodicTaskIF* PusService20 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_PARAMETER_SERVICE_20", 3, 2048 * 4, 2, nullptr);
    result = PusService20->addComponent(objects::PUS_SERVICE_20);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component PUS Parameter Service 20 failed" << std::endl;
    }

    PeriodicTaskIF* PusService23 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_FILE_MGMT_23", 3, 2048 * 4, 2, nullptr);
    result = PusService23->addComponent(objects::PUS_SERVICE_23);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component PUS Test Service 23 failed" << std::endl;
    }

    PeriodicTaskIF* PusService200 =
            TaskFactory::instance()->createPeriodicTask(
            "PUS_MODE_MGMT_200", 2, 2048 * 4, 1.2, nullptr);
    result = PusService200->addComponent(objects::PUS_SERVICE_200);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component PUS Mode "
                 "Commanding Service 200 failed" << std::endl;
    }

    PeriodicTaskIF* PusService201 =
            TaskFactory::instance()->createPeriodicTask(
            "PUS_HEALTH_MGMT_201", 3, 1024 * 4, 1.2, nullptr);
    result = PusService201->addComponent(objects::PUS_SERVICE_201);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component PUS Health "
                 "Commanding Service 201 failed" << std::endl;
    }

    /* Test Task */
    PeriodicTaskIF* TestTask = TaskFactory::instance()->
            createPeriodicTask("TEST_TASK", 3, 2048 * 4, 1, nullptr);
    result = TestTask->addComponent(objects::TEST_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component Test Task failed" << std::endl;
    }

    /* SD Card handler task */
    PeriodicTaskIF* SDcardTask = TaskFactory::instance()->
            createPeriodicTask("SD_CARD_TASK",2,2048 * 4, 1, nullptr);
    result = SDcardTask->addComponent(objects::SD_CARD_HANDLER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component SDCardHandler to SDCardTask failed" << std::endl;
    }

    /* Core Controller task */
    PeriodicTaskIF* SystemStateTask = TaskFactory::instance()->
            createPeriodicTask("SYSTEM_STATE_TSK", 2, 2048 * 4, 10, nullptr);
    result = SystemStateTask->addComponent(objects::SYSTEM_STATE_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component Core Controller failed" << std::endl;
    }

    PeriodicTaskIF* CoreController = TaskFactory::instance()->
            createPeriodicTask("CORE_CONTROLLER", 6, 2048 * 4, 1, nullptr);
    result = CoreController->addComponent(objects::CORE_CONTROLLER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component Core Controller failed" << std::endl;
    }

    /* SPI Communication Interface*/
    PeriodicTaskIF* SpiComTask = TaskFactory::instance()->
            createPeriodicTask("SPI_COM_IF", 8, 1024 * 4, 0.4, nullptr);
    result = SpiComTask->addComponent(objects::SPI_DEVICE_COM_IF);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Adding component SPI Com Task failed" << std::endl;
    }

    sif::info << "Starting mission tasks.." << std::endl;

    TmTcPollingTask -> startTask();
    TmTcBridge -> startTask();
    PacketDistributorTask -> startTask();
    PollingSequenceTableTaskDefault -> startTask();
    EventManager -> startTask();

    PusService01 -> startTask();
    PusService02 -> startTask();
    PusService03 -> startTask();
    PusService05 -> startTask();
    PusService06 -> startTask();
    PusService08 -> startTask();
    PusService09 -> startTask();
    PusService17 -> startTask();
    PusService20 -> startTask();
    PusService23 -> startTask();
    PusService200 -> startTask();
    PusService201 -> startTask();

    TestTask -> startTask();
    SpiComTask->startTask();
    SDcardTask -> startTask();
    SystemStateTask -> startTask();
    CoreController->startTask();

    /* Task Monitor can be used to track stack usage of tasks */
    // todo: will be replaced and performed for each task by using freeRTOS
    // functionality
    FreeRTOSStackMonitor* TaskMonitor = objectManager->
            get<FreeRTOSStackMonitor>(objects::FREERTOS_TASK_MONITOR);
    TaskMonitor->insertPeriodicTask(objects::SERIAL_POLLING_TASK,
            TmTcPollingTask);
    TaskMonitor->insertPeriodicTask(objects::SERIAL_TMTC_BRIDGE, TmTcBridge);
    TaskMonitor->insertPeriodicTask(objects::PUS_PACKET_DISTRIBUTOR,
            PacketDistributorTask);
    TaskMonitor->insertPeriodicTask(objects::PUS_PACKET_DISTRIBUTOR,
            PacketDistributorTask);
    TaskMonitor->insertPeriodicTask(objects::EVENT_MANAGER, EventManager);

    TaskMonitor->insertPeriodicTask(objects::PUS_SERVICE_1, PusService01);
    TaskMonitor->insertPeriodicTask(objects::PUS_SERVICE_2, PusService02);
    TaskMonitor->insertPeriodicTask(objects::PUS_SERVICE_3, PusService03);
    TaskMonitor->insertPeriodicTask(objects::PUS_SERVICE_5, PusService05);
    TaskMonitor->insertPeriodicTask(objects::PUS_SERVICE_6, PusService06);
    TaskMonitor->insertPeriodicTask(objects::PUS_SERVICE_8, PusService08);
    TaskMonitor->insertPeriodicTask(objects::PUS_SERVICE_9, PusService09);
    TaskMonitor->insertPeriodicTask(objects::PUS_SERVICE_17, PusService17);
    TaskMonitor->insertPeriodicTask(objects::PUS_SERVICE_23, PusService23);
    TaskMonitor->insertPeriodicTask(objects::PUS_SERVICE_200, PusService200);
    TaskMonitor->insertPeriodicTask(objects::PUS_SERVICE_201, PusService201);

    TaskMonitor->insertPeriodicTask(objects::TEST_TASK, TestTask);
    TaskMonitor->insertPeriodicTask(objects::SPI_DEVICE_COM_IF, SpiComTask);
    TaskMonitor->insertFixedTimeslotTask(objects::DUMMY_HANDLER,
            PollingSequenceTableTaskDefault);

    TaskMonitor->setPeriodicOperation(10);
    //taskMonitor->setPrintMode();
    TaskMonitor->setCheckMode(2000);
    TaskMonitorTask->startTask();

    /* Comment out for mission build */
    boardTestTaskInit();

    sif::info << "Remaining heap size: " << std::dec
            << xPortGetFreeHeapSize() << std::endl;
    size_t remainingFactoryStack = TaskManagement::getTaskStackHighWatermark();
    if(remainingFactoryStack < 3000) {
        sif::warning << "Factory Task: Remaining stack size: "
                << remainingFactoryStack << " bytes" << std::endl;
    }

    sif::info << "Tasks started." << std::endl;
}

void boardTestTaskInit() {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
    FreeRTOSStackMonitor* taskMonitor = objectManager->
                get<FreeRTOSStackMonitor>(objects::FREERTOS_TASK_MONITOR);

    /* Polling Sequence Table Test */
    FixedTimeslotTaskIF * PollingSequenceTableTaskTest =
            TaskFactory::instance()->createFixedTimeslotTask(
             "PST_TASK_ARDUINO", 3, 2048 * 4, 0.4, nullptr);
    result = pst::pollingSequenceInitTest(PollingSequenceTableTaskTest);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "creating PST failed" << std::endl;
    }
    taskMonitor->insertFixedTimeslotTask(objects::ARDUINO_0,
            PollingSequenceTableTaskTest);


#ifdef ISIS_OBC_G20
    /* LED Test Task */
    PeriodicTaskIF* LedTask = TaskFactory::instance()->
    		createPeriodicTask("LED_TASK", 1, 1024 * 4, 0.4, nullptr);
    result = LedTask->addComponent(objects::LED_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
    	sif::error << "Add component UART0 Task failed" << std::endl;
    }
#endif

    /* UART0 Test Task */
//  PeriodicTaskIF* UART0Task = TaskFactory::instance()->
//          createPeriodicTask("UART0_TASK",1,2048,2,NULL);
//  result = UART0Task->addComponent(objects::AT91_UART0_TEST_TASK);
//  if (result != HasReturnvaluesIF::RETURN_OK) {
//      sif::error << "Add component UART0 Task failed" << std::endl;
//  }

    /* UART2 Test Task */
//    PeriodicTaskIF* UART2Task = TaskFactory::instance()->
//            createPeriodicTask("UART2_TASK",1,2048,2,NULL);
//    result = UART2Task->addComponent(objects::AT91_UART2_TEST_TASK);
//    if (result != HasReturnvaluesIF::RETURN_OK) {
//        sif::error << "Add component UART2 Task failed" << std::endl;
//  }

    /* I2C Test Task */
//  PeriodicTaskIF* I2CTask = TaskFactory::instance()->
//          createPeriodicTask("I2C_TASK",4,2048 * 4, 0.1, NULL);
//  result = I2CTask->addComponent(objects::AT91_I2C_TEST_TASK);
//  if (result != HasReturnvaluesIF::RETURN_OK) {
//      sif::error << "Add component I2C Task failed" << std::endl;
//  }

    /* SPI Test Task */
//    PeriodicTaskIF* SPITask = TaskFactory::instance()->
//            createPeriodicTask("SPI_TASK",4, 2048, 1, nullptr);
//    result = SPITask->addComponent(objects::AT91_SPI_TEST_TASK);
//    if (result != HasReturnvaluesIF::RETURN_OK) {
//        sif::error << "Add component SPI Task failed" << std::endl;
//    }

    sif::info << "Starting test tasks.." << std::endl;

    //PollingSequenceTableTaskTest -> startTask ();
    //SPITask -> startTask();
    //I2CTask -> startTask();
    //UART2Task -> startTask();
#ifdef ETHERNET
    //UART0Task -> startTask()
#endif
#ifdef ISIS_OBC_G20
    LedTask->startTask();
#endif
}
