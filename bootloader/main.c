#include "main.h"
#include "config/bootloaderConfig.h"

#include <board.h>
#include <AT91SAM9G20.h>
#include <board_memories.h>
#include <peripherals/dbgu/dbgu.h>
#include <peripherals/pio/pio.h>
#include <peripherals/aic/aic.h>
#include <peripherals/pio/pio.h>
#include <cp15/cp15.h>

#include <core/timer.h>
#include <hal/Timing/RTT.h>
#include <hal/Drivers/LED.h>


// The AT91SAM9G20-EK does not have a pre-installed NOR-Flash. Therefore,
// we only include the NorFlash boot header for iOBC projects.
#ifdef ISIS_OBC_G20
#include <bootloader/core/iobc/bootIOBC.h>
#else
#include <bootloader/core/at91/bootAt91.h>
#endif


#ifdef ISIS_OBC_G20
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <FreeRTOSConfig.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Storage/FRAM.h>
#include <hal/Storage/NORFlash.h>
#else
#include <core/watchdog.h>
#endif

#if DEBUG_IO_LIB == 1
#include <utility/trace.h>
#endif

#include <stdbool.h>
#include <string.h>

#define WATCHDOG_KICK_INTERVAL_MS 15


void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType);
void initialize_iobc_peripherals();
void idle_loop();
void perform_bootloader_core_operation();
int perform_iobc_copy_operation_to_sdram();

#ifdef ISIS_OBC_G20
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
#if DEBUG_IO_LIB == 1
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
#endif

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
            WATCHDOG_KICK_INTERVAL_MS / portTICK_RATE_MS, 0);
    if(retval != 0) {
#if DEBUG_IO_LIB == 1
        TRACE_ERROR("Starting iOBC Watchdog Feed Task failed!\r\n");
#endif
    }
#else
    initiate_external_watchdog();
#endif

    //-------------------------------------------------------------------------
    // Enable I-Cache
    //-------------------------------------------------------------------------
    CP15_Enable_I_Cache();

    LED_start();


    //-------------------------------------------------------------------------
    // Configure SDRAM
    //-------------------------------------------------------------------------
    BOARD_ConfigureSdram(BOARD_SDRAM_BUSWIDTH);

#ifndef ISIS_OBC_G20
    feed_watchdog_if_necessary();
#endif

    //-------------------------------------------------------------------------
    // iOBC Bootloader
    //-------------------------------------------------------------------------
#ifdef ISIS_OBC_G20
    // otherwise, try to copy SDCard binary to SDRAM
    // Core Task. Custom interrupts should be configured inside a task.
    xTaskCreate(handler_task, "HANDLER_TASK", 512, NULL, 2,
            &handler_task_handle_glob);
    xTaskCreate(init_task, "INIT_TASK", 512, handler_task_handle_glob,
            3, NULL);
    vTaskStartScheduler();
#if DEBUG_IO_LIB == 1
    TRACE_ERROR("FreeRTOS scheduler error!\n\r");
#endif
    for(;;) {};
#else
    //-------------------------------------------------------------------------
    // AT91SAM9G20-EK Bootloader
    //-------------------------------------------------------------------------
    // Configure RTT for second time base.
    RTT_start();

    //-------------------------------------------------------------------------
    // AT91 Bootloader
    //-------------------------------------------------------------------------
    perform_bootloader_core_operation();

    // to see its alive, will not be reached later.
    idle_loop();
#endif

}


void perform_bootloader_core_operation() {
#ifdef ISIS_OBC_G20
	int result = perform_iobc_copy_operation_to_sdram();
#elif defined(AT91SAM9G20_EK)
    TRACE_INFO_WP("-- SOURCE Bootloader --\n\r");
    TRACE_INFO_WP("-- %s --\n\r", BOARD_NAME);
    TRACE_INFO_WP("-- Software version v%d.%d --\n\r", SW_VERSION, SW_SUBVERSION);
    TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
    TRACE_INFO("Running initialization task..\n\r");
    int result = copy_nandflash_binary_to_sdram(false);
#endif
    if(result != 0) {
        // error
    }

    go_to_jump_address(SDRAM_DESTINATION, 0);
}

#ifdef ISIS_OBC_G20
int perform_iobc_copy_operation_to_sdram() {
	// determine which binary should be copied to SDRAM first.
	BootSelect boot_select = BOOT_NOR_FLASH;
	int result = 0;
	if(boot_select == BOOT_NOR_FLASH) {
		size_t sizeToCopy = 0;
		memcpy(&sizeToCopy, (const void *) (NORFLASH_SA5_ADDRESS + 0x14), 4);
		result = copy_norflash_binary_to_sdram(sizeToCopy);
	}
	else {
		result = copy_sdcard_binary_to_sdram(boot_select);
		if(result != 0) {
			// fatal failure. boot from NOR-Flash
			//result = copy_norflash_binary_to_sdram(256);
		}
	}
	return result;
}
#endif

void idle_loop() {
    uint32_t last_time = RTT_GetTime();
    LED_dark(led_2);
    for(;;) {
        uint32_t curr_time = RTT_GetTime();
        if(curr_time - last_time >= 1) {
#if DEBUG_IO_LIB == 1
            TRACE_INFO("Bootloader idle..\n\r");
#endif
            LED_toggle(led_2);
            //DBGU_PutChar('t');
            last_time = curr_time;
        }
    }
}

void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType)
{
    typedef void(*fctType)(volatile unsigned int, volatile unsigned int);
    void(*pFct)(volatile unsigned int r0_val, volatile unsigned int r1_val);

    pFct = (fctType)jumpAddr;
    pFct(0/*dummy value in r0*/, matchType/*matchType in r1*/);

    while(1);//never reach
}

#ifdef ISIS_OBC_G20
void init_task(void * args) {
#if DEBUG_IO_LIB == 1
	TRACE_INFO("\n\rStarting FreeRTOS task scheduler.\n\r");
    TRACE_INFO_WP("-- SOURCE Bootloader --\n\r");
    TRACE_INFO_WP("-- %s --\n\r", BOARD_NAME);
    TRACE_INFO_WP("-- Software version v%d.%d --\n\r", SW_VERSION, SW_SUBVERSION);
    TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
    TRACE_INFO("Running initialization task..\n\r");
#endif

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


void initialize_iobc_peripherals() {
    RTT_start();
#ifdef ISIS_OBC_G20
    int result = FRAM_start();
    if(result != 0) {
    	// This should not happen!
    	TRACE_ERROR("initialize_iobc_peripherals: Could not start FRAM!\n\r");
    }
#endif
}


void handler_task(void * args) {
#if DEBUG_IO_LIB == 1
    TRACE_INFO("Running handler task..\n\r");
#endif
    // Wait for initialization to finish
    vTaskSuspend(NULL);

    //perform_bootloader_core_operation();

    // will not be reached when bootloader is finished.
    idle_loop();
//    while(1) {
//    	LED_toggle(led_3);
//    	LED_toggle(led_4);
//    	vTaskDelay(1000);
//    }
}
#endif


