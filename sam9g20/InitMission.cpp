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

#include <freertos/FreeRTOS.h>
#include <unittest/internal/InternalUnitTester.h>
#include <config/OBSWConfig.h>
#include <utility/compile_time.h>

#if DISPLAY_FACTORY_ALLOCATION_SIZE == 1
#include <new>
static size_t allocatedSize = 0;
#endif

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
 * Don't forget to start the tasks !
 * @author  R. Mueller, J. Meier
 * @ingroup init
 */
void initMission(void) {
	sif::info << "Initiating mission specific code." << std::endl;

    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    // Allocate object manager here, as global constructors
    // might not be executed, depending on buildchain
    sif::info << "Creating objects." << std::endl;
    objectManager = new ObjectManager(Factory::produce);

    objectManager -> initialize();

    sif::info << "Creating tasks.." << std::endl;

    /* TMTC Communication Tasks */
    PeriodicTaskIF * TmTcPollingTask = nullptr;
    PeriodicTaskIF* TmTcBridge = nullptr;
#ifdef ETHERNET
    /* EMAC polling task */
    TmTcPollingTask = TaskFactory::instance()->
            createPeriodicTask("EMAC_PollingTask", 8, 1024 * 4 , 0.1, nullptr);
    result = TmTcPollingTask->addComponent(objects::EMAC_POLLING_TASK);

    /* UDP Task for Ethernet Communication */
    TmTcBridge = TaskFactory::instance()->
            createPeriodicTask("UDP_TMTC_TASK",6, 2048 * 4, 0.2, nullptr);
    result = TmTcBridge->addComponent(objects::UDP_TMTC_BRIDGE);
#else
    /* Serial Polling Task */
    TmTcPollingTask = TaskFactory::instance()->
            createPeriodicTask("SERIAL_TC_POLLING", 8, 3096 * 4, 0.1, nullptr);
    result = TmTcPollingTask ->addComponent(objects::SERIAL_POLLING_TASK);

    /* Serial Bridge Task for UART Communication */
    TmTcBridge = TaskFactory::instance()->
            createPeriodicTask("SERIAL_TMTC_TASK", 7, 2048 * 4, 0.1, nullptr);
    result = TmTcBridge->addComponent(objects::SERIAL_TMTC_BRIDGE);
#endif

    /* Packet Distributor Taks */
    PeriodicTaskIF* PacketDistributorTask =
    		TaskFactory::instance()-> createPeriodicTask(
    		"TMTC_DISTRIBUTOR", 6, 2048 * 4, 0.4, nullptr);
    result = PacketDistributorTask->
            addComponent(objects::CCSDS_PACKET_DISTRIBUTOR);
    result = PacketDistributorTask->addComponent(
    		objects::PUS_PACKET_DISTRIBUTOR);
    result = PacketDistributorTask->addComponent(objects::PUS_FUNNEL);


    /* Polling Sequence Table Default */
    FixedTimeslotTaskIF * PollingSequenceTableTaskDefault =
    TaskFactory::instance()-> createFixedTimeslotTask(
    		"PST_TASK_DEFAULT", 5, 2048 * 4, 0.4, nullptr);
    result = pst::pollingSequenceInitDefault(PollingSequenceTableTaskDefault);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "creating PST failed" << std::endl;
    }

    /* Event Manager */
    PeriodicTaskIF* EventManager = TaskFactory::instance()->createPeriodicTask(
    		"EVENT_MANAGER", 8, 2048 * 4, 0.2, nullptr);
    result = EventManager->addComponent(objects::EVENT_MANAGER);

    /* Internal Error Reporter */
    PeriodicTaskIF* InternalErrorReporter = TaskFactory::instance()->
            createPeriodicTask("INT_ERR_RPRTR", 1, 1024 * 4, 2.0, nullptr);
    result = InternalErrorReporter->addComponent(
    		objects::INTERNAL_ERROR_REPORTER);

    /* PUS Services */

    /* Verification Service */
    PeriodicTaskIF* PusService01 = TaskFactory::instance()->createPeriodicTask(
    		"PUS_VERFICIATION_1", 5, 2048 * 4, 0.4, nullptr);
    result = PusService01->addComponent(objects::PUS_SERVICE_1_VERIFICATION);

    /* Event Reporter */
    PeriodicTaskIF* PusService05 = TaskFactory::instance()-> createPeriodicTask(
    		"PUS_EVENT_RPRTR_5", 4, 2048 * 4, 0.2, nullptr);
    PusService05->addComponent(objects::PUS_SERVICE_5_EVENT_REPORTING);

    /* PUS High Priority */
    PeriodicTaskIF* PusHighPriorityTask = TaskFactory::instance()->
    		createPeriodicTask("PUS_HIGH_PRIO", 5, 2048 * 4, 0.2, nullptr);
    PusHighPriorityTask->addComponent(objects::PUS_SERVICE_2_DEVICE_ACCESS);
    PusHighPriorityTask->addComponent(objects::PUS_SERVICE_6_MEM_MGMT);
    PusHighPriorityTask->addComponent(objects::PUS_SERVICE_9_TIME_MGMT);

    /* PUS Medium Priority */
    PeriodicTaskIF* PusMediumPriorityTask = TaskFactory::instance()->
    		createPeriodicTask("PUS_MED_PRIO", 4, 2048 * 4, 0.8, nullptr);
    PusMediumPriorityTask->addComponent(objects::PUS_SERVICE_3_HOUSEKEEPING);
    PusMediumPriorityTask->addComponent(objects::PUS_SERVICE_8_FUNCTION_MGMT);
    PusMediumPriorityTask->addComponent(objects::PUS_SERVICE_200_MODE_MGMT);
    PusMediumPriorityTask->addComponent(objects::PUS_SERVICE_201);
    PusMediumPriorityTask->addComponent(objects::PUS_SERVICE_23);

    /* PUS Low Priority */
    PeriodicTaskIF* PusLowPriorityTask = TaskFactory::instance()->
    		createPeriodicTask("PUS_LOW_PRIO", 3, 2048 * 4, 1.6, nullptr);
    PusLowPriorityTask->addComponent(objects::PUS_SERVICE_20);
    PusLowPriorityTask->addComponent(objects::PUS_SERVICE_17_TEST);

    /* SD Card handler task */
    PeriodicTaskIF* SDCardTask = TaskFactory::instance()->
            createPeriodicTask("SD_CARD_TASK", 2, 2048 * 4, 5.0, nullptr);
    result = SDCardTask->addComponent(objects::SD_CARD_HANDLER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component SDCardTask failed" << std::endl;
    }

    /* Software image task */
    PeriodicTaskIF* SoftwareImageTask = TaskFactory::instance()->
            createPeriodicTask("SW_IMG_TASK", 3, 2048 * 4, 2, nullptr);
    result = SoftwareImageTask->addComponent(objects::SOFTWARE_IMAGE_HANDLER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component Software Image Task failed" << std::endl;
    }

    /* Core Controller task */
    PeriodicTaskIF* CoreController = TaskFactory::instance()->
            createPeriodicTask("CORE_CONTROLLER", 6, 2048 * 4, 1, nullptr);
    result = CoreController->addComponent(objects::CORE_CONTROLLER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component Core Controller failed" << std::endl;
    }
    PeriodicTaskIF* SystemStateTask = TaskFactory::instance()->
            createPeriodicTask("SYSTEM_STATE_TSK", 2, 2048 * 4, 100, nullptr);
    result = SystemStateTask->addComponent(objects::SYSTEM_STATE_TASK);
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

    /* Test Task */
    PeriodicTaskIF* TestTask = TaskFactory::instance()->
            createPeriodicTask("TEST_TASK", 1, 3072 * 4, 3, nullptr);
    result = TestTask->addComponent(objects::TEST_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component Test Task failed" << std::endl;
    }

