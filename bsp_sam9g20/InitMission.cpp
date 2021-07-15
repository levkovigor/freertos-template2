#include "ObjectFactory.h"
#include <OBSWVersion.h>
#include <FSFWConfig.h>
#include <OBSWConfig.h>
#include <pollingsequence/PollingSequenceFactory.h>
#include <objects/systemObjectList.h>

#include <mission/utility/InitMission.h>
#include <bsp_sam9g20/utility/compile_time.h>

#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/timemanager/Clock.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/tests/internal/InternalUnitTester.h>
#include <fsfw/osal/freertos/TaskManagement.h>

#include <freertos/FreeRTOS.h>

extern "C" {
#include <board.h>
#include <AT91SAM9G20.h>
}

#include <cstring>

#if OBSW_TRACK_FACTORY_ALLOCATION_SIZE == 1 || OBSW_MONITOR_ALLOCATION == 1
#include <new>
#if OBSW_TRACK_FACTORY_ALLOCATION_SIZE == 1
static size_t allocatedSize = 0;
#endif
#if OBSW_MONITOR_ALLOCATION == 1
bool config::softwareInitializationComplete = false;
#endif
#endif


#if defined(ISIS_OBC_G20) && defined(ADD_CR)
bool addCrToOstreams = true;
#else
bool addCrToOstreams = false;
#endif /* ISIS_OBC_G20 && ADD_CR */

#if FSFW_CPP_OSTREAM_ENABLED == 1
namespace sif {
/* Set up output streams
 * Don't call these streams in HAL callback functions ! */
ServiceInterfaceStream debug("DEBUG", addCrToOstreams);
ServiceInterfaceStream info("INFO", addCrToOstreams);
ServiceInterfaceStream warning("WARNING", addCrToOstreams);
ServiceInterfaceStream error("ERROR", addCrToOstreams);
}
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */

/* will be created in main */
ObjectManagerIF* objectManager = nullptr;

/* Board Tests, not used in mission */
#if OBSW_ADD_TEST_CODE == 1
void boardTestTaskInit();
#endif

void genericMissedDeadlineFunc();
void initTasks(void);
void runMinimalTask(void);


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
    printf("\r\n-- FreeRTOS task scheduler started --\n\r");
    printf("-- SOURCE On-Board Software --\n\r");
    printf("-- %s --\n\r", BOARD_NAME_PRINT);
    printf("-- Software version %s v%d.%d.%d --\n\r", SW_NAME, SW_VERSION, SW_SUBVERSION,
            SW_SUBSUBVERSION);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

#if FSFW_CPP_OSTREAM_ENABLED == 0
    sif::setToAddCrAtEnd(true);
#endif

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Initiating mission specific code." << std::endl;
#else
    sif::printInfo("Initiating mission specific code.\n");
#endif

#if OBSW_PERFORM_SIMPLE_TASK == 1

    runMinimalTask();

#else

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Creating objects." << std::endl;
#else
    sif::printInfo("Creating objects.\n");
#endif

    ObjectManager* objManager = ObjectManager::instance();
    objManager->setObjectFactoryFunction(Factory::produce, nullptr);
    objManager->initialize();

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Creating tasks.." << std::endl;
#else
    sif::printInfo("Creating tasks..\n");
#endif
    initTasks();

#endif /* OBSW_PERFORM_SIMPLE_TASK == 1 */
}

