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
#if OBSW_ADD_TEST_CODE == 1
void boardTestTaskInit();
#endif
void genericMissedDeadlineFunc();
void printAddError(object_id_t objectId);

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
            createPeriodicTask("SERIAL_TC_POLLING", 8, 3096 * 4, 0.1,
                    genericMissedDeadlineFunc);
    result = TmTcPollingTask ->addComponent(objects::SERIAL_POLLING_TASK);

    /* Serial Bridge Task for UART Communication */
    TmTcBridge = TaskFactory::instance()->
            createPeriodicTask("SERIAL_TMTC_TASK", 7, 2048 * 4, 0.1,
                    genericMissedDeadlineFunc);
    result = TmTcBridge->addComponent(objects::SERIAL_TMTC_BRIDGE);
#endif

    /* Packet Distributor Taks */
    PeriodicTaskIF* PacketDistributorTask =
            TaskFactory::instance()-> createPeriodicTask(
                    "TMTC_DISTRIBUTOR", 6, 2048 * 4, 0.4,
                    genericMissedDeadlineFunc);
    result = PacketDistributorTask->
            addComponent(objects::CCSDS_PACKET_DISTRIBUTOR);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::CCSDS_PACKET_DISTRIBUTOR);
    }
    result = PacketDistributorTask->addComponent(
            objects::PUS_PACKET_DISTRIBUTOR);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::PUS_PACKET_DISTRIBUTOR);
    }
    result = PacketDistributorTask->addComponent(objects::TM_FUNNEL);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::TM_FUNNEL);
    }

    /* Polling Sequence Table Default */
    FixedTimeslotTaskIF * PollingSequenceTableTaskDefault =
            TaskFactory::instance()-> createFixedTimeslotTask(
                    "PST_TASK_DEFAULT", 5, 2048 * 4, 0.4,
                    genericMissedDeadlineFunc);
    result = pst::pollingSequenceInitDefault(PollingSequenceTableTaskDefault);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "InitMission: Creating PST failed!" << std::endl;
    }

    /* Event Manager */
    PeriodicTaskIF* EventManager = TaskFactory::instance()->createPeriodicTask(
            "EVENT_MANAGER", 8, 2048 * 4, 0.2,
            genericMissedDeadlineFunc);
    result = EventManager->addComponent(objects::EVENT_MANAGER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::EVENT_MANAGER);
    }

    /* Internal Error Reporter */
    PeriodicTaskIF* InternalErrorReporter = TaskFactory::instance()->
            createPeriodicTask("INT_ERR_RPRTR", 1, 1024 * 4, 2.0,
                    genericMissedDeadlineFunc);
    result = InternalErrorReporter->addComponent(
            objects::INTERNAL_ERROR_REPORTER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::INTERNAL_ERROR_REPORTER);
    }

    /* PUS Services */

    /* Verification Service */
    PeriodicTaskIF* PusService01 = TaskFactory::instance()->createPeriodicTask(
            "PUS_VERFICIATION_1", 5, 2048 * 4, 0.4,
            genericMissedDeadlineFunc);
    result = PusService01->addComponent(objects::PUS_SERVICE_1_VERIFICATION);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::PUS_SERVICE_1_VERIFICATION);
    }

    /* Event Reporter */
    PeriodicTaskIF* PusService05 = TaskFactory::instance()-> createPeriodicTask(
            "PUS_EVENT_RPRTR_5", 4, 2048 * 4, 0.2,
            genericMissedDeadlineFunc);
    result = PusService05->addComponent(objects::PUS_SERVICE_5_EVENT_REPORTING);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::PUS_SERVICE_5_EVENT_REPORTING);
    }

    /* PUS High Priority */
    PeriodicTaskIF* PusHighPriorityTask = TaskFactory::instance()->
            createPeriodicTask("PUS_HIGH_PRIO", 5, 2048 * 4, 0.2,
                    genericMissedDeadlineFunc);
    result = PusHighPriorityTask->addComponent(
            objects::PUS_SERVICE_2_DEVICE_ACCESS);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::PUS_SERVICE_2_DEVICE_ACCESS);
    }
    result = PusHighPriorityTask->addComponent(objects::PUS_SERVICE_6_MEM_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::PUS_SERVICE_2_DEVICE_ACCESS);
    }
    result = PusHighPriorityTask->addComponent(
            objects::PUS_SERVICE_9_TIME_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::PUS_SERVICE_2_DEVICE_ACCESS);
    }

    /* PUS Medium Priority */
    PeriodicTaskIF* PusMediumPriorityTask = TaskFactory::instance()->
            createPeriodicTask("PUS_MED_PRIO", 4, 2048 * 4, 0.4,
                    genericMissedDeadlineFunc);

    result = PusMediumPriorityTask->addComponent(
            objects::PUS_SERVICE_3_HOUSEKEEPING);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::PUS_SERVICE_3_HOUSEKEEPING);
    }
    result = PusMediumPriorityTask->addComponent(
            objects::PUS_SERVICE_8_FUNCTION_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::PUS_SERVICE_8_FUNCTION_MGMT);
    }
    result = PusMediumPriorityTask->addComponent(
            objects::PUS_SERVICE_200_MODE_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::PUS_SERVICE_200_MODE_MGMT);
    }
    result = PusMediumPriorityTask->addComponent(
            objects::PUS_SERVICE_201_HEALTH);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::PUS_SERVICE_201_HEALTH);
    }


    /* PUS Low Priority */
    PeriodicTaskIF* PusLowPriorityTask = TaskFactory::instance()->
            createPeriodicTask("PUS_LOW_PRIO", 3, 2048 * 4, 1.6,
                    genericMissedDeadlineFunc);
    PusLowPriorityTask->addComponent(objects::PUS_SERVICE_20_PARAM_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::PUS_SERVICE_20_PARAM_MGMT);
    }
    PusLowPriorityTask->addComponent(objects::PUS_SERVICE_17_TEST);
    if (result != HasReturnvaluesIF::RETURN_OK) {
    	printAddError(objects::PUS_SERVICE_17_TEST);
    }

    /* PUS File Management */
    PeriodicTaskIF* PusFileManagement = TaskFactory::instance()->
    		createPeriodicTask("PUS_FILE_MGMT", 4, 2048 * 4, 0.4,
    				genericMissedDeadlineFunc);
    result = PusFileManagement->addComponent(objects::PUS_SERVICE_23_FILE_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
    	printAddError(objects::PUS_SERVICE_23_FILE_MGMT);
    }
    /* SD Card handler task */
    PeriodicTaskIF* SDCardTask = TaskFactory::instance()->
            createPeriodicTask("SD_CARD_TASK", 3, 2048 * 4, 0.4,
                    genericMissedDeadlineFunc);
    result = SDCardTask->addComponent(objects::SD_CARD_HANDLER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::SD_CARD_HANDLER);
    }

    /* Software image task */
    PeriodicTaskIF* SoftwareImageTask = TaskFactory::instance()->
            createPeriodicTask("SW_IMG_TASK", 2, 2048 * 4, 2,
                    genericMissedDeadlineFunc);
    result = SoftwareImageTask->addComponent(objects::SOFTWARE_IMAGE_HANDLER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::SOFTWARE_IMAGE_HANDLER);
    }

    /* Core Controller task */
    PeriodicTaskIF* CoreController = TaskFactory::instance()->
            createPeriodicTask("CORE_CONTROLLER", 6, 2048 * 4, 1,
                    genericMissedDeadlineFunc);
    result = CoreController->addComponent(objects::CORE_CONTROLLER);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::CORE_CONTROLLER);
    }
    PeriodicTaskIF* SystemStateTask = TaskFactory::instance()->
            createPeriodicTask("SYSTEM_STATE_TSK", 2, 2048 * 4, 100,
                    genericMissedDeadlineFunc);
    result = SystemStateTask->addComponent(objects::SYSTEM_STATE_TASK);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::SYSTEM_STATE_TASK);
    }

    PeriodicTaskIF* ThermalController = TaskFactory::instance()->
            createPeriodicTask("THERMAL_CTRL", 6, 2048 * 4, 1,
                    genericMissedDeadlineFunc);
    result = ThermalController->addComponent(objects::THERMAL_CONTROLLER);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::THERMAL_CONTROLLER);
    }

    /* SPI Communication Interface*/
    PeriodicTaskIF* SpiComTask = TaskFactory::instance()->
            createPeriodicTask("SPI_COM_IF", 8, 1024 * 4, 0.4,
                    genericMissedDeadlineFunc);
    result = SpiComTask->addComponent(objects::SPI_DEVICE_COM_IF);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::SPI_DEVICE_COM_IF);
    }


