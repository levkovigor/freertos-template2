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
void init_task(void* args);
void initialize_iobc_peripherals();

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
#ifndef freeRTOS
    setup_timer_interrupt();
#endif

    //-------------------------------------------------------------------------
    // Initiate watchdog for iOBC
    //-------------------------------------------------------------------------
#ifdef ISIS_OBC_G20
#ifndef freeRTOS
    initiate_external_watchdog();
#else
    int retval = WDT_startWatchdogKickTask(
            WATCHDOG_KICK_INTERVAL_MS / portTICK_RATE_MS, FALSE);
    if(retval != 0) {
        TRACE_ERROR("Starting iOBC Watchdog Feed Task failed!\r\n");
    }
#endif
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

#ifndef freeRTOS
    feed_watchdog_if_necessary();
#endif

    // verify hamming code of image in sdram. code size is either written in
    // memory or extracted from FRAM.
    // if successfull, copy norflash to sdram
//    int result = copy_norflash_binary_to_sdram("Test", 256);
//    if(result != 0) {
//        // error
//    }

#ifdef freeRTOS
    // otherwise, try to copy SDCard binary to SDRAM
    // Core Task. Custom interrupts should be configured inside a task.
    xTaskCreate(init_task, (const char*)"INIT_TASK", 2048, NULL, 1, NULL);
#else
    // Configure RTT for seocnd time base.
    RTT_start();
    uint32_t last_time = RTT_GetTime();
    for(;;) {
        feed_watchdog_if_necessary();
        uint32_t curr_time = RTT_GetTime();
        if(curr_time - last_time > 5) {
            TRACE_INFO("Bootloader idle..\n\r");
            last_time = curr_time;
        }
    }
#endif

#ifdef freeRTOS
    TRACE_INFO("Starting FreeRTOS task scheduler.\n\r");
    vTaskStartScheduler();
#endif
}

void init_task(void * args) {
    initialize_iobc_peripherals();
    uint32_t last_time = RTT_GetTime();
    for(;;) {
        uint32_t curr_time = RTT_GetTime();
        if(curr_time - last_time > 5) {
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



