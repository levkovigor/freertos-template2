#include <board.h>
#include <AT91SAM9G20.h>
#include <board_memories.h>

#include "main.h"

// The AT91SAM9G20-EK does not have a pre-installed NOR-Flash. Therefore,
// we only include the NorFlash boot header for iOBC projects.
#ifdef ISIS_OBC_G20
#include <core/bootNorFlash.h>
#else
#include <core/bootNandFlash.h>
#endif

#include <core/timer.h>

#ifndef freeRTOS
#include <core/watchdog.h>
#else
#include <hal/Timing/WatchDogTimer.h>
#endif
#include <hal/Timing/RTT.h>

#include <cp15/cp15.h>
#include <utility/trace.h>
#include <peripherals/dbgu/dbgu.h>
#include <peripherals/pio/pio.h>
#include <peripherals/aic/aic.h>
#include <peripherals/pio/pio.h>

#ifdef freeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <FreeRTOSConfig.h>
#endif

#include <stdbool.h>
#include <string.h>

#ifndef SW_VERSION
#define SW_VERSION 0
#endif

#ifndef SW_SUBVERSION
#define SW_SUBVERSION 0
#endif

#define WATCHDOG_KICK_INTERVAL_MS 10

void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType);
void initialize_iobc_peripherals();
void idle_loop();
void perform_bootloader_core_operation();

#ifdef freeRTOS
void init_task(void* args);
void handler_task(void * args);
static TaskHandle_t handler_task_handle_glob = NULL;
#endif



/**
 * @brief	Bootloader which will copy the primary software to SDRAM and
 * 			execute it
 * @author 	R. Mueller
 */
int main()
{
    //-------------------------------------------------------------------------
    // Configure traces
    //-------------------------------------------------------------------------
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

    //-------------------------------------------------------------------------
    // Initiate periodic MS interrupt
    //-------------------------------------------------------------------------
#ifndef ISIS_OBC_G20
    setup_timer_interrupt();
#endif

    //-------------------------------------------------------------------------
    // Initiate watchdog for iOBC
    //-------------------------------------------------------------------------
#ifdef ISIS_OBC_G20
    int retval = WDT_startWatchdogKickTask(
            WATCHDOG_KICK_INTERVAL_MS / portTICK_RATE_MS, FALSE);
    if(retval != 0) {
        TRACE_ERROR("Starting iOBC Watchdog Feed Task failed!\r\n");
    }
#else
    initiate_external_watchdog();
#endif

    TRACE_INFO_WP("\n\r-- SOURCE Bootloader --\n\r");
    TRACE_INFO_WP("-- %s --\n\r", BOARD_NAME);
    TRACE_INFO_WP("-- Software version v%d.%d --\n\r", SW_VERSION, SW_SUBVERSION);
    TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

    //-------------------------------------------------------------------------
    // Enable I-Cache
    //-------------------------------------------------------------------------
    CP15_Enable_I_Cache();

    //-------------------------------------------------------------------------
    // Configure SDRAM
    //-------------------------------------------------------------------------
    TRACE_INFO("Initiating SDRAM\n\r");
    BOARD_ConfigureSdram(BOARD_SDRAM_BUSWIDTH);

#ifndef ISIS_OBC_G20
    feed_watchdog_if_necessary();
#endif

#ifdef freeRTOS
    // otherwise, try to copy SDCard binary to SDRAM
    // Core Task. Custom interrupts should be configured inside a task.
    xTaskCreate(handler_task, (const char*)"HANDLER_TASK", 1024, NULL, 2,
            &handler_task_handle_glob);
    xTaskCreate(init_task, (const char*)"INIT_TASK", 1024,
            handler_task_handle_glob , 1, NULL);

    TRACE_INFO("Starting FreeRTOS task scheduler.\n\r");
    vTaskStartScheduler();
    TRACE_ERROR("FreeRTOS scheduler error!\n\r");
    for(;;) {};
#else
    // Configure RTT for second time base.
    RTT_start();

    // Do some copy stuff here.
    perform_bootloader_core_operation();

    // to see its alive, will not be reached later.
    idle_loop();
#endif

}


void perform_bootloader_core_operation() {
    // do all the fancy stuff here.
    // verify hamming code of image in sdram. code size is either written in
    // memory or extracted from FRAM.
    // if successfull, copy norflash to sdram
//    int result = copy_norflash_binary_to_sdram("Test", 256);
//    if(result != 0) {
//        // error
//    }
}

void idle_loop() {
    uint32_t last_time = RTT_GetTime();
    for(;;) {
        uint32_t curr_time = RTT_GetTime();
        if(curr_time - last_time > 60) {
            TRACE_INFO("Bootloader idle..\n\r");
            last_time = curr_time;
        }
    }
}

void initialize_iobc_peripherals() {
    RTT_start();
#ifdef ISIS_OBC_G20

#endif
}

void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType)
{
    typedef void(*fctType)(volatile unsigned int, volatile unsigned int);
    void(*pFct)(volatile unsigned int r0_val, volatile unsigned int r1_val);

    pFct = (fctType)jumpAddr;
    pFct(0/*dummy value in r0*/, matchType/*matchType in r1*/);

    while(1);//never reach
}

#ifdef freeRTOS
void init_task(void * args) {
    TRACE_INFO("Running initialization task..\n\r");
    initialize_iobc_peripherals();
    // perform initialization which needs to be inside a task.

    // start handler task
    TaskHandle_t handler_task_handle = (TaskHandle_t) args;
    if(handler_task_handle != NULL) {
        while(eTaskGetState(handler_task_handle) != eSuspended) {
            vTaskDelay(1);
        }
        vTaskResume(handler_task_handle);
    }

    // Initialization task not needed anymore, deletes itself.
    vTaskDelete(NULL);
}

void handler_task(void * args) {
    TRACE_INFO("Running Handler task..\n\r");
    // Wait for initialization to finish
    vTaskSuspend(NULL);

    perform_bootloader_core_operation();

    // will not be reached when bootloader is finished.
    idle_loop();
}
#endif