#if OBSW_ADD_TEST_CODE == 1
    InternalUnitTester unitTestClass;
    result = unitTestClass.performTests();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "InitMission: Internal unit tests did not run "
                << "successfully!" << std::endl;
    }

    /* Comment out for mission build */
    boardTestTaskInit();
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

    PusFileManagement->startTask();
    PusHighPriorityTask->startTask();
    PusMediumPriorityTask->startTask();
    PusLowPriorityTask->startTask();

    SDCardTask -> startTask();
    SoftwareImageTask -> startTask();

    CoreController->startTask();
    SystemStateTask -> startTask();
    ThermalController -> startTask();
    SpiComTask->startTask();

    sif::info << "Remaining FreeRTOS heap size: " << std::dec
            << xPortGetFreeHeapSize() << " bytes." << std::endl;
    size_t remainingFactoryStack = TaskManagement::getTaskStackHighWatermark();
    if(remainingFactoryStack < 3000) {
        sif::warning << "Factory Task: Remaining stack size: "
                << remainingFactoryStack << " bytes" << std::endl;
    }
#if OBSW_DISPLAY_FACTORY_ALLOCATION_SIZE == 1
    sif::info << "Allocated size by new function: " << allocatedSize
            << std::endl;
#endif
    sif::info << "Tasks started." << std::endl;
}