void initTasks(void) {
    TaskFactory* taskFactory = TaskFactory::instance();
    if(taskFactory == nullptr) {
        return;
    }
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    /* TMTC Communication Tasks */
    PeriodicTaskIF * tmTcPollingTask = nullptr;
    PeriodicTaskIF* tmTcBridge = nullptr;
#if OBSW_ENABLE_ETHERNET == 1 && OBSW_USE_ETHERNET_TMTC_BRIDGE == 1
    /* EMAC polling task */
    tmTcPollingTask = TaskFactory::instance()->createPeriodicTask(
            "EMAC_PollingTask", 8, 1024 * 4 , 0.1, nullptr);
    result = tmTcPollingTask->addComponent(objects::EMAC_POLLING_TASK);

    /* UDP Task for Ethernet Communication */
    tmTcBridge = TaskFactory::instance()->
            createPeriodicTask("UDP_TMTC_TASK", 6, 2048 * 4, 0.2, nullptr);
    result = tmTcBridge->addComponent(objects::UDP_TMTC_BRIDGE);
#else
    /* Serial Polling Task */
    tmTcPollingTask = taskFactory->createPeriodicTask(
            "SERIAL_TC_POLLING", 8, 3096 * 4, 0.1, genericMissedDeadlineFunc);
    result = tmTcPollingTask ->addComponent(objects::SERIAL_POLLING_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("Serial Polling", objects::SERIAL_POLLING_TASK);
    }
    /* Serial Bridge Task for UART Communication */
    tmTcBridge = TaskFactory::instance()->createPeriodicTask(
            "SERIAL_TMTC_TASK", 7, 2048 * 4, 0.1, genericMissedDeadlineFunc);
    result = tmTcBridge->addComponent(objects::SERIAL_TMTC_BRIDGE);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("Serial TMTC bridge", objects::SERIAL_TMTC_BRIDGE);
    }
#endif /* OBSW_ENABLE_ETHERNET == 1 && OBSW_USE_ETHERNET_TMTC_BRIDGE == 1 */

    /* Packet Distributor Taks */
    PeriodicTaskIF* packetDistributorTask = taskFactory->createPeriodicTask(
            "TMTC_DISTRIBUTOR", 6, 2048 * 4, 0.4, genericMissedDeadlineFunc);

    result = packetDistributorTask->addComponent(objects::CCSDS_PACKET_DISTRIBUTOR);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("CCSDS distributor", objects::CCSDS_PACKET_DISTRIBUTOR);
    }
    result = packetDistributorTask->addComponent(objects::PUS_PACKET_DISTRIBUTOR);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS packet distributor", objects::PUS_PACKET_DISTRIBUTOR);
    }
    result = packetDistributorTask->addComponent(objects::TM_FUNNEL);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("TM funnel", objects::TM_FUNNEL);
    }

    /* Polling Sequence Table Default */
    FixedTimeslotTaskIF * pollingSequenceTableTaskDefault = taskFactory-> createFixedTimeslotTask(
                    "PST_TASK_DEFAULT", 8, 2048 * 4, 0.4, genericMissedDeadlineFunc);
    result = pst::pollingSequenceInitDefault(pollingSequenceTableTaskDefault);
    if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "InitMission: Creating PST failed!" << std::endl;
#else
        sif::printError("InitMission: Creating PST failed!\n");