#ifdef DEBUG
    InternalUnitTester unitTestClass;
    result = unitTestClass.performTests();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Framework Unit Test did not run successfully!"
                << std::endl;
    }
#endif

    sif::info << "Starting mission tasks.." << std::endl;

    TmTcPollingTask -> startTask();
    TmTcBridge -> startTask();
    InternalErrorReporter -> startTask();
    PacketDistributorTask -> startTask();
    PollingSequenceTableTaskDefault -> startTask();
    EventManager -> startTask();

    PusService01 -> startTask();
    PusService05 -> startTask();

    PusHighPriorityTask->startTask();
    PusMediumPriorityTask->startTask();
    PusLowPriorityTask->startTask();

    SDCardTask -> startTask();
    SoftwareImageTask -> startTask();

    SystemStateTask -> startTask();
    CoreController->startTask();
    SpiComTask->startTask();

    TestTask -> startTask();

    /* Comment out for mission build */
    boardTestTaskInit();

    sif::info << "Remaining FreeRTOS heap size: " << std::dec
            << xPortGetFreeHeapSize() << " bytes." << std::endl;
    size_t remainingFactoryStack = TaskManagement::getTaskStackHighWatermark();
    if(remainingFactoryStack < 3000) {
        sif::warning << "Factory Task: Remaining stack size: "
                << remainingFactoryStack << " bytes" << std::endl;
    }
#if DISPLAY_FACTORY_ALLOCATION_SIZE == 1
    sif::info << "Allocated size by new function: " << allocatedSize
            << std::endl;
#endif
    sif::info << "Tasks started." << std::endl;
}

void boardTestTaskInit() {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;

    /* Polling Sequence Table Test */
    FixedTimeslotTaskIF * PollingSequenceTableTaskTest =
            TaskFactory::instance()->createFixedTimeslotTask(
             "PST_TASK_ARDUINO", 4, 2048 * 4, 0.4, nullptr);
    result = pst::pollingSequenceInitTest(PollingSequenceTableTaskTest);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "creating PST failed" << std::endl;
    }


#ifdef ISIS_OBC_G20
    /* LED Test Task */
    PeriodicTaskIF* LedTask = TaskFactory::instance()->
    		createPeriodicTask("LED_TASK", 1, 1024 * 4, 0.4, nullptr);
    result = LedTask->addComponent(objects::LED_TASK);
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


#if DISPLAY_FACTORY_ALLOCATION_SIZE == 1
void* operator new(size_t size) {
    allocatedSize += size;
    return std::malloc(size);
}
#endif