#if OBSW_ADD_TEST_CODE == 1
void boardTestTaskInit() {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;

    /* Polling Sequence Table Test */
    FixedTimeslotTaskIF * PollingSequenceTableTaskTest =
            TaskFactory::instance()->createFixedTimeslotTask(
                    "PST_TASK_ARDUINO", 4, 2048 * 4, 0.4, genericMissedDeadlineFunc);
    result = pst::pollingSequenceInitTest(PollingSequenceTableTaskTest);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "creating PST failed" << std::endl;
    }


    /* Test Task */
    PeriodicTaskIF* TestTask = TaskFactory::instance()->
            createPeriodicTask("TEST_TASK", 1, 3072 * 4, 3,
                    genericMissedDeadlineFunc);
    result = TestTask->addComponent(objects::TEST_TASK);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        printAddError(objects::TEST_TASK);
    }


    /* LED Task */
    PeriodicTaskIF* LedTask = TaskFactory::instance()->
            createPeriodicTask("LED_TASK", 1, 1024 * 4, 0.4, nullptr);
    result = LedTask->addComponent(objects::LED_TASK);

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
    TestTask -> startTask();
    //SPITask -> startTask();
    //I2CTask -> startTask();
    //UART2Task -> startTask();
#ifdef ETHERNET
    //UART0Task -> startTask()
#endif
    LedTask->startTask();
}
#endif


#if OBSW_DISPLAY_FACTORY_ALLOCATION_SIZE == 1
void* operator new(size_t size) {
    allocatedSize += size;
    return std::malloc(size);
}
#endif

void printAddError(object_id_t objectId) {
    sif::error << "InitMission::printAddError: Adding 0x" << std::hex
            << std::setfill('0') << std::setw(8) << objectId
            << " failed!" << std::dec << std::endl;
}


void genericMissedDeadlineFunc() {
#if OBSW_PRINT_MISSED_DEADLINES == 1
    sif::debug << "PeriodicTask: " << pcTaskGetName(NULL) <<
            " missed deadline!" << std::endl;
#endif
}