#endif
    }

    /* Event Manager */
    PeriodicTaskIF* eventManager = taskFactory->createPeriodicTask(
            "EVENT_MANAGER", 6, 2048 * 4, 0.2, genericMissedDeadlineFunc);
    result = eventManager->addComponent(objects::EVENT_MANAGER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("Event Manager", objects::EVENT_MANAGER);
    }

    /* PUS Services */

    // Verification Service
    PeriodicTaskIF* pusService1 = taskFactory->createPeriodicTask(
            "PUS_VERFICIATION_1", 5, 2048 * 4, 0.4, genericMissedDeadlineFunc);
    result = pusService1->addComponent(objects::PUS_SERVICE_1_VERIFICATION);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 1", objects::PUS_SERVICE_1_VERIFICATION);
    }

    // Event Reporter
    PeriodicTaskIF* pusService05 = taskFactory-> createPeriodicTask(
            "PUS_EVENT_RPRTR_5", 5, 2048 * 4, 0.2, genericMissedDeadlineFunc);
    result = pusService05->addComponent(objects::PUS_SERVICE_5_EVENT_REPORTING);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 5", objects::PUS_SERVICE_5_EVENT_REPORTING);
    }

    // PUS High Priority
    PeriodicTaskIF* PusHighPriorityTask = taskFactory->createPeriodicTask(
            "PUS_HIGH_PRIO", 5, 2048 * 4, 0.2, genericMissedDeadlineFunc);
    result = PusHighPriorityTask->addComponent(objects::PUS_SERVICE_2_DEVICE_ACCESS);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 2", objects::PUS_SERVICE_2_DEVICE_ACCESS);
    }
    result = PusHighPriorityTask->addComponent(objects::PUS_SERVICE_6_MEM_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 6", objects::PUS_SERVICE_6_MEM_MGMT);
    }
    result = PusHighPriorityTask->addComponent(objects::PUS_SERVICE_9_TIME_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 9", objects::PUS_SERVICE_2_DEVICE_ACCESS);
    }

    // PUS Medium Priority
    PeriodicTaskIF* pusMediumPriorityTask = taskFactory->
            createPeriodicTask("PUS_MED_PRIO", 3, 2048 * 4, 0.4, genericMissedDeadlineFunc);

    result = pusMediumPriorityTask->addComponent(objects::PUS_SERVICE_3_HOUSEKEEPING);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 3", objects::PUS_SERVICE_3_HOUSEKEEPING);
    }
    result = pusMediumPriorityTask->addComponent(objects::PUS_SERVICE_8_FUNCTION_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 8", objects::PUS_SERVICE_8_FUNCTION_MGMT);
    }
    result = pusMediumPriorityTask->addComponent(objects::PUS_SERVICE_200_MODE_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 200", objects::PUS_SERVICE_200_MODE_MGMT);
    }
    result = pusMediumPriorityTask->addComponent(objects::PUS_SERVICE_201_HEALTH);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 201", objects::PUS_SERVICE_201_HEALTH);
    }


    // PUS Low Priority
    PeriodicTaskIF* lowPriorityTask = taskFactory->
            createPeriodicTask("PUS_LOW_PRIO", 2, 2048 * 4, 1.6, genericMissedDeadlineFunc);
    result = lowPriorityTask->addComponent(objects::PUS_SERVICE_20_PARAMETERS);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 20", objects::PUS_SERVICE_20_PARAMETERS);
    }
    result = lowPriorityTask->addComponent(objects::PUS_SERVICE_17_TEST);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 17", objects::PUS_SERVICE_17_TEST);
    }
    result = lowPriorityTask->addComponent(objects::INTERNAL_ERROR_REPORTER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("internal error reporter",
                objects::INTERNAL_ERROR_REPORTER);
    }

    /* PUS File Management */
    PeriodicTaskIF* pusFileManagement = taskFactory->
            createPeriodicTask("PUS_FILE_MGMT", 4, 2048 * 4, 0.4, genericMissedDeadlineFunc);
    result = pusFileManagement->addComponent(objects::PUS_SERVICE_23_FILE_MGMT);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PUS 23", objects::PUS_SERVICE_23_FILE_MGMT);
    }
    /* SD Card handler task */
#ifdef AT91SAM9G20_EK
    float sdCardTaskPeriod = 0.6;
#else
    /* iOBC SD-Cards SLC are usually slower than modern SD-Cards,
    therefore a spearate task period can be set here */
    float sdCardTaskPeriod = 0.8;
#endif
    PeriodicTaskIF* sdCardTask = taskFactory->createPeriodicTask(
            "SD_CARD_TASK", 3, 2048 * 4, sdCardTaskPeriod, genericMissedDeadlineFunc);
    result = sdCardTask->addComponent(objects::SD_CARD_HANDLER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("SD Card Handler", objects::SD_CARD_HANDLER);
    }

    /* Software image task */
    PeriodicTaskIF* softwareImageTask = taskFactory->createPeriodicTask(
            "SW_IMG_TASK", 2, 2048 * 4, 2, genericMissedDeadlineFunc);
    result = softwareImageTask->addComponent(objects::SOFTWARE_IMAGE_HANDLER);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("SW Image Handler", objects::SOFTWARE_IMAGE_HANDLER);
    }

#ifdef AT91SAM9G20_EK
    uint8_t coreCtrlPrio = 5;
#else
    uint8_t coreCtrlPrio = 6;
#endif
    /* Core Controller task */
    PeriodicTaskIF* coreController = taskFactory->createPeriodicTask(
            "CORE_CONTROLLER", coreCtrlPrio, 2048 * 4, 1, genericMissedDeadlineFunc);
    result = coreController->addComponent(objects::CORE_CONTROLLER);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("Core controller", objects::CORE_CONTROLLER);
    }
    PeriodicTaskIF* systemStateTask = taskFactory->createPeriodicTask(
            "SYSTEM_STATE_TSK", 2, 2048 * 4, 100, genericMissedDeadlineFunc);
    result = systemStateTask->addComponent(objects::SYSTEM_STATE_TASK);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("System State Task", objects::SYSTEM_STATE_TASK);
    }

    PeriodicTaskIF* thermalController = taskFactory->createPeriodicTask(
            "THERMAL_CTRL", 6, 2048 * 4, 1, genericMissedDeadlineFunc);
    result = thermalController->addComponent(objects::THERMAL_CONTROLLER);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("Thermal Controller", objects::THERMAL_CONTROLLER);
    }

    // Commented because the ACS controller is not finished yet
//    PeriodicTaskIF *attitudeController = taskFactory->createPeriodicTask("ATTITUDE_CTRL", 6,
//                    2048 * 4, 0.4, genericMissedDeadlineFunc);
//    result = attitudeController->addComponent(objects::ATTITUDE_CONTROLLER);
//    if (result != HasReturnvaluesIF::RETURN_OK) {
//        initmission::printAddObjectError("Attitude Controller", objects::ATTITUDE_CONTROLLER);
//    }

#if OBSW_ADD_SPI_TEST_TASK == 0
    /* SPI Communication Interface*/
    PeriodicTaskIF* spiComTask = taskFactory->createPeriodicTask(
            "SPI_COM_IF", 8, 1024 * 4, 0.4, genericMissedDeadlineFunc);
    result = spiComTask->addComponent(objects::SPI_DEVICE_COM_IF);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("SPI ComIF", objects::SPI_DEVICE_COM_IF);
    }
#endif

#if OBSW_PERFORM_INTERNAL_UNIT_TESTS == 1
    InternalUnitTester unitTestClass;
    InternalUnitTester::TestConfig testConfig;
    testConfig.testArrayPrinter = false;
    result = unitTestClass.performTests(testConfig);
#endif

#if OBSW_ADD_TEST_CODE == 1
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "InitMission: Internal unit tests did not run "
                << "successfully!" << std::endl;
#else
        sif::printError("InitMission: Internal unit tests did not run "
                "successfully!\n");
#endif
    }

    /* Comment out for mission build */
    boardTestTaskInit();
#endif /* OBSW_ADD_TEST_CODE == 1 */

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Starting mission tasks.." << std::endl;
#else
    sif::printInfo("Starting mission tasks..\n");
#endif

    tmTcPollingTask -> startTask();
    tmTcBridge -> startTask();
    packetDistributorTask -> startTask();
    pollingSequenceTableTaskDefault -> startTask();
    eventManager -> startTask();

    pusService1 -> startTask();
    pusService05 -> startTask();

    pusFileManagement -> startTask();
    PusHighPriorityTask -> startTask();
    pusMediumPriorityTask -> startTask();
    lowPriorityTask -> startTask();

    sdCardTask -> startTask();
    softwareImageTask -> startTask();

    coreController->startTask();
    systemStateTask -> startTask();
    // ThermalController -> startTask();
    // AttitudeController->startTask();
#if OBSW_ADD_SPI_TEST_TASK == 0
    spiComTask->startTask();
#endif

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Remaining FreeRTOS heap size: " << std::dec
            << xPortGetFreeHeapSize() << " bytes." << std::endl;
#else
    sif::printInfo("Remaining FreeRTOS heap size: %lu bytes.\n",
            static_cast<unsigned long>(xPortGetFreeHeapSize()));
#endif
    size_t remainingFactoryStack = TaskManagement::getTaskStackHighWatermark();
    if(remainingFactoryStack < 3000) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "Factory Task: Remaining stack size: "
                << remainingFactoryStack << " bytes" << std::endl;
#else
        sif::printWarning("Factory Task: Remaining stack size: %lu bytes\n",
                static_cast<unsigned long>(remainingFactoryStack));
#endif
    }
#if OBSW_TRACK_FACTORY_ALLOCATION_SIZE == 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Allocated size by new function: " << allocatedSize
            << " bytes." << std::endl;
#else
    sif::printInfo( "Allocated size by new function: %lu bytes.\n",
            static_cast<unsigned long>(allocatedSize));
#endif
#endif

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Tasks started." << std::endl;
#else
    sif::printInfo("Tasks started.\n");
#endif
}

#if OBSW_ADD_TEST_CODE == 1
void boardTestTaskInit() {
    TaskFactory* taskFactory = TaskFactory::instance();
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;

    /* Polling Sequence Table Test */
#if OBSW_ADD_TEST_PST == 1
    FixedTimeslotTaskIF * pollingSequenceTableTaskTest =
            taskFactory->createFixedTimeslotTask(
                    "PST_TEST_TASK", 4, 2048 * 4, 0.4, genericMissedDeadlineFunc);
    result = pst::pollingSequenceInitTest(pollingSequenceTableTaskTest);
    if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "boardTestTaskInit: Creating test PST failed" << std::endl;
#else
        sif::printError("boardTestTaskInit: Creating test PST failed\n");
#endif
    }
#endif /* OBSW_ADD_TEST_PST == 1 */


    /* Test Task */
    PeriodicTaskIF* testTask = taskFactory->createPeriodicTask(
            "TEST_TASK", 1, 3072 * 4, 3, genericMissedDeadlineFunc);
    result = testTask->addComponent(objects::TEST_TASK);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("test task", objects::TEST_TASK);
    }
    // Don't run the UART test tasks together with the respective communication interfaces!
    // UART0 Test Task
#if OBSW_ADD_UART_0_TEST_TASK == 1
    result = testTask->addComponent(objects::AT91_UART0_TEST_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component UART0 Task failed" << std::endl;
    }
#endif
    // UART2 Test Task
#if OBSW_ADD_UART_2_TEST_TASK == 1
    result = testTask->addComponent(objects::AT91_UART2_TEST_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("UART2 Test Task", objects::AT91_UART2_TEST_TASK);
    }
#endif
    // I2C Test Task
#if OBSW_ADD_I2C_TEST_TASK == 1
    result = testTask->addComponent(objects::AT91_I2C_TEST_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Add component I2C Task failed" << std::endl;
    }
#endif
    // SPI Test Task
#if OBSW_ADD_SPI_TEST_TASK == 1
    result = testTask->addComponent(objects::AT91_SPI_TEST_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("SPI test task", objects::AT91_SPI_TEST_TASK);
    }
#endif
#if OBSW_ADD_PVCH_TEST == 1
    result = testTask->addComponent(objects::PVCH_TEST_TASK);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("PVCH Test Task", objects::PVCH_TEST_TASK);
    }
#endif

    /* LED Task */
#if OBSW_ADD_LED_TASK == 1
    PeriodicTaskIF* ledTask = taskFactory->createPeriodicTask(
            "LED_TASK", 1, 1024 * 4, 0.4, nullptr);
    result = ledTask->addComponent(objects::LED_TASK);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        initmission::printAddObjectError("LED task", objects::LED_TASK);
    }
#endif

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Starting test tasks.." << std::endl;
#else
    sif::printInfo("Starting test tasks..\n");
#endif

    testTask -> startTask();

#if OBSW_ADD_LED_TASK == 1
    ledTask->startTask();
#endif

#if OBSW_ADD_TEST_PST == 1
    pollingSequenceTableTaskTest -> startTask ();
#endif

}
#endif


#if OBSW_TRACK_FACTORY_ALLOCATION_SIZE == 1 || OBSW_MONITOR_ALLOCATION == 1
void* operator new(size_t size) {
#if OBSW_TRACK_FACTORY_ALLOCATION_SIZE == 1
    allocatedSize += size;
#endif
#if OBSW_MONITOR_ALLOCATION == 1
    if(config::softwareInitializationComplete) {
        /* To prevent infinite recursion in some cases. */
        config::softwareInitializationComplete = false;
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "Software Initialization complete but memory "
                << "is allocated!" << std::endl;
#else
        sif::printWarning("Software Initialization complete but memory "
                "is allocated!\n");
#endif
    }
#endif
    return std::malloc(size);
}
#endif


void genericMissedDeadlineFunc() {
#if OBSW_PRINT_MISSED_DEADLINES == 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "PeriodicTask: " << pcTaskGetName(NULL) << " missed deadline!" << std::endl;
#else
    sif::printDebug("PeriodicTask: %s missed deadline.\n", pcTaskGetName(NULL));
#endif
#endif
}

void runMinimalTask(void) {
    while(1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Alive" << std::endl;
#else
        sif::printInfo("Alive\n");
#endif
        vTaskDelay(1000);
    }
}

